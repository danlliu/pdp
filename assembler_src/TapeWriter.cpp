
#include <bitset>
#include <iostream>
#include <string>
#include <vector>

#include "TapeWriter.hpp"

TapeWriter::TapeWriter(std::ostream &os_in) : os(os_in) {
    os << "  3 2 1\n";
}

void TapeWriter::writeWord(const std::bitset<18> &word, const std::string& annotation) {
    os << "8 O O O\n";
    os << "7      \n";
    for (int i = 6; i >= 1; --i) {
        os << i;
        if (word[i-1])    os << " O";
        else            os << "  ";
        if (word[i+5])  os << " O";
        else            os << "  ";
        if (word[i+11]) os << " O";
        else            os << "  ";
        os << "\n";

        if (i == 4) {
            os << "------- " << annotation << "\n";
        }
    }
}

