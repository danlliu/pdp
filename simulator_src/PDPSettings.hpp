
//
// PDP-1 Simulator
// PDP Settings
//

#pragma once

#include <bitset>
#include <string>

struct PDPSettings {
    bool            debug         = false;
    bool            extend        = false;
    unsigned int    memory_size   = 4096;
    std::bitset<6>  senseSwitches;
    std::string     tapeFile;
};

PDPSettings parseSettings(int argc, char** argv);

