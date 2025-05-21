// NODECompiler.cpp – Massively Expanded NODE Compiler Core

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <memory>
#include <cctype>
#include <cstdlib>

// --- NODE Operation Table ---
struct NODEInstruction {
    std::string llvmEquivalent;
    std::string nasmEquivalent;
};

std::unordered_map<std::string, NODEInstruction> instructionMap = {
    {"xor_eq",  {"%result = xor i64 %X, %Y", "XOR RAX, RBX"}},
    {"not_eq",  {"%cmp = icmp ne i64 %X, %Y", "CMP RAX, RBX\nJNE throw_handler"}},
    {"and_eq",  {"%result = and i64 %X, %Y", "AND RAX, RBX"}},
    {"or_eq",   {"%result = or i64 %X, %Y", "OR RAX, RBX"}},
    {"not",     {"%result = xor i64 %X, -1", "NOT RAX"}},
    {"shiftl",  {"%result = shl i64 %X, %Y", "SHL RAX, CL"}},
    {"shiftr",  {"%result = lshr i64 %X, %Y", "SHR RAX, CL"}},

    {"new",     {"%ptr = call i8* @malloc(i64 %size)", "CALL malloc"}},
    {"delete",  {"call void @free(i8* %ptr)", "CALL free"}},
    {"move",    {"store i64 %X, i64* %Y", "MOV [Y], X"}},
    {"copy",    {"%X = load i64, i64* %Y", "MOV X, [Y]"}},

    {"if_lt",   {"%cmp = icmp slt i64 %X, %Y", "CMP RAX, RBX\nJL true_label"}},
    {"if_gt",   {"%cmp = icmp sgt i64 %X, %Y", "CMP RAX, RBX\nJG true_label"}},
    {"if_eq",   {"%cmp = icmp eq i64 %X, %Y", "CMP RAX, RBX\nJE true_label"}},
    {"throw",   {"br label %throw_handler", "JMP throw_handler"}},

    {"call",    {"call i64 @func(i64 %X, i64 %Y)", "CALL func"}},
    {"routine", {"define i64 @func(i64 %X, i64 %Y) { ... }", "func PROC\n...\nRET"}},
    {"return",  {"ret i64 %X", "MOV RAX, X\nRET"}},
    {"exit",    {"call void @exit(i32 0)", "MOV RDI, 0\nCALL exit"}},

    {"add",     {"%res = add i64 %X, %Y", "ADD RAX, RBX"}},
    {"sub",     {"%res = sub i64 %X, %Y", "SUB RAX, RBX"}},
    {"mul",     {"%res = mul i64 %X, %Y", "IMUL RAX, RBX"}},
    {"div",     {"%res = sdiv i64 %X, %Y", "IDIV RBX"}},
    {"mod",     {"%res = srem i64 %X, %Y", "IDIV RBX (RDX = remainder)"}},

    {"push",    {"N/A", "PUSH RAX"}},
    {"pop",     {"N/A", "POP RAX"}},
    {"save",    {"N/A", "PUSHAD"}},
    {"restore", {"N/A", "POPAD"}},
    {"swap",    {"%tmp = %X; %X = %Y; %Y = %tmp", "XCHG RAX, RBX"}},

    {"start",   {"define i32 @main() {", "main PROC"}},
    {"label",   {"label: ", "label:"}},
    {"goto",    {"br label %X", "JMP label"}},

    {"trap",    {"call void @trap()", "INT 3"}},
    {"panic",   {"call void @puts\ncall exit", "MOV RDI, msg\nCALL puts\nCALL exit"}},

    {"inc",     {"%res = add i64 %X, 1", "INC RAX"}},           // Increment
    {"dec",     {"%res = sub i64 %X, 1", "DEC RAX"}},           // Decrement
    {"bitnot",  {"%res = xor i64 %X, -1", "NOT RAX"}},          // Bitwise NOT
    {"land",    {"%res = and i64 %X, %Y\n%res = icmp ne i64 %res, 0", "AND RAX, RBX\nSETNE AL"}}, // Logical AND
    {"lor",     {"%res = or i64 %X, %Y\n%res = icmp ne i64 %res, 0", "OR RAX, RBX\nSETNE AL"}},   // Logical OR
    {"print",   {"call void @print(i64 %X)", "MOV RDI, RAX\nCALL print"}}, // Print
    {"input",   {"%X = call i64 @input()", "CALL input"}},                 // Input
    {"nop",     {"", "NOP"}},                                             // No operation

    {"and",     {"%res = and i64 %X, %Y", "AND RAX, RBX"}}, // 0x1D2
    {"or",      {"%res = or i64 %X, %Y", "OR RAX, RBX"}},   // 0x1D3
    {"xor",     {"%res = xor i64 %X, %Y", "XOR RAX, RBX"}}, // 0x1D4
    {"not",     {"%res = xor i64 %X, -1", "NOT RAX"}},      // 0x1D5
    {"and_eq",  {"%result = and i64 %X, %Y", "AND RAX, RBX"}}, // 0x1D6
    {"or_eq",   {"%result = or i64 %X, %Y", "OR RAX, RBX"}},   // 0x1D7
    {"xor_eq",  {"%result = xor i64 %X, %Y", "XOR RAX, RBX"}}, // 0x1D8
    {"not_eq",  {"%cmp = icmp ne i64 %X, %Y", "CMP RAX, RBX\nJNE throw_handler"}}, // 0x1D9
    {"nullptr", {"XOR RAX, RAX", "XOR RAX, RAX"}}, // 0x1E0
    {"throw",   {"br label %throw_handler", "JMP throw_handler"}}, // 0x1E1
    {"try",     {"; try block", "; try block"}}, // 0x1E2
    {"catch",   {"; catch block", "; catch block"}}, // 0x1E3
    {"if",      {"%cmp = icmp", "CMP RAX, RBX"}}, // 0x205
    {"for",     {"; for loop", "; for loop"}}, // 0x20B
    {"new",     {"%ptr = call i8* @malloc(i64 %size)", "CALL malloc"}}, // 0x201
    {"delete",  {"call void @free(i8* %ptr)", "CALL free"}}, // 0x202
    {"reinterpret_cast", {"; reinterpret_cast", "; reinterpret_cast"}}, // 0x1DA
    {"dynamic_cast", {"; dynamic_cast", "; dynamic_cast"}}, // 0x1DB
    {"static_cast", {"; static_cast", "; static_cast"}}, // 0x1DC
    {"const_cast", {"; const_cast", "; const_cast"}}, // 0x1DD
    {"is_base_of", {"; is_base_of", "; is_base_of"}}, // 0x1DE
};

std::string translateLine(const std::string& line, int lineNumber, std::ostream& log) {
    std::stringstream result;
    std::string trimmed = std::regex_replace(line, std::regex("#.*"), ""); // Remove single-line comments
    trimmed = std::regex_replace(trimmed, std::regex("\\*\\*.*\\*\\*"), ""); // Remove multi-line comments

    // Handle Let as Init
    std::string processed = std::regex_replace(trimmed, std::regex("\\bLet\\b"), "Init");

    // Handle | as macro delimiters (emit as NASM comment)
    processed = std::regex_replace(processed, std::regex("\\|"), "; |");

    // Handle : as separator (emit as NASM comment)
    processed = std::regex_replace(processed, std::regex(":"), "; :");

    // Handle @, $, ! as specifiers/modifiers (emit as NASM comment)
    processed = std::regex_replace(processed, std::regex("(@|\\$|!)"), "; $&");

    // Handle ^ as exponent (emit as NASM comment for now)
    processed = std::regex_replace(processed, std::regex("\\^"), "; ^ (exponent)");

    // Handle ~>, ->, <-, <~, <<, >> (emit as NASM comment for now)
    processed = std::regex_replace(processed, std::regex("~>|->|<-|<~|<<|>>"), "; $&");

    // Handle [] as buffer/mediator (emit as NASM comment)
    processed = std::regex_replace(processed, std::regex("\\[|\\]"), "; []");

    std::istringstream iss(processed);
    std::string token;
    bool matched = false;

    while (iss >> token) {
        auto it = instructionMap.find(token);
        if (it != instructionMap.end()) {
            result << it->second.nasmEquivalent << "\n";
            matched = true;
        }
    }
    if (!matched && !processed.empty()) {
        result << "; " << processed << "\n"; // Emit as NASM comment if not recognized
        log << "[Line " << lineNumber << "] Warning: No recognized NODE operation in: " << line << "\n";
    }
    return result.str();
}

void compileNODEFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return;
    }

    std::ofstream asmFile("output.asm");
    std::ofstream logFile("compile.log");
    asmFile << "section .text\nglobal _start\n_start:\n";

    std::string line;
    int lineNumber = 0;
    while (std::getline(inFile, line)) {
        ++lineNumber;
        std::string asmCode = translateLine(line, lineNumber, logFile);
        if (!asmCode.empty()) asmFile << asmCode;
    }

    asmFile << "\nMOV RAX, 60\nXOR RDI, RDI\nSYSCALL\n";
    // Add runtime for print/input
    asmFile << R"(
; --- Runtime support for print and input (Windows x64) ---
section .data
    input_buffer: times 32 db 0
    output_buffer: times 32 db 0
    bytes_written: dq 0
    bytes_read: dq 0
    hStdIn: dq 0
    hStdOut: dq 0

section .text

extern GetStdHandle
extern WriteConsoleA
extern ReadConsoleA
extern wsprintfA

print:
    ; RDI = integer to print
    sub     rsp, 40                ; shadow space for Win64 ABI
    lea     rcx, [output_buffer]   ; lpOut buffer
    mov     rdx, output_buffer     ; lpOut buffer
    mov     r8, 32                 ; buffer size
    mov     r9, "%lld", 0          ; format string
    mov     rax, rdi               ; value to print
    mov     [rsp+32], rax          ; pass value on stack
    mov     rcx, rdx               ; wsprintfA(lpOut, format, value)
    mov     rdx, r9
    mov     r8, [rsp+32]
    call    wsprintfA

    mov     rcx, -11               ; STD_OUTPUT_HANDLE
    call    GetStdHandle
    mov     [hStdOut], rax
    mov     rcx, rax               ; hConsoleOutput
    lea     rdx, [output_buffer]   ; lpBuffer
    mov     r8d, 32                ; nNumberOfCharsToWrite
    lea     r9, [bytes_written]    ; lpNumberOfCharsWritten
    push    0                      ; lpReserved
    call    WriteConsoleA
    add     rsp, 48
    ret

input:
    mov     rcx, -10               ; STD_INPUT_HANDLE
    call    GetStdHandle
    mov     [hStdIn], rax
    mov     rcx, rax               ; hConsoleInput
    lea     rdx, [input_buffer]    ; lpBuffer
    mov     r8d, 32                ; nNumberOfCharsToRead
    lea     r9, [bytes_read]       ; lpNumberOfCharsRead
    push    0                      ; lpReserved
    call    ReadConsoleA
    ; Convert ASCII to integer (simple, not robust)
    lea     rsi, [input_buffer]
    xor     rax, rax
    xor     rcx, rcx
.next_digit:
    mov     bl, [rsi + rcx]
    cmp     bl, 13                 ; CR
    je      .done
    cmp     bl, 10                 ; LF
    je      .done
    cmp     bl, 0
    je      .done
    sub     bl, '0'
    cmp     bl, 9
    ja      .done
    imul    rax, rax, 10
    add     rax, rbx
    inc     rcx
    jmp     .next_digit
.done:
    ret

section .data
    fmt: db "%lld",0
)";

    asmFile.close();
    inFile.close();
    logFile.close();

    std::cout << "Assembling and linking output.asm...\n";
    int result = system("nasm -g -f elf64 output.asm -o output.o && ld -g output.o -o output.exe");
    if (result == 0) {
        std::cout << "[SUCCESS] Compilation successful: output.exe generated.\nLogs: compile.log\n";
    }
    else {
        std::cerr << "❌ Compilation failed. Check NASM/LD installation.\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        compileNODEFile(argv[1]);
        return 0;
    }

    std::string input;
    std::cout << "NODE Compiler Shell (v1.0)\nType NODE instructions (e.g., xor_eq, add, throw). Ctrl+C to exit.\n\n";
    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, input);
        if (!input.empty()) std::cout << translateLine(input, 0, std::cout);
    }
    return 0;
}

