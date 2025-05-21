NODECompiler/
├── src/
│   ├── main.cpp
│   ├── lexer.h / lexer.cpp
│   ├── parser.h / parser.cpp
│   ├── macro.h / macro.cpp
│   ├── analyzer.h / analyzer.cpp
│   ├── codegen.h / codegen.cpp
│   └── nasm_emitter.h / nasm_emitter.cpp
├── samples/
│   └── hello.node
├── output/
│   ├── program.asm
│   ├── program.obj
│   ├── program.exe
│   ├── trace.log
│   └── ast.txt
├── Makefile
├── build.bat
└── README.md

CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./src
OBJS = main.o lexer.o parser.o macro.o analyzer.o codegen.o

all: output/program.exe

main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

lexer.o: src/lexer.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

parser.o: src/parser.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

macro.o: src/macro.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

analyzer.o: src/analyzer.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

codegen.o: src/codegen.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

output/program.exe: $(OBJS)
	$(CXX) -o $@ $^

clean:
	rm -f *.o output/*.exe output/*.obj output/*.asm

build.bat


@echo off
if not exist output mkdir output
if not exist samples mkdir samples

cl /EHsc /I src src\main.cpp src\lexer.cpp src\parser.cpp src\macro.cpp src\analyzer.cpp src\codegen.cpp /Fe:output\NODECompiler.exe


