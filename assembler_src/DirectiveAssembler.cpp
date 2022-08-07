
#include <bitset>
#include <iostream>
#include <string>
#include <utility>

#include "DirectiveAssembler.hpp"

#include "LineParser.hpp"
#include "LabelResolver.hpp"
#include "AssemblerCommon.hpp"

// DirectiveResolver takes care of everything but .fill

std::bitset<18> assembleDirective(const DirectiveLine& dl, const LabelResolver& lr) {
    unsigned int mc = 0;
    if (dl.directive == ".fill") {
        if (size(dl.operands) != 1) throw InvalidDirectiveOperands{dl.directive, dl.operands};

        std::optional<int> imm;
        if ((imm = parseImmediate(dl.operands[0], false))) {
            mc = onesComplement(imm.value(), 18);
        } else {
            mc = onesComplement(lr.resolveLabel(dl.operands[0], dl.line), 18);
        }
    } else {
        throw InvalidDirective{dl.directive};
    }
    return {mc};
}

