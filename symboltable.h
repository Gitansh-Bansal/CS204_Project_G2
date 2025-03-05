#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <unordered_map>
#include <cstdint>

using namespace std;

const uint32_t TEXT_SEGMENT_START = 0x00000000;
const uint32_t DATA_SEGMENT_START = 0x10000000;
const uint32_t HEAP_SEGMENT_START = 0x10008000;
const uint32_t STACK_SEGMENT_START = 0x7FFFFDC;

enum MemorySegment {
    TEXT,
    DATA,
    HEAP,
    STACK
};

class SymbolTable {
private:
    unordered_map<string, uint32_t> symbols;
    
    uint32_t textCursor;
    uint32_t dataCursor;
    
    MemorySegment currentSegment;

public:
    SymbolTable();
    
    void addSymbol(const string& name, uint32_t address);
    uint32_t getSymbolAddress(const string& name);
    bool hasSymbol(const string& name);
    void setCurrentSegment(MemorySegment segment);
    MemorySegment getCurrentSegment();
    uint32_t getCurrentAddress();
    void incrementAddress(uint32_t increment);
    void resetCursors();
    void printSymbolTable();
};

#endif
