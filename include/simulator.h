#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstdint>
#include <string>
#include <vector>
#include "memory.h"
#include "register_state.h"
#include "parser.h"
using namespace std;

class Simulator {
private:
    RegisterState regState;
    Memory memory;
    uint32_t pc;
    uint32_t clock;

    void fetch();

    struct DecodedInstruction {
        uint8_t opcode = 0;
        int rd=0, rs1=0, rs2=0;
        int funct3=0, funct7=0;
        int32_t imm=0;
    };
    
    DecodedInstruction decode();
    void execute(DecodedInstruction& decodedInst);
    void memoryAccess(DecodedInstruction& decodedInst);
    void writeBack(DecodedInstruction& decodedInst);

public:
    Simulator();
    void reset();
    bool loadProgram(const string& filename);
    void printRegisters() const;
    void printMemory(uint32_t start_addr, uint32_t end_addr, char format) const;
    void run();
    void step();
    bool isRunning() const;
    uint32_t getClock() const;
};

#endif
