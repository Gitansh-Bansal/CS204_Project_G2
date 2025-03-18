#include "memory.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
using namespace std;

Memory::Memory() {
    reset();
}

void Memory::reset() {
    mem.clear();
}

uint8_t Memory::read_byte(uint32_t address) const {
    auto it = mem.find(address);
    if (it != mem.end()) {
        return it->second;
    }
    return 0;
}

void Memory::write_byte(uint32_t address, uint8_t value) {
    mem[address] = value;
}

uint32_t Memory::read_word(uint32_t address) const {
    uint32_t word = 0;
    word |= static_cast<uint32_t>(read_byte(address)) << 0;
    word |= static_cast<uint32_t>(read_byte(address + 1)) << 8;
    word |= static_cast<uint32_t>(read_byte(address + 2)) << 16;
    word |= static_cast<uint32_t>(read_byte(address + 3)) << 24;
    return word;
}

void Memory::write_word(uint32_t address, uint32_t value) {
    write_byte(address, (value >> 0) & 0xFF);
    write_byte(address + 1, (value >> 8) & 0xFF);
    write_byte(address + 2, (value >> 16) & 0xFF);
    write_byte(address + 3, (value >> 24) & 0xFF);
}

uint16_t Memory::read_half(uint32_t address) const {
    uint16_t half = 0;
    half |= static_cast<uint16_t>(read_byte(address)) << 0;
    half |= static_cast<uint16_t>(read_byte(address + 1)) << 8;
    return half;
}

void Memory::write_half(uint32_t address, uint16_t value) {
    write_byte(address, (value >> 0) & 0xFF);
    write_byte(address + 1, (value >> 8) & 0xFF);
}

bool Memory::load_from_file(const string& filename) {

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return false;
    }
    
    string line;
    while (getline(file, line)) {
        
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        istringstream iss(line);
        string addr_str, value_str;
        
        // Parse address and value
        if (!(iss >> addr_str >> value_str)) {
            continue;  
        }
        
        // Convert hex strings to integers
        uint32_t address, value;
        try {
            address = stoul(addr_str, nullptr, 16);
            value = stoul(value_str, nullptr, 16);
            
            if (value <= 0xFF) 
            {
                write_byte(address, static_cast<uint8_t>(value));
            }
             else
            {
                write_word(address, value);
            }
        } catch (const exception& e) {
            cerr << "Error parsing line: " << line << endl;
            continue;
        }
    }
    
    file.close();
    return true;
}


