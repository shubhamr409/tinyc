
#pragma once

#include <cstdint>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

enum class OpCode {
    PushConst,
    Load,
    Declare,
    Store,

    Add,
    Sub,
    Mul,
    Div,
    Neg,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,

    JumpIfFalse,
    Jump,

    Print,
    Halt
};

struct Instruction {
    OpCode op;
    std::int64_t operand = 0;
    std::string name;
};

inline const char* opcodeName(OpCode op) {
    switch (op) {
        case OpCode::PushConst: return "PUSH_CONST";
        case OpCode::Load: return "LOAD";
        case OpCode::Declare: return "DECLARE";
        case OpCode::Store: return "STORE";

        case OpCode::Add: return "ADD";
        case OpCode::Sub: return "SUB";
        case OpCode::Mul: return "MUL";
        case OpCode::Div: return "DIV";
        case OpCode::Neg: return "NEG";
        case OpCode::Less: return "LESS";
        case OpCode::LessEqual: return "LESS_EQUAL";
        case OpCode::Greater: return "GREATER";
        case OpCode::GreaterEqual: return "GREATER_EQUAL";
        case OpCode::Equal: return "EQUAL";
        case OpCode::NotEqual: return "NOT_EQUAL";

        case OpCode::JumpIfFalse: return "JUMP_IF_FALSE";
        case OpCode::Jump: return "JUMP";

        case OpCode::Print: return "PRINT";
        case OpCode::Halt: return "HALT";
    }

    return "UNKNOWN";
}

inline void dumpBytecode(
    const std::vector<Instruction>& code,
    std::ostream& out) {

    for (std::size_t i = 0; i < code.size(); ++i) {
        const Instruction& instruction = code[i];

        out << std::setw(4) << i << "  "
            << opcodeName(instruction.op);

        switch (instruction.op) {
            case OpCode::PushConst:
                out << ' ' << instruction.operand;
                break;

            case OpCode::Load:
            case OpCode::Declare:
            case OpCode::Store:
                out << ' ' << instruction.name;
                break;

            case OpCode::JumpIfFalse:
            case OpCode::Jump:
                out << ' ' << instruction.operand;
                break;

            default:
                break;
        }

        out << '\n';
    }
}
