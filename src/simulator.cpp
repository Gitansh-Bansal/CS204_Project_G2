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

void Simulator::memoryAccess(DecodedInstruction& decodedInst) 

void Simulator::writeBack(DecodedInstruction& decodedInst)