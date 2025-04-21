#include "simulator.h"
#include <iostream>
#include <iomanip>

// Constructor
Simulator::Simulator() {
    pc = 0;
    clock = 1;
    
    // Initialize pipeline control knobs
    enable_pipelining = true;
    enable_data_forwarding = true;
    print_registers_each_cycle = false;
    print_pipeline_registers = true;
    trace_instruction = false;
    trace_instruction_num = 0;
    print_branch_prediction = true;
    branch_prediction_enabled = true;

    // Initialize pipeline buffers
    if_id.valid = false;
    id_ex.valid = false;
    ex_mem.valid = false;
    mem_wb.valid = false;
    
    if_id.terminate = false;
    id_ex.terminate = false;
    ex_mem.terminate = false;
    mem_wb.terminate = false;
        
    cout << "Simulator initialized!" << endl;
}

//funnction to reset the simulator
void Simulator::reset() {
    regState.reset();
    memory.reset();
    pc = 0;
    clock = 1;
        
    // Reset branch predictor
    branch_predictor.reset();
    
    // Reset pipeline buffers
    if_id.valid = false;
    id_ex.valid = false;
    ex_mem.valid = false;
    mem_wb.valid = false;
    
    if_id.terminate = false;
    id_ex.terminate = false;
    ex_mem.terminate = false;
    mem_wb.terminate = false;
    
    cout << "Simulator reset!" << endl;
}

//function to load program from a file
bool Simulator::loadProgram(const string& filename) {
    return memory.loadFromFile(filename);
}

//function to run the simulation
void Simulator::run() {
    while (true) {
        // Execute one step
        step();
        
        // Check termination conditions
        if (enable_pipelining) {
            // For pipelined mode, check if we've hit termination and drained the pipeline
            if (mem_wb.terminate) {
                cout << "Simulation ended after " << clock -1 << " cycles." << endl;
                break;
            }
        } else {
            // For non-pipelined mode
            if (regState.getIR() == 0xffffffff) {
                cout << "Simulation ended after " << clock -1 << " cycles." << endl;
                break;
            }
        }
    }
}

//function to run single instruction
void Simulator::step() {
    cout<<"=============== Cycle "<<clock<<" ==============="<<endl;
    // Check if already terminated
    if (enable_pipelining && mem_wb.terminate) {
        cout << "Program execution has already completed. No more instructions to execute." << endl;
        return;
    } else if (!enable_pipelining && regState.getIR() == 0xffffffff) {
        cout << "Program execution has already completed. No more instructions to execute." << endl;
        return;
    }

    if (enable_pipelining) {
        // Execute pipeline stages in reverse order
        if (!enable_data_forwarding) detectHazards();

        cout<<endl<<"stageWriteBack"<<endl;
        stageWriteBack();
        cout<<endl<<"stageMemory"<<endl;
        stageMemory();
        cout<<endl<<"stageExecute"<<endl;
        stageExecute();
        cout<<endl<<"stageDecode"<<endl;
        stageDecode();
        cout<<endl<<"stageFetch"<<endl;
        if (!if_id.terminate) stageFetch();
        
        // Detect hazards for next cycle
        if (enable_data_forwarding) forwardData();

        // Update statistics
        clock++;
        
        // Print debug information if enabled
        if (print_registers_each_cycle) {
            cout<<"\n---------- Registers ----------\n";
            printRegisters();
        }
        
        if (print_pipeline_registers) printPipelineRegisters();

        if(print_branch_prediction) branch_predictor.print_state();

    } else {
        // Non-pipelined implementation
        fetch();
        DecodedInstruction decodedInst = decode();
        execute(decodedInst);
        memoryAccess(decodedInst);
        writeBack(decodedInst);
        clock+=5;

        // Print debug information if enabled
        if (print_registers_each_cycle) {
            cout<<"\n---------- Registers ----------\n";
            printRegisters();
        }
    }
}

// function to check if the simulator is running
bool Simulator::isRunning() const {
    // Check if we've reached a termination condition
    if (mem_wb.terminate) {
        return false;
    }
    
    // Check if there are still valid instructions in the pipeline
    if (enable_pipelining) {
        return if_id.valid || id_ex.valid || ex_mem.valid || mem_wb.valid;
    }
    
    // For non-pipelined mode
    return regState.getIR() != 0xffffffff;
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

void Simulator::stageFetch() {
    // Skip fetch if stalled
    if (stall_if) {
        return;
    }
    
    uint32_t current_pc = regState.getPC();
    uint32_t instruction = memory.readWord(current_pc);
    
    // Check for termination condition (-1 or 0xffffffff)
    if (instruction == 0xffffffff && !flush_if_id) {
        if_id.pc = current_pc;
        if_id.terminate = true;
        if_id.valid = true;
        if_id.instruction = instruction;
        regState.setIR(instruction);
        return;
    }
    
    regState.setIR(instruction);
    
    if (!flush_if_id) {
        if_id.pc = current_pc;
        if_id.instruction = instruction;
        if_id.valid = true;
        if_id.terminate = false;  // Reset terminate flag for non-terminate instructions
        
        // Add branch prediction here
        if (branch_prediction_enabled) {
            // Check if this is a branch instruction (opcode 0x63) or jump (0x6F, 0x67)
            uint32_t opcode = instruction & 0x7F;
            if (opcode == 0x63 || opcode == 0x6F || opcode == 0x67) {
                bool predicted_taken = branch_predictor.predict(current_pc);
                if (predicted_taken) {
                    // If predicted taken, get target from BTB
                    uint32_t predicted_target = branch_predictor.get_target(current_pc);
                    regState.setPC(predicted_target);
                } else {
                    // If predicted not taken, increment PC normally
                    regState.setPC(current_pc + 4);
                }
            } else {
                // For non-branch instructions, increment PC normally
                regState.setPC(current_pc + 4);
            }
        } else {
            // If branch prediction disabled, increment PC normally
            regState.setPC(current_pc + 4);
        }
        
        if (trace_instruction && instructions_executed == trace_instruction_num) {
            cout << "Cycle " << clock << ": Instruction " << trace_instruction_num 
                 << " fetched: 0x" << hex << instruction << dec << endl;
        }
    } else {
        // During flush, invalidate everything in IF/ID
        if_id.valid = false;
        if_id.terminate = false;
        if_id.instruction = 0x00000013;  // NOP instruction
        flush_if_id = false;
        
        // For statistics
        control_hazards++;
    }
}

void Simulator::stageDecode() {

    
    // Skip decode if stalled
    if (stall_id) {
        // Insert NOP in ID/EX buffer during stall
        id_ex.valid = false;
        id_ex.reg_write = false;
        id_ex.mem_read = false;
        id_ex.mem_write = false;
        id_ex.branch = false;
        id_ex.jump = false;
        id_ex.alu_op = NONE;
        id_ex.branch_cond = BranchCondition::INVALID;
        return;
    }

    if (clock == 1) {
        id_ex.valid = false;
        return;
    }

    
    // If flush is active, insert NOP in ID/EX buffer
    if (flush_id_ex) {
        id_ex.valid = false;
        id_ex.reg_write = false;
        id_ex.mem_read = false;
        id_ex.mem_write = false;
        id_ex.branch = false;
        id_ex.jump = false;
        id_ex.alu_op = NONE;
        id_ex.branch_cond = BranchCondition::INVALID;
        flush_id_ex = false;
        return;
    }
    
    // Skip if IF/ID buffer doesn't contain a valid instruction
    if (!if_id.valid) {
        id_ex.valid = false;
        return;
    }
    
    // Call the existing decode function
    DecodedInstruction decodedInst = decode();

    if (if_id.terminate){
        id_ex.terminate = true;
    }
    
    
    // Map the decoded instruction to the ID/EX buffer
    id_ex.pc = if_id.pc;
    id_ex.rd = decodedInst.rd;
    id_ex.rs1 = decodedInst.rs1;
    id_ex.rs2 = decodedInst.rs2;
    id_ex.valid = true;
    
    // Set control signals based on the opcode
    uint32_t opcode = decodedInst.opcode;
    uint32_t funct3 = decodedInst.funct3;
    uint32_t funct7 = decodedInst.funct7;
    
    // Default control signals
    id_ex.reg_write = false;
    id_ex.mem_read = false;
    id_ex.mem_write = false;
    id_ex.branch = false;
    id_ex.jump = false;
    id_ex.alu_op = NONE;
    id_ex.branch_cond = BranchCondition::INVALID;
    id_ex.mem_width = 0;
    id_ex.ALU_src = false;  // Default to using register
    
    // Set control signals based on opcode
    switch (opcode) {
        case 0x33: // R-type
            id_ex.reg_write = true;
            id_ex.ALU_src = false;
            
            // Map funct3 and funct7 to AluOperation
            switch (funct3) {
                case 0x0:  // ADD/SUB/MUL
                    if (funct7 == 0x00) id_ex.alu_op = ADD;
                    else if (funct7 == 0x20) id_ex.alu_op = SUB;
                    else if (funct7 == 0x01) id_ex.alu_op = MUL;
                    break;
                case 0x1:  // SLL
                    id_ex.alu_op = SLL;
                    break;
                case 0x2:  // SLT
                    id_ex.alu_op = SLT;
                    break;
                case 0x4:  // XOR/DIV
                    if (funct7 == 0x00) id_ex.alu_op = XOR;
                    else if (funct7 == 0x01) id_ex.alu_op = DIV;
                    break;
                case 0x5:  // SRL/SRA
                    if (funct7 == 0x00) id_ex.alu_op = SRL;
                    else if (funct7 == 0x20) id_ex.alu_op = SRA;
                    break;
                case 0x6:  // OR/REM
                    if (funct7 == 0x00) id_ex.alu_op = OR;
                    else if (funct7 == 0x01) id_ex.alu_op = REM;
                    break;
                case 0x7:  // AND
                    id_ex.alu_op = AND;
                    break;
            }
            break;
            
        case 0x13: // I-type ALU
            id_ex.reg_write = true;
            id_ex.ALU_src = true;  // Use immediate
            
            switch (funct3) {
                case 0x0:  // ADDI
                    id_ex.alu_op = ADD;
                    break;
                case 0x7:  // ANDI
                    id_ex.alu_op = AND;
                    break;
                case 0x6:  // ORI
                    id_ex.alu_op = OR;
                    break;
            }
            break;
            
        case 0x03: // Load instructions (I-type)
            id_ex.reg_write = true;
            id_ex.mem_read = true;
            id_ex.ALU_src = true;  // Use immediate for address calculation
            id_ex.alu_op = ADD;    // Address calculation
            id_ex.mem_width = funct3;
            break;
            
        case 0x23: // Store instructions (S-type)
            id_ex.mem_write = true;
            id_ex.ALU_src = true;  // Use immediate for address calculation
            id_ex.alu_op = ADD;    // Address calculation
            id_ex.mem_width = funct3;
            break;
            
        case 0x63: // Branch instructions (SB-type)
            id_ex.branch = true;
            id_ex.ALU_src = false;
            id_ex.alu_op = SUB;    // For comparison
            
            switch (funct3) {
                case 0x0: id_ex.branch_cond = BranchCondition::BEQ; break;
                case 0x1: id_ex.branch_cond = BranchCondition::BNE; break;
                case 0x4: id_ex.branch_cond = BranchCondition::BLT; break;
                case 0x5: id_ex.branch_cond = BranchCondition::BGE; break;
                default: id_ex.branch_cond = BranchCondition::INVALID; break;
            }
            break;
            
        case 0x37: // LUI (U-type)
            id_ex.reg_write = true;
            id_ex.ALU_src = true;  // Use immediate
            id_ex.alu_op = LUI;
            break;
            
        case 0x17: // AUIPC (U-type)
            id_ex.reg_write = true;
            id_ex.ALU_src = true;  // Use immediate
            id_ex.alu_op = AUIPC;
            break;
            
        case 0x6F: // JAL (UJ-type)
            id_ex.reg_write = true;
            id_ex.jump = true;
            id_ex.ALU_src = true;  // Use immediate for target calculation
            id_ex.alu_op = ADD;    // PC + imm
            id_ex.is_jal= true;
            break;
            
        case 0x67: // JALR (I-type)
            id_ex.reg_write = true;
            id_ex.jump = true;
            id_ex.ALU_src = true;  // Use immediate
            id_ex.alu_op = ADD;    // rs1 + imm
            id_ex.is_jal = false;
            break;
    }
    
    // Update statistics
    if (id_ex.valid) {
        if (id_ex.mem_read || id_ex.mem_write) {
            data_transfer_instructions++;
        } else if (id_ex.branch || id_ex.jump) {
            control_instructions++;
        } else {
            alu_instructions++;
        }
    }
}

void Simulator::stageExecute() {
    // Skip if ID/EX buffer doesn't contain a valid instruction
    if (!id_ex.valid) {
        ex_mem.valid = false;
        return;
    }

    if (id_ex.terminate){
        ex_mem.terminate = true;
    }
    
    // Initialize EX/MEM buffer with values from ID/EX
    ex_mem.pc = id_ex.pc;
    ex_mem.rd = id_ex.rd;
    ex_mem.reg_write = id_ex.reg_write;
    ex_mem.mem_read = id_ex.mem_read;
    ex_mem.mem_write = id_ex.mem_write;
    ex_mem.mem_width = id_ex.mem_width;
    ex_mem.valid = true;
    ex_mem.branch_taken = false;
    
    // For debugging/tracing
    if (trace_instruction && instructions_executed == trace_instruction_num) {
        cout << "Cycle " << clock << ": Instruction " << trace_instruction_num 
             << " in execute stage" << endl;
    }

    // Get operand values
    uint32_t operand1 = regState.getTemp("RA");
    uint32_t operand2;
    
    // Select second operand based on ALU_src
    if (id_ex.ALU_src) {
        operand2 = regState.getTemp("IMM");
    } else {
        operand2 = regState.getTemp("RB");
    }
    
    // Store rs2_val for potential store instruction
    ex_mem.rs2_val = regState.getTemp("RB");
    
    // Perform ALU operation based on alu_op
    uint32_t alu_result = 0;
    
    switch (id_ex.alu_op) {
        case ADD:
            alu_result = operand1 + operand2;
            break;
            
        case SUB:
            alu_result = operand1 - operand2;
            break;
            
        case MUL:
            alu_result = operand1 * operand2;
            break;
            
        case SLL:
            alu_result = operand1 << (operand2 & 0x1F);
            break;
            
        case SLT:
            alu_result = (static_cast<int32_t>(operand1) < static_cast<int32_t>(operand2)) ? 1 : 0;
            break;
            
        case XOR:
            alu_result = operand1 ^ operand2;
            break;
            
        case DIV:
            if (operand2 != 0) {
                alu_result = static_cast<int32_t>(operand1) / static_cast<int32_t>(operand2);
            } else {
                alu_result = 0xFFFFFFFF; // Division by zero
                cout << "Warning: Division by zero attempted" << endl;
            }
            break;
            
        case SRL:
            alu_result = static_cast<uint32_t>(operand1) >> (operand2 & 0x1F);
            break;
            
        case SRA:
            alu_result = static_cast<int32_t>(operand1) >> (operand2 & 0x1F);
            break;
            
        case OR:
            alu_result = operand1 | operand2;
            break;
            
        case REM:
            if (operand2 != 0) {
                alu_result = static_cast<int32_t>(operand1) % static_cast<int32_t>(operand2);
            } else {
                alu_result = operand1; // Remainder when dividing by zero is the dividend
                cout << "Warning: Remainder by zero attempted" << endl;
            }
            break;
            
        case AND:
            alu_result = operand1 & operand2;
            break;
            
        case LUI:
            alu_result = operand2; // Immediate value for LUI
            break;
            
        case AUIPC:
            alu_result = operand1 + operand2; // PC + immediate for AUIPC

            break;
            
        default:
            alu_result = 0;
            break;
    }
    
    // Handle branch instructions
    if (id_ex.branch) {
        bool take_branch = false;
        
        switch (id_ex.branch_cond) {
            case BranchCondition::BEQ:
                take_branch = (operand1 == operand2);
                break;
            case BranchCondition::BNE:
                take_branch = (operand1 != operand2);
                break;
            case BranchCondition::BLT:
                take_branch = (static_cast<int32_t>(operand1) < static_cast<int32_t>(operand2));
                break;
            case BranchCondition::BGE:
                take_branch = (static_cast<int32_t>(operand1) >= static_cast<int32_t>(operand2));
                break;
            default:
                take_branch = false;
                break;
        }
        
        if (branch_prediction_enabled) {
            bool was_predicted_taken = branch_predictor.was_predicted_taken(id_ex.pc);
            uint32_t predicted_target = branch_predictor.get_target(id_ex.pc);
            
            // Update branch predictor with actual outcome
            if (take_branch) {
                uint32_t actual_target = id_ex.pc + regState.getTemp("IMM");
                branch_predictor.update(id_ex.pc, actual_target, true);
                
                // Check if prediction was wrong
                if (!was_predicted_taken || predicted_target != (id_ex.pc + regState.getTemp("IMM"))) {
                    // Mispredicted - flush pipeline and update PC
                    regState.setPC(id_ex.pc + regState.getTemp("IMM"));
                    flush_if_id = true;
                    flush_id_ex = true;
                    branch_mispredictions++;
                    control_hazards++;
                    stalls_control_hazards += 2;
                }
            } else {
                branch_predictor.update(id_ex.pc, id_ex.pc + 4, false);
                
                // Check if prediction was wrong
                if (was_predicted_taken) {
                    // Mispredicted - flush pipeline and update PC
                    regState.setPC(id_ex.pc + 4);
                    flush_if_id = true;
                    flush_id_ex = true;
                    branch_mispredictions++;
                    control_hazards++;
                    stalls_control_hazards += 2;
                }
            }
        } else {
            // Without branch prediction, always flush on taken branches
            if (take_branch) {
                regState.setPC(id_ex.pc + regState.getTemp("IMM"));
                flush_if_id = true;
                flush_id_ex = true;
                control_hazards++;
                stalls_control_hazards += 2;
            }
        }
        
        ex_mem.branch_taken = take_branch;
    }
    
    // Handle jump instructions (JAL, JALR)
    if (id_ex.jump) {
        // Calculate return address (PC + 4)
        alu_result = id_ex.pc + 4;
        
        // Calculate jump target
        uint32_t jump_target;
        if(id_ex.is_jal) {
            jump_target = id_ex.pc + regState.getTemp("IMM");
        } else {
            jump_target = (regState.getTemp("RA") + regState.getTemp("IMM")) & 0xFFFFFFFE; // JALR target
        }
        
        // Update PC and flush pipeline
        regState.setPC(jump_target);
        
        // Invalidate any instructions in the pipeline
        if_id.valid = false;
        if_id.terminate = false;
        flush_if_id = true;
        flush_id_ex = true;
        
        // Update branch predictor if enabled
        if (branch_prediction_enabled) {
            branch_predictor.update(id_ex.pc, jump_target, true);
        }
        
        // Update statistics
        control_hazards++;
        stalls_control_hazards += 2;
    }
    
    // Store ALU result in EX/MEM buffer
    ex_mem.alu_result = alu_result;
    
    // Store result in temporary register for compatibility with existing code
    regState.setTemp("RZ", alu_result);
    regState.setTemp("RM", regState.getTemp("RB"));
}

void Simulator::stageMemory() {
    if (!ex_mem.valid) {
        mem_wb.valid = false;
        return;
    }
    
    if (ex_mem.terminate){
        mem_wb.terminate = true;
    }

    mem_wb.rd = ex_mem.rd;
    mem_wb.reg_write = ex_mem.reg_write;
    mem_wb.valid = true;
    
    // Default: pass ALU result to WB stage
    uint32_t result = regState.getTemp("RZ");
    
    // For debugging/tracing
    if (trace_instruction && instructions_executed == trace_instruction_num) {
        cout << "Cycle " << clock << ": Instruction " << trace_instruction_num 
             << " in memory stage" << endl;
    }

    uint32_t address = regState.getTemp("RZ");
    
    // Handle memory operations
    if (ex_mem.mem_read) {
        switch (ex_mem.mem_width) {
            case 0x0: // LB - Load Byte
                result = static_cast<int32_t>(static_cast<int8_t>(memory.readByte(address)));
                break;
                
            case 0x1: // LH - Load Half-word
                result = static_cast<int32_t>(static_cast<int16_t>(memory.readHalf(address)));
                break;
                
            case 0x2: // LW - Load Word
                result = memory.readWord(address);
                break;
                
            case 0x4: // LBU - Load Byte Unsigned
                result = static_cast<uint32_t>(memory.readByte(address));
                break;
                
            case 0x5: // LHU - Load Half-word Unsigned
                result = static_cast<uint32_t>(memory.readHalf(address));
                break;
                
            default:
                cerr << "Error: Unknown load width: 0x" << hex << ex_mem.mem_width << dec << endl;
                break;
        }
        
        // Update MDR with loaded value
        regState.setTemp("MDR", result);
        regState.setTemp("RY", result);
        
        // For statistics
        data_transfer_instructions++;
    } else if (ex_mem.mem_write) {
        uint32_t data = ex_mem.rs2_val;
        
        switch (ex_mem.mem_width) {
            case 0x0: // SB - Store Byte
                memory.writeByte(address, data & 0xFF);
                break;
                
            case 0x1: // SH - Store Half-word
                memory.writeHalf(address, data & 0xFFFF);
                break;
                
            case 0x2: // SW - Store Word
                memory.writeWord(address, data);
                break;
                
            default:
                cerr << "Error: Unknown store width: 0x" << hex << ex_mem.mem_width << dec << endl;
                break;
        }
        
        // For statistics
        data_transfer_instructions++;
        
       
        mem_wb.reg_write = false;
    } else {
        
        regState.setTemp("RY", result);
    }
    
    
    if (ex_mem.branch_taken) {
        // Branch was taken, update statistics
        control_instructions++;
    }
}

void Simulator::stageWriteBack() {
    if (!mem_wb.valid) {
        return;
    }
    
    if (trace_instruction && instructions_executed == trace_instruction_num) {
        cout << "Cycle " << clock << ": Instruction " << trace_instruction_num 
             << " in write back stage" << endl;
    }
    
    if (mem_wb.reg_write && mem_wb.rd != 0) {
        regState.setGen(mem_wb.rd, regState.getTemp("RY"));
        
        // for statistics
        instructions_executed++;
    }
    
    // check for data forwarding to resolve data hazards
    if (enable_data_forwarding && mem_wb.reg_write && mem_wb.rd != 0) {
        if (if_id.valid) {
            uint32_t rs1 = (if_id.instruction >> 15) & 0x1F;
            uint32_t rs2 = (if_id.instruction >> 20) & 0x1F;
            
            if (rs1 == mem_wb.rd || rs2 == mem_wb.rd) {
                data_hazards++;
            }
        }
    }
}


//function to fetch instruction
void Simulator::fetch() {
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
            // regState.setTemp("RM", regState.getGen(decodedInst.rs2));
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

    regState.setTemp("RM", regState.getTemp("RB"));

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

    regState.setTemp("MDR", regState.getTemp("RM"));
    regState.setTemp("MAR", regState.getTemp("RZ"));
    
    switch (opcode) {
        case 0x03: 
        {
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
