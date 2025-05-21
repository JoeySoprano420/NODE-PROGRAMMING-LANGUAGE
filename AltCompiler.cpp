// lexer.h
#pragma once
#include <string>
#include <vector>

enum class TokenType {
    Identifier, Number, String,
    Init, Start, Return, If, Else, For, While,
    Equal, ImmutableEqual, Plus, Minus, Mul, Div, Mod, Power,
    Less, Greater, LEQ, GEQ, And, Or, Xor, Not,
    AndEq, OrEq, XorEq, NotEq,
    ArrowR, ArrowL, Raise, Flag, Rollback, Run,
    LParen, RParen, LBrace, RBrace, LBracket, RBracket,
    Semicolon, Colon, Comma,
    Comment, MacroDelim,
    StringLiteral,
    EndOfFile, Unknown
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    std::string toString() const;
};

class Lexer {
public:
    Lexer(const std::string& src);
    std::vector<Token> tokenize();

private:
    char peek() const;
    char get();
    void skipWhitespace();
    Token identifier();
    Token number();
    Token string();
    Token comment();
    Token makeToken(TokenType, const std::string& val);

    std::string source;
    size_t pos;
    int line;
    int col;
};

// parser.h
#pragma once
#include "lexer.h"
#include <memory>
#include <vector>

struct ASTNode { virtual ~ASTNode() = default; };

struct ProgramNode : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> body;
    void dump(std::ostream& out) const;
};

struct DeclarationNode : ASTNode {
    std::string name;
    std::string value;
    bool immutable;
};

struct AssignmentNode : ASTNode {
    std::string name;
    std::string value;
    bool immutable;
};

struct IfNode : ASTNode {
    std::string condition;
    std::vector<std::unique_ptr<ASTNode>> trueBlock;
    std::vector<std::unique_ptr<ASTNode>> falseBlock;
};

struct ForNode : ASTNode {
    std::string init;
    std::string condition;
    std::string increment;
    std::vector<std::unique_ptr<ASTNode>> body;
};

struct WhileNode : ASTNode {
    std::string condition;
    std::vector<std::unique_ptr<ASTNode>> body;
};

struct MacroNode : ASTNode {
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> body;
};

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<ProgramNode> parseProgram();

private:
    const Token& peek() const;
    const Token& get();
    bool match(TokenType);

    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseDeclaration();
    std::unique_ptr<ASTNode> parseAssignment();
    std::unique_ptr<ASTNode> parseIf();
    std::unique_ptr<ASTNode> parseFor();
    std::unique_ptr<ASTNode> parseWhile();
    std::unique_ptr<ASTNode> parseMacro();

    const std::vector<Token>& tokens;
    size_t pos;
};

// macro.h
#pragma once
#include "parser.h"
#include <unordered_map>

class MacroProcessor {
public:
    void expandMacros(std::unique_ptr<ProgramNode>& program);

private:
    std::unordered_map<std::string, std::vector<std::unique_ptr<ASTNode>>> macros;
    void storeMacro(const MacroNode* macro);
    void expand(std::vector<std::unique_ptr<ASTNode>>& block);
};

// analyzer.h
#pragma once
#include "parser.h"
#include <unordered_set>
#include <stdexcept>

class SemanticAnalyzer {
public:
    void analyze(const std::unique_ptr<ProgramNode>& program);

private:
    std::unordered_set<std::string> declared;
    void analyzeNode(const ASTNode* node);
};

// codegen.h
#pragma once
#include "parser.h"
#include <string>
#include <sstream>

struct IntermediateRepresentation {
    std::ostringstream output;
    void dump(std::ostream& out) const { out << output.str(); }
};

class CodeGenerator {
public:
    IntermediateRepresentation generate(const std::unique_ptr<ProgramNode>& program);
};

// nasm_emitter.h
#pragma once
#include "codegen.h"
#include <fstream>

class NASMEmitter {
public:
    void emit(const IntermediateRepresentation& ir, const std::string& path) {
        std::ofstream out(path);
        out << "section .text\n";
        out << "global main\n";
        out << "main:\n";
        out << ir.output.str();
        out << "\nret\n";
    }
};

