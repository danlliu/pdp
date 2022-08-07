
#include <array>
#include <bitset>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

#include "TapeReader.hpp"

TapeReader::TapeReader(const std::string &filename) : is{filename} {
    std::string garbage;
    std::getline(is, garbage);
}

std::optional<std::bitset<18>> TapeReader::readWord() {
    std::array<std::string, 8> lines;
    for (int i = 0; i < 8; ++i) {
        if (!std::getline(is, lines[i])) return {};
        if (i == 5) if (!std::getline(is, lines[i])) return {};
    }

    if (lines[0] != "8 O O O") return {};
    if (lines[1] != "7      ") return {};

    std::bitset<18> word;
    for (int i = 6; i >= 1; --i) {
        std::string line = lines[8 - i];
        if (line[2] == 'O') word.set(i-1);
        if (line[4] == 'O') word.set(i+5);
        if (line[6] == 'O') word.set(i+11);
    }

    return word;
}

