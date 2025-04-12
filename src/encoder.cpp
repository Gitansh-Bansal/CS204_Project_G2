// including libraries and header files
#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <bitset>
#include <limits>
#include <unordered_map>
#include <iomanip>
#include "encoder.h"
using namespace std;

//unordered map for opcode
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

//unordered map for funct3
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

//unordered map for funct7
static const unordered_map<string, uint32_t> funct7Map = {
    {"add", 0x00}, {"sub", 0x20}, {"sll", 0x00}, {"slt", 0x00},
    {"sra", 0x20}, {"srl", 0x00}, {"and", 0x00}, {"or", 0x00},
    {"xor", 0x00}, {"mul", 0x01}, {"div", 0x01}, {"rem", 0x01}
};

//function to parse register to integer
int32_t parseRegister(const string& reg, const int& linenum) {
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

    // check if register is empty
    if (reg.empty()) return -1;
    // check if register is in the map
    if (reg_map.find(reg) != reg_map.end()) {
        return reg_map[reg];
    }
    cerr << "Error: Invalid register " << reg << " at line " << linenum << endl;
    return -1;
}

//function to parse immediate to integer
int32_t parseImmediate(const string& imm, const int& linenum, bool& running) {

    //check if immediate is in hex , binary or decimal
    try {
        size_t pos;
        int result;
        if (imm.size() > 2 && imm.substr(0, 2) == "0x") {
            result =  stoi(imm.substr(2), &pos, 16);
            if (pos != imm.substr(2).size()) {
                throw std::runtime_error("Invalid immediate value found!!");
            }
            return result;
        } 
        else if (imm.size() > 2 && imm.substr(0, 2) == "0b") {
            result =  stoi(imm.substr(2), &pos, 2);
            if (pos != imm.substr(2).size()) {
                throw std::runtime_error("Invalid immediate value found!!");
            }      
            return result;
        }
        else {
            result =  stoi(imm, &pos);
            if (pos != imm.size()) {
                throw std::runtime_error("Invalid immediate value found!!");
            }      
            return result;
        }
    }
    //catch exception if immediate is invalid
    catch (const exception&) {
        cerr << "Error: Invalid immediate value "<<imm<<" at line " << linenum << endl;
        running = false;
        return 0;
    }
}

//function to parse immediate to 64 bit integer for double word
int64_t Immediate_64(const string& imm, const int& linenum, bool& running) {

    //check if immediate is in hex , binary or decimal
    try {
        if (imm.size() > 2 && imm.substr(0, 2) == "0x") {
            return stoll(imm.substr(2), nullptr, 16);
        } 
        if (imm.size() > 2 && imm.substr(0, 2) == "0b") {
            return stoll(imm.substr(2), nullptr, 2);
        }
        return stoll(imm);
    }
    //catch exception if immediate is invalid
    catch (const exception&) {
        cerr << "Error: Invalid immediate value "<<imm<<" at line " << linenum << endl;
        running = false;
        return 0;
    }
}

//function to convert integer to hex string
string intToHex(uint32_t value) {
    stringstream ss;
    ss << "0x" << hex << setw(8) << setfill('0') << value;
    return ss.str();
}


//function to encode R type instructiosn
uint32_t encodeR(const Instruction& instr) {
    uint32_t opcode = opcodeMap.at(instr.opcode);
    uint32_t funct3 = funct3Map.at(instr.opcode);
    uint32_t funct7 = funct7Map.at(instr.opcode);

    //check if operands are less than 3
    if (instr.operands.size() < 3 || instr.operands.size() > 3) {
        cerr << "Error: Invalid I-type instruction format at line " << instr.lineNumber << endl;
        return 0;
    }
    
    //parse operands to integer
    int32_t rd = parseRegister(instr.operands[0],instr.lineNumber);
    int32_t rs1 = parseRegister(instr.operands[1],instr.lineNumber);
    int32_t rs2 = parseRegister(instr.operands[2],instr.lineNumber);
    
    //check if operands are valid
    if (rd < 0 || rs1 < 0 || rs2 < 0) {
        return 0;
    }
    
    //returning encoded instruction
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

//function to encode I type instructions
uint32_t encodeI(const Instruction& instr, uint32_t address, const SymbolTable& symbolTable) {
    bool running = true;
    uint32_t opcode = opcodeMap.at(instr.opcode);
    uint32_t funct3 = funct3Map.at(instr.opcode);
    int32_t rd, rs1, imm;

    //check if operands are less than 3
    if (instr.operands.size() < 3 || instr.operands.size() > 3) {
        cerr << "Error: Invalid I-type instruction format at line " << instr.lineNumber << endl;
        return 0;
    }

    // jalr instruction
    if (instr.opcode == "jalr") {
        rd = parseRegister(instr.operands[0],instr.lineNumber);
        if (parseRegister(instr.operands[1],instr.lineNumber)!= -1) {
            rs1 = parseRegister(instr.operands[1],instr.lineNumber);
            imm = parseImmediate(instr.operands[2],instr.lineNumber,running);

        }
        else if (parseRegister(instr.operands[2],instr.lineNumber)!= -1) {
            rs1 = parseRegister(instr.operands[2],instr.lineNumber);
            imm = parseImmediate(instr.operands[1],instr.lineNumber,running);
        }
        else {
            return 0;
        }
    } 

    // load instructions (lb, lh, lw, ld)
    else if (instr.opcode == "lb" || instr.opcode == "lh" || instr.opcode == "lw" || instr.opcode == "ld") {
        rd = parseRegister(instr.operands[0],instr.lineNumber);
        imm = parseImmediate(instr.operands[1],instr.lineNumber,running);
        rs1 = parseRegister(instr.operands[2],instr.lineNumber);
    } 

    // other I-type
    else {
        rd = parseRegister(instr.operands[0],instr.lineNumber);
        rs1 = parseRegister(instr.operands[1],instr.lineNumber);
        
        if (symbolTable.hasSymbol(instr.operands[2])) {
            imm = symbolTable.getSymbolAddress(instr.operands[2]);
        } else {
            imm = parseImmediate(instr.operands[2],instr.lineNumber,running);
        }
    }

    if (!running) {
        cerr << "Error: Invalid immediate value at line " << instr.lineNumber << endl;
        return 0;
    }
    
    //check if immediate is in range
    if (imm < -2048 || imm > 2047) {
        cerr << "Error: Immediate value out of range [-2048, 2047] at line " << instr.lineNumber << endl;
        return 0;
    }

    //check if registers are valid
    if (rd < 0 || rs1 < 0) {
        return 0;
    }
    
    //returning encoded instruction
    uint32_t encodedInstr =  (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
    return encodedInstr;
}


//function to encode S type instructions
uint32_t encodeS(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable){
    bool running = true;
    uint32_t opcode = opcodeMap.at(instr.opcode);
    uint32_t funct3 = funct3Map.at(instr.opcode);
    int32_t rs1, rs2, imm;

    //check if operands are less than 3
    if (instr.operands.size() < 3 || instr.operands.size() > 3) {
        cerr << "Error: Invalid S-type instruction format at line " << instr.lineNumber << endl;
        return 0;
    }

    //parse operands to integer
    rs2 = parseRegister(instr.operands[0],instr.lineNumber);
    imm = parseImmediate(instr.operands[1],instr.lineNumber,running);
    rs1 = parseRegister(instr.operands[2],instr.lineNumber);
    
    //check if operands are valid
    if (rs1 < 0 || rs2 < 0) {
        return 0;
    }
    if (!running) {
        cerr << "Error: Invalid immediate value at line " << instr.lineNumber << endl;
        return 0;
    }
    //check if immediate is in range
    if (imm < -2048 || imm > 2047) {
        cerr << "Error: Immediate value out of range [-2048, 2047] at line " << instr.lineNumber << endl;
        return 0;
    }

    //returning encoded instruction
    uint32_t imm_11_5 = (imm >> 5) & 0x7F;
    uint32_t imm_4_0 = imm & 0x1F;
    return (imm_11_5 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm_4_0 << 7) | opcode;
}

//function to encode SB type instructions
uint32_t encodeSB(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable){
    uint32_t opcode = opcodeMap.at(instr.opcode);
    uint32_t funct3 = funct3Map.at(instr.opcode);
    
    //check if operands are less than 3
    if (instr.operands.size() < 3 || instr.operands.size() > 3) {
        cerr << "Error: Invalid SB-type instruction format at line " << instr.lineNumber << endl;
        return 0;
    }

    //parse operands to integer
    int32_t rs1 = parseRegister(instr.operands[0],instr.lineNumber);
    int32_t rs2 = parseRegister(instr.operands[1],instr.lineNumber);
    
    //checking if label is in symbol table
    int32_t labelAddress;
    if (symbolTable.hasSymbol(instr.operands[2])) {
        labelAddress = symbolTable.getSymbolAddress(instr.operands[2]);
    } else {
        cerr << "Error: Undefined label in branch instruction at line " << instr.lineNumber << endl;
        return 0;
    }
    
    //calculating offset
    int32_t offset = labelAddress - addr;
    
    //check if registers are valid
    if (rs1 < 0 || rs2 < 0) {
        return 0;
    }
    
    //check if offset is in range
    if (offset % 2 != 0 || offset > 4095 || offset < -4096) {
        cerr << "Error: Branch offset out of range at line " << instr.lineNumber << endl;
        return 0;
    }
      
    //returning encoded instruction
    uint32_t imm_12 = (offset >> 12) & 0x1;
    uint32_t imm_11 = (offset >> 11) & 0x1;
    uint32_t imm_10_5 = (offset >> 5) & 0x3F;
    uint32_t imm_4_1 = (offset >> 1) & 0xF;
    return (imm_12 << 31) | (imm_10_5 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm_4_1 << 8) | (imm_11 << 7) | opcode;
}

//function to encode U type instructions
uint32_t encodeU(const Instruction& instr, uint32_t address, const SymbolTable& symbolTable) {
    bool running = true;
    uint32_t opcode = opcodeMap.at(instr.opcode);

    //check if operands are less than 2
    if (instr.operands.size() < 2 || instr.operands.size() > 2) {   
        cerr << "Error: Invalid U-type instruction format at line " << instr.lineNumber << endl;
        return 0;
    }
    
    int32_t rd = parseRegister(instr.operands[0],instr.lineNumber);

    //check if register is valid
    if (rd < 0) {
        return 0;
    }

    //parse immediate to integer
    int32_t imm;
    if (symbolTable.hasSymbol(instr.operands[1])) {
        imm = symbolTable.getSymbolAddress(instr.operands[1]);
    } else {
        imm = parseImmediate(instr.operands[1],instr.lineNumber,running);
    }
    if (!running) {
        cerr << "Error: Invalid immediate value at line " << instr.lineNumber << endl;
        return 0;
    }
    //check if immediate is in range
    if (imm < -1048576 || imm > 1048574) { 
        cerr << "Error: Immediate value out of range [-1048576, 1048574] at line " << instr.lineNumber << endl;
        return 0;
    }

    //returning encoded instruction
    uint32_t imm_31_12 = imm & 0xFFFFF; 
    uint32_t encodedInstr =  (imm_31_12 << 12) | (rd << 7) | opcode;
    return encodedInstr;
}

//function to encode UJ type instructions
uint32_t encodeUJ(const Instruction& instr, uint32_t addr, const SymbolTable& symbolTable){
    uint32_t opcode = opcodeMap.at(instr.opcode);

    //check if operands are less than 2
    if (instr.operands.size() < 2 || instr.operands.size() > 2) {
        cerr << "Error: Invalid UJ-type instruction format at line " << instr.lineNumber << endl;
        return 0;
    }

    int32_t rd = parseRegister(instr.operands[0],instr.lineNumber);

    //checking if label is in symbol table
    int32_t targetAddress;
    if (symbolTable.hasSymbol(instr.operands[1])) {
        targetAddress = symbolTable.getSymbolAddress(instr.operands[1]);
    } else {
        cerr << "Error: Undefined label in jump instruction at line " << instr.lineNumber << endl;
        return 0;
    }
    //calculating offset
    int32_t offset = targetAddress - addr;
    
    //check if register is valid
    if (rd < 0) {
        return 0;
    }

    //check if offset is in range
    if (offset % 2 != 0 || offset > 1048575 || offset < -1048576) {
        cerr << "Error: Jump offset out of range at line " << instr.lineNumber << endl;
        return 0;
    }

    //returning encoded instruction
    uint32_t imm_20 = (offset >> 20) & 0x1;
    uint32_t imm_19_12 = (offset >> 12) & 0xFF;
    uint32_t imm_11 = (offset >> 11) & 0x1;
    uint32_t imm_10_1 = (offset >> 1) & 0x3FF;
    return (imm_20 << 31) | (imm_10_1 << 21) | (imm_11 << 20) | (imm_19_12 << 12) | (rd << 7) | opcode;

}

//function to encode directives
vector<uint8_t> encodeDirective(const Instruction& instr) {
    bool running = true;
    //vector to store data
    vector<uint8_t> data;

    //opcode is .byte
    if (instr.opcode == ".byte") {
        //parsing each operand to integer
        for (const auto& operand : instr.operands) {
            int32_t value = parseImmediate(operand, instr.lineNumber,running);
            if (!running) {
                cerr << "Error: Invalid value at line " << instr.lineNumber << endl;
                return vector<uint8_t>();
            }
            if (value > numeric_limits<int8_t>::max() || value < numeric_limits<int8_t>::min()) cerr << "Error: Too big entry "<< value << " to fit in byte at line " << instr.lineNumber<< endl;
            
            data.push_back(static_cast<uint8_t>(value & 0xFF));
        }
    } 

    //opcode is .half
    else if (instr.opcode == ".half") {
        //parsing each operand to integer
        for (const auto& operand : instr.operands) {
            int32_t value = parseImmediate(operand, instr.lineNumber,running); 
            if (!running) {
                cerr << "Error: Invalid value at line " << instr.lineNumber << endl;
                return vector<uint8_t>();
            }
            if (value > numeric_limits<int16_t>::max() || value < numeric_limits<int16_t>::min()) cerr << "Error: Too big entry "<< value << " to fit in half word at line " << instr.lineNumber<< endl;

            //pushing each byte to data vector
            data.push_back(static_cast<uint8_t>(value & 0xFF));
            data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        }
    }

    //opcode is .word
    else if (instr.opcode == ".word") {
        //parsing each operand to integer
        for (const auto& operand : instr.operands) {
            int32_t value = parseImmediate(operand, instr.lineNumber,running);
            if (!running) {
                cerr << "Error: Invalid value at line " << instr.lineNumber << endl;
                return vector<uint8_t>();
            }
            if (value > numeric_limits<int32_t>::max() || value < numeric_limits<int32_t>::min()) cerr << "Error: Too big entry "<< value << " to fit in word at line " << instr.lineNumber<< endl;
            //pushing each byte to data vector
            data.push_back(static_cast<uint8_t>(value & 0xFF));
            data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
            data.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
            data.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
        }
    } 

    //opcode is .dword
    else if (instr.opcode == ".dword") {
        //parsing each operand to integer
        for (const auto& operand : instr.operands) {
            int64_t value = Immediate_64(operand, instr.lineNumber,running);
            if (!running) {
                cerr << "Error: Invalid value at line " << instr.lineNumber << endl;
                return vector<uint8_t>();
            }
            if (value > numeric_limits<int64_t>::max() || value < numeric_limits<int64_t>::min()) cerr << "Error: Too big entry "<< value << " to fit in double word at line " << instr.lineNumber<< endl;
            //pushing each byte to data vector
            for (int i = 0; i < 8; i++) {
                data.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
            }
        }
    } 

    //opcode is .asciiz
    else if (instr.opcode == ".asciiz") {
        if (instr.operands.size() != 1) {
            cerr << "Error: Invalid .asciiz directive format at line " << instr.lineNumber << endl;
            return vector<uint8_t>();
        }
        //extracting string from operand
        string str = instr.operands[0];
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
            str = str.substr(1, str.size() - 2);
        }
        else {
            cerr << "Error: Invalid string format at line " << instr.lineNumber << endl;
            return vector<uint8_t>();
        }
        //pushing each byte to data vector
        for (char c : str) {
            data.push_back(static_cast<uint8_t>(c));
        }
        //pushing null terminator
        data.push_back(0);       
    }
    return data;
}

//function to encode instructions
uint32_t encodeInstruction(const Instruction& instr, uint32_t address, const SymbolTable& symbolTable) {
    //switch case to encode instructions based on type
    switch (instr.type) {
        case R_TYPE:
            return encodeR(instr);
        case I_TYPE:
            return encodeI(instr, address, symbolTable);
        case S_TYPE:
            return encodeS(instr, address, symbolTable);
        case SB_TYPE:
            return encodeSB(instr, address, symbolTable);
        case U_TYPE:
            return encodeU(instr, address, symbolTable);
        case UJ_TYPE:
            return encodeUJ(instr, address, symbolTable);
        default:
            cerr << "Error: Unknown instruction type at line " << instr.lineNumber << endl;
            return 0;
    }
}

//function to generate encoding comment
string generateEncodingComment(const Instruction& instr, uint32_t machineCode) {

    //stringstream to store comment
    stringstream comment;

    //switch case to generate comment based on instruction type
    switch (instr.type) {

        //R type instruction
        case R_TYPE: {
            uint32_t opcode = machineCode & 0x7F;
            uint32_t rd = (machineCode >> 7) & 0x1F;
            uint32_t funct3 = (machineCode >> 12) & 0x7;
            uint32_t rs1 = (machineCode >> 15) & 0x1F;
            uint32_t rs2 = (machineCode >> 20) & 0x1F;
            uint32_t funct7 = (machineCode >> 25) & 0x7F;
            
            comment << bitset<7>(opcode) << "-"
                    << bitset<3>(funct3) << "-"
                    << bitset<7>(funct7) << "-"
                    << bitset<5>(rd) << "-"
                    << bitset<5>(rs1) << "-"
                    << bitset<5>(rs2) << "-"
                    << "NULL";
            break;
        }

        //I type instruction
        case I_TYPE: {
            uint32_t opcode = machineCode & 0x7F;
            uint32_t rd = (machineCode >> 7) & 0x1F;
            uint32_t funct3 = (machineCode >> 12) & 0x7;
            uint32_t rs1 = (machineCode >> 15) & 0x1F;
            uint32_t imm = (machineCode >> 20) & 0xFFF;
            
            comment << bitset<7>(opcode) << "-"
                    << bitset<3>(funct3) << "-"
                    << "NULL" << "-"
                    << bitset<5>(rd) << "-"
                    << bitset<5>(rs1) << "-"
                    << bitset<12>(imm);
            break;
        }

        //S type instruction
        case S_TYPE: {
            uint32_t opcode = machineCode & 0x7F;
            uint32_t imm_4_0 = (machineCode >> 7) & 0x1F;
            uint32_t funct3 = (machineCode >> 12) & 0x7;
            uint32_t rs1 = (machineCode >> 15) & 0x1F;
            uint32_t rs2 = (machineCode >> 20) & 0x1F;
            uint32_t imm_11_5 = (machineCode >> 25) & 0x7F;
            uint32_t imm = (imm_11_5 << 5) | imm_4_0;
            
            comment << bitset<7>(opcode) << "-"
                    << bitset<3>(funct3) << "-"
                    << "NULL" << "-"
                    << "NULL" << "-"
                    << bitset<5>(rs1) << "-"
                    << bitset<5>(rs2) << "-"
                    << bitset<12>(imm);
            break;
        }

        //SB type instruction
        case SB_TYPE: {
            uint32_t opcode = machineCode & 0x7F;
            uint32_t imm_11 = (machineCode >> 7) & 0x1;
            uint32_t imm_4_1 = (machineCode >> 8) & 0xF;
            uint32_t funct3 = (machineCode >> 12) & 0x7;
            uint32_t rs1 = (machineCode >> 15) & 0x1F;
            uint32_t rs2 = (machineCode >> 20) & 0x1F;
            uint32_t imm_10_5 = (machineCode >> 25) & 0x3F;
            uint32_t imm_12 = (machineCode >> 31) & 0x1;
            
            uint32_t imm = (imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1);
            
            comment << bitset<7>(opcode) << "-"
                    << bitset<3>(funct3) << "-"
                    << "NULL" << "-"
                    << "NULL" << "-"
                    << bitset<5>(rs1) << "-"
                    << bitset<5>(rs2) << "-"
                    << bitset<12>(imm>>1);
            break;
        }
        
        //U type instruction
        case U_TYPE: {
            uint32_t opcode = machineCode & 0x7F;
            uint32_t rd = (machineCode >> 7) & 0x1F;
            uint32_t imm_31_12 = (machineCode >> 12) & 0xFFFFF;
            
            comment << bitset<7>(opcode) << "-"
                    << "NULL" << "-"
                    << "NULL" << "-"
                    << bitset<5>(rd) << "-"
                    << "NULL" << "-"
                    << "NULL" << "-"
                    << bitset<20>(imm_31_12);
            break;
        }
        
        //UJ type instruction
        case UJ_TYPE: {
            uint32_t opcode = machineCode & 0x7F;
            uint32_t rd = (machineCode >> 7) & 0x1F;
            uint32_t imm_19_12 = (machineCode >> 12) & 0xFF;
            uint32_t imm_11 = (machineCode >> 20) & 0x1;
            uint32_t imm_10_1 = (machineCode >> 21) & 0x3FF;
            uint32_t imm_20 = (machineCode >> 31) & 0x1;
            uint32_t imm = (imm_20 << 20) | (imm_19_12 << 12) | (imm_11 << 11) | (imm_10_1 << 1);
            
            comment << bitset<7>(opcode) << "-"
                    << "NULL" << "-"
                    << "NULL" << "-"
                    << bitset<5>(rd) << "-"
                    << "NULL" << "-"
                    << "NULL" << "-"
                    << bitset<20>(imm>>1);
            break;
        }

        //default case
        default:
            comment <<"Unknown instruction type";
    }
   
    return comment.str();
}

//function to format output line
string formatOutputLine(uint32_t address, uint32_t machineCode, const Instruction& instr, const string& encodingComment) {

    string line="";

    //adding address, machine code, opcode 
    line += intToHex(address) + " " + intToHex(machineCode) + " , ";
    line += instr.opcode + " ";
    
    //adding operands
    for (size_t i=0; i<instr.operands.size(); i++) {
        line+=instr.operands[i];
        if (i<instr.operands.size() - 1) {
            line += ",";
        }
    }

    //adding encoding comment
    line += " # " + encodingComment;
    return line;
}
