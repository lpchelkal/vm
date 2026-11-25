#pragma once

enum Opcode {
    STP = 0,

    MV,
    LI,
    LF,
    LK,

    JMP,
    JZ,
    JNZ,

    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    NEG,

    FADD,
    FSUB,
    FMUL,
    FDIV,
    FNEG,

    I2F,
    F2I,

    EQ,
    LT,
    LE,

    FEQ,
    FLT,
    FLE,

    IN,
    FIN,

    PR,

    ALLOC,
    FREE,
    LOAD64,
    STORE64,

    OPCODE_COUNT,
};
