
//
// PDP-1 Simulator
// PDP Processor State
//

#pragma once

#include <array>
#include <bitset>
#include <utility>
#include <vector>

#include "PDPSettings.hpp"

#define WORD std::bitset<18>

struct PDPState {
    std::bitset<16> pc = 0; // including extended PC
    std::bitset<12> ma = 0;
    std::bitset<5>  ir = 0;

    bool overflow = false;
    bool extend   = false;

    WORD mb = 0;
    WORD ac = 0;
    WORD io = 0;

    std::vector<WORD> cm;

    bool running = true;

    PDPState(unsigned int size) : cm(size, {0}) {}
};

class PDPProcessor {

private:

    PDPSettings settings;
    PDPState state;

    WORD readMemory(unsigned int addr, bool indirect);

    void writeMemory(unsigned int addr, bool indirect, WORD word);

    bool executeInstruction(unsigned long instr);

public:

    PDPProcessor(const PDPSettings &settings);

    bool isDebug() const { return settings.debug; }

    bool step();

};

