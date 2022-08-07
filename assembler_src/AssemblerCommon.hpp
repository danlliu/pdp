
//
// PDP-1 Assembler
// Common functionality
//

#pragma once

#include <optional>
#include <regex>
#include <string>

unsigned int onesComplement(int num, unsigned int bits);

std::optional<int> parseImmediate(const std::optional<std::string>& ivalue, bool requiresPrefix = true);

