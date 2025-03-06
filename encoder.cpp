#include <string>
#include <cstdint>
#include <vector>
using namespace std;

static const unordered_map<string, uint32_t> opcodeMap = {
    // R-type
    {"add", 0x33}, {"sub", 0x33}, {"sll", 0x33}, {"slt", 0x33},
    {"sra", 0x33}, {"srl", 0x33}, {"and", 0x33}, {"or", 0x33},
    {"xor", 0x33}, {"mul", 0x33}, {"div", 0x33}, {"rem", 0x33},
    // I-type
    {"addi", 0x13}, {"andi", 0x13}, {"ori", 0x13}, {"jalr", 0x67},
    {"lb", 0x03}, {"lh", 0x03}, {"lw", 0x03}, {"ld", 0x03},
    // S-type
    {"sb", 0x23}, {"sh", 0x23}, {"sw", 0x23}, {"sd", 0x23},
    // SB-type
    {"beq", 0x63}, {"bne", 0x63}, {"blt", 0x63}, {"bge", 0x63},
    // U-type
    {"lui", 0x37}, {"auipc", 0x17},
    // UJ-type
    {"jal", 0x6F}
};

static const unordered_map<string, uint32_t> funct3Map = {
    // R-type
    {"add", 0x0}, {"sub", 0x0}, {"sll", 0x1}, {"slt", 0x2},
    {"sra", 0x5}, {"srl", 0x5}, {"and", 0x7}, {"or", 0x6},
    {"xor", 0x4}, {"mul", 0x0}, {"div", 0x4}, {"rem", 0x6},
    // I-type
    {"addi", 0x0}, {"andi", 0x7}, {"ori", 0x6}, {"jalr", 0x0},
    {"lb", 0x0}, {"lh", 0x1}, {"lw", 0x2}, {"ld", 0x3},
    // S-type
    {"sb", 0x0}, {"sh", 0x1}, {"sw", 0x2}, {"sd", 0x3},
    // SB-type
    {"beq", 0x0}, {"bne", 0x1}, {"blt", 0x4}, {"bge", 0x5}
};

static const unordered_map<string, uint32_t> funct7Map = {
    {"add", 0x00}, {"sub", 0x20}, {"sll", 0x00}, {"slt", 0x00},
    {"sra", 0x20}, {"srl", 0x00}, {"and", 0x00}, {"or", 0x00},
    {"xor", 0x00}, {"mul", 0x01}, {"div", 0x01}, {"rem", 0x01}
};

uint32_t encodeInstruction(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);

uint32_t encodeR(const Instruction& instr) {
    uint32_t opcode = opcodeMap.at(instr.opcode);
    uint32_t funct3 = funct3Map.at(instr.opcode);
    uint32_t funct7 = funct7Map.at(instr.opcode);
    
    int32_t rd = parseRegister(instr.operands[0]);
    int32_t rs1 = parseRegister(instr.operands[1]);
    int32_t rs2 = parseRegister(instr.operands[2]);
    
    if (rd < 0 || rs1 < 0 || rs2 < 0) {
        cerr << "Error: Invalid register in R-type instruction at line " << instr.lineNumber << endl;
        return 0;
    }
    
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

uint32_t encodeI(const Instruction& instr, uint32_t address, const SymbolTable& symbolTable) {
    uint32_t opcode = opcodeMap.at(instr.opcode);
    uint32_t funct3 = funct3Map.at(instr.opcode);
    int32_t rd, rs1, imm;

    if (instr.operands.size() < 3) {
        cerr << "Error: Invalid I-type instruction format at line " << instr.lineNumber << endl;
        return 0;
    }

    // jalr instruction
    if (instr.opcode == "jalr") {
        rd = parseRegister(instr.operands[0]);
        if (parseRegister(instr.operands[1])!= -1) {
            rs1 = parseRegister(instr.operands[1]);
            imm = parseImmediate(instr.operands[2]);
        }
        else if (parseRegister(instr.operands[2])!= -1) {
            rs1 = parseRegister(instr.operands[2]);
            imm = parseImmediate(instr.operands[1]);
        }
        else {
            cerr << "Error: Invalid I-type instruction format at line " << instr.lineNumber << endl;
            return 0;
        }
    } 

    // load instructions (lb, lh, lw, ld)
    else if (instr.opcode == "lb" || instr.opcode == "lh" || instr.opcode == "lw" || instr.opcode == "ld") {
        rd = parseRegister(instr.operands[0]);
        imm = parseImmediate(instr.operands[1]);
        rs1 = parseRegister(instr.operands[2]);
    } 

    // other I-type
    else {
        rd = parseRegister(instr.operands[0]);
        rs1 = parseRegister(instr.operands[1]);
        
        if (symbolTable.hasSymbol(instr.operands[2])) {
            imm = symbolTable.getSymbolAddress(instr.operands[2]);
        } else {
            imm = parseImmediate(instr.operands[2]);
        }
    }
    
    if (rd < 0 || rs1 < 0) {
        cerr << "Error: Invalid register in I-type instruction at line " << instr.lineNumber << endl;
        return 0;
    }
    
    uint32_t encodedInstr =  (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
    return encodedInstr;
}

uint32_t encodeS(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
{
    uint32_t opcode = opcodeMap.at(instr.opcode);
    uint32_t funct3 = funct3Map.at(instr.opcode);
    int32_t rs1, rs2, imm;

    if (instr.operands.size() < 3) {
        cerr << "Error: Invalid I-type instruction format at line " << instr.lineNumber << endl;
        return 0;
    }

    rs2 = parseRegister(instr.operands[0]);
    imm = parseImmediate(instr.operands[1]);
    rs1 = parseRegister(instr.operands[2]);
    
    if (rs1 < 0 || rs2 < 0) {
        cerr << "Error: Invalid register in I-type instruction at line " << instr.lineNumber << endl;
        return 0;
    }

    // Split immediate into two parts for S-type encoding
    uint32_t imm_11_5 = (imm >> 5) & 0x7F;
    uint32_t imm_4_0 = imm & 0x1F;
    
    return (imm_11_5 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm_4_0 << 7) | opcode;

}
uint32_t encodeSB(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);


uint32_t encodeSB(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable){
    uint32_t opcode = opcodeMap.at(instr.opcode);
    uint32_t funct3 = funct3Map.at(instr.opcode);
    
    int32_t rs1 = parseRegister(instr.operands[0]);
    int32_t rs2 = parseRegister(instr.operands[1]);
    
    int32_t labelAddress;
    if (symbolTable.hasSymbol(instr.operands[2])) {
        labelAddress = symbolTable.getSymbolAddress(instr.operands[2]);
    } else {
        cerr << "Error: Undefined label in branch instruction at line " << instr.lineNumber << endl;
        return 0;
    }
    
    int32_t offset = labelAddress - address;
    
    if (rs1 < 0 || rs2 < 0) {
        cerr << "Error: Invalid register in SB-type instruction at line " << instr.lineNumber << endl;
        return 0;
    }
    
    if (offset % 2 != 0 || offset > 4095 || offset < -4096) {
        cerr << "Error: Branch offset out of range at line " << instr.lineNumber << endl;
        return 0;
    }
    
    uint32_t imm_12 = (offset >> 12) & 0x1;
    uint32_t imm_11 = (offset >> 11) & 0x1;
    uint32_t imm_10_5 = (offset >> 5) & 0x3F;
    uint32_t imm_4_1 = (offset >> 1) & 0xF;
    
    return (imm_12 << 31) | (imm_10_5 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm_4_1 << 8) | (imm_11 << 7) | opcode;
}


uint32_t encodeU(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
uint32_t encodeUJ(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);

vector<uint8_t> encodeDirective(const Instruction& instr);
