
#include <iostream>
#include <string>
#include <utility>
#include <regex>
#include <variant>
#include <vector>

#include "LineParser.hpp"

LineType parseInstruction(const std::smatch &match, unsigned int index) {
    std::string label    = match[2];
    std::string opcode   = match[3];
    std::string operand  = match[6];
    std::string indirect = match[5];
    return InstructionLine {
        label == ""   ? std::nullopt : std::optional<std::string>(label),
        opcode,
        operand == "" ? std::nullopt : std::optional<std::string>(operand),
        index,
        indirect == "&"
    };
}

LineType parseDirective(const std::smatch &match, unsigned int index) {
    std::string label     = match[2];
    std::string directive = match[3];
    std::string operands  = match[4];

    std::vector<std::string> operandsList;

    // split operands by whitespace
    std::regex  operandRegex("[^ \t\r]+");
    for (auto it = std::sregex_iterator(begin(operands), end(operands), operandRegex); it != std::sregex_iterator(); ++it) {
        operandsList.emplace_back(it->str());
    }

    return DirectiveLine{
        label == ""   ? std::nullopt : std::optional<std::string>(label),
        directive,
        operandsList,
        index
    };
}

LineType parseMacro(const std::smatch &match, unsigned int index) {
    std::string label    = match[2];
    std::string macro    = match[3];
    std::string operands = match[4];

    std::vector<std::string> operandsList;

    // split operands by whitespace
    std::regex  operandRegex("[^ \t\r]+");
    for (auto it = std::sregex_iterator(begin(operands), end(operands), operandRegex); it != std::sregex_iterator(); ++it) {
        operandsList.emplace_back(it->str());
    }

    return MacroInvocation {
        label,
        macro,
        operandsList,
        index
    };
}

LineType parseLine(const std::string &line, unsigned int index) {
    std::regex instructionLineRegex("(([A-Za-z0-9]+): )?[ \t\r]*([a-z]{3,4})([ \t\r]+(&)?([^ \t\r]+))?");
    std::regex directiveLineRegex("(([A-Za-z0-9]+): )?[ \t\r]*(\\.[a-z]*)(([ \t\r]+([^ \t\r]+))*)");
    std::regex macroInvocationRegex("(([A-Za-z0-9]+): )?[ \t\r]*([A-Za-z0-9]+)(([ \t\r]+([^ \t\r]+))*)");
    std::smatch match;
    if (std::regex_match(line, match, instructionLineRegex)) {
        return parseInstruction(match, index);
    } else if (std::regex_match(line, match, directiveLineRegex)) {
        return parseDirective(match, index);
    } else if (std::regex_match(line, match, macroInvocationRegex)) {
        return parseMacro(match, index);
    } else {
        std::cerr << "Formatting error: line \"" << line << "\"" << std::endl;
        throw LineParseError{"Formatting error: line \"" + line + "\""};
    }
}

