#include "simulator.h"
#include <iostream>
#include <iomanip>

// Constructor
Simulator::Simulator() {
    pc=0;
    clock=0;
    cout<<"Simulator initialized!"<<endl;
}

//funnction to reset the simulator
void Simulator::reset() {
    regState.reset();
    memory.reset();
    pc = 0;
    clock = 0;
}

//function to load program from a file
bool Simulator::loadProgram(const string& filename) {
    return memory.loadFromFile(filename);
}

//function to run the simulation
void Simulator::run() {

    while (isRunning()) {
        step();
    }
    cout << "Simulation ended after " << clock << " cycles." << endl;
}

//function to run single instruction
void Simulator::step() {
    cout << "\n===== Cycle " << dec << clock << " =====" << endl;

    if (pipeliningEnabled) {
        fetchPipeline();
        decodePipeline();
        executePipeline();
        memoryAccessPipeline();
        writeBackPipeline();
        clock++;
    }
    else {
        fetch();
        if (!isRunning()) return;
        DecodedInstruction decodedInst = decode();
        execute(decodedInst);
        memoryAccess(decodedInst);
        writeBack(decodedInst);
        clock++;
    }
}

// function to check if the simulator is running
bool Simulator::isRunning() const {
    return regState.getIR()!=0xffffffff;
}

//function to get the current clock cycle
uint32_t Simulator::getClock() const {
    return clock;
}

//function to print all registers
void Simulator::printRegisters() const {
    regState.printAll();
}

//function to print memory
void Simulator::printMemory(uint32_t start_addr, uint32_t end_addr, char format) const{
    memory.printMemory(start_addr, end_addr, format);
}

// fetch function for pipelined version
// takes care of the stalls and calls the non-pipelined fetch function
void Simulator::fetchPipeline() {
    if (control.isFetchEmpty()) {
        fetch();
        return;
    }
    FetchControl fetchCtrl = control.getFetchControl();
    if (fetchCtrl.stall) {
        cout << "Fetch stage is stalled." << endl;
        return;
    }
    if (fetchCtrl.flush) {
        cout << "Fetch stage is flushed." << endl;
        return;
    }
    fetch();
}

// decode function for pipelined version
void Simulator::decodePipeline() {
    if (control.isDecodeEmpty()) {
        cout << "Decode stage: Control queue empty, skipping." << endl;
        return;
    }
    DecodeControl decodeCtrl = control.getDecodeControl();
    if (decodeCtrl.stall) {
        cout << "Decode stage is stalled." << endl;
        return;
    }

    DecodedInstruction decodedInst = decode(); // decodes the instruction using non-pipelined decode function
    if (decodedInst.opcode == 0xFFFFFFFF) {
        cout << "Decode stage: Termination instruction detected." << endl;
        return;
    }

    ExecuteControl exeCtrl;
    MemoryControl memCtrl;
    WriteBackControl wbCtrl;

    switch (decodedInst.opcode) {
        case 0x33: // R-type
            exeCtrl.aluSrc = false; // use rs2
            switch (decodedInst.funct3) {
                case 0x0: 
                    if (decodedInst.funct7 == 0x00) exeCtrl.aluOp = ADD;
                    else if (decodedInst.funct7 == 0x20) exeCtrl.aluOp = SUB;
                    else if (decodedInst.funct7 == 0x01) exeCtrl.aluOp = MUL;
                    else exeCtrl.aluOp = NONE; // unknown funct7
                    break;
                case 0x1: exeCtrl.aluOp = SLL; break; 
                case 0x2: exeCtrl.aluOp = SLT; break; 
                case 0x3: exeCtrl.aluOp = SLTU; break; 
                case 0x4: /
                    if (decodedInst.funct7 == 0x00) exeCtrl.aluOp = XOR;
                    else if (decodedInst.funct7 == 0x01) exeCtrl.aluOp = DIV; 
                    else exeCtrl.aluOp = NONE;
                    break;
                case 0x5: 
                    if (decodedInst.funct7 == 0x00) exeCtrl.aluOp = SRL;
                    else if (decodedInst.funct7 == 0x20) exeCtrl.aluOp = SRA;
                    else exeCtrl.aluOp = NONE;
                    break;
                case 0x6: 
                    if (decodedInst.funct7 == 0x00) exeCtrl.aluOp = OR;
                     else if (decodedInst.funct7 == 0x01) exeCtrl.aluOp = REM; 
                    else exeCtrl.aluOp = NONE;
                    break;
                case 0x7: exeCtrl.aluOp = AND; break; 
                default: exeCtrl.aluOp = NONE; break; // invalid funct3
            }
            wbCtrl.regWrite = (decodedInst.rd != 0);    // dont write to x0
            wbCtrl.memToReg = false;
            break;

        case 0x13: // I-type 
            exeCtrl.aluSrc = true; // use immediate 
            switch (decodedInst.funct3) {
                case 0x0: exeCtrl.aluOp = ADD; break;  // ADDI
                case 0x1: exeCtrl.aluOp = SLL; break;  // SLLI (uses 5-bit immediate) 
                case 0x2: exeCtrl.aluOp = SLT; break;  // SLTI
                case 0x3: exeCtrl.aluOp = SLTU; break; // SLTIU
                case 0x4: exeCtrl.aluOp = XOR; break;  // XORI
                case 0x5: // SRLI/SRAI (uses 5-bit immediate)
                    if (((decodedInst.imm >> 5) & 0x7F) == 0x00) exeCtrl.aluOp = SRL; // check upper bits of imm 
                    else if (((decodedInst.imm >> 5) & 0x7F) == 0x20) exeCtrl.aluOp = SRA; // check upper bits of imm
                    else exeCtrl.aluOp = NONE;
                    break;
                case 0x6: exeCtrl.aluOp = OR; break;   // ORI
                case 0x7: exeCtrl.aluOp = AND; break;  // ANDI
                default: exeCtrl.aluOp = NONE; break; // invalid funct3
            }
            wbCtrl.regWrite = (decodedInst.rd != 0);
            wbCtrl.memToReg = false;
            break;

        case 0x03: // I-type load
            exeCtrl.aluSrc = true; // use imm
            exeCtrl.aluOp = ADD; // address: RA + IMM
            memCtrl.memRead = true;
            memCtrl.memWrite = false;
            switch (decodedInst.funct3) {
                case 0x0: memCtrl.memWidth = 1; memCtrl.signExtend = true; break;  // LB
                case 0x1: memCtrl.memWidth = 2; memCtrl.signExtend = true; break;  // LH
                case 0x2: memCtrl.memWidth = 4; memCtrl.signExtend = false; break; // LW
                case 0x4: memCtrl.memWidth = 1; memCtrl.signExtend = false; break; // LBU
                case 0x5: memCtrl.memWidth = 2; memCtrl.signExtend = false; break; // LHU
                default: memCtrl.memRead = false; break; // invalid func3
            }
            wbCtrl.regWrite = (decodedInst.rd != 0) && memCtrl.memRead; // write only if valid load and rd!=x0
            wbCtrl.memToReg = memCtrl.memRead; // write from memory to register
            break;

        case 0x23: // S-type 
            exeCtrl.aluSrc = true; // use imm
            exeCtrl.aluOp = ADD; // address: RA + IMM
            memCtrl.memRead = false;
            memCtrl.memWrite = true;
            switch (decodedInst.funct3) {
                case 0x0: memCtrl.memWidth = 1; break; // SB
                case 0x1: memCtrl.memWidth = 2; break; // SH
                case 0x2: memCtrl.memWidth = 4; break; // SW
                default: memCtrl.memWrite = false; break; // invalid func3
            }
            wbCtrl.regWrite = false;
            wbCtrl.memToReg = false;
            break;

            case 0x63: // SB-type
            exeCtrl.aluSrc = false; // use rs2
            exeCtrl.branch = true; 
            exeCtrl.aluOp = AluOperation::SUB; // generally sub is used in ALU for branch comparison

            switch (decodedInst.funct3) {
                case 0x0: 
                    exeCtrl.branchType = BEQ;
                    break;
                case 0x1:
                    exeCtrl.branchType = BNE;
                    break;
                case 0x4:
                    exeCtrl.branchType = BLT;
                    break;
                case 0x5: 
                    exeCtrl.branchType = BGE;
                    break;
                case 0x6: 
                    exeCtrl.branchType = BLTU;
                    break;
                case 0x7: 
                    exeCtrl.branchType = BGEU;
                    break;
                default:
                    cout << "  Warning: Invalid funct3 (0x" << hex << decodedInst.funct3 << dec << ") for Branch opcode. Treating as invalid." << endl;
                    exeCtrl.branchType = INVALID;
                    exeCtrl.branch = false; // invalid branch
                    exeCtrl.aluOp = NONE; 
                    break;
            }
            memCtrl.memRead = false;
            memCtrl.memWrite = false;
            wbCtrl.regWrite = false;
            wbCtrl.memToReg = false;
            break;

        case 0x6F: // UJ-type 
            exeCtrl.aluOp = ADD; // target: PC + imm (execute needs PC)
            exeCtrl.jump = true;
            // writes PC+4 to rd
            wbCtrl.regWrite = (decodedInst.rd != 0);
            wbCtrl.memToReg = false; 
            break;

        case 0x67: // I-type JALR
            exeCtrl.aluSrc = true; // use imm
            exeCtrl.aluOp = ADD; // address: RA + IMM
            exeCtrl.jump = true;
            // writes PC+4 to rd
            wbCtrl.regWrite = (decodedInst.rd != 0);
            wbCtrl.memToReg = false; 
            break;

        case 0x37: // U-type LUI
            exeCtrl.aluSrc = true; // use imm
            exeCtrl.aluOp = LUI; 
            wbCtrl.regWrite = (decodedInst.rd != 0);
            wbCtrl.memToReg = false; 
            break;

        case 0x17: // U-type AUIPC
            exeCtrl.aluSrc = true; // use imm
            exeCtrl.aluOp = AUIPC; 
            wbCtrl.regWrite = (decodedInst.rd != 0);
            wbCtrl.memToR = false; 
            break;

        default:
            cout << "  Decode stage: unrecognized opcode." << endl;
            break;
    }


    control.addExecuteControl(exeCtrl);
    control.addMemoryControl(memCtrl);
    control.addWriteBackControl(wbCtrl);

    // cout << "  Generated Controls -> EX:{op:" << static_cast<int>(exeCtrl.aluOp) << ", aluSrc:" << exeCtrl.aluSrc << ", branch:" << exeCtrl.branch << ", jump:" << exeCtrl.jump << "}"
        //  << " MEM:{memRead:" << memCtrl.memRead << ", memWrite:" << memCtrl.memWrite << ", width:" << memCtrl.memWidth << ", signExt:" << memCtrl.signExtend << "}"
        //  << " WB:{regWrite:" << wbCtrl.regWrite << ", memToReg:" << wbCtrl.memToReg << ", rd:" << wbCtrl.rd << "}" << endl;


    // fetch control for next cycle
    control.addFetchControl();

    // add decode control for next cycle
    control.addDecodeControl();

    // Hazard Detection Logic 
}







//function to fetch instruction
void Simulator::fetch() {
    cout << "FETCH STAGE:\n";
    uint32_t PC = regState.getPC();
    if(PC%4!=0) {   
        cerr << "Error: Unaligned memory access at address 0x" << hex << PC << dec << endl;
        regState.setIR(0xffffffff);
    }
    uint32_t instruction = memory.readWord(PC);
    if (instruction == 0) {
        cout << "ERROR : Invalid instruction at PC 0x" << hex << regState.getPC() << endl;
        regState.setIR(0xFFFFFFFF);
        return;
    }
    regState.setIR(instruction);
    regState.setTemp("PC_TEMP", PC+4);
}

// function to perform decode stage
Simulator::DecodedInstruction Simulator::decode() {
    uint32_t instruction = regState.getIR();
    cout << "\nDECODE STAGE:\n";
    DecodedInstruction decodedInst;

    decodedInst.opcode = instruction & 0x7F;

    switch(decodedInst.opcode) {
        case(0x33): // r type
            decodedInst.rd     = (instruction >> 7)  & 0x1F;
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1    = (instruction >> 15) & 0x1F;
            decodedInst.rs2    = (instruction >> 20) & 0x1F;
            decodedInst.funct7 = (instruction >> 25) & 0x7F;
            regState.setTemp("RA", regState.getGen(decodedInst.rs1));
            regState.setTemp("RB", regState.getGen(decodedInst.rs2));
            break;
        case(0x13): // i type
            decodedInst.rd     = (instruction >> 7)  & 0x1F;
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1    = (instruction >> 15) & 0x1F;
            decodedInst.imm = (instruction >> 20) & 0xFFF;
            if (decodedInst.imm & 0x800) decodedInst.imm |= 0xFFFFF000;
            regState.setTemp("RA", regState.getGen(decodedInst.rs1));
            regState.setTemp("IMM", decodedInst.imm);
            break;
        case(0x03):  // i type load instructions
            decodedInst.rd     = (instruction >> 7)  & 0x1F;
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1    = (instruction >> 15) & 0x1F;
            decodedInst.imm = (instruction >> 20) & 0xFFF;
            if (decodedInst.imm & 0x800) decodedInst.imm |= 0xFFFFF000;
            regState.setTemp("RA", regState.getGen(decodedInst.rs1));
            regState.setTemp("IMM", decodedInst.imm);
            break;
        case(0x23): // s type
            decodedInst.imm = (instruction >> 7) & 0x1F;
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1    = (instruction >> 15) & 0x1F;
            decodedInst.rs2    = (instruction >> 20) & 0x1F;
            decodedInst.imm |= ((instruction >> 25) & 0x7F) << 5;
            if (decodedInst.imm & 0x800) decodedInst.imm |= 0xFFFFF000;     
            regState.setTemp("RA", regState.getGen(decodedInst.rs1));
            regState.setTemp("RB", regState.getGen(decodedInst.rs2));
            regState.setTemp("IMM", decodedInst.imm);
            regState.setTemp("RM", regState.getGen(decodedInst.rs2));
            break;
        case(0x63): // sb type
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1 = (instruction >> 15) & 0x1F;
            decodedInst.rs2 = (instruction >> 20) & 0x1F;
            decodedInst.imm = ((instruction >> 31) & 0x1) << 12 |
                             ((instruction >> 25) & 0x3F) << 5 |
                             ((instruction >> 8) & 0xF) << 1 |
                             ((instruction >> 7) & 0x1) << 11;
            if (decodedInst.imm & 0x1000) decodedInst.imm |= 0xFFFFE000;
            regState.setTemp("RA", regState.getGen(decodedInst.rs1));
            regState.setTemp("RB", regState.getGen(decodedInst.rs2));
            regState.setTemp("IMM", decodedInst.imm);
            break;
        case(0x6F): // uj type
            decodedInst.rd     = (instruction >> 7) & 0x1F;
            decodedInst.imm    = ((instruction >> 12) & 0xFF) << 12;  
            decodedInst.imm   |= ((instruction >> 20) & 0x1) << 11;   
            decodedInst.imm   |= ((instruction >> 21) & 0x3FF) << 1; 
            decodedInst.imm   |= ((instruction >> 31) & 0x1) << 20; 
            if (decodedInst.imm & 0x100000) decodedInst.imm |= 0xFFF00000;
            regState.setTemp("IMM", decodedInst.imm);
            break;
        case(0x67):  // jalr
            decodedInst.rd     = (instruction >> 7) & 0x1F;
            decodedInst.funct3 = (instruction >> 12) & 0x07;
            decodedInst.rs1    = (instruction >> 15) & 0x1F;
            decodedInst.imm    = (instruction >> 20) & 0xFFF;
            if (decodedInst.imm & 0x800) decodedInst.imm |= 0xFFFFF000;
            regState.setTemp("RA", regState.getGen(decodedInst.rs1));
            regState.setTemp("IMM", decodedInst.imm);
            break;
        case(0x37):  // lui
            decodedInst.rd     = (instruction >> 7) & 0x1F;
            decodedInst.imm    = (instruction & 0xFFFFF000);
            regState.setTemp("IMM", decodedInst.imm);
            break;
        case(0x17):  // auipc
            decodedInst.rd     = (instruction >> 7) & 0x1F;
            decodedInst.imm    = (instruction & 0xFFFFF000);
            regState.setTemp("IMM", decodedInst.imm);
            break;
        default:     // Unknown instruction
            cerr << "Unknown instruction with opcode 0x" << hex << decodedInst.opcode << dec << endl;
            regState.setIR(0xFFFFFFFF);
            break;
    }

    return decodedInst;
}

// function to perform execute stage
void Simulator::execute(DecodedInstruction& decodedInst) 
 {
    cout << "\nEXECUTE:\n";
    
    uint32_t opcode = decodedInst.opcode;
    uint32_t funct3 = decodedInst.funct3;
    uint32_t funct7 = decodedInst.funct7;
    
    int32_t rs1_val = regState.getTemp("RA");
    int32_t rs2_val = regState.getTemp("RB");
    int32_t imm_val = regState.getTemp("IMM");

    int32_t rz_val = 0;
    uint32_t next_pc = regState.getTemp("PC_TEMP");
    
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
                    regState.setIR(0xFFFFFFFF);
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
                    regState.setIR(0xFFFFFFFF);
                    break;    
            }
            break;
        }

        case 0x03: { // LB, LH, LW, LD
            rz_val = rs1_val + imm_val;
            //regState.setTemp("MAR", rz_val);
            break;
            
        }
        
        case 0x67: { // JALR
            rz_val = regState.getTemp("PC_TEMP");
            next_pc = (rs1_val + imm_val) & ~1; 
            break;
        }
        
        
        case 0x23: { // SB, SH, SW, SD
            rz_val = rs1_val + imm_val;
            //regState.setTemp("MAR", rz_val);
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
                next_pc = regState.getPC() + imm_val;
            }
            break;
        }
        
        case 0x37: { // LUI
            rz_val = imm_val;
            break;
           
        }
        
        case 0x17: { // AUIPC
            rz_val = regState.getPC() + imm_val;
            break;
        }
        
        case 0x6F: { // JAL
            rz_val = regState.getTemp("PC_TEMP"); 
            next_pc = regState.getPC() + imm_val; 
            break;
        }
        
        default:
            cout << "Unknown opcode: 0x" << hex << opcode << dec << endl;
            regState.setIR(0xFFFFFFFF);
            break;
    }
    
    regState.setTemp("RZ", rz_val);
    regState.setPC(next_pc);
}

// function to perform memory access stage
void Simulator::memoryAccess(DecodedInstruction& decodedInst) {
    cout << "\nMEMORY ACCESS STAGE:" << endl;
    
    uint32_t opcode = decodedInst.opcode;
    uint32_t funct3 = decodedInst.funct3;
    
    switch (opcode) {
        case 0x03: 
        {
            regState.setTemp("MDR", regState.getTemp("RM"));
            regState.setTemp("MAR", regState.getTemp("RZ"));
            uint32_t address = regState.getTemp("MAR");
            switch (funct3) {
                case 0x0: // LB
                    regState.setTemp("MDR",static_cast<int32_t>(static_cast<int8_t>(memory.readByte(address))));
                    break;
                case 0x1: // LH
                    regState.setTemp("MDR",static_cast<int32_t>(static_cast<int16_t>(memory.readHalf(address))));
                    break;
                case 0x2: // LW
                    regState.setTemp("MDR",static_cast<int32_t>(memory.readWord(address)));
                    break;
                case 0x3: // LD
                    cerr << "Error: 64-bit load not supported" << endl;
                    regState.setIR(0xFFFFFFFF);
                    break;
                default:
                    cerr << "Unknown load operation (funct3: 0x" << hex << funct3 << ")" << dec << endl;
                    regState.setIR(0xFFFFFFFF);
                    break;
            }
            regState.setTemp("RY", regState.getTemp("MDR"));
            break;
        }
        case 0x23: 
        {
            regState.setTemp("MDR", regState.getTemp("RM"));
            regState.setTemp("MAR", regState.getTemp("RZ"));
            uint32_t address = regState.getTemp("MAR");
            uint32_t data = regState.getTemp("MDR");
            switch (funct3) {
                case 0x0: // SB
                    memory.writeByte(address, data & 0xFF);
                    break;
                case 0x1: // SH
                    memory.writeHalf(address, data & 0xFFFF);
                    break;
                case 0x2: // SW
                    memory.writeWord(address, data);
                    break;
                case 0x3: // SD
                    cerr << "Error: 64-bit store not supported" << endl;
                    regState.setIR(0xFFFFFFFF);
                    break;
                default:
                    cerr << "Unknown store operation (funct3: 0x" << hex << funct3 << ")" << dec << endl;
                    regState.setIR(0xFFFFFFFF);
                    break;
            }
            break;
        }
        default:
            regState.setTemp("RY", regState.getTemp("RZ"));
            break;
    }
    
}

// function to perform write back stage
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
            regState.setIR(0xFFFFFFFF);
            break;
    }
    
    if (writeToReg) {
        regState.setGen(rd, result);
    } else {
        cout << "No register write back needed for this instruction" << endl;
    }
}
