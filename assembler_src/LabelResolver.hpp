
//
// PDP-1 Assembler
// Label Resolution
//

#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

struct LabelInfo {
    std::string     label;
    unsigned int    line;
};

class LabelResolver {

private:

    std::unordered_map<std::string, unsigned int> symbolicLabels;
    std::array<std::vector<unsigned int>, 256>    numericLabels;

public:

    /**
     * Initializes the LabelResolver.
     */
    LabelResolver() {}

    /**
     * Adds the provided label to the LabelResolver's data structures.
     * @param label: the label to add
     * @throws InvalidLabelError if the label's format is invalid
     */
    void addLabel(const LabelInfo& label);

    /**
     * Resolves the label, returning the address of the specified label.
     * @param label: the label to add
     * @return the resolved label address
     * @throws UndefinedLabelError if the label is not found
     */
    unsigned int resolveLabel(const std::string& label, unsigned int currentPosition) const;

public:

    struct InvalidLabelError {
        std::string label;
        std::string reason = "";
    };

    struct DuplicateLabelError {
        std::string label;
    };

    struct UndefinedLabelError {
        std::string label;
    };

};

