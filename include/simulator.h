#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstdint>
#include "register_state.h"
#include "parser.h"

using namespace std;

struct DecodedInstruction {
    uint8_t opcode;      
    InstructionType type;   
    int rd;              
    int rs1;             
    int rs2;
    int funct3;
    int funct7;
    int32_t imm;
};

void fetch(RegisterState &regState);    // read the instruction from memory address stored in PC and store the instruction in IR
DecodedInstruction decode(RegisterState &regState);     // decode the instruction stored in IR and return a Decoded Instruction
void execute(DecodedInstruction decodedInstr, RegisterState &regState);     // use the decoded instruction and perform the execute step according to the instruction type
void memoryAccess(DecodedInstruction decodedInstr, RegisterState &regState, Memory &memory);    // memory access steps
void writeBack(DecodedInstruction& decodedInst, RegisterState& regState);   // write back step

#endif