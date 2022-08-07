
#include <algorithm>
#include <array>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "LabelResolver.hpp"

void LabelResolver::addLabel(const LabelInfo& label) {
    std::regex symbolicRegex("([A-Za-z][A-Za-z0-9]{0,7})");
    std::regex numericRegex("([0-9]{1,3})");

    if (std::regex_match(label.label, symbolicRegex)) {
#ifdef DEBUG
        std::cout << "Found symbolic label " << label.label << std::endl;
#endif
        if (symbolicLabels.find(label.label) != end(symbolicLabels)) {
            throw DuplicateLabelError{label.label};
        }
        symbolicLabels[label.label] = label.line;
    } else if (std::regex_match(label.label, numericRegex)) {
#ifdef DEBUG
        std::cout << "Found numeric label " << label.label << std::endl;
#endif
        unsigned int index = stoul(label.label);
        if (index < 0 || index > 255) {
            throw InvalidLabelError{label.label, "Numeric label out of range."};
        }
        numericLabels[index].emplace_back(label.line);
    } else {
        throw InvalidLabelError{label.label, "Invalid label format."};
    }
}

unsigned int LabelResolver::resolveLabel(const std::string& label, unsigned int currentPosition) const {
    std::regex symbolicRegex("([A-Za-z][A-Za-z0-9]{0,7})");
    std::regex numericRegex("([0-9]{1,3})(f|b)");
    std::smatch numericMatch;

    if (std::regex_match(label, symbolicRegex)) {
        auto it = symbolicLabels.find(label);
        if (it == end(symbolicLabels)) {
            throw UndefinedLabelError{label};
        }
        return it->second;
    } else if (std::regex_match(label, numericMatch, numericRegex)) {
        unsigned int index = stoul(numericMatch[1]);
        char direction = numericMatch[2].str()[0];

        const std::vector<unsigned int> &vec = numericLabels[index];
        if (size(vec) == 0) {
            throw UndefinedLabelError{label};
        }

        if (direction == 'f') {
            for (unsigned int i = 0; i < size(vec); ++i) {
                if (vec[i] <= currentPosition) continue;
                return vec[i];
            }
        } else {
            for (unsigned int i = 0; i < size(vec); ++i) {
                if (vec[i] >= currentPosition) {
                    if (i == 0) throw UndefinedLabelError{label};
                    return vec[i-1];
                }
            }
            return vec.back();
        }
        throw UndefinedLabelError{label};
    } else {
        throw InvalidLabelError{label, "Invalid label format."};
    }
}
