#include "memory.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

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

