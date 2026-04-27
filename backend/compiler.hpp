#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include "ast.hpp"
#include "json.hpp"

using json = nlohmann::json;

// ── Bytecode Definitions ─────────────────────────────────────────

enum class Op : uint8_t {
    PUSH, LOAD, STORE,
    ADD, SUB, MUL, DIV, POW, NEG,
    PRINT, HALT
};

inline const char* opStr(Op op) {
    switch (op) {
        case Op::PUSH:  return "PUSH";
        case Op::LOAD:  return "LOAD";
        case Op::STORE: return "STORE";
        case Op::ADD:   return "ADD";
        case Op::SUB:   return "SUB";
        case Op::MUL:   return "MUL";
        case Op::DIV:   return "DIV";
        case Op::POW:   return "POW";
        case Op::NEG:   return "NEG";
        case Op::PRINT: return "PRINT";
        case Op::HALT:  return "HALT";
    }
    return "???";
}

struct Instruction {
    Op op;
    int arg = -1;
};

struct Chunk {
    std::vector<Instruction> code;
    std::vector<double> constants;
    std::vector<std::string> varNames;

    int addConstant(double v) {
        // Check for existing constant
        for (int i = 0; i < (int)constants.size(); i++) {
            if (constants[i] == v) return i;
        }
        constants.push_back(v);
        return (int)constants.size() - 1;
    }

    int resolveVar(const std::string& name) {
        for (int i = 0; i < (int)varNames.size(); i++) {
            if (varNames[i] == name) return i;
        }
        varNames.push_back(name);
        return (int)varNames.size() - 1;
    }

    void emit(Op op, int arg = -1) {
        code.push_back({op, arg});
    }

    json toJson() const {
        json instrs = json::array();
        for (int i = 0; i < (int)code.size(); i++) {
            const auto& ins = code[i];
            std::string comment;
            if (ins.op == Op::PUSH && ins.arg >= 0 && ins.arg < (int)constants.size()) {
                // Format large integers without scientific notation
                double val = constants[ins.arg];
                if (val == std::floor(val) && std::abs(val) < 1e15) {
                    comment = "const[" + std::to_string(ins.arg) + "] = " + std::to_string((long long)val);
                } else {
                    comment = "const[" + std::to_string(ins.arg) + "] = " + std::to_string(val);
                }
            } else if ((ins.op == Op::LOAD || ins.op == Op::STORE) &&
                       ins.arg >= 0 && ins.arg < (int)varNames.size()) {
                comment = "var[" + std::to_string(ins.arg) + "] = " + varNames[ins.arg];
            }
            instrs.push_back({
                {"ip", i},
                {"op", opStr(ins.op)},
                {"arg", ins.arg},
                {"comment", comment}
            });
        }
        return {
            {"instructions", instrs},
            {"constants", constants},
            {"varNames", varNames}
        };
    }
};

// ── Compiler (AST → Bytecode) ────────────────────────────────────

class Compiler {
public:
    Chunk compile(const AstNode& root) {
        chunk_ = Chunk();
        compileNode(root);
        chunk_.emit(Op::HALT);
        return chunk_;
    }

private:
    Chunk chunk_;

    void compileNode(const AstNode& node) {
        if (auto* p = dynamic_cast<const Program*>(&node)) {
            for (auto& s : p->stmts) compileNode(*s);
        } else if (auto* let = dynamic_cast<const LetStmt*>(&node)) {
            compileNode(*let->expr);
            int idx = chunk_.resolveVar(let->name);
            chunk_.emit(Op::STORE, idx);
        } else if (auto* pr = dynamic_cast<const PrintStmt*>(&node)) {
            compileNode(*pr->expr);
            chunk_.emit(Op::PRINT);
        } else if (auto* num = dynamic_cast<const NumLit*>(&node)) {
            int idx = chunk_.addConstant(num->value);
            chunk_.emit(Op::PUSH, idx);
        } else if (auto* var = dynamic_cast<const VarRef*>(&node)) {
            int idx = chunk_.resolveVar(var->name);
            chunk_.emit(Op::LOAD, idx);
        } else if (auto* bin = dynamic_cast<const BinOp*>(&node)) {
            compileNode(*bin->left);
            compileNode(*bin->right);
            if (bin->op == "+") chunk_.emit(Op::ADD);
            else if (bin->op == "-") chunk_.emit(Op::SUB);
            else if (bin->op == "*") chunk_.emit(Op::MUL);
            else if (bin->op == "/") chunk_.emit(Op::DIV);
            else if (bin->op == "^") chunk_.emit(Op::POW);
        } else if (auto* neg = dynamic_cast<const UnaryNeg*>(&node)) {
            compileNode(*neg->operand);
            chunk_.emit(Op::NEG);
        }
    }
};
