#pragma once

#include <stdexcept>
#include <string>

#include "core/program.h"

class VMError : public std::runtime_error {
    size_t pc;
    Instruction instr;

public:
    VMError(std::string text_, size_t pc_, Instruction instr_)
        : std::runtime_error(text_), pc(pc_), instr(instr_) {
    }

    size_t getPc() const { return pc; }
    Instruction getInstr() const { return instr; }
};
