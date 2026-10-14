
#include "optimizer.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace {

using Value = std::int64_t;

bool isFoldableBinary(OpCode op) {
    return op == OpCode::Add ||
           op == OpCode::Sub ||
           op == OpCode::Mul ||
           op == OpCode::Div ||
           op == OpCode::Less ||
           op == OpCode::LessEqual ||
           op == OpCode::Greater ||
           op == OpCode::GreaterEqual ||
           op == OpCode::Equal ||
           op == OpCode::NotEqual;
}

bool tryFoldBinary(
    OpCode op,
    Value a,
    Value b,
    Value& result) {

    const Value maximum = std::numeric_limits<Value>::max();
    const Value minimum = std::numeric_limits<Value>::min();

    switch (op) {
        case OpCode::Add:
            if ((b > 0 && a > maximum - b) ||
                (b < 0 && a < minimum - b)) {
                return false;
            }

            result = a + b;
            return true;

        case OpCode::Sub:
            if ((b < 0 && a > maximum + b) ||
                (b > 0 && a < minimum + b)) {
                return false;
            }

            result = a - b;
            return true;

        case OpCode::Mul:
            if (a == 0 || b == 0) {
                result = 0;
                return true;
            }

            if ((a == -1 && b == minimum) ||
                (b == -1 && a == minimum)) {
                return false;
            }

            if (a > 0) {
                if ((b > 0 && a > maximum / b) ||
                    (b < 0 && b < minimum / a)) {
                    return false;
                }
            } else {
                if ((b > 0 && a < minimum / b) ||
                    (b < 0 && a < maximum / b)) {
                    return false;
                }
            }

            result = a * b;
            return true;

        case OpCode::Div:
            if (b == 0) {
                return false;
            }

            if (a == minimum && b == -1) {
                return false;
            }

            result = a / b;
            return true;

        case OpCode::Less:
            result = a < b ? 1 : 0;
            return true;

        case OpCode::LessEqual:
            result = a <= b ? 1 : 0;
            return true;

        case OpCode::Greater:
            result = a > b ? 1 : 0;
            return true;

        case OpCode::GreaterEqual:
            result = a >= b ? 1 : 0;
            return true;

        case OpCode::Equal:
            result = a == b ? 1 : 0;
            return true;

        case OpCode::NotEqual:
            result = a != b ? 1 : 0;
            return true;

        default:
            return false;
    }
}

std::vector<Instruction> optimizeOnePass(
    const std::vector<Instruction>& code) {

    std::vector<Instruction> output;
    output.reserve(code.size());

    // Maps each original instruction index to its new index.
    std::vector<std::size_t> remap(code.size() + 1, 0);

    std::size_t i = 0;

    while (i < code.size()) {
        // Fold PUSH_CONST value; NEG.
        if (i + 1 < code.size() &&
            code[i].op == OpCode::PushConst &&
            code[i + 1].op == OpCode::Neg) {

            const Value value = code[i].operand;
            const Value minimum =
                std::numeric_limits<Value>::min();

            if (value != minimum) {
                const std::size_t newIndex = output.size();

                remap[i] = newIndex;
                remap[i + 1] = newIndex;

                output.push_back({
                    OpCode::PushConst, -value, ""
                });

                i += 2;
                continue;
            }
        }

        // Fold PUSH_CONST a; PUSH_CONST b; a foldable binary operation.
        if (i + 2 < code.size() &&
            code[i].op == OpCode::PushConst &&
            code[i + 1].op == OpCode::PushConst &&
            isFoldableBinary(code[i + 2].op)) {

            const Value a = code[i].operand;
            const Value b = code[i + 1].operand;
            Value result = 0;

            if (tryFoldBinary(code[i + 2].op, a, b, result)) {
                const std::size_t newIndex = output.size();

                remap[i] = newIndex;
                remap[i + 1] = newIndex;
                remap[i + 2] = newIndex;

                output.push_back({
                    OpCode::PushConst, result, ""
                });

                i += 3;
                continue;
            }
        }

        remap[i] = output.size();
        output.push_back(code[i]);
        ++i;
    }

    // The end position is a possible jump destination.
    remap[code.size()] = output.size();

    // Rewrite jump operands using the new instruction positions.
    for (Instruction& instruction : output) {
        if (instruction.op != OpCode::Jump &&
            instruction.op != OpCode::JumpIfFalse) {
            continue;
        }

        const std::int64_t oldTarget = instruction.operand;

        if (oldTarget < 0 ||
            static_cast<std::uint64_t>(oldTarget) > code.size()) {
            // Preserve malformed input rather than rewriting its target.
            return code;
        }

        instruction.operand = static_cast<std::int64_t>(
            remap[static_cast<std::size_t>(oldTarget)]
        );
    }

    return output;
}

} // namespace

std::vector<Instruction> optimizeBytecode(
    const std::vector<Instruction>& code) {

    std::vector<Instruction> current = code;

    // Repeat folding so newly adjacent constants can also be folded.
    while (true) {
        std::vector<Instruction> next = optimizeOnePass(current);

        // Every successful fold reduces the number of instructions.
        if (next.size() == current.size()) {
            return current;
        }

        current = std::move(next);
    }
}
