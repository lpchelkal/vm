#include "verifier.h"
#include "core/opcodes.h"
#include "core/program.h"
#include <cstddef>
#include <unordered_map>
#include <vector>

namespace {

const std::unordered_map<Opcode, OpInfo> op_table = {
    {STP,  {Format::Abc, false, false, false, DCheck::None,     ControlFlow::Stop,        {RegType::Any, RegType::Any, RegType::Any}}},

    {MV,   {Format::Abc, true,  true,  false, DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Any, RegType::Any}}},
    {LI,   {Format::Ad,  true,  false, false, DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Any, RegType::Any}}},
    {LF,   {Format::Ad,  true,  false, false, DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Any, RegType::Any}}},
    {LK,   {Format::Ad,  true,  false, false, DCheck::Constant, ControlFlow::Normal,      {RegType::Any, RegType::Any, RegType::Any}}},

    {JMP,  {Format::Ad,  false, false, false, DCheck::Jump,     ControlFlow::Jump,        {RegType::Any, RegType::Any, RegType::Any}}},
    {JZ,   {Format::Ad,  true,  false, false, DCheck::Jump,     ControlFlow::Conditional, {RegType::Int, RegType::Any, RegType::Any}}},
    {JNZ,  {Format::Ad,  true,  false, false, DCheck::Jump,     ControlFlow::Conditional, {RegType::Int, RegType::Any, RegType::Any}}},

    {ADD,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Int, RegType::Int}}},
    {SUB,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Int, RegType::Int}}},
    {MUL,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Int, RegType::Int}}},
    {DIV,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Int, RegType::Int}}},
    {MOD,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Int, RegType::Int}}},
    {NEG,  {Format::Abc, true,  true,  false, DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Int, RegType::Any}}},

    {FADD, {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Float, RegType::Float}}},
    {FSUB, {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Float, RegType::Float}}},
    {FMUL, {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Float, RegType::Float}}},
    {FDIV, {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Float, RegType::Float}}},
    {FNEG, {Format::Abc, true,  true,  false, DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Float, RegType::Any}}},

    {I2F,  {Format::Abc, true,  true,  false, DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Int, RegType::Any}}},
    {F2I,  {Format::Abc, true,  true,  false, DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Float, RegType::Any}}},

    {EQ,   {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Int, RegType::Int}}},
    {LT,   {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Int, RegType::Int}}},
    {LE,   {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Int, RegType::Int}}},

    {FEQ,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Float, RegType::Float}}},
    {FLT,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Float, RegType::Float}}},
    {FLE,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Float, RegType::Float}}},

    {IN,   {Format::Abc, true,  false, false, DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Any, RegType::Any}}},
    {FIN,  {Format::Abc, true,  false, false, DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Any, RegType::Any}}},
    {PR,   {Format::Abc, true,  false, false, DCheck::None,     ControlFlow::Normal,      {RegType::Any, RegType::Any, RegType::Any}}},

    {CALL, {Format::Ad,  true,  false, false, DCheck::Jump,     ControlFlow::Call,        {RegType::Any, RegType::Any, RegType::Any}}},
    {RET,  {Format::Abc, true,  false, false, DCheck::None,     ControlFlow::Return,      {RegType::Any, RegType::Any, RegType::Any}}},

    // добавлю когда узнаю формат комманд
    // {ALLOC,   {Format::Abc, true,  true,  false, DCheck::None, ControlFlow::Normal}},
    // {FREE,    {Format::Abc, true,  false, false, DCheck::None, ControlFlow::Normal}},
    // {LOAD64,  {Format::Ad,  true,  false, false, DCheck::None, ControlFlow::Normal}},
    // {STORE64, {Format::Ad,  true,  false, false, DCheck::None, ControlFlow::Normal}},
};

const OpInfo* instruction_info(Opcode op) {
    auto it = op_table.find(op);
    return it == op_table.end() ? nullptr : &it->second;
}

std::vector<size_t> next_instructions(size_t pc, const Instruction& ins, const OpInfo& info) {
    std::vector<size_t> res;
    switch(info.flow) {
        case ControlFlow::Normal: res.push_back(pc + 1); break;
        case ControlFlow::Jump: res.push_back(ins.d()); break;
        case ControlFlow::Conditional: res.push_back(ins.d()); res.push_back(pc + 1); break;
        case ControlFlow::Call: res.push_back(ins.d()); res.push_back(pc + 1); break;
        case ControlFlow::Return: break;
        case ControlFlow::Stop: break;
    }
    return res;
}

bool is_go_forward(ControlFlow cf) {
    if(cf == ControlFlow::Normal || cf == ControlFlow::Conditional || cf == ControlFlow::Call)
        return true;
    return false;
}

std::vector<std::vector<size_t>> build_graph(const Program& program) {
    const size_t program_size = program.code.size();
    std::vector<std::vector<size_t>> graph(program_size);

    for(size_t pc = 0; pc < program_size; pc++) {
        const Instruction& ins = program.code[pc];
        const OpInfo* info = instruction_info(ins.opcode());
        if(!info) continue;

        for(size_t next : next_instructions(pc, ins, *info)) {
            if(next >= program_size) continue;
            graph[pc].push_back(next);
        }
    }
    return graph;
}

std::vector<std::vector<size_t>> build_reverse_graph(std::vector<std::vector<size_t>>& graph) {
    std::vector<std::vector<size_t>> reverse_graph(graph.size());
    for (size_t pc = 0; pc < graph.size(); pc++)
        for (size_t next : graph[pc])
            reverse_graph[next].push_back(pc);
    return reverse_graph;
}

bool can_terminate_program(ControlFlow cf) {
    if(cf == ControlFlow::Stop || cf == ControlFlow::Return) return true;
    return false;
}

RegType join(RegType a, RegType b) {
    return a == b ? a : RegType::Any;
}

struct Edge {
    size_t pc;
    std::vector<RegType> state;
};

std::vector<Edge> successors(size_t pc, const Instruction& ins, const OpInfo& info, const std::vector<RegType>& out) {
    switch(info.flow) {
    case ControlFlow::Normal: return {{pc + 1, out}};
    case ControlFlow::Jump: return {{ins.d(), out}};
    case ControlFlow::Conditional: return {{ins.d(), out}, {pc + 1, out}};
    case ControlFlow::Call: {
        const size_t a = ins.a();
        std::vector<RegType> caller = out;
        caller[a] = RegType::Any;
        for(size_t r = a + 1; r <= MAX_REGISTER; r++) caller[r] = RegType::None;
        std::vector<RegType> callee(MAX_REGISTER + 1, RegType::Any);
        return {{ins.d(), callee}, {pc + 1, caller}};
    }
    case ControlFlow::Return: return {};;
    case ControlFlow::Stop: return {};
    }
    return {};
}

void write_result(const Program& program, const Instruction& ins, std::vector<RegType>& state) {
    switch(ins.opcode()) {
    case LI: case IN:
    case ADD: case SUB: case MUL: case DIV: case MOD: case NEG:
    case EQ:  case LT:  case LE:
    case FEQ: case FLT: case FLE:
    case F2I:
        state[ins.a()] = RegType::Int;
        break;
    case LF: case FIN:
    case FADD: case FSUB: case FMUL: case FDIV: case FNEG:
    case I2F:
        state[ins.a()] = RegType::Float;
        break;
    case MV:
        state[ins.a()] = state[ins.b()];
        break;
    case LK:
        if (ins.d() < program.constants.size()) {
            auto type = program.constants[ins.d()].type;
            state[ins.a()] = type == Constant::Type::Int ? RegType::Int : RegType::Float;
        }
        break;
    default:
        break;
    }
}

} // namespace

void Verifier::error(size_t pc, const std::string& message) {
    errors.push_back({pc, Severity::Error, message});
    return;
}

void Verifier::warning(size_t pc, const std::string& message) {
    errors.push_back({pc, Severity::Warning, message});
    return;
}


std::vector<VerifyError> Verifier::verify() {
    errors.clear();
    for(size_t pc = 0; pc < program.code.size(); pc++) 
        check_instruction(pc, program.code[pc]);
    check_program();
    return errors;
}

void Verifier::check_unused_fields(size_t pc, const Instruction& ins, const OpInfo& info) {
    if(!info.use_a && ins.a() != 0) error(pc, "field 'a' must be 0");
    if(info.format != Format::Abc) return;
    if(!info.use_b && ins.b() != 0) error(pc, "field 'b' must be 0");
    if(!info.use_c && ins.c() != 0) error(pc, "field 'c' must be 0");
    return;
}

void Verifier::check_d_field(size_t pc, const Instruction& ins, const OpInfo& info) {
    if(info.d == DCheck::Constant && ins.d() >= program.constants.size())
        error(pc, "there is no constant with that number");
    if(info.d == DCheck::Jump && ins.d() >= program.code.size())
        error(pc, "jump target out of range");
    return;
}

void Verifier::check_registers(size_t pc, const Instruction& ins, const OpInfo& info) {
    if(info.use_a && ins.a() > MAX_REGISTER)
        error(pc, "register 'a' must be in range [0; 255]");
    if(info.format == Format::Ad) return;
    if(info.use_b && ins.b() > MAX_REGISTER)
        error(pc, "register 'b' must be in range [0; 255]");
    if(info.use_c && ins.c() > MAX_REGISTER)
        error(pc, "register 'c' must be in range [0; 255]");
    return;
}

void Verifier::check_instruction(size_t pc, const Instruction& ins) {
    const OpInfo* info = instruction_info(ins.opcode());
    if(!info) {
        error(pc, "unknown opcode");
        return;
    }
    check_unused_fields(pc, ins, *info);
    check_d_field(pc, ins, *info);
    check_registers(pc, ins, *info);
    return;
}

void Verifier::check_program_graph() {
    const size_t program_size = program.code.size();
    std::vector<std::vector<size_t>> graph = build_graph(program);
    std::vector<bool> visited(program_size, 0);
    std::vector<size_t> to_check = {0};

    while(!to_check.empty()) {
        size_t pc = to_check.back();
        to_check.pop_back();

        if(visited[pc]) continue;
        visited[pc] = true;

        const Instruction& ins = program.code[pc];
        const OpInfo* info = instruction_info(ins.opcode());
        if(!info) continue;

        if(pc + 1 >= program_size && is_go_forward(info->flow))
            error(pc, "execution can go past the end");
        for(size_t next : graph[pc]) if(!visited[next]) to_check.push_back(next);
    }
    for(size_t pc = 0; pc < program_size; pc++)
        if(!visited[pc]) warning(pc, "instruction is unreachable");

    std::vector<std::vector<size_t>> reverse_graph = build_reverse_graph(graph);
    std::vector<bool> can_terminate(program_size, false);
    to_check.clear();

    for(size_t pc = 0; pc < program_size; pc++) {
        const OpInfo* info = instruction_info(program.code[pc].opcode());
        if(info && visited[pc] && can_terminate_program(info->flow)) {
            can_terminate[pc] = true;
            to_check.push_back(pc);
        }
    }
    while(!to_check.empty()) {
        size_t pc = to_check.back();
        to_check.pop_back();
        for(size_t from : reverse_graph[pc]) {
            if(can_terminate[from]) continue;
            can_terminate[from] = true;
            to_check.push_back(from);
        }
    }
    if(!can_terminate[0]) error(0, "program has no path to termination");
    for(size_t pc = 0; pc < program_size; pc++) {
        if(!visited[pc] || !can_terminate[pc] || !instruction_info(program.code[pc].opcode()))
            continue;
        for(size_t next : graph[pc])
            if(visited[next] && !can_terminate[next])
                warning(pc, "there could be an infinite loop");
    }
}

void Verifier::check_types() {
    const size_t n = program.code.size();
    std::vector<std::vector<RegType>> in_state(n);
    std::vector<bool> reached(n, false);
    std::vector<size_t> to_check = {0};

    in_state[0].assign(MAX_REGISTER + 1, RegType::None);
    reached[0] = true;

    while(!to_check.empty()) {
        const size_t pc = to_check.back();
        to_check.pop_back();

        const Instruction& ins = program.code[pc];
        const OpInfo* info = instruction_info(ins.opcode());
        if(!info) continue;

        std::vector<RegType> out = in_state[pc];
        write_result(program, ins, out);

        for (const Edge& e : successors(pc, ins, *info, out)) {
            if(e.pc >= n) continue;
            if(!reached[e.pc]) {
                in_state[e.pc] = e.state;
                reached[e.pc] = true;
                to_check.push_back(e.pc);
                continue;
            }
            bool changed = false;
            for(size_t r = 0; r <= MAX_REGISTER; r++) {
                RegType j = join(in_state[e.pc][r], e.state[r]);
                if(j != in_state[e.pc][r]) { in_state[e.pc][r] = j; changed = true; }
            }
            if(changed) to_check.push_back(e.pc);
        }
    }
    for(size_t pc = 0; pc < n; pc++) {
        if(!reached[pc]) continue;
        const Instruction& ins = program.code[pc];
        const OpInfo* info = instruction_info(ins.opcode());
        if(info) check_types_at(pc, ins, *info, in_state[pc]);
    }
}

void Verifier::check_types_at(size_t pc, const Instruction& ins, const OpInfo& info, const std::vector<RegType>& regs) {
    auto check = [&](size_t r, RegType expected, const char* field) {
        if(expected == RegType::Any) return;
        RegType actual = regs[r];
        if(actual == RegType::Any) return;
        if(actual == RegType::None) {
            error(pc, std::string("register '") + field + "' not initialized");
            return;
        }
        if(actual != expected)
            error(pc, std::string("types of register '") + field + "' not matched");
    };
    if(info.use_a) check(ins.a(), info.regs.a, "a");
    if(info.format == Format::Abc) {
        if(info.use_b) check(ins.b(), info.regs.b, "b");
        if(info.use_c) check(ins.c(), info.regs.c, "c");
    }
}

void Verifier::check_program() {
    if(program.code.empty()) {
        error(0, "program is empty");
        return;
    }
    check_program_graph();
    check_types();
    return;
}
