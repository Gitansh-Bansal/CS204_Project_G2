#include<bits/stdc++.h>
#include "register_state.h"
using namespace std;

RegisterState::RegisterState() {
    reset();
}

void RegisterState::reset() {
    for (int i = 0; i < 32; i++) {
        regFile[i] = 0;
    }
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
    cout<<"All Registers Reset to their Default Values!"<<endl;
}

uint32_t RegisterState::getPC() const {
    return pc;
}

void RegisterState::setPC(uint32_t val) {
    pc = val;
    cout<<"PC set to 0x"<<hex<<setw(8)<<setfill('0')<<val<<endl;
}

void RegisterState::incrementPC(int offset) {
    pc += offset;
    cout<<"PC set to 0x"<<hex<<setw(8)<<setfill('0')<<pc<<endl;
}

uint32_t RegisterState::getIR() const {
    return ir;
}

void RegisterState::setIR(uint32_t val) {
    ir = val;
    cout<<"IR set to 0x"<<hex<<setw(8)<<setfill('0')<<val<<endl;
}

int32_t RegisterState::getGen(uint32_t reg_num) const {
    if (reg_num >= 32) {
        cerr << "Error: Invalid register number: " << reg_num << endl;
        return 0;
    }
    return reg_file[reg_num];
}

void RegisterState::setGen(uint32_t reg_num, int32_t val) {
    if (reg_num >= 32) {
        cerr << "Error: Invalid register number: " << reg_num << endl;
        return;
    }
    
    if (reg_num != 0) {
        reg_file[reg_num] = val;
    }
    cout<<"Value 0x"<<hex<<setw(8)<<setfill('0')<<" written to Register x"<<dec<<reg_num<<endl;
}

int32_t RegisterState::getTemp(const string& reg_name) const {
    auto it = temp_registers.find(reg_name);
    if (it == temp_registers.end()) {
        cerr << "Error: Temporary register '" << reg_name << "' not found" << endl;
        return 0;
    }
    return it->second;
}

void RegisterState::setTemp(const string& reg_name, int32_t val) {
    temp_registers[reg_name] = val;
    cout<<"Value 0x"<<hex<<setw(8)<<setfill('0')<<" written to Temporary Register "<<reg_name<<endl;
}

void RegisterState::printAll() const {
    cout << "General Purpose Registers:" << endl;
    for (uint32_t i = 0; i < 32; i++) {
        cout << "x" << i << ": 0x" << hex << setw(8) << setfill('0') << getGen(i);
        
        // Print 4 registers per line
        if ((i + 1) % 4 == 0) {
            cout << endl;
        } else {
            cout << "\t";
        }
    }
    
    cout << endl << "Special Registers:" << endl;
    cout << "PC: 0x" << hex << setw(8) << setfill('0') << pc << endl;
    cout << "IR: 0x" << hex << setw(8) << setfill('0') << ir << endl;
    
    cout << endl << "Temporary Registers:" << endl;
    for (const auto& reg : tempReg) {
        cout << reg.first << ": 0x" << hex << setw(8) << setfill('0') << reg.second << endl;
    }
    
    cout << dec;
}
