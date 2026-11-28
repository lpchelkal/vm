#pragma once

#include <bit>
#include <cstdint>

struct Value {
    uint64_t bits = 0;
    static Value Int(int64_t x) { return {std::bit_cast<uint64_t>(x)}; }
    static Value Float(double x) { return {std::bit_cast<uint64_t>(x)}; }
    int64_t asInt() const { return std::bit_cast<int64_t>(bits); }
    double asFloat() const { return std::bit_cast<double>(bits); }
};
