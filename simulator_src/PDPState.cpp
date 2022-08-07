
#include <array>
#include <bitset>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>

#include "PDPState.hpp"

#include "TapeReader.hpp"

#ifdef DEBUG
#define DEBUG_PRINT(x) std::cout << x << std::endl;
#else
#define DEBUG_PRINT(x)
#endif

WORD PDPProcessor::readMemory(unsigned int addr, bool indirect) {
    if (!indirect) return state.cm[addr];
    WORD w = state.cm[addr];
    indirect = w[12];
    while (indirect) {
        addr = w.to_ulong() & 07777;
        w = state.cm[addr];
        indirect = w[12];
    }
    return w;
}

void PDPProcessor::writeMemory(unsigned int addr, bool indirect, WORD word) {
    if (!indirect) {
        state.cm[addr] = word;
        return;
    }
    WORD w = state.cm[addr];
    indirect = w[12];
    while (indirect) {
        addr = w.to_ulong() & 07777;
        w = state.cm[addr];
        indirect = w[12];
    }
    state.cm[addr] = word;
}

int onesComplementToInt(const WORD &oc) {
    if (oc[17]) {
        return -((~static_cast<int>(oc.to_ulong())) & 0377777);
    } else {
        return static_cast<int>(oc.to_ulong());
    }
}

WORD intToOnesComplement(int val) {
    if (val < 0) {
        return {static_cast<unsigned long>(((~-val) & 0377777) | 0400000)};
    } else {
        return {static_cast<unsigned long>(val & 0377777)};
    }
}

PDPProcessor::PDPProcessor(const PDPSettings &settings_in) : settings{settings_in}, state {settings_in.memory_size} {
    std::optional<std::bitset<18>> line;
    TapeReader tr(settings.tapeFile);
    unsigned int i = 0;
    while ((line = tr.readWord())) {
        DEBUG_PRINT("Read memory word " << i << " = " << line.value());
        state.cm[i++] = line.value();
    }
}

bool PDPProcessor::step() {
    if (!state.running) return false;

    unsigned long instr = state.cm[state.pc.to_ulong()].to_ulong();
    unsigned long opcode6 = (instr >> 12) & 076;
    bool indirect = (instr & 0010000);
    DEBUG_PRINT("indirect = " << indirect);

    unsigned long operand12 = instr & 07777;

    bool incPC = true;

    switch (opcode6) {
    case 040:
        {
            // add
            DEBUG_PRINT("add " << operand12);
            WORD memoryContents = readMemory(operand12, indirect);
            int ac = onesComplementToInt(state.ac);
            int cy = onesComplementToInt(memoryContents);
            DEBUG_PRINT("C(Y)     = " << memoryContents << " (" << cy << ")");
            DEBUG_PRINT("ac       = " << state.ac << " (" << ac << ")");
            WORD newAC = intToOnesComplement(ac + cy);
            DEBUG_PRINT("new ac   = " << newAC << " (" << ac + cy << ")");
            state.ac = newAC;

            break;
        }

    case 042:
        {
            // sub
            DEBUG_PRINT("sub " << operand12);
            WORD memoryContents = readMemory(operand12, indirect);
            int ac = onesComplementToInt(state.ac);
            int cy = onesComplementToInt(memoryContents);
            DEBUG_PRINT("C(Y)       = " << memoryContents << " (" << cy << ")");
            DEBUG_PRINT("ac         = " << state.ac << " (" << ac << ")");
            WORD newAC = intToOnesComplement(ac - cy);
            DEBUG_PRINT("new ac     = " << newAC << " (" << ac - cy << ")");
            state.ac = newAC;

            break;
        }

    case 054:
        // mul
        std::cerr << "TODO: mul" << std::endl;
        break;

    case 056:
        // div
        std::cerr << "TODO: div" << std::endl;
        break;

    case 044:
        {
            // idx
            DEBUG_PRINT("idx " << operand12);
            WORD memoryContents = readMemory(operand12, indirect);
            int cy = onesComplementToInt(memoryContents);
            DEBUG_PRINT("C(Y)     = " << memoryContents << " (" << cy << ")");

            WORD newAC = intToOnesComplement(cy + 1);
            DEBUG_PRINT("new ac   = " << newAC << " (" << cy + 1 << ")");
            DEBUG_PRINT("new C(Y) = " << newAC << " (" << cy + 1 << ")");
            state.ac = newAC;
            writeMemory(operand12, indirect, newAC);
            break;
        }

    case 076:
        {
            // operate group
            DEBUG_PRINT("operate group");

            unsigned int address = operand12;
            switch (address) {
            case 04000:
                // cli
                DEBUG_PRINT("cli");
                state.io = 0;
                break;

            case 00400:
                // hlt
                DEBUG_PRINT("hlt");
                state.running = false;
                return false;
            }
            break;
        }

    }

    if (incPC) state.pc = state.pc.to_ulong() + 1;

    return true;
}

