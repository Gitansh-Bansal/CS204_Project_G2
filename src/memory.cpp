#include "memory.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
using namespace std;

Memory::Memory() {
    memoryMap.clear();
    cout << "Memory Initialized!" << endl;
}

void Memory::reset() {
    memoryMap.clear();
    cout<<"Memory Cleared."<<endl;
}

uint8_t Memory::readByte(uint32_t address) const {
    auto it = memoryMap.find(address);
    if (it != memoryMap.end()) {
        return it->second;
    }
    return 0;
}

void Memory::writeByte(uint32_t address, uint8_t value) {
    memoryMap[address] = value;
    cout<<hex<<"Memory Write: 0x"<<address<<" = 0x"<<(int)value<<endl;
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
                memoryMap[address] = value;
            }
            else
            {
                memoryMap[address] = ((value >> 0) & 0xFF);
                memoryMap[address + 1] = ((value >> 8) & 0xFF);
                memoryMap[address + 2] = ((value >> 16) & 0xFF);
                memoryMap[address + 3] = ((value >> 24) & 0xFF);
            }
        } catch (const exception& e) {
            cerr << "Error parsing line: " << line << endl;
            continue;
        }
    }
    cout<<"Instruction and Data Memory Loaded."<<endl;
    file.close();
    return true;
}

void Memory::printMemory(uint32_t start_addr, uint32_t end_addr, char format) const {
    // Ensure valid range
    if (start_addr > end_addr) {
        cerr << "Error: Invalid memory range!" << endl;
        return;
    }
    cout << endl;
    start_addr -= start_addr % 4; // Align to word boundary
    end_addr += 4 - end_addr % 4; // Align to word boundary
    // add a header |Address| +3 | +2 | +1 | +0 |
    if (format == 'h') {
        cout << "|  Address  | +3 | +2 | +1 | +0 |" << endl;
        cout << "|-----------|----|----|----|----|" << endl;
    } else if (format == 'a') {
        cout << "|  Address  |  +3  |  +2  |  +1  |  +0  |" << endl;
        cout << "|-----------|------|------|------|------|" << endl;
    } else if (format == 'd') {
        cout << "|  Address  |  +3 |  +2 |  +1 |  +0 |" << endl;
        cout << "|-----------|-----|-----|-----|-----|" << endl;
    } else {
        cerr << "Error: Invalid format!" << endl;
        return;
    }
    // Iterate in reverse order
    for (uint32_t addr = end_addr; addr >= start_addr; addr -= 4) {
        cout << "| 0x" << hex << setw(8) << setfill('0') << addr << "| ";

        // Print hex values
        if (format == 'h') {
            for (int i = 3; i >= 0; i--) { 
                cout << setw(2) << setfill('0') 
                     << static_cast<int>(readByte(addr + i)) << " | ";
            }
        }  

        // Print ASCII or hex if not valid
        else if (format == 'a') {
            for (int i = 3; i >= 0; i--) { 
                uint8_t byte = readByte(addr + i);
                if (byte >= 32 && byte <= 126) {
                    cout << "   " << static_cast<char>(byte) << " | ";
                } else {
                    cout << "0x" << hex << setw(2) << setfill('0') 
                         << static_cast<int>(byte) << " | ";
                }
            }
        }

        // Print decimal
        else if (format == 'd') {
            for (int i = 3; i >= 0; i--) { 
                int8_t signedByte = static_cast<int8_t>(readByte(addr + i)); // ✅ Correct sign extension
                cout << dec << setw(3) << setfill(' ') << static_cast<int>(signedByte) << " | ";
            }
        }
        else {
            cerr << "Error: Invalid format!" << endl;
            return;
        }

        cout << endl;
        if (addr < 4) break; // ✅ Prevent underflow when addr = 0
    }
    
    cout << dec; 
}



