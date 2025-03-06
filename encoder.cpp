#include <string>
#include <cstdint>
#include <vector>
using namespace std;

uint32_t encodeInstruction(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);

uint32_t encodeR(const Instruction& instr);
uint32_t encodeI(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeS(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeSB(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeU(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeUJ(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);

vector<uint8_t> encodeDirective(const Instruction& instr);
