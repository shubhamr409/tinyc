# TinyC update: comparison operators

Complete updated source files are included at their project-relative paths. Replace the corresponding files in the existing TinyC project with these versions.

Added operators: `<`, `<=`, `>`, `>=`, `==`, `!=`.

The update includes lexer tokens, recursive-descent parser precedence, bytecode opcodes, VM execution, constant folding for comparisons, regression tests, and updated documentation.

Validation performed: CMake build with GNU C++ 14.2.0 on Linux; CTest passed; 14 individual compiler checks passed; normal and optimized sample execution matched.
