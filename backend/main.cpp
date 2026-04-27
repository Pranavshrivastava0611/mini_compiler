#include "httplib.h"
#include "json.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "compiler.hpp"
#include "vm.hpp"

#include <iostream>
#include <string>

using json = nlohmann::json;

// Set CORS headers on every response
void setCors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

int main() {
    httplib::Server svr;

    // Handle preflight CORS
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        res.set_content("", "text/plain");
    });

    // ── Health Check ─────────────────────────────
    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        res.set_content("{\"ok\":true}", "application/json");
    });

    // ── Full Compile Pipeline ────────────────────
    svr.Post("/compile", [](const httplib::Request& req, httplib::Response& res) {
        setCors(res);

        json result;
        try {
            // Parse the request body
            json body = json::parse(req.body);
            std::string source = body.value("source", "");

            if (source.empty()) {
                result["error"] = "Empty source code";
                result["tokens"] = json::array();
                result["ast"] = json::object();
                result["bytecode"] = json::object();
                result["executionTrace"] = json::array();
                result["output"] = json::array();
                res.set_content(result.dump(), "application/json");
                return;
            }

            // ── Stage 1: Lexing ──
            Lexer lexer(source);
            auto tokens = lexer.tokenize();

            json tokensJson = json::array();
            for (auto& t : tokens) {
                tokensJson.push_back(t.toJson());
            }

            // ── Stage 2: Parsing ──
            Parser parser(tokens);
            auto ast = parser.parse();
            json astJson = ast->toJson();

            // ── Stage 3: Compilation ──
            Compiler compiler;
            Chunk chunk = compiler.compile(*ast);
            json bytecodeJson = chunk.toJson();

            // ── Stage 4: Execution ──
            VM vm;
            auto vmResult = vm.execute(chunk);

            json traceJson = json::array();
            for (auto& step : vmResult.trace) {
                traceJson.push_back(step.toJson());
            }

            // Build response
            result["tokens"] = tokensJson;
            result["ast"] = astJson;
            result["bytecode"] = bytecodeJson;
            result["executionTrace"] = traceJson;
            result["output"] = vmResult.output;
            result["error"] = vmResult.error.empty() ? json(nullptr) : json(vmResult.error);

        } catch (const std::exception& e) {
            result["error"] = e.what();
            if (!result.contains("tokens")) result["tokens"] = json::array();
            if (!result.contains("ast")) result["ast"] = json::object();
            if (!result.contains("bytecode")) result["bytecode"] = json::object();
            if (!result.contains("executionTrace")) result["executionTrace"] = json::array();
            if (!result.contains("output")) result["output"] = json::array();
        }

        res.set_content(result.dump(), "application/json");
    });

    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║  vm_lang server running on port 8080     ║\n";
    std::cout << "║  http://localhost:8080                    ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";
    svr.listen("0.0.0.0", 8080);

    return 0;
}
