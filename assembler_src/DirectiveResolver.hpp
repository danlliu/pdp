
//
// PDP-1 Assembler
// Macro resolver
//

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct PDPMacro {
    std::string              name;
    std::vector<std::string> parameters;
    std::vector<std::string> contentLines;
};

std::unordered_map<std::string, PDPMacro> searchMacros(const std::vector<std::string> &lines);

std::vector<std::string> resolveDirectives(const std::vector<std::string> &lines, const std::unordered_map<std::string, PDPMacro> &macros);

struct UndefinedMacro {
    std::string macroName;
};

