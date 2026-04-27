# vm_lang — Bytecode Compiler + Stack VM

A complete end-to-end programming language pipeline with a **C++ backend** (lexer, parser, AST, compiler, and stack VM) and a **Next.js React frontend** for interactive visualization and step-through debugging.

## Features

- **Lexer**: Tokenizes source code with support for comments and newlines
- **Parser**: Recursive descent parser building an Abstract Syntax Tree (AST)
- **Compiler**: AST → stack-based bytecode with constant pooling
- **VM**: Stack machine execution with real-time state tracing
- **Visualizer**: React frontend showing tokens, AST, bytecode disassembly, and VM state
- **Step-through Debugger**: Debug your code instruction by instruction with full stack/variable inspection

## Building and Running

### Prerequisites
- CMake 3.16+
- C++17 Compiler
- Node.js & npm

### 1. Build & Run Backend
```bash
# From the project root
mkdir -p build && cd build
cmake ..
make
./vm_lang
```
*Server runs on http://localhost:8080*

### 2. Run Frontend
```bash
cd frontend
npm install
npm run dev
```
*Frontend runs on http://localhost:3000*

## Supported Language Syntax
```python
# Declare variables
let a = 5
let b = 10

# Print expressions
print a + b * 2
print 2 ^ 8

# Grouping and powers
let hyp = (a * a + b * b) ^ 0.5
print hyp

# Unary negation
print -a
```
