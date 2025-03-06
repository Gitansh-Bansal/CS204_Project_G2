#include <string>
#include <cstdint>
#include <vector>
using namespace std;

uint32_t encodeInstruction(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);

uint32_t encodeR(const Instruction& instr) {
    uint32_t opcode = opcodeMap.at(instr.opcode);
    uint32_t funct3 = funct3Map.at(instr.opcode);
    uint32_t funct7 = funct7Map.at(instr.opcode);
    
    int32_t rd = parseRegister(instr.operands[0]);
    int32_t rs1 = parseRegister(instr.operands[1]);
    int32_t rs2 = parseRegister(instr.operands[2]);
    
    if (rd < 0 || rs1 < 0 || rs2 < 0) {
        std::cerr << "Error: Invalid register in R-type instruction at line " << instr.lineNumber << std::endl;
        return 0;
    }
    
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

uint32_t encodeI(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeS(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeSB(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeU(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeUJ(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);

vector<uint8_t> encodeDirective(const Instruction& instr);
