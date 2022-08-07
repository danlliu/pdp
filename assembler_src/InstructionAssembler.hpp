
//
// PDP-1 Assembler
// Instruction assembler
//

#pragma once

#include <bitset>
#include <string>
#include <utility>

#include "LineParser.hpp"
#include "LabelResolver.hpp"

/**
 * Assembles the given instruction into machine code
 * @param il: the instruction to assemble
 * @param lr: a LabelResolver used to resolve any labels
 * @return a std::bitset<18> containing the machine code for the provided instruction
 */
std::bitset<18> assembleInstruction(const InstructionLine& il, const LabelResolver& lr);

struct InvalidOpcode {
    std::string opcode;
};

struct InvalidOperand {
    std::string opcode;
    std::optional<std::string> operand;
};

