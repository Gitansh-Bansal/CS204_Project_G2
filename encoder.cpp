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

int32_t parseRegister(const string& reg) {
    unordered_map <string, int> reg_map = {
        {"x0", 0}, {"x1", 1}, {"x2", 2}, {"x3", 3},
        {"x4", 4}, {"x5", 5}, {"x6", 6}, {"x7", 7},
        {"x8", 8}, {"x9", 9}, {"x10", 10}, {"x11", 11},
        {"x12", 12}, {"x13", 13}, {"x14", 14}, {"x15", 15},
        {"x16", 16}, {"x17", 17}, {"x18", 18}, {"x19", 19},
        {"x20", 20}, {"x21", 21}, {"x22", 22}, {"x23", 23},
        {"x24", 24}, {"x25", 25}, {"x26", 26}, {"x27", 27},
        {"x28", 28}, {"x29", 29}, {"x30", 30}, {"x31", 31},
        {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3},
        {"tp", 4}, {"t0", 5}, {"t1", 6}, {"t2", 7},
        {"s0", 8}, {"fp", 8}, {"s1", 9}, {"a0", 10},
        {"a1", 11}, {"a2", 12}, {"a3", 13}, {"a4", 14},
        {"a5", 15}, {"a6", 16}, {"a7", 17}, {"s2", 18},
        {"s3", 19}, {"s4", 20}, {"s5", 21}, {"s6", 22},
        {"s7", 23}, {"s8", 24}, {"s9", 25}, {"s10", 26},
        {"s11", 27}, {"t3", 28}, {"t4", 29}, {"t5", 30},
        {"t6", 31}
    };
    if (reg.empty()) return -1;
    if (reg_map.find(reg) != reg_map.end()) {
        return reg_map[reg];
    }
    return -1;
}

int32_t parseImmediate(const string& imm, const int& linenum) {
    try {
        if (imm.size() > 2 && imm.substr(0, 2) == "0x") {
            return stoi(imm.substr(2), nullptr, 16);
        } 
        if (imm.size() > 2 && imm.substr(0, 2) == "0b") {
            return stoi(imm.substr(2), nullptr, 2);
        }
        return stoi(imm);
    } catch (const exception&) {
        cerr << "Error: Invalid immediate value at line " << linenum << endl;
        return 0;
    }
}

string intToHex(uint32_t value) {
    stringstream ss;
    ss << "0x" << hex << setw(8) << setfill('0') << value;
    return ss.str();
}


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
            imm = parseImmediate(instr.operands[2],instr.lineNumber);
        }
        else if (parseRegister(instr.operands[2])!= -1) {
            rs1 = parseRegister(instr.operands[2]);
            imm = parseImmediate(instr.operands[1],instr.lineNumber);
        }
        else {
            cerr << "Error: Invalid I-type instruction format at line " << instr.lineNumber << endl;
            return 0;
        }
    } 

    // load instructions (lb, lh, lw, ld)
    else if (instr.opcode == "lb" || instr.opcode == "lh" || instr.opcode == "lw" || instr.opcode == "ld") {
        rd = parseRegister(instr.operands[0]);
        imm = parseImmediate(instr.operands[1],instr.lineNumber);
        rs1 = parseRegister(instr.operands[2]);
    } 

    // other I-type
    else {
        rd = parseRegister(instr.operands[0]);
        rs1 = parseRegister(instr.operands[1]);
        
        if (symbolTable.hasSymbol(instr.operands[2])) {
            imm = symbolTable.getSymbolAddress(instr.operands[2]);
        } else {
            imm = parseImmediate(instr.operands[2],instr.lineNumber);
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
    imm = parseImmediate(instr.operands[1],instr.lineNumber);
    rs1 = parseRegister(instr.operands[2]);
    
    if (rs1 < 0 || rs2 < 0) {
        cerr << "Error: Invalid register in I-type instruction at line " << instr.lineNumber << endl;
        return 0;
    }

    uint32_t imm_11_5 = (imm >> 5) & 0x7F;
    uint32_t imm_4_0 = imm & 0x1F;
    
    return (imm_11_5 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm_4_0 << 7) | opcode;
}

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


uint32_t encodeU(const Instruction& instr, uint32_t address, const SymbolTable& symbolTable) {
    uint32_t opcode = opcodeMap.at(instr.opcode);
    
    int32_t rd = parseRegister(instr.operands[0]);

    if (rd < 0) {
        cerr << "Error: Invalid register in U-type instruction at line " << instr.lineNumber << endl;
        return 0;
    }

    int32_t imm;
    
    if (symbolTable.hasSymbol(instr.operands[1])) {
        imm = symbolTable.getSymbolAddress(instr.operands[1]);
    } else {
        imm = parseImmediate(instr.operands[1],instr.lineNumber);
    }
    
    uint32_t imm_31_12 = imm & 0xFFFFF;     // 20 bits of immediate, in case its bigger
    
    uint32_t encodedInstr =  (imm_31_12 << 12) | (rd << 7) | opcode;
    return encodedInstr;
}


uint32_t encodeUJ(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable);
{
    uint32_t opcode = opcodeMap.at(instr.opcode);
    int32_t rd = parseRegister(instr.operands[0],instr.lineNumber);

    // Get target label address
    int32_t targetAddress;
    if (symbolTable.hasSymbol(instr.operands[1])) {
        targetAddress = symbolTable.getSymbolAddress(instr.operands[1]);
    } else {
        cerr << "Error: Undefined label in jump instruction at line " << instr.lineNumber << endl;
        return 0;
    }

    // Calculate jump offset (relative to PC)
    int32_t offset = targetAddress - address;
    
    if (rd < 0) {
        cerr << "Error: Invalid register in UJ-type instruction at line " << instr.lineNumber << endl;
        return 0;
    }

    // Check if offset is in range and aligned
    if (offset % 2 != 0 || offset > 1048575 || offset < -1048576) {
        cerr << "Error: Jump offset out of range at line " << instr.lineNumber << endl;
        return 0;
    }

    // Extract bits for UJ encoding
    uint32_t imm_20 = (offset >> 20) & 0x1;
    uint32_t imm_19_12 = (offset >> 12) & 0xFF;
    uint32_t imm_11 = (offset >> 11) & 0x1;
    uint32_t imm_10_1 = (offset >> 1) & 0x3FF;
    return (imm_20 << 31) | (imm_10_1 << 21) | (imm_11 << 20) | (imm_19_12 << 12) | (rd << 7) | opcode;

}

vector<uint8_t> encodeDirective(const Instruction& instr) {
    vector<uint8_t> data;
    
    if (instr.opcode == ".byte") {
        for (const auto& operand : instr.operands) {
            int32_t value = parseImmediate(operand);
            auto val1 = value & 0xFF;
            if (value != val1) cerr << "Error: Too big entry "<< value << " to fit in byte at line " << instr.lineNumber<< endl;
            
            data.push_back(static_cast<uint8_t>(val1));
        }
    } else if (instr.opcode == ".half") {
        for (const auto& operand : instr.operands) {
            int32_t value = parseImmediate(operand); 
            if (value >> 16 != 0) cerr << "Error: Too big entry "<< value << " to fit in half word at line " << instr.lineNumber<< endl;
            
            data.push_back(static_cast<uint8_t>(value & 0xFF));
            data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        }
    } else if (instr.opcode == ".word") {
        for (const auto& operand : instr.operands) {
            int32_t value = parseImmediate(operand);
            if (value >> 32 != 0) cerr << "Error: Too big entry "<< value << " to fit in word at line " << instr.lineNumber<< endl;

            data.push_back(static_cast<uint8_t>(value & 0xFF));
            data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
            data.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
            data.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
        }
    } else if (instr.opcode == ".dword") {
        for (const auto& operand : instr.operands) {
            int64_t value = parseImmediate(operand);
            if (value >> 64 != 0) cerr << "Error: Too big entry "<< value << " to fit in double word at line " << instr.lineNumber<< endl;

            for (int i = 0; i < 8; i++) {
                data.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
            }
        }
    } else if (instr.opcode == ".asciiz") {
        string str = instr.operands[0];
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
            str = str.substr(1, str.size() - 2);
        }
        
        for (char c : str) {
            data.push_back(static_cast<uint8_t>(c));
        }
        data.push_back(0);        // null terminator
    }
    
    return data;
}
