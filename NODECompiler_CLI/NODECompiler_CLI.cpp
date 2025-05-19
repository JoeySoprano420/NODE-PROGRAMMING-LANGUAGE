
// NODECompiler_CLI.cpp
#include <iostream>
#include <fstream>
#include <string>

void print_help() {
    std::cout << "NODECompiler - AOT Symbolic Compiler\n";
    std::cout << "Usage: nodec [file.node] [--help|--doc|--grammar]\n";
}

void show_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << path << "\n";
        return;
    }
    std::string line;
    while (std::getline(file, line)) std::cout << line << "\n";
    file.close();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    std::string arg = argv[1];
    if (arg == "--help") {
        print_help();
    } else if (arg == "--doc") {
        show_file("NODE_Language_Overview.spec");
    } else if (arg == "--grammar") {
        show_file("NODE_Language_Grammar.ebnf");
    } else {
        std::cout << "Compiling: " << arg << "\n";
        // Stub: Compilation logic goes here
    }

    return 0;
}
