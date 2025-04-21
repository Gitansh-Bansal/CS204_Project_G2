#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstdint>
#include <string>
#include "register_state.h"
#include "memory.h"
#include "branch_predictor.h"

enum AluOperation {
    NONE, 
    ADD,
    SUB,
    MUL,
    SLL,
    SLT,
    XOR,
    DIV,
    SRL,
    SRA,
    OR,
    REM,
    AND,
    LUI,  
    AUIPC
};

enum BranchCondition {
    BEQ, 
    BNE, 
    BLT, 
    BGE, 
    INVALID
};

// IF/ID Buffer
struct IF_ID_Buffer {
    bool terminate;
    uint32_t pc; 
    uint32_t instruction; 
    bool valid;   
};

// ID/EX Buffer
struct ID_EX_Buffer {
    bool terminate;
    uint32_t pc;   
    uint32_t rs1_val; 
    uint32_t rs2_val;  
    uint32_t rd;  
    uint32_t imm; 
    bool reg_write; 
    bool mem_read; 
    bool mem_write; 
    bool branch;  
    bool jump;  
    AluOperation alu_op; 
    BranchCondition branch_cond; 
    uint8_t mem_width; 
    uint32_t rs1; 
    uint32_t rs2;  
    bool valid;  
    bool ALU_src; 
    bool is_jal;    
    uint32_t chk_type;
};

// EX/MEM Buffer
struct EX_MEM_Buffer {
    bool terminate;
    uint32_t pc; 
    uint32_t alu_result;  
    uint32_t rs2_val; 
    uint32_t rd; 
    bool reg_write;  
    bool mem_read; 
    bool mem_write;    
    uint8_t mem_width;  
    bool branch_taken;
    bool branch;
    bool jump;
    bool valid;       
    uint32_t chk_type;
};

// MEM/WB Buffer
struct MEM_WB_Buffer {
    bool terminate;
    uint32_t rd;  
    bool reg_write;  
    bool valid;   
    uint32_t pc;
    uint32_t chk_type;
};


class Simulator {
public:
    Simulator();
    
    void reset();
    bool loadProgram(const std::string& filename);
    
    void run();
    void step();
    bool isRunning() const;
    uint32_t getClock() const;
    
    void printRegisters() const;
    void printMemory(uint32_t start_addr, uint32_t end_addr, char format) const;
    
    const RegisterState& getRegisterState() const { return regState; }
    RegisterState& getRegisterState() { return regState; }
    
    const Memory& getMemory() const { return memory; }
    Memory& getMemory() { return memory; }

    // phase 3
    bool enable_pipelining;
    bool enable_data_forwarding;
    bool print_registers_each_cycle;
    bool print_pipeline_registers;
    bool trace_instruction;
    uint32_t trace_instruction_num;
    bool print_branch_prediction;
    bool branch_prediction_enabled;
    
    // Pipeline statistics
    uint32_t total_cycles;
    uint32_t instructions_executed;
    uint32_t data_transfer_instructions;
    uint32_t alu_instructions;
    uint32_t control_instructions;
    uint32_t pipeline_stalls;
    uint32_t data_hazards;
    uint32_t control_hazards;
    uint32_t branch_mispredictions;
    uint32_t stalls_data_hazards;
    uint32_t stalls_control_hazards;
    
    // Knob control
    void setKnob1(bool enabled);
    void setKnob2(bool enabled);
    void setKnob3(bool enabled);
    void setKnob4(bool enabled);
    void setKnob5(int target);
    void setKnob6(bool enabled);
    void printStatistics(bool writeToFile) const;
    
private:
    // Structure for decoded instruction
    struct DecodedInstruction {
        uint32_t opcode = 0;
        uint32_t rd = 0;
        uint32_t funct3 = 0;
        uint32_t rs1 = 0;
        uint32_t rs2 = 0;
        uint32_t funct7 = 0;
        int32_t imm = 0;
    };
    
    RegisterState regState;
    Memory memory;
    uint32_t pc;
    uint32_t clock;
    
    // Pipeline stages
    void fetch();
    DecodedInstruction decode();
    void execute(DecodedInstruction& decodedInst);
    void memoryAccess(DecodedInstruction& decodedInst);
    void writeBack(DecodedInstruction& decodedInst);


    // phase 3
    BranchPredictor branch_predictor;

    // Pipeline buffers
    IF_ID_Buffer if_id;
    ID_EX_Buffer id_ex;
    EX_MEM_Buffer ex_mem;
    MEM_WB_Buffer mem_wb;
    
    // Pipeline control signals
    bool stall_if;
    bool stall_id;
    bool flush_if_id;
    bool flush_id_ex;
    
    // Pipeline stage methods
    void pipelinedStep();
    void stageFetch();
    void stageDecode();
    void stageExecute();
    void stageMemory();
    void stageWriteBack();

    // Hazard detection and forwarding
    void detectHazards();
    void forwardData();

    void printPipelineRegisters() const;
};

#endif // SIMULATOR_H
