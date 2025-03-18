#ifndef REGISTER_STATE_H
#define REGISTER_STATE_H

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

class RegisterState {
private:
    vector<int32_t> regFile;  // 32 general purpose registers
    uint32_t pc;                    
    uint32_t ir;                    
      
    unordered_map<string, int32_t> tempRegisters;  // for other registers like RM, RY, RA, RB, etc...
    
public:
    RegisterState();
    
    // reset all registers to initial values
    void reset();
    
    uint32_t getPC() const;
    void setPC(uint32_t val);
    void incrementPC(int offset = 4);
    
    uint32_t getIR() const;
    void setIR(uint32_t val);
    
    // general purpose register operations
    int32_t get(uint32_t reg_num) const;
    void set(uint32_t reg_num, int32_t val);
    
    // temporary register operations
    int32_t getTemp(const string& reg_name) const;
    void setTemp(const string& reg_name, int32_t val);
    
    void printAll() const;
};

#endif 
