
#include <optional>
#include <regex>
#include <string>

#include "AssemblerCommon.hpp"

unsigned int onesComplement(int num, unsigned int bits) {
    unsigned int result = 0;
    if (num < 0) {
        result |= (1 << (bits-1));
        num = ~(-num);
    }
    return result | (num & ((1 << (bits - 1))-1));
}

std::optional<int> parseImmediate(const std::optional<std::string>& ivalue, bool requiresPrefix) {
    std::regex ivalueRegex(requiresPrefix ? "#(\\-?[0-9]+)" : "(\\-?[0-9]+)");
    std::smatch ivalueMatch;
    if (!ivalue) return {};
    if (!std::regex_match(ivalue.value(), ivalueMatch, ivalueRegex)) return {};
    return stoi(ivalueMatch[1].str());
}

