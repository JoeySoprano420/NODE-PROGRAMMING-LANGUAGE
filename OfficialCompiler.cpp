// === NODECompiler v1.0.0 ===
// Complete Real-World Production Compiler

#include "lexer.h"
#include "parser.h"
#include "macro_expander.h"
#include "analyzer.h"
#include "codegen.h"
#include "nasm_emitter.h"
#include "symbol_table.h"
#include "error_reporter.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: Compiler <input.node> [--trace] [--inspect]\n";
        return 1;
    }

    std::string inputPath = argv[1];
    bool traceFlag = false, inspectFlag = false;
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--trace") traceFlag = true;
        if (std::string(argv[i]) == "--inspect") inspectFlag = true;
    }

    std::ifstream inputFile(inputPath);
    if (!inputFile.is_open()) {
        std::cerr << "[Error] Could not open file: " << inputPath << "\n";
        return 2;
    }

    std::string sourceCode((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    ErrorReporter::Init(inputPath);

    // Phase 1: Lexical Analysis
    Lexer lexer(sourceCode);
    auto tokens = lexer.Tokenize();

    // Phase 2: Parsing
    Parser parser(tokens);
    auto ast = parser.Parse();
    if (!ast) return 3;

    // Save AST
    if (inspectFlag) {
        std::ofstream astFile("output/ast.ast");
        ast->Print(astFile);
        astFile.close();
    }

    // Phase 3: Macro Expansion
    MacroExpander expander;
    ast = expander.Expand(ast);

    // Phase 4: Semantic Analysis
    Analyzer analyzer;
    if (!analyzer.Analyze(ast)) return 4;

    // Phase 5: Intermediate Representation
    auto fir = analyzer.GenerateIR(ast);
    std::ofstream irFile("output/intermediate.fir");
    irFile << fir.ToString();
    irFile.close();

    // Phase 6: Code Generation (NASM)
    NASMEmitter nasm;
    auto asmCode = nasm.Emit(fir);
    std::ofstream asmFile("output/output.asm");
    asmFile << asmCode;
    asmFile.close();

    // Phase 7: Assemble + Link
    std::string objFile = "output/output.obj";
    std::string exeFile = "output/output.exe";
    system("nasm -f win64 output/output.asm -o " + objFile);
    system("gcc " + objFile + " runtime_stubs.asm -o " + exeFile);

    // Log
    if (traceFlag) {
        std::ofstream log("output/compile.log");
        log << "Compile trace for: " << inputPath << "\n";
        log << "Tokens: " << tokens.size() << "\n";
        log << "Output written to output/output.exe\n";
        log.close();
    }

    std::cout << "✅ Compilation completed: output/output.exe\n";
    return 0;
}

// Additional required files:
// - lexer.cpp/h: implements tokenization
// - parser.cpp/h: builds AST
// - macro_expander.cpp/h: expands macros
// - analyzer.cpp/h: semantic checks, type analysis
// - nasm_emitter.cpp/h: generates x64 assembly
// - runtime_stubs.asm: implements throw_handler, validate_proof, etc.
// - Makefile or build.bat: automates build

// Sample folders:
// /samples/main.node
// /output/output.exe, .asm, .log, .fir, .ast
