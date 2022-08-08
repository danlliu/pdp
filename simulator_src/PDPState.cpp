
#include <array>
#include <bitset>
#include <iostream>
#include <optional>
#include <stdlib.h>
#include <utility>
#include <vector>

#include "PDPState.hpp"

#include "TapeReader.hpp"

#ifdef DEBUG
#define DEBUG_PRINT(x) std::cout << x << std::endl;
#else
#define DEBUG_PRINT(x)
#endif

#define MINUS_ZERO 0777777

// had to do it
#define HALT_AND_CATCH_FIRE exit(1);

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
    state.extend = settings.extend;

    std::optional<std::bitset<18>> line;
    TapeReader tr(settings.tapeFile);
    unsigned int i = 0;
    while ((line = tr.readWord())) {
        DEBUG_PRINT("Read memory word " << i << " = " << line.value());
        state.cm[i++] = line.value();
    }
}

// printState

void PDPProcessor::printState() const {
    std::cout << "\n";
    std::cout << "PC:      " << state.pc << "\n";
    std::cout << "INSTR: " << state.cm[state.pc.to_ulong()] << "\n";
    std::cout << "AC:    " << state.ac << "\n";
    std::cout << "IO:    " << state.io << "\n";
    std::cout << "SW:                " << settings.senseSwitches << "\n";
    std::cout << "PF:                " << state.pf << "\n" << std::endl;
}

// execution logic

bool PDPProcessor::step() {
    if (!state.running) return false;

    unsigned long instr = state.cm[state.pc.to_ulong()].to_ulong();
    return executeInstruction(instr);
}

bool PDPProcessor::executeInstruction(unsigned long instr) {
    unsigned long opcode6 = (instr >> 12) & 076;
    bool indirect = (instr & 0010000);
    DEBUG_PRINT("executing instruction " << instr);
    DEBUG_PRINT("indirect = " << indirect);

    unsigned long operand12 = instr & 07777;

    bool incPC = true;

    switch (opcode6) {
    case 040:
        {
            // add
            DEBUG_PRINT("add " << operand12);
            WORD memoryContents = readMemory(operand12, indirect);
            unsigned long ac = state.ac.to_ulong();
            unsigned long cy = memoryContents.to_ulong();
            DEBUG_PRINT("C(Y)     = " << memoryContents << " (" << cy << ")");
            DEBUG_PRINT("ac       = " << state.ac << " (" << ac << ")");

            // one's complement addition
            unsigned long sum = ac + cy;
            if (sum & 01000000) sum = (sum & 0777777) + 1;

            // minus zero stuff
            if (sum == MINUS_ZERO) {
                DEBUG_PRINT("converting -0 to +0");
                sum = 0;
            }

            WORD newAC {sum};

            // check signs to set overflow
            if (state.ac[17] == memoryContents[17]) {
                DEBUG_PRINT("ac and C(Y) have same sign");
                if (newAC[17] != state.ac[17]) {
                    DEBUG_PRINT("add resulted in overflow");
                    state.overflow = true;
                }
            }

            DEBUG_PRINT("new ac   = " << newAC << " (" << sum << ")");
            state.ac = newAC;

            break;
        }

    case 042:
        {
            // sub
            DEBUG_PRINT("sub " << operand12);
            WORD memoryContents = readMemory(operand12, indirect);
            unsigned long ac = state.ac.to_ulong();
            unsigned long cy = memoryContents.to_ulong();
            DEBUG_PRINT("C(Y)       = " << memoryContents << " (" << cy << ")");
            DEBUG_PRINT("ac         = " << state.ac << " (" << ac << ")");

            // one's complement subtraction with hacky bit stuff
            ac |= 01000000;
            unsigned long diff = ac - cy;
            if (!(diff & 01000000)) diff -= 1;
            else diff &= 0777777;

            // minus zero shenanigans
            if (diff == MINUS_ZERO && !(ac == MINUS_ZERO && cy == 0)) {
                DEBUG_PRINT("converting -0 to +0");
                diff = 0;
            }

            WORD newAC {diff};

            // check signs to set overflow
            if (state.ac[17] != newAC[17]) {
                DEBUG_PRINT("sub resulted in overflow");
                state.overflow = true;
            }

            DEBUG_PRINT("new ac     = " << newAC << " (" << diff << ")");
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

    case 046:
        {
            // isp
            DEBUG_PRINT("isp " << operand12);
            WORD memoryContents = readMemory(operand12, indirect);
            int cy = onesComplementToInt(memoryContents);
            DEBUG_PRINT("C(Y)     = " << memoryContents << " (" << cy << ")");

            WORD newAC = intToOnesComplement(cy + 1);
            DEBUG_PRINT("new ac   = " << newAC << " (" << cy + 1 << ")");
            DEBUG_PRINT("new C(Y) = " << newAC << " (" << cy + 1 << ")");
            state.ac = newAC;
            writeMemory(operand12, indirect, newAC);
            if (cy + 1 >= 0) {
                DEBUG_PRINT("isp skipping");
                state.pc = (state.pc.to_ulong() + 1);
            }
            break;
        }

    case 002:
        {
            // and
            DEBUG_PRINT("and " << operand12);
            WORD cy = readMemory(operand12, indirect);
            DEBUG_PRINT("C(Y)     = " << cy);
            DEBUG_PRINT("ac       = " << state.ac);
            WORD newAC = cy & state.ac;
            DEBUG_PRINT("new ac   = " << newAC);
            state.ac = newAC;
            break;
        }

    case 006:
        {
            // xor
            DEBUG_PRINT("xor " << operand12);
            WORD cy = readMemory(operand12, indirect);
            DEBUG_PRINT("C(Y)     = " << cy);
            DEBUG_PRINT("ac       = " << state.ac);
            WORD newAC = cy ^ state.ac;
            DEBUG_PRINT("new ac   = " << newAC);
            state.ac = newAC;
            break;
        }

    case 004:
        {
            // ior
            DEBUG_PRINT("ior " << operand12);
            WORD cy = readMemory(operand12, indirect);
            DEBUG_PRINT("C(Y)     = " << cy);
            DEBUG_PRINT("ac       = " << state.ac);
            WORD newAC = cy | state.ac;
            DEBUG_PRINT("new ac   = " << newAC);
            state.ac = newAC;
            break;
        }

    case 020:
        {
            // lac
            DEBUG_PRINT("lac " << operand12);
            WORD cy = readMemory(operand12, indirect);
            DEBUG_PRINT("C(Y)     = " << cy);
            DEBUG_PRINT("new ac   = " << cy);
            state.ac = cy;
            break;
        }

    case 024:
        {
            // dac
            DEBUG_PRINT("dac " << operand12);
            DEBUG_PRINT("ac       = " << state.ac);
            DEBUG_PRINT("new C(Y) = " << state.ac);
            writeMemory(operand12, indirect, state.ac);
            break;
        }

    case 026:
        {
            // dap
            DEBUG_PRINT("dap " << operand12);
            DEBUG_PRINT("ac       = " << state.ac);
            unsigned int acLow12 = state.ac.to_ulong() & 07777;
            WORD ap {acLow12};
            DEBUG_PRINT("ap       = " << ap);
            WORD cy = readMemory(operand12, indirect);
            DEBUG_PRINT("C(Y)     = " << cy);
            WORD newCY = (cy & WORD{0770000}) | ap;
            DEBUG_PRINT("new C(Y) = " << newCY);
            writeMemory(operand12, indirect, newCY);
            break;
        }

    case 030:
        {
            // dip
            DEBUG_PRINT("dip " << operand12);
            DEBUG_PRINT("ac       = " << state.ac);
            unsigned int acHigh5 = state.ac.to_ulong() & 0760000;
            WORD ip {acHigh5};
            DEBUG_PRINT("ip       = " << ip);
            WORD cy = readMemory(operand12, indirect);
            DEBUG_PRINT("C(Y)     = " << cy);
            WORD newCY = (cy & WORD{0017777}) | ip;
            DEBUG_PRINT("new C(Y) = " << newCY);
            writeMemory(operand12, indirect, newCY);
            break;
        }

    case 022:
        {
            // lio
            DEBUG_PRINT("lio " << operand12);
            WORD cy = readMemory(operand12, indirect);
            DEBUG_PRINT("C(Y)     = " << cy);
            DEBUG_PRINT("new io   = " << cy);
            state.io = cy;
            break;
        }

    case 032:
        {
            // dio
            DEBUG_PRINT("dio " << operand12);
            DEBUG_PRINT("io       = " << state.io);
            DEBUG_PRINT("new C(Y) = " << state.io);
            writeMemory(operand12, indirect, state.io);
            break;
        }

    case 034:
        {
            // dzm
            DEBUG_PRINT("dzm " << operand12);
            DEBUG_PRINT("new C(Y) = 0");
            writeMemory(operand12, indirect, WORD{0});
            break;
        }

    case 010:
        {
            // xct
            DEBUG_PRINT("xct " << operand12);
            WORD cy = readMemory(operand12, indirect);
            unsigned long toRun = cy.to_ulong();
            executeInstruction(toRun);
            break;
        }

    case 060:
        {
            // jmp
            DEBUG_PRINT("jmp " << operand12);
            DEBUG_PRINT("new pc   = " << operand12);
            state.pc = {operand12};
            incPC = false;
            break;
        }

    case 062:
        {
            // jsp
            DEBUG_PRINT("jsp " << operand12);
            WORD newAC = {state.pc.to_ulong() + 1};
            // bitset and PDP bit orderings are reversed
            newAC.set(17, state.overflow);
            newAC.set(16, state.extend);
            DEBUG_PRINT("new ac   = " << newAC);
            state.ac = newAC;
            DEBUG_PRINT("new pc   = " << operand12);
            state.pc = {operand12};
            incPC = false;
            break;
        }

    case 016:
        {
            if (indirect) {
                // jda
                DEBUG_PRINT("jda " << operand12);

                DEBUG_PRINT("C(Y)     = " << state.ac << "( " << onesComplementToInt(state.ac) << ")");
                writeMemory(operand12, false, state.ac);

                WORD newAC = {state.pc.to_ulong() + 1};
                // bitset and PDP bit orderings are reversed
                newAC.set(17, state.overflow);
                newAC.set(16, state.extend);
                DEBUG_PRINT("new ac   = " << newAC);
                state.ac = newAC;

                DEBUG_PRINT("new pc   = " << operand12 + 1);
                state.pc = operand12 + 1;
                incPC = false;
                break;
            } else {
                // cal
                DEBUG_PRINT("cal");
                DEBUG_PRINT("C(0o100) = " << state.ac);
                writeMemory(0100, false, state.ac);

                WORD newAC = {state.pc.to_ulong() + 1};
                // bitset and PDP bit orderings are reversed
                newAC.set(17, state.overflow);
                newAC.set(16, state.extend);
                DEBUG_PRINT("new ac   = " << newAC);
                state.ac = newAC;

                // i hope i read the documentation right
                DEBUG_PRINT("new pc   = " << 0101);
                state.pc = 0101;
                incPC = false;
                break;
            }
        }

    case 050:
        {
            // sad
            DEBUG_PRINT("sad " << operand12);
            WORD cy = readMemory(operand12, indirect);
            DEBUG_PRINT("C(Y)     = " << cy);
            DEBUG_PRINT("ac       = " << state.ac);
            if (cy != state.ac) {
                DEBUG_PRINT("C(Y) and ac differ, skipping");
                state.pc = {state.pc.to_ulong() + 1};
            }
            break;
        }

    case 052:
        {
            // sas
            DEBUG_PRINT("sas " << operand12);
            WORD cy = readMemory(operand12, indirect);
            DEBUG_PRINT("C(Y)     = " << cy);
            DEBUG_PRINT("ac       = " << state.ac);
            if (cy == state.ac) {
                DEBUG_PRINT("C(Y) and ac differ, skipping");
                state.pc = {state.pc.to_ulong() + 1};
            }
            break;
        }

    case 070:
        {
            // law
            DEBUG_PRINT("law " << (indirect ? "-" : "") << operand12);
            WORD newAC = intToOnesComplement((indirect?-1:1) * static_cast<int>(operand12));
            DEBUG_PRINT("new ac   = " << newAC << "(" << (indirect ? "-" : "") << operand12 << ")");
            break;
        }

    case 066:
        {
            // shift group
            unsigned int opcode9 = (instr & 0777000) >> 9;
            unsigned int operand9 = (instr & 0777);

            unsigned int shiftAmount = 0;
            while (operand9) {
                shiftAmount += operand9 & 1;
                operand9 >>= 1;
            }

            switch (opcode9) {
            case 0671:
                {
                    // rar
                    DEBUG_PRINT("rar " << shiftAmount);
                    DEBUG_PRINT("ac       = " << state.ac);
                    WORD newAC = (state.ac >> shiftAmount) | (state.ac << (18 - shiftAmount));
                    DEBUG_PRINT("new ac   = " << newAC);
                    state.ac = newAC;
                    break;
                }
            case 0661:
                {
                    // ral
                    DEBUG_PRINT("ral " << shiftAmount);
                    DEBUG_PRINT("ac       = " << state.ac);
                    WORD newAC = (state.ac << shiftAmount) | (state.ac >> (18 - shiftAmount));
                    DEBUG_PRINT("new ac   = " << newAC);
                    state.ac = newAC;
                    break;
                }
            case 0675:
                {
                    // rar
                    DEBUG_PRINT("sar " << shiftAmount);
                    DEBUG_PRINT("ac       = " << state.ac);
                    WORD newAC = state.ac >> shiftAmount;
                    DEBUG_PRINT("new ac   = " << newAC);
                    state.ac = newAC;
                    break;
                }
            case 0665:
                {
                    // ral
                    DEBUG_PRINT("sal " << shiftAmount);
                    DEBUG_PRINT("ac       = " << state.ac);
                    WORD newAC = state.ac << shiftAmount;
                    DEBUG_PRINT("new ac   = " << newAC);
                    state.ac = newAC;
                    break;
                }

            case 0672:
                {
                    // rar
                    DEBUG_PRINT("rir " << shiftAmount);
                    DEBUG_PRINT("io       = " << state.io);
                    WORD newIO = (state.io >> shiftAmount) | (state.io << (18 - shiftAmount));
                    DEBUG_PRINT("new io   = " << newIO);
                    state.io = newIO;
                    break;
                }
            case 0662:
                {
                    // ril
                    DEBUG_PRINT("ril " << shiftAmount);
                    DEBUG_PRINT("io       = " << state.io);
                    WORD newIO = (state.io << shiftAmount) | (state.io >> (18 - shiftAmount));
                    DEBUG_PRINT("new ac   = " << newIO);
                    state.io = newIO;
                    break;
                }
            case 0676:
                {
                    // sir
                    DEBUG_PRINT("sir " << shiftAmount);
                    DEBUG_PRINT("io       = " << state.io);
                    WORD newIO = state.io >> shiftAmount;
                    DEBUG_PRINT("new io   = " << newIO);
                    state.io = newIO;
                    break;
                }
            case 0666:
                {
                    // sil
                    DEBUG_PRINT("sil " << shiftAmount);
                    DEBUG_PRINT("io       = " << state.io);
                    WORD newIO = state.io << shiftAmount;
                    DEBUG_PRINT("new io   = " << newIO);
                    state.io = newIO;
                    break;
                }

            case 0673:
                {
                    // rcr
                    DEBUG_PRINT("rcr " << shiftAmount);
                    DEBUG_PRINT("ac       = " << state.ac);
                    DEBUG_PRINT("io       = " << state.io);
                    WORD newAC = (state.ac >> shiftAmount) | (state.io << (18 - shiftAmount));
                    WORD newIO = (state.io >> shiftAmount) | (state.ac << (18 - shiftAmount));
                    DEBUG_PRINT("new ac   = " << newAC);
                    DEBUG_PRINT("new io   = " << newIO);
                    state.ac = newAC;
                    state.io = newIO;
                    break;
                }
            case 0663:
                {
                    // rcl
                    DEBUG_PRINT("rcl " << shiftAmount);
                    DEBUG_PRINT("ac       = " << state.ac);
                    DEBUG_PRINT("io       = " << state.io);
                    WORD newAC = (state.ac << shiftAmount) | (state.io >> (18 - shiftAmount));
                    WORD newIO = (state.io << shiftAmount) | (state.ac >> (18 - shiftAmount));
                    DEBUG_PRINT("new ac   = " << newAC);
                    DEBUG_PRINT("new io   = " << newIO);
                    state.ac = newAC;
                    state.io = newIO;
                    break;
                }
            case 0677:
                {
                    // scr
                    DEBUG_PRINT("scr " << shiftAmount);
                    DEBUG_PRINT("ac       = " << state.ac);
                    DEBUG_PRINT("io       = " << state.io);
                    WORD newAC = state.ac >> shiftAmount;
                    WORD newIO = (state.io >> shiftAmount) | (state.ac << (18 - shiftAmount));
                    DEBUG_PRINT("new ac   = " << newAC);
                    DEBUG_PRINT("new io   = " << newIO);
                    state.ac = newAC;
                    state.io = newIO;
                    break;
                }
            case 0667:
                {
                    // scl
                    DEBUG_PRINT("scl " << shiftAmount);
                    DEBUG_PRINT("ac       = " << state.ac);
                    DEBUG_PRINT("io       = " << state.io);
                    WORD newAC = (state.ac << shiftAmount) | (state.io >> (18 - shiftAmount));
                    WORD newIO = state.io << shiftAmount;
                    DEBUG_PRINT("new ac   = " << newAC);
                    DEBUG_PRINT("new io   = " << newIO);
                    state.ac = newAC;
                    state.io = newIO;
                    break;
                }

            default:
                DEBUG_PRINT("idk shift");
                HALT_AND_CATCH_FIRE;
                break;
            }
            break;
        }

    case 064:
        {
            // skip group
            DEBUG_PRINT((indirect ? "skpn" : "skp"));
            bool conditionMatched = false;
            if (operand12 & 00100) {
                DEBUG_PRINT("...za");
                DEBUG_PRINT("ac       = " << state.ac);
                conditionMatched = conditionMatched || (state.ac.none());
            }
            if (operand12 & 00200) {
                DEBUG_PRINT("...pa");
                DEBUG_PRINT("ac       = " << state.ac);
                conditionMatched = conditionMatched || (!state.ac[17]);
            }
            if (operand12 & 00400) {
                DEBUG_PRINT("...ma");
                DEBUG_PRINT("ac       = " << state.ac);
                conditionMatched = conditionMatched || (state.ac[17]);
            }
            if (operand12 & 01000) {
                DEBUG_PRINT("...zo");
                DEBUG_PRINT("overflow = " << state.overflow);
                conditionMatched = conditionMatched || (!state.overflow);
                state.overflow = false;
            }
            if (operand12 & 02000) {
                DEBUG_PRINT("...pi");
                DEBUG_PRINT("io       = " << state.io);
                conditionMatched = conditionMatched || (!state.io[17]);
            }
            if (operand12 & 00070) {
                unsigned int switches = (operand12 >> 3) & 07;
                DEBUG_PRINT("...zs " << switches);
                DEBUG_PRINT("switches = " << settings.senseSwitches);

                if (switches == 7) conditionMatched = conditionMatched || settings.senseSwitches.all();
                else conditionMatched = conditionMatched || settings.senseSwitches[switches - 1];
            }
            if (operand12 & 00007) {
                unsigned int flags = operand12 & 07;
                DEBUG_PRINT("...zf " << flags);
                DEBUG_PRINT("flags    = " << state.pf);

                if (flags == 7) conditionMatched = conditionMatched || state.pf.all();
                else conditionMatched = conditionMatched || state.pf[flags - 1];
            }

            if (conditionMatched != indirect) {
                DEBUG_PRINT("skipping");
                state.pc = {state.pc.to_ulong() + 1};
            }
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
                state.io = {0};
                break;

            case 00100:
                {
                    // lap
                    DEBUG_PRINT("lap");
                    DEBUG_PRINT("ac       = " << state.ac);
                    DEBUG_PRINT("pc       = " << state.pc);

                    WORD newAC = {state.pc.to_ulong()};
                    newAC[17] = state.ac[17] || state.overflow;
                    newAC[16] = state.extend;

                    DEBUG_PRINT("new ac   = " << newAC);
                    state.ac = newAC;
                    break;
                }

            case 01000:
                {
                    // cma
                    DEBUG_PRINT("cma");
                    DEBUG_PRINT("ac       = " << state.ac);

                    WORD newAC = ~state.ac;
                    DEBUG_PRINT("new ac   = " << newAC);
                    state.ac = newAC;
                    break;
                }

            case 00400:
                // hlt
                DEBUG_PRINT("hlt");
                state.running = false;
                incPC = false;
                break;

            case 00200:
                // cla
                DEBUG_PRINT("cla");
                state.ac = {0};
                break;

            case 00000:
                // nop
                DEBUG_PRINT("nop");
                break;

            default:
                {
                    if (address & 07) {
                        bool set = address & 010;
                        unsigned int flags = address & 07;
                        if (flags == 7 && set) {
                            state.pf.set();
                        } else {
                            state.pf[flags - 1] = set;
                        }
                        break;
                    }

                    DEBUG_PRINT("idk op");
                    HALT_AND_CATCH_FIRE;
                    break;
                }
            }
            break;
        }

    default:
        DEBUG_PRINT("idk");
        HALT_AND_CATCH_FIRE;

    }

    if (incPC) state.pc = state.pc.to_ulong() + 1;

    return state.running;
}
