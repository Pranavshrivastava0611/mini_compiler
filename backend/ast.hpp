#pragma once
#include <memory>
#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

// ── AST Node Definitions ──────────────────────────────────────────

struct AstNode {
    virtual ~AstNode() = default;
    virtual json toJson() const = 0;
};

using AstPtr = std::unique_ptr<AstNode>;

struct NumLit : AstNode {
    double value;
    explicit NumLit(double v) : value(v) {}
    json toJson() const override {
        return { {"type", "Number"}, {"value", value} };
    }
};

struct VarRef : AstNode {
    std::string name;
    explicit VarRef(std::string n) : name(std::move(n)) {}
    json toJson() const override {
        return { {"type", "Var"}, {"name", name} };
    }
};

struct BinOp : AstNode {
    std::string op;
    AstPtr left, right;
    BinOp(std::string o, AstPtr l, AstPtr r)
        : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
    json toJson() const override {
        return {
            {"type", "BinOp"},
            {"op", op},
            {"left", left->toJson()},
            {"right", right->toJson()}
        };
    }
};

struct UnaryNeg : AstNode {
    AstPtr operand;
    explicit UnaryNeg(AstPtr o) : operand(std::move(o)) {}
    json toJson() const override {
        return {
            {"type", "UnaryNeg"},
            {"operand", operand->toJson()}
        };
    }
};

struct LetStmt : AstNode {
    std::string name;
    AstPtr expr;
    LetStmt(std::string n, AstPtr e) : name(std::move(n)), expr(std::move(e)) {}
    json toJson() const override {
        return {
            {"type", "Let"},
            {"name", name},
            {"expr", expr->toJson()}
        };
    }
};

struct PrintStmt : AstNode {
    AstPtr expr;
    explicit PrintStmt(AstPtr e) : expr(std::move(e)) {}
    json toJson() const override {
        return {
            {"type", "Print"},
            {"expr", expr->toJson()}
        };
    }
};

struct Program : AstNode {
    std::vector<AstPtr> stmts;
    json toJson() const override {
        json arr = json::array();
        for (auto& s : stmts) arr.push_back(s->toJson());
        return { {"type", "Program"}, {"stmts", arr} };
    }
};
