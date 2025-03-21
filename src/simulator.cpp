#include "simulator.h"
#include <iostream>
#include <iomanip>

Simulator::Simulator() {
    pc=0;
    clock=0;
    cout<<"Simulator initialized!"<<endl;
}

void Simulator::reset() {
    regState.reset();
    memory.reset();
    pc = 0;
    clock = 0;
}

bool Simulator::loadProgram(const string& filename) {
    return memory.loadFromFile(filename);
}

void Simulator::run() {
    while (isRunning()) {
        step();
    }
    cout << "Simulation completed after " << clock << " cycles." << endl;
}

void Simulator::step() {
    cout << "\n===== Cycle " << clock << " =====" << endl;
    
    fetch();
    uint32_t instruction = regState.getIR();
    DecodedInstruction decodedInst = decode();
    execute(decodedInst);
    memoryAccess(decodedInst);
    writeBack(decodedInst);
    
    clock++;
}

bool Simulator::isRunning() const {
    return pc!=0xffffffff;
}

uint32_t Simulator::getClock() const {
    return clock;
}

void Simulator::printRegisters() const {
    regState.printAll();
}

void Simulator::printMemory(uint32_t start_addr, uint32_t end_addr) const {
    if (start_addr > end_addr || start_addr<0 || end_addr>0xFFFFFFFF) {
        cout << "Invalid memory range!" << endl;
        return;
    }
    memory.printMemory(start_addr, end_addr);
}

// read the instruction stored at the pc and 
// store it in IR (does not return anything)
void Simulator::fetch() {
    cout << "FETCH STAGE: Fetching the instruction stored at current PC 0x" << hex << regState.getPC() << dec << endl;
    uint32_t pc = regState.getPC();
    uint32_t instruction = memory.readWord(pc);
    regState.setIR(instruction);
    regState.setTemp("PC_TEMP", pc+4);
}

// decodes the instruction stored in the IR
// returns a DecodedInstruction
Simulator::DecodedInstruction Simulator::decode() {
    uint32_t instruction = regState.getIR();
    cout << "DECODE STAGE : Decoding the instruction " << hex << "0x" << instruction << dec << endl;
    DecodedInstruction decodedInst;

    decodedInst.opcode = instruction & 0x7F;

    switch(decodedInst.opcode) {
        case(0x33): // r type
            decodedInst.rd     = (instruction >> 7)  & 0x1F;
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1    = (instruction >> 15) & 0x1F;
            decodedInst.rs2    = (instruction >> 20) & 0x1F;
            decodedInst.funct7 = (instruction >> 25) & 0x7F;
            break;
        case(0x13): // i type
            decodedInst.rd     = (instruction >> 7)  & 0x1F;
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1    = (instruction >> 15) & 0x1F;
            decodedInst.imm = (instruction >> 20) & 0xFFF;
            if (decodedInst.imm & 0x800)
                decodedInst.imm |= 0xFFFFF000;
            break;
        case(0x03):  // i type load instructions
            decodedInst.rd     = (instruction >> 7)  & 0x1F;
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1    = (instruction >> 15) & 0x1F;
            decodedInst.imm = (instruction >> 20) & 0xFFF;
            if (decodedInst.imm & 0x800)
                decodedInst.imm |= 0xFFFFF000;
            break;
        case(0x23): // s type
            decodedInst.imm = (instruction >> 7) & 0x1F;
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1    = (instruction >> 15) & 0x1F;
            decodedInst.rs2    = (instruction >> 20) & 0x1F;
            decodedInst.imm |= ((instruction >> 25) & 0x7F) << 5;
            if (decodedInst.imm & 0x800)
                decodedInst.imm |= 0xFFFFF000;            
            break;
        case(0x63): // sb type
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1 = (instruction >> 15) & 0x1F;
            decodedInst.rs2 = (instruction >> 20) & 0x1F;
            decodedInst.imm = ((instruction >> 31) & 0x1) << 12 |
                             ((instruction >> 25) & 0x3F) << 5 |
                             ((instruction >> 8) & 0xF) << 1 |
                             ((instruction >> 7) & 0x1) << 11;
            if (decodedInst.imm & 0x1000)
                decodedInst.imm |= 0xFFFFE000;
            break;
        case(0x6F): // uj type
            decodedInst.rd     = (instruction >> 7) & 0x1F;
            decodedInst.imm    = ((instruction >> 12) & 0xFF) << 12;  
            decodedInst.imm   |= ((instruction >> 20) & 0x1) << 11;   
            decodedInst.imm   |= ((instruction >> 21) & 0x3FF) << 1; 
            decodedInst.imm   |= ((instruction >> 31) & 0x1) << 20; 
            if (decodedInst.imm & 0x100000)
                decodedInst.imm |= 0xFFF00000;
            break;
        case(0x67):  // jalr
            decodedInst.rd     = (instruction >> 7) & 0x1F;
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1    = (instruction >> 15) & 0x1F;
            decodedInst.imm    = (instruction >> 20) & 0xFFF;
            if (decodedInst.imm & 0x800)
                decodedInst.imm |= 0xFFFFF000;
            break;
        case(0x37):  // lui
            decodedInst.rd     = (instruction >> 7) & 0x1F;
            decodedInst.imm    = (instruction & 0xFFFFF000);
            break;
        case(0x17):  // auipc
            decodedInst.rd     = (instruction >> 7) & 0x1F;
            decodedInst.imm    = (instruction & 0xFFFFF000);
            break;
        default:     // Unknown instruction
            cout << "Unknown instruction with opcode 0x" << hex << decodedInst.opcode << dec << endl;
            break;
    }
    regState.setTemp("RA", getGen(decodedInst.rs1));
    regState.setTemp("RB", getGen(decodedInst.rs2));
    regState.setTemp("IMM", decodedInst.imm);
    regState.setTemp("RM",getGen(decodedInst.rs2));

    return decodedInst;
}

void Simulator::execute(DecodedInstruction& decodedInst) 
 {
    cout << "EXECUTE: Performing operation for instruction with opcode 0x" << hex << decodedInst.opcode << dec << endl;
    
    uint32_t opcode = decodedInst.opcode;
    uint32_t funct3 = decodedInst.funct3;
    uint32_t funct7 = decodedInst.funct7;
    
    int32_t rs1_val = regState.getTemp("RA");
    int32_t rs2_val = regState.getTemp("RB");
    int32_t imm_val = regState.getTemp("IMM");

    int32_t rz_val = 0;
    uint32_t next_pc = pc + 4;
    
    switch (opcode) 
    {
        // R-type instructions
        case 0x33: 
        {
            switch (funct3) 
            {
                case 0x0: 
                    if (funct7 == 0x00) // ADD
                        rz_val = rs1_val + rs2_val;
                    else if (funct7 == 0x20) // SUB
                        rz_val = rs1_val - rs2_val;
                    else if (funct7 == 0x01) // MUL
                        rz_val = rs1_val * rs2_val;
                    break;

                case 0x1: // SLL
                    rz_val = rs1_val << (rs2_val & 0x1F);
                    break;

                case 0x2: // SLT
                    rz_val = (rs1_val < rs2_val) ? 1 : 0;
                    break;

                case 0x4: 
                    if (funct7 == 0x00) // XOR
                        rz_val = rs1_val ^ rs2_val;
                    else if (funct7 == 0x01) // DIV
                        rz_val = (rs2_val != 0) ? (rs1_val / rs2_val) : -1;
                    break;

                case 0x5: 
                    if (funct7 == 0x00) // SRL
                        rz_val = static_cast<uint32_t>(rs1_val) >> (rs2_val & 0x1F);
                    else if (funct7 == 0x20) // SRA
                        rz_val = rs1_val >> (rs2_val & 0x1F);
                    break;
                    
                case 0x6: 
                    if (funct7 == 0x00) // OR
                        rz_val = rs1_val | rs2_val;
                    else if (funct7 == 0x01) // REM
                        rz_val = (rs2_val != 0) ? (rs1_val % rs2_val) : rs1_val;
                    break;
                        
                case 0x7: // AND
                    rz_val = rs1_val & rs2_val;
                    break;

                default:
                    cout << "Unknown funct3: 0x" << hex << funct3 << dec << endl;
                    break;    
                    
            }
            break;
        }
        
        // I-type instructions
        case 0x13: {
            
            switch (funct3) 
            {
                case 0x0: // ADDI
                    rz_val = rs1_val + imm_val;
                    break;
                    
                case 0x7: // ANDI
                    rz_val = rs1_val & imm_val;
                    break;
                case 0x6: // ORI
                    rz_val = rs1_val | imm_val;
                    break;
                default:
                    cout << "Unknown funct3: 0x" << hex << funct3 << dec << endl;
                    break;    
            }
            break;
        }

        case 0x03: { // LB, LH, LW, LD
            rz_val = rs1_val + imm_val;
            regState.setTemp("MAR", rz_val);
            break;
            
        }
        
        case 0x67: { // JALR
            rz_val = pc + 4;
            next_pc = (rs1_val + imm_val) & ~1; 
            break;
        }
        
        
        case 0x23: { // SB, SH, SW, SD
            rz_val = rs1_val + imm_val;
            regState.setTemp("MAR", rz_val);
            break;
        }
        
        case 0x63: { 
            bool take_branch = false;
            
            switch (funct3) {
                case 0x0: // BEQ
                    take_branch = (rs1_val == rs2_val);
                    break;
                case 0x1: // BNE
                    take_branch = (rs1_val != rs2_val);
                    break;
                case 0x4: // BLT
                    take_branch = (rs1_val < rs2_val);
                    break;
                case 0x5: // BGE
                    take_branch = (rs1_val >= rs2_val);
                    break;
            }
            
            if (take_branch) 
            {
                next_pc = pc + imm_val;
            }
            break;
        }
        
        case 0x37: { // LUI
            rz_val = imm_val<<12;
            break;
           
        }
        
        case 0x17: { // AUIPC
            imm_val = imm_val << 12;
            rz_val = pc + imm_val;
            break;
        }
        
        case 0x6F: { // JAL
            rz_val = pc + 4; 
            next_pc = pc + imm_val; 
            break;
        }
        
        default:
            cout << "Unknown opcode: 0x" << hex << opcode << dec << endl;
            break;
    }
    
    regState.setTemp("RZ", rz_val);
    regState.setPC(next_pc);
     
}

void Simulator::memoryAccess(DecodedInstruction& decodedInst) {
    cout << "\nMEMORY ACCESS STAGE:" << endl;
    
    uint32_t opcode = decodedInst.opcode;
    uint32_t funct3 = decodedInst.funct3;
    
    uint32_t address = regState.getTemp("MAR");
    
    switch (opcode) {
        case 0x03: {
            
            switch (funct3) {
                case 0x0: 
                    regState.setTemp("MDR",static_cast<int32_t>(static_cast<int8_t>(memory.readByte(address))));
                    regState.setTemp("RY",static_cast<int32_t>(static_cast<int8_t>(memory.readByte(address))));
                    break;
                    
                case 0x1: 
                    regState.setTemp("MDR",static_cast<int32_t>(static_cast<int16_t>(memory.readHalf(address))));
                    regState.setTemp("RY",static_cast<int32_t>(static_cast<int16_t>(memory.readHalf(address))));
                    break;
                    
                case 0x2: 
                    regState.setTemp("MDR",static_cast<int32_t>(memory.readWord(address)));
                    regState.setTemp("RY",static_cast<int32_t>(memory.readWord(address)));
                    break;

                case 0x3:
                    cerr << "Error: 64-bit load not supported" << endl;
                    break;

                default:
                    cout << "Unknown load operation (funct3: 0x" << hex << funct3 << dec << ")" << endl;
                    break;
            }
            break;
        }
        
        case 0x23: {
            uint32_t data = regState.getTemp("RB");
            
            switch (funct3) {
                case 0x0: 
                    memory.writeByte(address, data & 0xFF);
                    break;
                    
                case 0x1: 
                    memory.writeHalf(address, data & 0xFFFF);
                    break;
                    
                case 0x2: 
                    memory.writeWord(address, data);
                    break;

                case 0x3:
                    cerr << "Error: 64-bit store not supported" << endl;
                    break;
                    
                default:
                    cout << "Unknown store operation with funct3: 0x" << hex << funct3 << dec << endl;
                    break;
            }
            break;
        }
        
        default:
            regState.setTemp("RY", regState.getTemp("RZ"));
            break;
    }
    
    pc = regState.getTemp("PC");
}


void Simulator::writeBack(DecodedInstruction& decodedInst) {
    cout << "\nWRITE BACK STAGE: " << endl;
    
    uint32_t opcode = decodedInst.opcode;
    uint32_t rd = decodedInst.rd;
    
    if (rd == 0) {
        cout << "Destination register is x0, skipping write back" << endl;
        return;
    }
    
    int32_t result = regState.getTemp("RY");
    
    bool writeToReg = false;
    
    switch (opcode) {
        case 0x33: // ADD, SUB, AND, OR, XOR, SLL, SLT, SRA, SRL, MUL, DIV, REM
            writeToReg = true;
            break;
    
        case 0x13: // ADDI, ANDI, ORI
            writeToReg = true;
            break;
            
        case 0x03: // LB, LH, LW, LD
            writeToReg = true;
            break;
            
        case 0x67: // JALR
            writeToReg = true;
            break;
            
        case 0x37: // LUI
            writeToReg = true;
            break;
            
        case 0x17: // AUIPC
            writeToReg = true;
            break;
            
        case 0x6F: // JAL
            writeToReg = true;
            break;
            
        case 0x23: // SB, SH, SW, SD
        case 0x63: // BEQ, BNE, BLT, BGE
            writeToReg = false;
            break;
            
        default:
            writeToReg = false;
            cout << "Unknown opcode for write back stage: 0x" << hex << opcode << dec << endl;
            break;
    }
    
    if (writeToReg) {
        regState.setGen(rd, result);
        // cout << "Wrote value 0x" << hex << result << dec << " to register x" << rd << endl;
    } else {
        cout << "No register write back needed for this instruction" << endl;
    }
}
