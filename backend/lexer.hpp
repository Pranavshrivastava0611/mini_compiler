#pragma once
#include <string>
#include <vector>
#include <cctype>
#include "json.hpp"

using json = nlohmann::json;

// ── Token Types ───────────────────────────────────────────────────

enum class TokenKind {
    Number, Ident, Let, Print,
    Plus, Minus, Star, Slash, Caret, Eq,
    LParen, RParen, Newline, Eof, Bad
};

inline const char* tokenKindStr(TokenKind k) {
    switch (k) {
        case TokenKind::Number:  return "Number";
        case TokenKind::Ident:   return "Ident";
        case TokenKind::Let:     return "Let";
        case TokenKind::Print:   return "Print";
        case TokenKind::Plus:    return "Plus";
        case TokenKind::Minus:   return "Minus";
        case TokenKind::Star:    return "Star";
        case TokenKind::Slash:   return "Slash";
        case TokenKind::Caret:   return "Caret";
        case TokenKind::Eq:      return "Eq";
        case TokenKind::LParen:  return "LParen";
        case TokenKind::RParen:  return "RParen";
        case TokenKind::Newline: return "Newline";
        case TokenKind::Eof:     return "Eof";
        case TokenKind::Bad:     return "Bad";
    }
    return "Unknown";
}

struct Token {
    TokenKind kind;
    std::string text;
    double value = 0;
    int line = 1;

    json toJson() const {
        json j = {
            {"kind", tokenKindStr(kind)},
            {"text", text},
            {"line", line}
        };
        if (kind == TokenKind::Number) {
            j["value"] = value;
        }
        return j;
    }
};

// ── Lexer ─────────────────────────────────────────────────────────

class Lexer {
public:
    explicit Lexer(std::string src) : src_(std::move(src)) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos_ < src_.size()) {
            char c = src_[pos_];

            // Skip spaces/tabs (not newlines)
            if (c == ' ' || c == '\t' || c == '\r') {
                pos_++;
                continue;
            }

            // Comments: skip to end of line
            if (c == '#') {
                while (pos_ < src_.size() && src_[pos_] != '\n') pos_++;
                continue;
            }

            // Newline
            if (c == '\n') {
                tokens.push_back({TokenKind::Newline, "\\n", 0, line_});
                line_++;
                pos_++;
                continue;
            }

            // Numbers (integer or decimal)
            if (std::isdigit(c) || (c == '.' && pos_ + 1 < src_.size() && std::isdigit(src_[pos_ + 1]))) {
                size_t start = pos_;
                while (pos_ < src_.size() && (std::isdigit(src_[pos_]) || src_[pos_] == '.')) pos_++;
                std::string text = src_.substr(start, pos_ - start);
                double val = std::stod(text);
                tokens.push_back({TokenKind::Number, text, val, line_});
                continue;
            }

            // Identifiers & keywords
            if (std::isalpha(c) || c == '_') {
                size_t start = pos_;
                while (pos_ < src_.size() && (std::isalnum(src_[pos_]) || src_[pos_] == '_')) pos_++;
                std::string text = src_.substr(start, pos_ - start);
                TokenKind kind = TokenKind::Ident;
                if (text == "let")   kind = TokenKind::Let;
                if (text == "print") kind = TokenKind::Print;
                tokens.push_back({kind, text, 0, line_});
                continue;
            }

            // Single-character operators
            TokenKind kind = TokenKind::Bad;
            std::string text(1, c);
            switch (c) {
                case '+': kind = TokenKind::Plus; break;
                case '-': kind = TokenKind::Minus; break;
                case '*': kind = TokenKind::Star; break;
                case '/': kind = TokenKind::Slash; break;
                case '^': kind = TokenKind::Caret; break;
                case '=': kind = TokenKind::Eq; break;
                case '(': kind = TokenKind::LParen; break;
                case ')': kind = TokenKind::RParen; break;
                default:
                    tokens.push_back({TokenKind::Bad, text, 0, line_});
                    pos_++;
                    continue;
            }
            tokens.push_back({kind, text, 0, line_});
            pos_++;
        }
        tokens.push_back({TokenKind::Eof, "", 0, line_});
        return tokens;
    }

private:
    std::string src_;
    size_t pos_ = 0;
    int line_ = 1;
};
