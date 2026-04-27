#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <unordered_map>
#include "compiler.hpp"
#include "json.hpp"

using json = nlohmann::json;

// ── Stack-Based Virtual Machine ──────────────────────────────────

struct TraceStep {
    int ip;
    std::string op;
    int arg;
    std::vector<double> stackBefore;
    std::vector<double> stackAfter;
    std::unordered_map<std::string, double> vars;
    std::vector<std::string> output;

    json toJson() const {
        json jvars = json::object();
        for (auto& [k, v] : vars) {
            // Format nicely: display integers without decimals
            if (v == std::floor(v) && std::abs(v) < 1e15) {
                jvars[k] = (long long)v;
            } else {
                jvars[k] = v;
            }
        }
        // Same for stacks
        auto fmtStack = [](const std::vector<double>& s) {
            json arr = json::array();
            for (double v : s) {
                if (v == std::floor(v) && std::abs(v) < 1e15) {
                    arr.push_back((long long)v);
                } else {
                    arr.push_back(v);
                }
            }
            return arr;
        };
        return {
            {"ip", ip},
            {"op", op},
            {"arg", arg},
            {"stackBefore", fmtStack(stackBefore)},
            {"stackAfter", fmtStack(stackAfter)},
            {"vars", jvars},
            {"output", output}
        };
    }
};

class VM {
public:
    struct Result {
        std::vector<TraceStep> trace;
        std::vector<std::string> output;
        std::string error;
    };

    Result execute(const Chunk& chunk) {
        Result result;
        stack_.clear();
        vars_.clear();
        vars_.resize(chunk.varNames.size(), 0.0);
        int ip = 0;

        while (ip < (int)chunk.code.size()) {
            const auto& ins = chunk.code[ip];

            if (ins.op == Op::HALT) break;

            TraceStep step;
            step.ip = ip;
            step.op = opStr(ins.op);
            step.arg = ins.arg;
            step.stackBefore = stack_;
            step.output = result.output;

            // Capture current variable state
            for (int i = 0; i < (int)chunk.varNames.size(); i++) {
                step.vars[chunk.varNames[i]] = vars_[i];
            }

            try {
                switch (ins.op) {
                    case Op::PUSH:
                        if (ins.arg < 0 || ins.arg >= (int)chunk.constants.size())
                            throw std::runtime_error("Invalid constant index");
                        stack_.push_back(chunk.constants[ins.arg]);
                        break;

                    case Op::LOAD:
                        if (ins.arg < 0 || ins.arg >= (int)vars_.size())
                            throw std::runtime_error("Invalid variable index");
                        stack_.push_back(vars_[ins.arg]);
                        break;

                    case Op::STORE: {
                        if (stack_.empty()) throw std::runtime_error("Stack underflow on STORE");
                        double val = stack_.back();
                        stack_.pop_back();
                        if (ins.arg < 0 || ins.arg >= (int)vars_.size())
                            vars_.resize(ins.arg + 1, 0.0);
                        vars_[ins.arg] = val;
                        // Update vars in trace after store
                        for (int i = 0; i < (int)chunk.varNames.size(); i++) {
                            step.vars[chunk.varNames[i]] = vars_[i];
                        }
                        break;
                    }

                    case Op::ADD: binOp([](double a, double b) { return a + b; }); break;
                    case Op::SUB: binOp([](double a, double b) { return a - b; }); break;
                    case Op::MUL: binOp([](double a, double b) { return a * b; }); break;
                    case Op::DIV: binOp([](double a, double b) {
                        if (b == 0) throw std::runtime_error("Division by zero");
                        return a / b;
                    }); break;
                    case Op::POW: binOp([](double a, double b) { return std::pow(a, b); }); break;

                    case Op::NEG:
                        if (stack_.empty()) throw std::runtime_error("Stack underflow on NEG");
                        stack_.back() = -stack_.back();
                        break;

                    case Op::PRINT: {
                        if (stack_.empty()) throw std::runtime_error("Stack underflow on PRINT");
                        double val = stack_.back();
                        stack_.pop_back();
                        std::string s;
                        if (val == std::floor(val) && std::abs(val) < 1e15) {
                            s = std::to_string((long long)val);
                        } else {
                            std::ostringstream oss;
                            oss << val;
                            s = oss.str();
                        }
                        result.output.push_back(s);
                        step.output = result.output; // update output after print
                        break;
                    }

                    case Op::HALT:
                        break;
                }
            } catch (const std::exception& e) {
                result.error = std::string("Runtime error at ip ") + std::to_string(ip) + ": " + e.what();
                step.stackAfter = stack_;
                result.trace.push_back(step);
                return result;
            }

            step.stackAfter = stack_;
            result.trace.push_back(step);
            ip++;
        }

        return result;
    }

private:
    std::vector<double> stack_;
    std::vector<double> vars_;

    template<typename F>
    void binOp(F func) {
        if (stack_.size() < 2) throw std::runtime_error("Stack underflow on binary op");
        double b = stack_.back(); stack_.pop_back();
        double a = stack_.back(); stack_.pop_back();
        stack_.push_back(func(a, b));
    }
};
