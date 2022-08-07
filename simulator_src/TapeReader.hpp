
//
// PDP-1 Simulator
// Punched Tape Reader
//

#include <bitset>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

class TapeReader {

private:

    std::ifstream is;

public:

    TapeReader(const std::string &filename);

    std::optional<std::bitset<18>> readWord();

};

