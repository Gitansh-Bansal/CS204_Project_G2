#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstdint>
#include <string>
#include "register_state.h"
#include "memory.h"

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
    
    // Add these methods to access the register state and memory
    const RegisterState& getRegisterState() const { return regState; }
    RegisterState& getRegisterState() { return regState; }
    
    const Memory& getMemory() const { return memory; }
    Memory& getMemory() { return memory; }
    
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
};

#endif // SIMULATOR_H
