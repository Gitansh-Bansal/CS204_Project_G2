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
}

uint32_t RegisterState::get_pc() const {
    return pc;
}

void RegisterState::set_pc(uint32_t val) {
    pc = val;
}

void RegisterState::increment_pc(int offset) {
    pc += offset;
}

uint32_t RegisterState::get_ir() const {
    return ir;
}

void RegisterState::set_ir(uint32_t val) {
    ir = val;
}

int32_t RegisterState::get(uint32_t reg_num) const {
    if (reg_num >= 32) {
        cerr << "Error: Invalid register number: " << reg_num << endl;
        return 0;
    }
    return reg_file[reg_num];
}

void RegisterState::set(uint32_t reg_num, int32_t val) {
    if (reg_num >= 32) {
        cerr << "Error: Invalid register number: " << reg_num << endl;
        return;
    }
    
    if (reg_num != 0) {
        reg_file[reg_num] = val;
    }
}

int32_t RegisterState::get_temp(const string& reg_name) const {
    auto it = temp_registers.find(reg_name);
    if (it == temp_registers.end()) {
        cerr << "Error: Temporary register '" << reg_name << "' not found" << endl;
        return 0;
    }
    return it->second;
}

void RegisterState::set_temp(const string& reg_name, int32_t val) {
    temp_registers[reg_name] = val;
}