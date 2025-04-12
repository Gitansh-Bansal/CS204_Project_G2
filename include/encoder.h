#ifndef ENCODER_H
#define ENCODER_H
#include <string>
#include <cstdint>
#include <vector>
#include "parser.h"
#include "symboltable.h" 
using namespace std;

// function to encode an instruction
uint32_t encodeInstruction(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);

// functions for encoding different types of instructions
uint32_t encodeR(const Instruction& instr);
uint32_t encodeI(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeS(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeSB(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeU(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeUJ(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);

// functions for encoding directives
vector<uint8_t> encodeDirective(const Instruction& instr);

// functions for gnerating the output line for an instruction
string generateEncodingComment(const Instruction& instr, uint32_t machineCode);
string formatOutputLine(uint32_t addr, uint32_t machineCode, const Instruction& instr, const string& encodingComment);

// helper functions for encoding
int32_t parseRegister(const string& reg, const int& linenum);
int32_t parseImmediate(const string& imm, const int& linenum, bool& running);
int64_t Immediate_64(const string& imm, const int& linenum, bool& running);
string intToHex(uint32_t value);

#endif
