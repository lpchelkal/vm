#pragma once

#include <iostream>
#include <stack>
#include <vector>

#include "core/errors.h"
#include "core/opcodes.h"
#include "core/program.h"
#include "core/value.h"

constexpr int64_t STACK_SIZE = 256 * 256;

struct Frame {
    size_t returnPs;
    size_t returnBase;
};

class VM {
    std::stack<Frame> frames;

    std::vector<Value> stack;

    Program program;
    size_t pc = 0;
    size_t base = 0;
    bool stoped = false;

    Value &reg(int64_t index) {
        if (index < 0 || base + index >= STACK_SIZE) {
            throw VMError("reg: index out of range", pc - 1,
                          program.code[pc - 1]);
        }
        return stack[base + index];
    }

    const Value &constant(int64_t index) {
        if (index < 0 ||
            index >= static_cast<int64_t>(program.constants.size())) {
            throw VMError("constant: index out of range", pc - 1,
                          program.code[pc - 1]);
        }
        return program.constants[index];
    }

    int64_t ib(const Instruction &i) { return reg(i.b()).asInt(); }
    int64_t ic(const Instruction &i) { return reg(i.c()).asInt(); }
    double fb(const Instruction &i) { return reg(i.b()).asFloat(); }
    double fc(const Instruction &i) { return reg(i.c()).asFloat(); }
    void setIntA(const Instruction &i, int64_t v) {
        reg(i.a()) = Value::Int(v);
    }
    void setFloatA(const Instruction &i, double v) {
        reg(i.a()) = Value::Float(v);
    }

    int64_t divisor(const Instruction &i) {
        int64_t c = ic(i);
        if (c == 0)
            throw VMError("division by zero", pc - 1, i);
        if (c == -1 && ib(i) == INT64_MIN)
            throw VMError("integer overflow", pc - 1, i);
        return c;
    }

    void jump(const Instruction &i) {
        if (i.d() >= program.code.size())
            throw VMError("jump out of program", pc - 1, i);
        pc = i.d();
    }

    using Handler = void (*)(VM &, const Instruction &);

    static const std::vector<Handler> &handlers() {
        static const std::vector<Handler> t = [] {
            using I = const Instruction &;
            std::vector<Handler> t(OPCODE_COUNT, nullptr);

            t[STP] = [](VM &v, I) { v.stoped = true; };
            t[MV] = [](VM &v, I i) { v.reg(i.a()) = v.reg(i.b()); };
            t[LI] = [](VM &v, I i) { v.setIntA(i, i.sd()); };
            t[LF] = [](VM &v, I i) { v.setFloatA(i, i.sd()); };
            t[LK] = [](VM &v, I i) { v.reg(i.a()) = v.constant(i.d()); };

            t[JMP] = [](VM &v, I i) { v.jump(i); };
            t[JZ] = [](VM &v, I i) {
                if (v.reg(i.a()).asInt() == 0)
                    v.jump(i);
            };
            t[JNZ] = [](VM &v, I i) {
                if (v.reg(i.a()).asInt() != 0)
                    v.jump(i);
            };

            t[ADD] = [](VM &v, I i) { v.setIntA(i, v.ib(i) + v.ic(i)); };
            t[SUB] = [](VM &v, I i) { v.setIntA(i, v.ib(i) - v.ic(i)); };
            t[MUL] = [](VM &v, I i) { v.setIntA(i, v.ib(i) * v.ic(i)); };
            t[DIV] = [](VM &v, I i) {
                int64_t c = v.divisor(i);
                v.setIntA(i, v.ib(i) / c);
            };
            t[MOD] = [](VM &v, I i) {
                int64_t c = v.divisor(i);
                v.setIntA(i, v.ib(i) % c);
            };
            t[NEG] = [](VM &v, I i) { v.setIntA(i, -v.ib(i)); };

            t[FADD] = [](VM &v, I i) { v.setFloatA(i, v.fb(i) + v.fc(i)); };
            t[FSUB] = [](VM &v, I i) { v.setFloatA(i, v.fb(i) - v.fc(i)); };
            t[FMUL] = [](VM &v, I i) { v.setFloatA(i, v.fb(i) * v.fc(i)); };
            t[FDIV] = [](VM &v, I i) { v.setFloatA(i, v.fb(i) / v.fc(i)); };
            t[FNEG] = [](VM &v, I i) { v.setFloatA(i, -v.fb(i)); };

            t[I2F] = [](VM &v, I i) {
                v.setFloatA(i, static_cast<double>(v.ib(i)));
            };
            t[F2I] = [](VM &v, I i) {
                v.setIntA(i, static_cast<int64_t>(v.fb(i)));
            };

            t[EQ] = [](VM &v, I i) { v.setIntA(i, v.ib(i) == v.ic(i)); };
            t[LT] = [](VM &v, I i) { v.setIntA(i, v.ib(i) < v.ic(i)); };
            t[LE] = [](VM &v, I i) { v.setIntA(i, v.ib(i) <= v.ic(i)); };
            t[FEQ] = [](VM &v, I i) { v.setIntA(i, v.fb(i) == v.fc(i)); };
            t[FLT] = [](VM &v, I i) { v.setIntA(i, v.fb(i) < v.fc(i)); };
            t[FLE] = [](VM &v, I i) { v.setIntA(i, v.fb(i) <= v.fc(i)); };

            t[IN] = [](VM &v, I i) {
                int64_t x;
                if (!(std::cin >> x))
                    throw VMError("IN: expected an integer", v.pc - 1, i);
                v.setIntA(i, x);
            };
            t[FIN] = [](VM &v, I i) {
                double x;
                if (!(std::cin >> x))
                    throw VMError("FIN: expected a number", v.pc - 1, i);
                v.setFloatA(i, x);
            };
            t[PR] = [](VM &v, I i) {
                std::cout << v.reg(i.a()).toString() << "\n";
            };

            t[CALL] = [](VM &v, I i) {
                v.frames.push(Frame{v.pc, v.base});
                v.base += i.a();
                v.jump(i);
            };

            t[RET] = [](VM &v, I i) {
                if (v.frames.empty()) {
                    v.stoped = true;
                    return;
                }

                Frame f = v.frames.top();
                v.frames.pop();
                v.reg(0) = v.reg(i.a());

                v.base = f.returnBase;
                v.pc = f.returnPs;
            };

            return t;
        }();
        return t;
    }

  public:
    explicit VM(const Program &program) : stack(STACK_SIZE), program(program) {}

    bool step() {
        const Instruction &ins = program.code[pc++];
        Opcode op = ins.opcode();

        if (op >= handlers().size() || handlers()[op] == nullptr) {
            throw VMError("unknown opcode " + std::to_string(op), pc - 1, ins);
        }

        try {
            handlers()[op](*this, ins);
        } catch (const TypeError &e) {
            throw VMError(e.what(), pc - 1, ins);
        }
        return !stoped;
    }

    void run() {
        while (pc < program.code.size()) {
            if (!step()) {
                break;
            }
        }
    }
};
