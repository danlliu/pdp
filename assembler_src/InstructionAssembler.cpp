
#include <bitset>
#include <optional>
#include <regex>
#include <string>
#include <utility>

#include "InstructionAssembler.hpp"

#include "AssemblerCommon.hpp"

#define ASSERT_OPERAND if (!il.operand) throw LineParseError{il.opcode + " instruction missing required operand"}
#define OPCODE_MR(OP, OPC) if (il.opcode == OP) { \
        ASSERT_OPERAND; \
        setOpcode(mc, OPC); \
        resolveMR(mc, il, lr); \
    }
#define SHIFT_GROUP(OP, OPC) if (il.opcode == OP) { \
        std::optional<int> imm = parseImmediate(il.operand); \
        if (!imm) throw InvalidOperand{il.opcode, il.operand}; \
        setOpcode(mc, OPC); \
        mc |= onesComplement(((1 << imm.value()) - 1), 13); \
    }
#define SKIP_GROUP(OP, ADDR) if (il.opcode == OP) { \
        setOpcode(mc, 064); \
        mc |= ADDR; \
    }
#define OPERATE_GROUP(OP, ADDR) if (il.opcode == OP) { \
        setOpcode(mc, 076); \
        mc |= ADDR; \
    }

void setOpcode(unsigned int &mc, unsigned int opcode) {
    if (opcode < 0100) {
        mc |= (opcode << 12);
    } else {
        mc |= (opcode << 9);
    }
}

void resolveMR(unsigned int &mc, const InstructionLine& il, const LabelResolver& lr) {
    if (il.indirect) {
        mc |= 01000;
    }

    std::optional<int> imm;
    if ((imm = parseImmediate(il.operand))) {
        unsigned int ivalue = static_cast<unsigned int>(imm.value());
        mc |= ivalue;
    } else {
        unsigned int resolved = lr.resolveLabel(il.operand.value(), il.line);
        mc |= (resolved & 0777);
    }
}

std::bitset<18> assembleInstruction(const InstructionLine& il, const LabelResolver& lr) {
    unsigned int mc = 0;

    OPCODE_MR("add", 040);
    OPCODE_MR("sub", 042);
    OPCODE_MR("mul", 054);
    OPCODE_MR("div", 056);
    OPCODE_MR("idx", 044);
    OPCODE_MR("isp", 046);
    OPCODE_MR("and", 002);
    OPCODE_MR("xor", 006);
    OPCODE_MR("ior", 004);
    OPCODE_MR("lac", 020);
    OPCODE_MR("dac", 024);
    OPCODE_MR("dap", 026);
    OPCODE_MR("dip", 030);
    OPCODE_MR("lio", 022);
    OPCODE_MR("dio", 032);
    OPCODE_MR("dzm", 034);
    OPCODE_MR("xct", 010);
    OPCODE_MR("jmp", 060);
    OPCODE_MR("jsp", 062);
    OPCODE_MR("cal", 016);
    OPCODE_MR("jda", 017);
    OPCODE_MR("sad", 050);
    OPCODE_MR("sas", 052);

    if (il.opcode == "law") {
        std::optional<int> imm = parseImmediate(il.operand);
        if (!imm) throw InvalidOperand{il.opcode, il.operand};
        setOpcode(mc, 070);
        // 13 bit one's complement (indirect + 12 "addr")
        mc |= onesComplement(imm.value(), 13);
    }

    SHIFT_GROUP("rar", 0671);
    SHIFT_GROUP("ral", 0661);
    SHIFT_GROUP("sar", 0675);
    SHIFT_GROUP("sal", 0665);

    SHIFT_GROUP("rir", 0672);
    SHIFT_GROUP("ril", 0662);
    SHIFT_GROUP("sir", 0676);
    SHIFT_GROUP("sil", 0666);

    SHIFT_GROUP("rcr", 0673);
    SHIFT_GROUP("rcl", 0663);
    SHIFT_GROUP("scr", 0677);
    SHIFT_GROUP("scl", 0667);

    SKIP_GROUP("sza", 000100);
    SKIP_GROUP("spa", 000200);
    SKIP_GROUP("sma", 000400);
    SKIP_GROUP("szo", 001000);
    SKIP_GROUP("spi", 002000);
    SKIP_GROUP("snza", 010100);
    SKIP_GROUP("snpa", 010200);
    SKIP_GROUP("snma", 010400);
    SKIP_GROUP("snzo", 011000);
    SKIP_GROUP("snpi", 012000);

    if (il.opcode == "skp" || il.opcode == "skpn") {
        if (!il.operand) throw InvalidOperand{il.opcode, il.operand};

        setOpcode(mc, 064);
        if (il.opcode == "skpn") mc |= 010000;

        std::string conditions = il.operand.value();
        while (conditions.length() > 0) {
            std::string condition = conditions.substr(0, 2);
            if (conditions.length() > 2) {
                if (conditions[2] != '|') throw InvalidOperand{il.opcode, il.operand};
                conditions = conditions.substr(3);
            }

            if (condition == "za") mc |= 000100;
            if (condition == "pa") mc |= 000200;
            if (condition == "ma") mc |= 000400;
            if (condition == "zo") mc |= 001000;
            if (condition == "pi") mc |= 002000;
        }
    }

    if (il.opcode == "szs") {
        std::optional<int> imm = parseImmediate(il.operand);
        if (!imm) throw InvalidOperand{il.opcode, il.operand};
        if (imm.value() < 0 || imm.value() > 7) throw InvalidOperand{il.opcode, il.operand};
        setOpcode(mc, 064);
        mc |= imm.value() << 3;
    }

    if (il.opcode == "szf") {
        std::optional<int> imm = parseImmediate(il.operand);
        if (!imm) throw InvalidOperand{il.opcode, il.operand};
        if (imm.value() < 0 || imm.value() > 7) throw InvalidOperand{il.opcode, il.operand};
        setOpcode(mc, 064);
        mc |= imm.value();
    }

    OPERATE_GROUP("cli", 04000);
    OPERATE_GROUP("lat", 02000);
    OPERATE_GROUP("lap", 00100);
    OPERATE_GROUP("cma", 01000);
    OPERATE_GROUP("hlt", 00400);
    OPERATE_GROUP("cla", 00200);
    OPERATE_GROUP("nop", 00000);

    if (il.opcode == "clf") {
        std::optional<int> imm = parseImmediate(il.operand);
        if (!imm) throw InvalidOperand{il.opcode, il.operand};
        if (imm.value() < 0 || imm.value() > 7) throw InvalidOperand{il.opcode, il.operand
            };
        setOpcode(mc, 076);
        mc |= imm.value();
    }

    if (il.opcode == "stf") {
        std::optional<int> imm = parseImmediate(il.operand);
        if (!imm) throw InvalidOperand{il.opcode, il.operand};
        if (imm.value() < 0 || imm.value() > 7) throw InvalidOperand{il.opcode, il.operand
            };
        setOpcode(mc, 076);
        mc |= 0010 + imm.value();
    }

    if (mc == 0) {
        throw InvalidOpcode{il.opcode};
    }

    // TODO: iot?

    return {mc};
}

