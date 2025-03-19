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

uint8_t Memory::readByte(uint32_t address) const {
    auto it = mem.find(address);
    if (it != mem.end()) {
        return it->second;
    }
    return 0;
}

void Memory::writeByte(uint32_t address, uint8_t value) {
    mem[address] = value;
}

uint32_t Memory::readWord(uint32_t address) const {
    uint32_t word = 0;
    word |= static_cast<uint32_t>(readByte(address)) << 0;
    word |= static_cast<uint32_t>(readByte(address + 1)) << 8;
    word |= static_cast<uint32_t>(readByte(address + 2)) << 16;
    word |= static_cast<uint32_t>(readByte(address + 3)) << 24;
    return word;
}

void Memory::writeWord(uint32_t address, uint32_t value) {
    writeByte(address, (value >> 0) & 0xFF);
    writeByte(address + 1, (value >> 8) & 0xFF);
    writeByte(address + 2, (value >> 16) & 0xFF);
    writeByte(address + 3, (value >> 24) & 0xFF);
}

uint16_t Memory::readHalf(uint32_t address) const {
    uint16_t half = 0;
    half |= static_cast<uint16_t>(readByte(address)) << 0;
    half |= static_cast<uint16_t>(readByte(address + 1)) << 8;
    return half;
}

void Memory::writeHalf(uint32_t address, uint16_t value) {
    writeByte(address, (value >> 0) & 0xFF);
    writeByte(address + 1, (value >> 8) & 0xFF);
}

bool Memory::loadFromFile(const string& filename) {

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
                writeByte(address, static_cast<uint8_t>(value));
            }
             else
            {
                writeWord(address, value);
            }
        } catch (const exception& e) {
            cerr << "Error parsing line: " << line << endl;
            continue;
        }
    }
    
    file.close();
    return true;
}

void Memory::printMemory(uint32_t start_addr, uint32_t end_addr) const {
    
    cout << "Memory dump from 0x" << hex << start_addr 
              << " to 0x" << end_addr << ":" << endl;
    
    for (uint32_t addr = start_addr; addr <= end_addr; addr += 16) {
        cout << "0x" << hex << setw(8) << setfill('0') << addr << ": ";
        
        // Print hex values
        for (int i = 0; i < 16; i++) {
            cout << setw(2) << setfill('0') 
                      << static_cast<int>(readByte(addr + i)) << " ";
            if (i == 7) {
                cout << " ";  
            }
        }
        
        // Print ASCII representation
        cout << " |";
        for (int i = 0; i < 16; i++) {
            uint8_t byte = readByte(addr + i);
            char c = (byte >= 32 && byte <= 126) ? static_cast<char>(byte) : '.';
            cout << c;
        }
        cout << "|" << endl;
    }
    
    cout << dec; 
}


