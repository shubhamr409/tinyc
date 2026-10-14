
#pragma once

#include "bytecode.hpp"

#include <vector>

std::vector<Instruction> optimizeBytecode(
    const std::vector<Instruction>& code
);
