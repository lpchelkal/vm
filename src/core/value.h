#pragma once

#include <bit>
#include <cstdint>
// #include <sstream>
// #include <stdexcept>
// #include <string>
// #include <variant>

// class TypeError : public std::runtime_error {
//   public:
//     using std::runtime_error::runtime_error;
// };

struct Value {
    uint64_t bits = 0;
    static Value Int(int64_t x) { return {std::bit_cast<uint64_t>(x)}; }
    static Value Float(double x) { return {std::bit_cast<uint64_t>(x)}; }
    int64_t asInt() const { return std::bit_cast<int64_t>(bits); }
    double asFloat() const { return std::bit_cast<double>(bits); }
};

// struct Value {
//     enum class Type { Nil, Int, Float };

//     std::variant<std::monostate, int64_t, double> data;

//     Value() = default;
//     static Value Int(int64_t x) {
//         Value v;
//         v.data = x;
//         return v;
//     }
//     static Value Float(double x) {
//         Value v;
//         v.data = x;
//         return v;
//     }

//     Type type() const { return static_cast<Type>(data.index()); }
//     bool isNil() const { return type() == Type::Nil; }
//     bool isInt() const { return type() == Type::Int; }
//     bool isFloat() const { return type() == Type::Float; }

//     int64_t asInt() const {
//         if (!isInt()) {
//             throw TypeError(std::string("expected int, got ") + typeName());
//         }

//         return std::get<int64_t>(data);
//     }
//     double asFloat() const {
//         if (!isFloat()) {
//             throw TypeError(std::string("expected float, got ") +
//             typeName());
//         }

//         return std::get<double>(data);
//     }

//     const char *typeName() const {
//         switch (type()) {
//         case Type::Nil:
//             return "nil";
//         case Type::Int:
//             return "int";
//         case Type::Float:
//             return "float";
//         }
//         return "?";
//     }

//     std::string toString() const {
//         std::ostringstream out;
//         switch (type()) {
//         case Type::Nil:
//             out << "nil";
//             break;
//         case Type::Int:
//             out << std::get<int64_t>(data);
//             break;
//         case Type::Float:
//             out << std::get<double>(data);
//             break;
//         }
//         return out.str();
//     }
// };
