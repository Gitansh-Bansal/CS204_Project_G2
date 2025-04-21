#include "simulator.h"
#include <iostream>
#include <iomanip>
#include <fstream>

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

    // Initialize statistics
    total_cycles = 0;
    instructions_executed = 0;
    data_transfer_instructions = 0;
    alu_instructions = 0;
    control_instructions = 0;
    pipeline_stalls = 0;
    data_hazards = 0;
    control_hazards = 0;
    branch_mispredictions = 0;
    stalls_data_hazards = 0;
    stalls_control_hazards = 0;
        
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

    // Reset statistics
    total_cycles = 0;
    instructions_executed = 0;
    data_transfer_instructions = 0;
    alu_instructions = 0;
    control_instructions = 0;
    pipeline_stalls = 0;
    data_hazards = 0;
    control_hazards = 0;
    branch_mispredictions = 0;
    stalls_data_hazards = 0;
    stalls_control_hazards = 0;
    
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
                cout << "Simulation ended after " << clock -6 << " cycles." << endl;
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

        if (!enable_data_forwarding) detectHazards();

        cout<<"\nWriteBack Stage:"<<endl;
        stageWriteBack();
        cout<<"\nMemory Access Stage:"<<endl;
        stageMemory();
        cout<<"\nExecute Stage:"<<endl;
        stageExecute();
        cout<<"\nInstruction Decode Stage:"<<endl;
        stageDecode();
        cout<<"\nInstruction Fetch Stage:"<<endl;
        if (!if_id.terminate) stageFetch();
        
        if (enable_data_forwarding) forwardData();

        clock++;
        
        if (print_registers_each_cycle) {
            cout<<"\n-------------------- Registers --------------------\n";
            printRegisters();
            cout<<"---------------------------------------------------\n";
        }
        
        if (print_pipeline_registers) printPipelineRegisters();

        if (print_branch_prediction) branch_predictor.print_state();

    } else {
        cout<<"\nFetch Stage:"<<endl;
        fetch();
        cout<<"\nDecode Stage:"<<endl;
        DecodedInstruction decodedInst = decode();
        cout<<"\nExecute Stage:"<<endl;
        execute(decodedInst);
        cout<<"\nMemory Access Stage:"<<endl;
        memoryAccess(decodedInst);
        cout<<"\nWriteBack Stage:"<<endl;
        writeBack(decodedInst);
        
        clock+=5;

        if (print_registers_each_cycle) {
            cout<<"\n-------------------- Registers --------------------\n";
            printRegisters();
            cout<<"---------------------------------------------------\n";
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
        
        if (trace_instruction && if_id.pc == trace_instruction_num) {
        
            cout<<"Instruction Tracing (Knob5): Instruction PC 0x"<<hex<<trace_instruction_num<<dec<<endl;
            //Print IF/ID buffer contents
            cout << "IF/ID Pipeline Register:" << endl;
            if (if_id.valid) {
            cout << "  PC: 0x" << hex << setw(8) << setfill('0') << if_id.pc << dec << endl;
            cout << "  Instruction: 0x" << hex << setw(8) << setfill('0') << if_id.instruction << dec << endl;
        }
            cout << "  Valid: " << (if_id.valid ? "True" : "False") << endl;
    

        }
    } else {
        // During flush, invalidate everything in IF/ID
        if_id.valid = false;
        if_id.terminate = false;
        if_id.instruction = 0x00000013;  // NOP instruction
        flush_if_id = false;
        
        // Removed control_hazards increment from here to avoid double counting
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
    id_ex.opcode = decodedInst.opcode;  // Set opcode
    id_ex.valid = true;
    
    // Set control signals based on the opcode
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
    switch (decodedInst.opcode) {
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

    //knob 5
    if(trace_instruction && id_ex.pc == trace_instruction_num) {

        cout << "Instruction Tracing (Knob5): Instruction PC 0x" << hex << trace_instruction_num << dec << endl;

        cout << "\nID/EX Pipeline Register:" << endl;
        if (id_ex.valid ) {    
        cout << "  PC: 0x" << hex << setw(8) << setfill('0') << id_ex.pc << dec << endl;
        cout << "  RS1 (reg num): " << id_ex.rs1 << endl;
        cout << "  RS2 (reg num): " << id_ex.rs2 << endl;
        cout << "  RD: " << id_ex.rd << endl;
        cout << "  RegWrite: " << (id_ex.reg_write ? "True" : "False") << endl;
        cout << "  MemRead: " << (id_ex.mem_read ? "True" : "False") << endl;
        cout << "  MemWrite: " << (id_ex.mem_write ? "True" : "False") << endl;
        cout << "  Branch: " << (id_ex.branch ? "True" : "False") << endl;
        cout << "  Jump: " << (id_ex.jump ? "True" : "False") << endl;
        
        // Print ALU operation
        cout << "  ALU Operation: ";
        switch (id_ex.alu_op) {
            case ADD: cout << "ADD"; break;
            case SUB: cout << "SUB"; break;
            case MUL: cout << "MUL"; break;
            case SLL: cout << "SLL"; break;
            case SLT: cout << "SLT"; break;
            case XOR: cout << "XOR"; break;
            case DIV: cout << "DIV"; break;
            case SRL: cout << "SRL"; break;
            case SRA: cout << "SRA"; break;
            case OR: cout << "OR"; break;
            case REM: cout << "REM"; break;
            case AND: cout << "AND"; break;
            case LUI: cout << "LUI"; break;
            case AUIPC: cout << "AUIPC"; break;
            case NONE: cout << "NONE"; break;
            default: cout << "UNKNOWN"; break;
        }
        cout << endl;
        
        // Print branch condition
        cout << "  Branch Condition: ";
        switch (id_ex.branch_cond) {
            case BranchCondition::BEQ: cout << "BEQ"; break;
            case BranchCondition::BNE: cout << "BNE"; break;
            case BranchCondition::BLT: cout << "BLT"; break;
            case BranchCondition::BGE: cout << "BGE"; break;
            case BranchCondition::INVALID: cout << "INVALID"; break;
            default: cout << "UNKNOWN"; break;
        }
        cout << endl;
        
        cout << "  Memory Width: " << static_cast<int>(id_ex.mem_width) << endl;
        cout << "  Terminate: " << (id_ex.terminate ? "True" : "False") << endl;
        }
        cout << "  Valid: " << (id_ex.valid ? "True" : "False") << endl;

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
    ex_mem.branch = id_ex.branch;
    ex_mem.jump = id_ex.jump;
    ex_mem.opcode = id_ex.opcode;  // Propagate opcode
    ex_mem.valid = true;
    ex_mem.branch_taken = false;
    
    
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
                    pipeline_stalls += 2;
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
                    pipeline_stalls += 2;
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
                pipeline_stalls += 2;
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
            bool was_predicted = branch_predictor.was_predicted_taken(id_ex.pc);
            uint32_t predicted_target = branch_predictor.get_target(id_ex.pc);
            
            branch_predictor.update(id_ex.pc, jump_target, true);
            
            // Only count as misprediction if prediction was wrong
            if (!was_predicted || predicted_target != jump_target) {
                branch_mispredictions++;
            }
        }
        
        // Always count control hazard for jumps
        control_hazards++;
        stalls_control_hazards += 2;
        pipeline_stalls += 2;
    }
    // Store ALU result in EX/MEM buffer
    ex_mem.alu_result = alu_result;

    // For debugging/tracing
    if(trace_instruction && id_ex.pc == trace_instruction_num) {
        cout << "Instruction Tracing (Knob5): Instruction PC 0x" << hex << trace_instruction_num << dec << endl;
        
        cout << "\nEX/MEM Pipeline Register:" << endl;
        if (ex_mem.valid) {
            cout << "  PC: 0x" << hex << setw(8) << setfill('0') << ex_mem.pc << dec << endl;
            cout << "  ALU Result: 0x" << hex << setw(8) << setfill('0') << ex_mem.alu_result << dec << endl;
            cout << "  RD: " << ex_mem.rd << endl;
            cout << "  RegWrite: " << (ex_mem.reg_write ? "True" : "False") << endl;
            cout << "  MemRead: " << (ex_mem.mem_read ? "True" : "False") << endl;
            cout << "  MemWrite: " << (ex_mem.mem_write ? "True" : "False") << endl;
            cout << "  Memory Width: " << static_cast<int>(ex_mem.mem_width) << endl;
            cout << "  Branch Taken: " << (ex_mem.branch_taken ? "True" : "False") << endl;
            cout << "  Terminate: " << (ex_mem.terminate ? "True" : "False") << endl;
        }
        cout << "  Valid: " << (ex_mem.valid ? "True" : "False") << endl;
    
    }

    
    
    
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
    mem_wb.opcode = ex_mem.opcode;  // Propagate opcode
    mem_wb.pc = ex_mem.pc;
    mem_wb.valid = true;
    
    // Default: pass ALU result to WB stage
    uint32_t result = regState.getTemp("RZ");
    
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
       
        mem_wb.reg_write = false;
    } else {
        regState.setTemp("RY", result);
    }

    // For debugging/tracing
    if(trace_instruction && ex_mem.pc == trace_instruction_num) {
        cout << "Instruction Tracing (Knob5): Instruction PC 0x" << hex << trace_instruction_num << dec << endl;
        cout << "\nMEM/WB Pipeline Register:" << endl;
        if (mem_wb.valid) {
            cout << "  Result: 0x" << hex << setw(8) << setfill('0') << regState.getTemp("RY") << dec << endl;
            cout << "  RD: " << mem_wb.rd << endl;
            cout << "  RegWrite: " << (mem_wb.reg_write ? "True" : "False") << endl;
            cout << "  Terminate: " << (mem_wb.terminate ? "True" : "False") << endl;
        }
        cout << "  Valid: " << (mem_wb.valid ? "True" : "False") << endl;
    }
}

void Simulator::stageWriteBack() {
    if (!mem_wb.valid) {
        return;
    }


    
    if (enable_pipelining) {
        instructions_executed++;

        // Classify instruction based on opcode from MEM/WB buffer
        switch(mem_wb.opcode) {
            case 0x03:  // Load instructions
            case 0x23:  // Store instructions
                data_transfer_instructions++;
                break;
                
            case 0x63:  // Branch instructions
            case 0x6F:  // JAL
            case 0x67:  // JALR
                control_instructions++;
                break;
                
            case 0x33:  // R-type ALU instructions
            case 0x13:  // I-type ALU instructions
            case 0x37:  // LUI
            case 0x17:  // AUIPC
                alu_instructions++;
                break;
                
            default:
                cerr << "Unknown instruction type in WriteBack stage, opcode: 0x" 
                     << hex << mem_wb.opcode << dec << endl;
                break;
        }
    }
    
    
    if (mem_wb.reg_write && mem_wb.rd != 0) {
        regState.setGen(mem_wb.rd, regState.getTemp("RY"));
    }

    if (trace_instruction && mem_wb.pc == trace_instruction_num) {
        cout << "Instruction Tracing (Knob5): Instruction PC 0x" << hex << trace_instruction_num << dec << endl;
        //print register file
        cout << "\nRegisters after write back:" << endl;
        regState.printGenRegisters();
        
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

    // For non-pipelined mode, classify instruction here
    if (!enable_pipelining) {
        
        switch(decodedInst.opcode) {
            case 0x03:  // Load instructions
            case 0x23:  // Store instructions
                data_transfer_instructions++;
                instructions_executed++;
                break;
                
            case 0x63:  // Branch instructions
            case 0x6F:  // JAL
            case 0x67:  // JALR
                control_instructions++;
                instructions_executed++;
                break;
                
            case 0x33:  // R-type ALU instructions
            case 0x13:  // I-type ALU instructions
            case 0x37:  // LUI
            case 0x17:  // AUIPC
                alu_instructions++;
                instructions_executed++;
                break;
                
            default:
                cerr << "Unknown instruction type in decode stage, opcode: 0x" 
                     << hex << decodedInst.opcode << dec << endl;
                break;
        }
    }

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
void Simulator::execute(DecodedInstruction& decodedInst) {    
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

void Simulator::detectHazards() {
    // Skip hazard detection if data forwarding is enabled
    if (enable_data_forwarding) {
        stall_if = false;
        stall_id = false;
        return;
    }
    
    // Extract source registers from IF/ID buffer instruction
    if (!if_id.valid) {
        stall_if = false;
        stall_id = false;
        return;
    }
    
    uint32_t instruction = if_id.instruction;
    uint32_t opcode = instruction & 0x7F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    
    // Initialize stall signals to false
    bool hazard_detected = false;
    bool is_load_use_hazard = false;
    bool is_ex_mem_hazard = false;
    
    // Check for RAW hazards with instruction in ID/EX stage
    if (id_ex.valid && id_ex.rd != 0) {
        // For R-type, I-type, and load instructions that write to a register
        if (id_ex.reg_write) {
            // Special case: load-use hazard
            if (id_ex.mem_read && (id_ex.rd == rs1 || id_ex.rd == rs2)) {
                // Load followed by instruction that uses the loaded value
                // This cannot be resolved by forwarding and requires a stall
                hazard_detected = true;
                is_load_use_hazard = true;
                data_hazards++;
                stalls_data_hazards += 2;  // Load-use hazard causes 2 stalls
                cout << "Load-use hazard detected: stalling pipeline for 2 cycles" << endl;
            }
            // RAW hazard with ALU instruction
            else if (!id_ex.mem_read && (id_ex.rd == rs1 || id_ex.rd == rs2)) {
                // This requires a stall without forwarding
                hazard_detected = true;
                data_hazards++;
                stalls_data_hazards += 1;  // ALU-use hazard causes 1 stall
                cout << "RAW hazard detected: stalling pipeline for 1 cycle" << endl;
            }
        }
    }
    
    // Check for RAW hazards with instruction in EX/MEM stage
    if (ex_mem.valid && ex_mem.rd != 0 && ex_mem.reg_write) {
        if (ex_mem.rd == rs1 || ex_mem.rd == rs2) {
            // This requires a stall without forwarding
            hazard_detected = true;
            is_ex_mem_hazard = true;
            data_hazards++;
            stalls_data_hazards += 1;  // EX/MEM hazard causes 1 stall
            cout << "RAW hazard with EX/MEM stage detected: stalling pipeline for 1 cycle" << endl;
        }
    }
    
    // Set stall signals if hazard is detected
    if (hazard_detected) {
        stall_if = true;
        
        // For EX/MEM hazards, release decode stall one cycle earlier
        if (is_ex_mem_hazard) {
            stall_id = false;  // Release decode stage one cycle earlier
        } else {
            stall_id = true;  // Keep decode stalled for ID/EX hazards
        }
        
        flush_id_ex = true;
        pipeline_stalls++;
        
        // Debug output
        cout << "Pipeline stalled due to hazard. Current state:" << endl;
        cout << "  IF/ID instruction: 0x" << hex << if_id.instruction << dec << endl;
        cout << "  ID/EX rd: " << id_ex.rd << ", rs1: " << rs1 << ", rs2: " << rs2 << endl;
    } else {
        stall_if = false;
        stall_id = false;
    }
}

void Simulator::forwardData() {
    stall_if = false;
    stall_id = false;
    flush_id_ex = false;

    if (!id_ex.valid) {
        return;
    }
    
    uint32_t rs1 = id_ex.rs1;
    uint32_t rs2 = id_ex.rs2;
    
    bool forward_a_from_ex_mem = false;
    bool forward_a_from_mem_wb = false;
    bool forward_b_from_ex_mem = false;
    bool forward_b_from_mem_wb = false;
    
    // check for forwarding from EX/MEM stage
    if (ex_mem.valid && ex_mem.reg_write && ex_mem.rd != 0) {
        // forward from EX/MEM to rs1
        if (ex_mem.rd == rs1) {
            forward_a_from_ex_mem = true;
            data_hazards++;
            cout << "Data hazard detected: forwarding from EX/MEM to RS1" << endl;
        }
        
        // forward from EX/MEM to rs2
        if (ex_mem.rd == rs2) {
            forward_b_from_ex_mem = true;
            data_hazards++;
            cout << "Data hazard detected: forwarding from EX/MEM to RS2" << endl;
        }
    }
    
    // check for forwarding from MEM/WB stage
    if (mem_wb.valid && mem_wb.reg_write && mem_wb.rd != 0) {
        // forward from MEM/WB to rs1 if not already forwarding from EX/MEM
        if (mem_wb.rd == rs1 && !forward_a_from_ex_mem) {
            forward_a_from_mem_wb = true;
            data_hazards++;
            cout << "Data hazard detected: forwarding from MEM/WB to RS1" << endl;
        }
        
        // forward from MEM/WB to rs2 if not already forwarding from EX/MEM
        if (mem_wb.rd == rs2 && !forward_b_from_ex_mem) {
            forward_b_from_mem_wb = true;
            data_hazards++;
            cout << "Data hazard detected: forwarding from MEM/WB to RS2" << endl;
        }
    }
    
    if (forward_a_from_ex_mem) {
        regState.setTemp("RA", ex_mem.alu_result);
    } else if (forward_a_from_mem_wb) {
        regState.setTemp("RA", regState.getTemp("RY"));
    }
    
    if (forward_b_from_ex_mem) {
        regState.setTemp("RB", ex_mem.alu_result);
    } else if (forward_b_from_mem_wb) {
        regState.setTemp("RB", regState.getTemp("RY"));
    }
    
    // check for load-use hazards (these always need stalls even with forwarding)
    if (id_ex.valid && id_ex.mem_read && id_ex.rd != 0) {
        if (if_id.valid) {
            uint32_t instruction = if_id.instruction;
            uint32_t rs1_if = (instruction >> 15) & 0x1F;
            uint32_t rs2_if = (instruction >> 20) & 0x1F;
            
            // check if the next instruction uses the result of the load
            if (id_ex.rd == rs1_if || id_ex.rd == rs2_if) {
                stall_if = true;
                stall_id = true;
                flush_id_ex = true;
                
                data_hazards++;
                stalls_data_hazards++;
                pipeline_stalls++;
                cout << "Load-use hazard detected: must stall even with forwarding" << endl;
            }
        }
    }
}



void Simulator::printPipelineRegisters() const {
    cout << "\n----- Pipeline Registers -----" << endl;
    
    // Print IF/ID Buffer contents
    cout << "IF/ID Pipeline Register:" << endl;
    if (if_id.valid) {
        cout << "  PC: 0x" << hex << setw(8) << setfill('0') << if_id.pc << dec << endl;
        cout << "  Instruction: 0x" << hex << setw(8) << setfill('0') << if_id.instruction << dec << endl;
    }
    cout << "  Valid: " << (if_id.valid ? "True" : "False") << endl;
    
    // Print ID/EX Buffer contents
    cout << "\nID/EX Pipeline Register:" << endl;
    if (id_ex.valid ) {    
        cout << "  PC: 0x" << hex << setw(8) << setfill('0') << id_ex.pc << dec << endl;
        cout << "  RS1 (reg num): " << id_ex.rs1 << endl;
        cout << "  RS2 (reg num): " << id_ex.rs2 << endl;
        cout << "  RD: " << id_ex.rd << endl;
        cout << "  RegWrite: " << (id_ex.reg_write ? "True" : "False") << endl;
        cout << "  MemRead: " << (id_ex.mem_read ? "True" : "False") << endl;
        cout << "  MemWrite: " << (id_ex.mem_write ? "True" : "False") << endl;
        cout << "  Branch: " << (id_ex.branch ? "True" : "False") << endl;
        cout << "  Jump: " << (id_ex.jump ? "True" : "False") << endl;
        
        // Print ALU operation
        cout << "  ALU Operation: ";
        switch (id_ex.alu_op) {
            case ADD: cout << "ADD"; break;
            case SUB: cout << "SUB"; break;
            case MUL: cout << "MUL"; break;
            case SLL: cout << "SLL"; break;
            case SLT: cout << "SLT"; break;
            case XOR: cout << "XOR"; break;
            case DIV: cout << "DIV"; break;
            case SRL: cout << "SRL"; break;
            case SRA: cout << "SRA"; break;
            case OR: cout << "OR"; break;
            case REM: cout << "REM"; break;
            case AND: cout << "AND"; break;
            case LUI: cout << "LUI"; break;
            case AUIPC: cout << "AUIPC"; break;
            case NONE: cout << "NONE"; break;
            default: cout << "UNKNOWN"; break;
        }
        cout << endl;
        
        // Print branch condition
        cout << "  Branch Condition: ";
        switch (id_ex.branch_cond) {
            case BranchCondition::BEQ: cout << "BEQ"; break;
            case BranchCondition::BNE: cout << "BNE"; break;
            case BranchCondition::BLT: cout << "BLT"; break;
            case BranchCondition::BGE: cout << "BGE"; break;
            case BranchCondition::INVALID: cout << "INVALID"; break;
            default: cout << "UNKNOWN"; break;
        }
        cout << endl;
        
        cout << "  Memory Width: " << static_cast<int>(id_ex.mem_width) << endl;
        cout << "  Terminate: " << (id_ex.terminate ? "True" : "False") << endl;
    }
    cout << "  Valid: " << (id_ex.valid ? "True" : "False") << endl;
    
    // Print EX/MEM Buffer contents
    cout << "\nEX/MEM Pipeline Register:" << endl;
    if (ex_mem.valid) {
        cout << "  PC: 0x" << hex << setw(8) << setfill('0') << ex_mem.pc << dec << endl;
        cout << "  ALU Result: 0x" << hex << setw(8) << setfill('0') << ex_mem.alu_result << dec << endl;
        cout << "  RD: " << ex_mem.rd << endl;
        cout << "  RegWrite: " << (ex_mem.reg_write ? "True" : "False") << endl;
        cout << "  MemRead: " << (ex_mem.mem_read ? "True" : "False") << endl;
        cout << "  MemWrite: " << (ex_mem.mem_write ? "True" : "False") << endl;
        cout << "  Memory Width: " << static_cast<int>(ex_mem.mem_width) << endl;
        cout << "  Branch Taken: " << (ex_mem.branch_taken ? "True" : "False") << endl;
        cout << "  Terminate: " << (ex_mem.terminate ? "True" : "False") << endl;
    }
    cout << "  Valid: " << (ex_mem.valid ? "True" : "False") << endl;
    
    // Print MEM/WB Buffer contents
    cout << "\nMEM/WB Pipeline Register:" << endl;
    if (mem_wb.valid) {
        cout << "  Result: 0x" << hex << setw(8) << setfill('0') << regState.getTemp("RY") << dec << endl;
        cout << "  RD: " << mem_wb.rd << endl;
        cout << "  RegWrite: " << (mem_wb.reg_write ? "True" : "False") << endl;
        cout << "  Terminate: " << (mem_wb.terminate ? "True" : "False") << endl;
    }
    cout << "  Valid: " << (mem_wb.valid ? "True" : "False") << endl;
    
    // Print pipeline control signals
    cout << "\nPipeline Control Signals:" << endl;
    cout << "  Stall IF: " << (stall_if ? "True" : "False") << endl;
    cout << "  Stall ID: " << (stall_id ? "True" : "False") << endl;
    cout << "  Flush IF/ID: " << (flush_if_id ? "True" : "False") << endl;
    cout << "  Flush ID/EX: " << (flush_id_ex ? "True" : "False") << endl;
    
    cout << "=================================================" << endl;
}



// Enable/disable pipelining
void Simulator::setKnob1(bool enabled) {
    enable_pipelining = enabled;
    
    // Reset pipeline buffers if enabled
    if (enabled) {
        // Reset IF/ID buffer
        if_id = {};
        if_id.valid = false;
        if_id.terminate = false;

        // Reset ID/EX buffer
        id_ex = {};
        id_ex.valid = false;
        id_ex.terminate = false;

        // Reset EX/MEM buffer
        ex_mem = {};
        ex_mem.valid = false;
        ex_mem.terminate = false;

        // Reset MEM/WB buffer
        mem_wb = {};
        mem_wb.valid = false;
        mem_wb.terminate = false;

        // Initialize IF stage with first instruction
        if_id.valid = true;
        if_id.pc = pc;
    }
}

// Enable/disable data forwarding
void Simulator::setKnob2(bool enabled) {
    enable_data_forwarding = enabled;
    
    // Reset related stats
    data_hazards = 0;
    stalls_data_hazards = 0;
    pipeline_stalls = 0;
}

// Enable/disable register file printing
void Simulator::setKnob3(bool enabled) {
    print_registers_each_cycle = enabled;
}

// Enable/disable pipeline state printing
void Simulator::setKnob4(bool enabled) {
    print_pipeline_registers = enabled;
    
}

// Instruction trace selector (Knob5)
void Simulator::setKnob5(int target_instr) {
    trace_instruction_num = target_instr;
    trace_instruction = (target_instr >= 0);

    


    if (trace_instruction) {
        if (target_instr % 4 != 0 || target_instr < 0) {
            cerr << "Error: Invalid instruction address for tracing: 0x" << hex << target_instr << dec << endl;
            trace_instruction = false;
        } else {
            cout << "Tracing instruction at PC: 0x" << hex << target_instr << dec << endl;
        }
    }
}

// Enable/disable branch predictor state printing
void Simulator::setKnob6(bool enabled) {
    print_branch_prediction = enabled;

}

void Simulator::printStatistics(bool writeToFile) const {
    cout << "\n============== Simulation Statistics ==============" << endl;
    if (enable_pipelining) {
        cout << "Stat1: Total cycles: " << clock - 1 << endl;
        cout << "Stat2: Total instructions executed: " << instructions_executed << endl;
        cout << "Stat3: CPI: " << fixed << setprecision(2) 
             << static_cast<double>(clock - 1) / instructions_executed << endl;
    }
    else{
        cout << "Stat1: Total cycles: " << clock - 6 << endl;
        cout << "Stat2: Total instructions executed: " << instructions_executed << endl;
        cout << "Stat3: CPI: " << fixed << setprecision(2) 
             << static_cast<double>(clock - 6) / instructions_executed << endl;
    }
    cout << "Stat4: Number of Data-transfer instructions: " << data_transfer_instructions << endl;
    cout << "Stat5: Number of ALU instructions: " << alu_instructions << endl;
    cout << "Stat6: Number of Control instructions: " << control_instructions << endl;
    cout << "Stat7: Number of stalls/bubbles: " << stalls_data_hazards + stalls_control_hazards << endl;
    cout << "Stat8: Number of data hazards: " << data_hazards << endl;
    cout << "Stat9: Number of control hazards: " << control_hazards << endl;
    cout << "Stat10: Number of branch mispredictions: " << branch_mispredictions << endl;
    cout << "Stat11: Number of stalls due to data hazards: " << stalls_data_hazards << endl;
    cout << "Stat12: Number of stalls due to control hazards: " << stalls_control_hazards << endl;
    cout << "================================================" << endl;

    if (writeToFile) {
        ofstream file("statistics.txt");
        if (file.is_open()) {
            file << "\n============== Simulation Statistics ==============" << endl;
            if (enable_pipelining) {
                file << "Stat1: Total cycles: " << clock - 1 << endl;
                file << "Stat2: Total instructions executed: " << instructions_executed << endl;
                file << "Stat3: CPI: " << fixed << setprecision(2) 
                 << static_cast<double>(clock - 1) / instructions_executed << endl;
            }
            else{
                file << "Stat1: Total cycles: " << clock - 6 << endl;
                file << "Stat2: Total instructions executed: " << instructions_executed << endl;
                file << "Stat3: CPI: " << fixed << setprecision(2) 
                 << static_cast<double>(clock - 6) / instructions_executed << endl;
            }
            file << "Stat4: Number of Data-transfer instructions: " << data_transfer_instructions << endl;
            file << "Stat5: Number of ALU instructions: " << alu_instructions << endl;
            file << "Stat6: Number of Control instructions: " << control_instructions << endl;
            file << "Stat7: Number of stalls/bubbles: " << stalls_data_hazards + stalls_control_hazards << endl;
            file << "Stat8: Number of data hazards: " << data_hazards << endl;
            file << "Stat9: Number of control hazards: " << control_hazards << endl;
            file << "Stat10: Number of branch mispredictions: " << branch_mispredictions << endl;
            file << "Stat11: Number of stalls due to data hazards: " << stalls_data_hazards << endl;
            file << "Stat12: Number of stalls due to control hazards: " << stalls_control_hazards << endl;
            file << "================================================" << endl;
            file.close();
        } else {
            cerr << "Error: Unable to open statistics file." << endl;
        }
    }
                
}
