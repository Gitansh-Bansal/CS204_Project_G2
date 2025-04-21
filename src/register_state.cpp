#include <bits/stdc++.h>
#include "register_state.h"
using namespace std;

// Constructor
RegisterState::RegisterState() {
    regFile.assign(32,0);
    regFile[2] = 0x7FFFFFDC;
    regFile[3] = 0x10000000;
    pc=0;
    ir=0;
    tempReg={
        {"RM", 0},
        {"RA", 0},
        {"RB", 0},
        {"RY", 0},
        {"RZ", 0},
        {"MAR", 0},
        {"MDR", 0},
        {"IMM", 0}
    };
    cout << "All Registers Initialized!" << endl;
}

//function to reset all registers
void RegisterState::reset() {
    regFile.assign(32, 0);
    regFile[2] = 0x7FFFFFDC;
    regFile[3] = 0x10000000;
    pc=0;
    ir=0;
    tempReg={
        {"RM", 0},
        {"RA", 0},
        {"RB", 0},
        {"RY", 0},
        {"RZ", 0},
        {"MAR", 0},
        {"MDR", 0},
        {"IMM", 0},
        {"PC_TEMP", 0}
    };
    cout << "All Registers Reset to their Default Values!" << endl;
}

//function to get value of PC
uint32_t RegisterState::getPC() const {
    return pc;
}

//function to set value of PC
void RegisterState::setPC(uint32_t val) {
    pc = val;
    cout << "PC set to 0x" << hex << setw(8) << setfill('0') << val << dec << endl;
}

//function to increment PC by offset
void RegisterState::incrementPC(int offset) {
    pc += offset;
    cout << "PC set to 0x" << hex << setw(8) << setfill('0') << pc << dec << endl;
}

//function to get value of IR
uint32_t RegisterState::getIR() const {
    return ir;
}

//function to set value of IR
void RegisterState::setIR(uint32_t val) {
    ir = val;
    cout << "IR set to 0x" << hex << setw(8) << setfill('0') << val << dec << endl;
}

//function to get value of general purpose register
int32_t RegisterState::getGen(uint32_t reg_num) const {
    if (reg_num >= 32) {
        cerr << "Error: Invalid register number: " << reg_num << endl;
        return 0;
    }
    return regFile[reg_num];
}

//function to set value of general purpose register
void RegisterState::setGen(uint32_t reg_num, int32_t val) {
    if (reg_num >= 32) {
        cerr << "Error: Invalid register number: " << reg_num << endl;
        return;
    }
    
    if (reg_num != 0) {
        regFile[reg_num] = val;
    }
    cout << "Value 0x" << hex << setw(8) << setfill('0') << val
         << " written to Register x" << dec << reg_num << endl;
}

//function to get value of temporary register
int32_t RegisterState::getTemp(const string& reg_name) const {
    auto it = tempReg.find(reg_name);
    if (it == tempReg.end()) {
        cerr << "Error: Temporary register '" << reg_name << "' not found" << endl;
        return 0;
    }
    return it->second;
}

//function to set value of temporary register
void RegisterState::setTemp(const string& reg_name, int32_t val) {
    tempReg[reg_name] = val;
    cout << "Value 0x" << hex << setw(8) << setfill('0') << val
         << " written to Temporary Register " << reg_name << dec << endl;
}

//function to print all registers
void RegisterState::printAll() const {
    cout << "General Purpose Registers:" << endl;
    for (uint32_t i = 0; i < 32; i++) {
        cout << "x" << dec <<i << ": 0x" << hex << setw(8) << setfill('0') << getGen(i);
        
        // Print 4 registers per line
        if ((i + 1) % 4 == 0) {
            cout << endl;
        } else {
            cout << "\t";
        }
    }
    
    cout << endl << "Special Registers:" << endl;
    cout << "PC: 0x" << hex << setw(8) << setfill('0') << pc << dec << endl;
    cout << "IR: 0x" << hex << setw(8) << setfill('0') << ir << dec << endl;
    
    cout << endl << "Temporary Registers:" << endl;
    for (const auto& reg : tempReg) {
        cout << reg.first << ": 0x" << hex << setw(8) << setfill('0') << reg.second << dec << endl;
    }
}

void RegisterState::printGenRegisters() const {
    cout << "General Purpose Registers:" << endl;
    for(uint32_t i = 0; i < 32; i++) {
        cout << "x" << dec <<i << ": 0x" << hex << setw(8) << setfill('0') << getGen(i);
        
        // Print 4 registers per line
        if ((i + 1) % 4 == 0) {
            cout << endl;
        } else {
            cout << "\t";
        }
    }
    cout << endl;
}