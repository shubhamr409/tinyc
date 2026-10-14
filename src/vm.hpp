
#pragma once

#include "bytecode.hpp"

#include <cstdint>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class VirtualMachine {
public:
    explicit VirtualMachine(std::ostream& output);

    void run(const std::vector<Instruction>& instructions);

private:
    std::ostream& output_;
    std::vector<std::int64_t> stack_;
    std::unordered_map<std::string, std::int64_t> variables_;

    std::int64_t pop();

    static std::int64_t checkedAdd(std::int64_t a, std::int64_t b);
    static std::int64_t checkedSub(std::int64_t a, std::int64_t b);
    static std::int64_t checkedMul(std::int64_t a, std::int64_t b);
};
