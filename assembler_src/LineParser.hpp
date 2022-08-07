
//
// PDP-1 Assembler
// Line Parser
//

#pragma once

#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#define LineType std::variant<InstructionLine, DirectiveLine, MacroInvocation>

struct InstructionLine {
    std::optional<std::string> label;
    std::string                opcode;
    std::optional<std::string> operand;
    unsigned int               line;
    bool                       indirect;
};

struct DirectiveLine {
    std::optional<std::string> label;
    std::string                directive;
    std::vector<std::string>   operands;
    unsigned int               line;
};

struct MacroInvocation {
    std::optional<std::string> label;
    std::string                macro;
    std::vector<std::string>   operands;
    unsigned int               line;
};

struct LineParseError {
    std::string                error;
};

struct InvalidDirective {
    std::string                error;
};

/**
 * Parses the provided line, returning an InstructionLine or DirectiveLine depending on its format.
 * @param line: the line to parse
 * @param index: the current line number
 * @return a std::variant of either an InstructionLine, DirectiveLine, or MacroInvocation
 * @throws LineParseError if the line format is unrecognized
 */
LineType parseLine(const std::string &line, unsigned int index);

