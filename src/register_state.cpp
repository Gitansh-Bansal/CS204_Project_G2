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