
//
// PDP-1 Assembler
// Directive assembler
//

#pragma once

#include <bitset>
#include <string>
#include <utility>

#include "LineParser.hpp"
#include "LabelResolver.hpp"

/**
 * Assembles the given directive into machine code
 * @param dl: the directive to assemble
 * @param lr: a LabelResolver used to resolve any labels
 * @return a std::bitset<18> containing the machine code for the provided instruction
 */
std::bitset<18> assembleDirective(const DirectiveLine& dl, const LabelResolver& lr);

struct InvalidDirectiveOperands {
    std::string directive;
    std::vector<std::string> operands;
};

