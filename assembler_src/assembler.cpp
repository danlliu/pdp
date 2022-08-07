
//
// PDP-1 Assembler
// A custom assembler for the PDP-1, inspired by the MIT PDP-1 Assembler and modern assemblers
//
// Assembler CLI
// ./assembler INFILE [OUTFILE]
//
// Writes (annotated) punched tape in ASCII art format to the output file!
//
// Assembly syntax
//
// Instruction lines:
// (<label>: )? <opcode> <operand>
// <label>: optional symbolic label. This can either be a unique alphanumeric string of up to 8 characters (starting with a letter), or a potentially repeated numeric label with value up to 255
// <opcode>: the instruction mnemonic
// <operand>: the operand to give the instruction. This can either be a numeric value (prefixed with '#') or a label (no prefix). If a numeric label is used, postfix 'f' or postfix 'b' are required. Postfix 'f' indicates the first label occuring after the current instruction that has a matching numeric value. Postfix 'b' indicates the last label occuring before the current instruction that has a matching numeric value. Indirection can be specified by placing a prefix '&'.
//
// Modified Instructions:
// The PDP-1 provides various "condition codes" for skips, which can be ORed together to form complex conditions. The syntax for custom skips is
//   skp f1|f2|f3|...
// or
//   skpn f1|f2|f3|...
// where f1, f2, f3, ... are condition flags chosen from the following:
// - za: zero accumulator
// - pa: positive accumulator
// - ma: negative accumulator
// - zo: zero overflow
// - pi: positive in/out register
// skpn inverts all conditions (requiring all to be false to skip)
//
// Directive lines:
// .<directive> <operand>
// <directive>: the directive to use
// <operand>: operands to the directives
//
// Available directives:
// - . <comment>
//   Inserts a comment
// - .fill  <N>
//   Inserts the 18-bit one's complement value <N>
// - .space <k>
//   Inserts k entries of +0s. (0o000000)
// - .macro <name> [<arg1> <arg2>...]
//   Defines a macro with the provided name and arguments. Macro names and parameters are alphanumeric strings. To avoid opcodes getting replaced, avoid utilizing short lowercase parameter names.
// - .endmacro
//   Signifies the end of a macro
//

#include <bitset>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "LineParser.hpp"
#include "LabelResolver.hpp"
#include "InstructionAssembler.hpp"
#include "DirectiveResolver.hpp"
#include "DirectiveAssembler.hpp"
#include "TapeWriter.hpp"

void usage() {
    std::cout << "Usage:\n"
              << "./assembler INFILE [OUTFILE]\n";
    exit(0);
}

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) usage();

    std::ifstream infile(argv[1]);
    std::optional<std::string> outfile;
    if (argc == 3) outfile = argv[2];
    std::string line = "";

    std::vector<std::string> allLines;
    std::vector<std::string> preprocessedLines;

    while (getline(infile, line)) {
#ifdef DEBUG
        std::cout << "Read line: " << line << std::endl;
#endif
        if (line == "") break;
        allLines.emplace_back(line);
    }

    std::vector<LineType> parsedLines;
    LabelResolver resolver;

    // 1st pass: note down macros

    auto macros = searchMacros(allLines);

    // 2nd pass: resolve multi-line directives (macros, space)

    preprocessedLines = resolveDirectives(allLines, macros);

#ifdef DEBUG
    for (auto prep : preprocessedLines) {
        std::cout << "Preprocessed line: " << prep << std::endl;
    }
#endif

    // 3rd pass: note down labels and addresses

    for (unsigned int i = 0; i < size(preprocessedLines); ++i) {
        std::string l = preprocessedLines[i];

        LineType parsed = parseLine(l, i);
        parsedLines.emplace_back(parsed);

        if (std::holds_alternative<InstructionLine>(parsed)) {
            InstructionLine il = std::get<InstructionLine>(parsed);

            if (il.label) {
                resolver.addLabel({il.label.value(), i});
            }
        } else if (std::holds_alternative<DirectiveLine>(parsed)) {
            DirectiveLine dl = std::get<DirectiveLine>(parsed);
            if (dl.label) {
                resolver.addLabel({dl.label.value(), i});
            }
        } else {
            std::cerr << "Something went wrong :(" << std::endl;
        }

        // there should be no macro invocations at this stage
    }

    // 4th pass: resolve hanging labels and assemble to MC

    std::vector<std::bitset<18>> machineCode;

    for (LineType parsed : parsedLines) {
        if (std::holds_alternative<InstructionLine>(parsed)) {
            std::bitset<18> mc = assembleInstruction(std::get<InstructionLine>(parsed), resolver);
            machineCode.emplace_back(mc);
        } else {
            std::bitset<18> mc = assembleDirective(std::get<DirectiveLine>(parsed), resolver);
            machineCode.emplace_back(mc);
        }
    }

    // Print out machine code as tape!

    TapeWriter *writer = nullptr;
    std::ofstream *os = nullptr;
    if (outfile) {
        os = new std::ofstream(outfile.value());
        writer = new TapeWriter(*os);
    } else {
        writer = new TapeWriter(std::cout);
    }

    for (int i = 0; i < size(machineCode); ++i) writer->writeWord(machineCode[i], preprocessedLines[i]);
    if (os) delete os;
    delete writer;

}

