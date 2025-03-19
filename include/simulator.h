#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstdint>
#include <string>
#include <vector>
#include "memory.h"
#include "register_state.h"
using namespace std;

class Simulator {
private:
    RegisterState regState;
    Memory memory;
    uint32_t pc;
    uint32_t clock;

    uint32_t fetch();

    struct DecodedInstruction {
        uint8_t opcode;
        string type;
        int rd, rs1, rs2;
        int funct3, funct7;
        int32_t imm;
    };
    
    DecodedInstruction decode(uint32_t instruction);
    void execute(DecodedInstruction& decodedInst);
    void memoryAccess(DecodedInstruction& decodedInst);
    void writeBack(DecodedInstruction& decodedInst);

public:
    Simulator();
    void reset();
    bool loadProgram(const string& filename);
    void run();
    void step();
    bool isRunning() const;
    uint32_t getClock() const;
};

#endif
