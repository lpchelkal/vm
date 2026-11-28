#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include "core/program.h"



const size_t MAX_REGISTER = 255;

enum class RegType {
    None,
    Int,
    Float,
    Any
};

enum class Severity {
    Error,
    Warning
};

class VerifyError {
public:
    size_t pc;
    Severity severity;
    std::string message;
};

enum class Format {
    Abc,
    Ad
};

enum class DCheck {
    None,
    Constant,
    Jump
};

class RegRequirements {
public:
    RegType a = RegType::Any;
    RegType b = RegType::Any;
    RegType c = RegType::Any;
};

enum class ControlFlow {
    Normal,      // pc + 1
    Stop,        // STP
    Jump,        // d
    Conditional, // d и pc + 1
    Call,        // d и pc + 1
    Return       // RET
};

class OpInfo {
public:
    Format format;
    bool use_a;
    bool use_b;
    bool use_c;
    DCheck d;
    ControlFlow flow;
    RegRequirements regs;
};

class Verifier {
public:
    explicit Verifier(const Program &program): program(program) {};

    std::vector<VerifyError> verify();
private:
    const Program& program;
    std::vector<VerifyError> errors;

    void check_program();
    void check_instruction(size_t pc, const Instruction& ins);

    void error(size_t pc, const std::string& message);
    void warning(size_t pc, const std::string& message);

    void check_unused_fields(size_t pc, const Instruction& ins, const OpInfo& info);
    void check_d_field(size_t pc, const Instruction& ins, const OpInfo& info);
    void check_registers(size_t pc, const Instruction& ins, const OpInfo& info);
    void check_program_graph();
    void check_types();
    void check_types_at(size_t pc, const Instruction& ins, const OpInfo& info, const std::vector<RegType>& regs);
};
