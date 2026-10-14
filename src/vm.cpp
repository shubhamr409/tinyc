
#include "vm.hpp"

#include <limits>
#include <stdexcept>

VirtualMachine::VirtualMachine(std::ostream& output)
    : output_(output) {}

std::int64_t VirtualMachine::pop() {
    if (stack_.empty()) {
        throw std::runtime_error("VM stack underflow");
    }

    const std::int64_t value = stack_.back();
    stack_.pop_back();
    return value;
}

std::int64_t VirtualMachine::checkedAdd(
    std::int64_t a, std::int64_t b) {

    const auto maximum = std::numeric_limits<std::int64_t>::max();
    const auto minimum = std::numeric_limits<std::int64_t>::min();

    if ((b > 0 && a > maximum - b) ||
        (b < 0 && a < minimum - b)) {
        throw std::runtime_error("Integer overflow in ADD");
    }

    return a + b;
}

std::int64_t VirtualMachine::checkedSub(
    std::int64_t a, std::int64_t b) {

    const auto maximum = std::numeric_limits<std::int64_t>::max();
    const auto minimum = std::numeric_limits<std::int64_t>::min();

    if ((b < 0 && a > maximum + b) ||
        (b > 0 && a < minimum + b)) {
        throw std::runtime_error("Integer overflow in SUB");
    }

    return a - b;
}

std::int64_t VirtualMachine::checkedMul(
    std::int64_t a, std::int64_t b) {

    const auto maximum = std::numeric_limits<std::int64_t>::max();
    const auto minimum = std::numeric_limits<std::int64_t>::min();

    if (a == 0 || b == 0) {
        return 0;
    }

    if ((a == -1 && b == minimum) ||
        (b == -1 && a == minimum)) {
        throw std::runtime_error("Integer overflow in MUL");
    }

    if (a > 0) {
        if ((b > 0 && a > maximum / b) ||
            (b < 0 && b < minimum / a)) {
            throw std::runtime_error("Integer overflow in MUL");
        }
    } else {
        if ((b > 0 && a < minimum / b) ||
            (b < 0 && a < maximum / b)) {
            throw std::runtime_error("Integer overflow in MUL");
        }
    }

    return a * b;
}


void VirtualMachine::run(
    const std::vector<Instruction>& instructions) {

    stack_.clear();
    variables_.clear();

    auto checkedTarget = [&](std::int64_t target) -> std::size_t {
        if (target < 0 ||
            static_cast<std::uint64_t>(target) >= instructions.size()) {
            throw std::runtime_error("Invalid bytecode jump target");
        }

        return static_cast<std::size_t>(target);
    };

    std::size_t ip = 0;

    while (ip < instructions.size()) {
        const Instruction& instruction = instructions[ip];
        bool jumped = false;

        switch (instruction.op) {
            case OpCode::PushConst:
                stack_.push_back(instruction.operand);
                break;

            case OpCode::Load: {
                const auto it = variables_.find(instruction.name);

                if (it == variables_.end()) {
                    throw std::runtime_error(
                        "Undefined variable '" + instruction.name + "'"
                    );
                }

                stack_.push_back(it->second);
                break;
            }

            case OpCode::Declare: {
                if (variables_.count(instruction.name)) {
                    throw std::runtime_error(
                        "Variable '" + instruction.name +
                        "' is already declared"
                    );
                }

                const std::int64_t value = pop();
                variables_.emplace(instruction.name, value);
                break;
            }

            case OpCode::Store: {
                const auto it = variables_.find(instruction.name);

                if (it == variables_.end()) {
                    throw std::runtime_error(
                        "Cannot assign to undefined variable '" +
                        instruction.name + "'"
                    );
                }

                it->second = pop();
                break;
            }

            case OpCode::Add: {
                const std::int64_t b = pop();
                const std::int64_t a = pop();
                stack_.push_back(checkedAdd(a, b));
                break;
            }

            case OpCode::Sub: {
                const std::int64_t b = pop();
                const std::int64_t a = pop();
                stack_.push_back(checkedSub(a, b));
                break;
            }

            case OpCode::Mul: {
                const std::int64_t b = pop();
                const std::int64_t a = pop();
                stack_.push_back(checkedMul(a, b));
                break;
            }

            case OpCode::Div: {
                const std::int64_t b = pop();
                const std::int64_t a = pop();

                if (b == 0) {
                    throw std::runtime_error("Division by zero");
                }

                if (a == std::numeric_limits<std::int64_t>::min()
                    && b == -1) {
                    throw std::runtime_error(
                        "Integer overflow in DIV"
                    );
                }

                stack_.push_back(a / b);
                break;
            }

            case OpCode::Neg: {
                const std::int64_t value = pop();

                if (value == std::numeric_limits<std::int64_t>::min()) {
                    throw std::runtime_error(
                        "Integer overflow in NEG"
                    );
                }

                stack_.push_back(-value);
                break;
            }

            case OpCode::Less:
            case OpCode::LessEqual:
            case OpCode::Greater:
            case OpCode::GreaterEqual:
            case OpCode::Equal:
            case OpCode::NotEqual: {
                const std::int64_t b = pop();
                const std::int64_t a = pop();
                bool result = false;

                switch (instruction.op) {
                    case OpCode::Less: result = a < b; break;
                    case OpCode::LessEqual: result = a <= b; break;
                    case OpCode::Greater: result = a > b; break;
                    case OpCode::GreaterEqual: result = a >= b; break;
                    case OpCode::Equal: result = a == b; break;
                    case OpCode::NotEqual: result = a != b; break;
                    default: break;
                }

                stack_.push_back(result ? 1 : 0);
                break;
            }

            case OpCode::JumpIfFalse: {
                const std::int64_t condition = pop();

                if (condition == 0) {
                    ip = checkedTarget(instruction.operand);
                    jumped = true;
                }

                break;
            }

            case OpCode::Jump:
                ip = checkedTarget(instruction.operand);
                jumped = true;
                break;

            case OpCode::Print:
                output_ << pop() << '\n';
                break;

            case OpCode::Halt:
                return;
        }

        // Jump instructions already changed the instruction pointer.
        if (!jumped) {
            ++ip;
        }
    }

    throw std::runtime_error(
        "Bytecode ended without a HALT instruction"
    );
}
