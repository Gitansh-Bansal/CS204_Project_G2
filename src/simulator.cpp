#include "simulator.h"
#include <iostream>
#include <iomanip>

Simulator::Simulator() {
    reset();
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
    
    uint32_t instruction = fetch();
    DecodedInstruction decodedInst = decode(instruction);
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

uint32_t Simulator::fetch() 

Simulator::DecodedInstruction Simulator::decode(uint32_t instruction)

void Simulator::execute(DecodedInstruction& decodedInst) 

void Simulator::memoryAccess(DecodedInstruction& decodedInst) {
    cout << "\nMEMORY ACCESS:" << endl;
    
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
            regState.tempReg["RY"] = regState.tempReg["RZ"];
            break;
    }
    
    pc = regState.tempReg["PC"];
}


void Simulator::writeBack(DecodedInstruction& decodedInst)