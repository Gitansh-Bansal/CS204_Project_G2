#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <string>
#include <unordered_map>

using namespace std;

class Memory {
private:
    unordered_map<uint32_t, uint8_t> memoryMap;
    
public:
    Memory();
    
    // reset memory to initial state
    void reset();
    
    uint8_t read_byte(uint32_t address) const;
    void write_byte(uint32_t address, uint8_t value);
    
    uint32_t read_word(uint32_t address) const;
    void write_word(uint32_t address, uint32_t value);
    
    uint16_t read_half(uint32_t address) const;
    void write_half(uint32_t address, uint16_t value);
    
    bool load_from_file(const string& filename);        // load memory contents from .mc fil
    
    void printMemory(uint32_t start_addr, uint32_t end_addr) const;    // print memory contents for debugging
};

#endif
