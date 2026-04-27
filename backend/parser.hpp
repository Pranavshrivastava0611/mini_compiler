#pragma once
#include <stdexcept>
#include <sstream>
#include "lexer.hpp"
#include "ast.hpp"

// ── Recursive Descent Parser ─────────────────────────────────────

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

    std::unique_ptr<Program> parse() {
        auto prog = std::make_unique<Program>();
        skipNewlines();
        while (!atEnd()) {
            prog->stmts.push_back(parseStatement());
            // Expect newline or EOF after each statement
            if (!atEnd()) {
                if (peek().kind == TokenKind::Newline) {
                    skipNewlines();
                } else {
                    error("Expected newline after statement");
                }
            }
        }
        return prog;
    }

private:
    std::vector<Token> tokens_;
    size_t pos_ = 0;

    // ── Helpers ──────────────────────────────────

    const Token& peek() const { return tokens_[pos_]; }
    
    bool atEnd() const { return peek().kind == TokenKind::Eof; }

    Token advance() {
        Token t = tokens_[pos_];
        if (pos_ < tokens_.size() - 1) pos_++;
        return t;
    }

    Token expect(TokenKind k, const std::string& msg) {
        if (peek().kind != k) error(msg);
        return advance();
    }

    void skipNewlines() {
        while (peek().kind == TokenKind::Newline) advance();
    }

    [[noreturn]] void error(const std::string& msg) {
        std::ostringstream oss;
        oss << "Parse error at line " << peek().line << ": " << msg
            << " (got '" << peek().text << "')";
        throw std::runtime_error(oss.str());
    }

    // ── Grammar Rules ────────────────────────────

    AstPtr parseStatement() {
        if (peek().kind == TokenKind::Let)   return parseLetStmt();
        if (peek().kind == TokenKind::Print) return parsePrintStmt();
        error("Expected 'let' or 'print'");
    }

    AstPtr parseLetStmt() {
        advance(); // consume 'let'
        Token name = expect(TokenKind::Ident, "Expected variable name after 'let'");
        expect(TokenKind::Eq, "Expected '=' after variable name");
        auto expr = parseExpr();
        return std::make_unique<LetStmt>(name.text, std::move(expr));
    }

    AstPtr parsePrintStmt() {
        advance(); // consume 'print'
        auto expr = parseExpr();
        return std::make_unique<PrintStmt>(std::move(expr));
    }

    // expr ::= term { ('+' | '-') term }
    AstPtr parseExpr() {
        auto left = parseTerm();
        while (peek().kind == TokenKind::Plus || peek().kind == TokenKind::Minus) {
            std::string op(1, advance().text[0]);
            auto right = parseTerm();
            left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
        }
        return left;
    }

    // term ::= power { ('*' | '/') power }
    AstPtr parseTerm() {
        auto left = parsePower();
        while (peek().kind == TokenKind::Star || peek().kind == TokenKind::Slash) {
            std::string op(1, advance().text[0]);
            auto right = parsePower();
            left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
        }
        return left;
    }

    // power ::= unary [ '^' power ]  (right-associative)
    AstPtr parsePower() {
        auto base = parseUnary();
        if (peek().kind == TokenKind::Caret) {
            advance(); // consume '^'
            auto exp = parsePower(); // right-recursive for right-associativity
            base = std::make_unique<BinOp>("^", std::move(base), std::move(exp));
        }
        return base;
    }

    // unary ::= '-' unary | primary
    AstPtr parseUnary() {
        if (peek().kind == TokenKind::Minus) {
            advance();
            auto operand = parseUnary();
            return std::make_unique<UnaryNeg>(std::move(operand));
        }
        return parsePrimary();
    }

    // primary ::= NUMBER | IDENT | '(' expr ')'
    AstPtr parsePrimary() {
        if (peek().kind == TokenKind::Number) {
            Token t = advance();
            return std::make_unique<NumLit>(t.value);
        }
        if (peek().kind == TokenKind::Ident) {
            Token t = advance();
            return std::make_unique<VarRef>(t.text);
        }
        if (peek().kind == TokenKind::LParen) {
            advance(); // consume '('
            auto expr = parseExpr();
            expect(TokenKind::RParen, "Expected ')'");
            return expr;
        }
        error("Expected number, identifier, or '('");
    }
};
