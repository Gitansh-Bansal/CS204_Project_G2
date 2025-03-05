#include "symboltable.h"
#include <iostream>
#include <iomanip>

using namespace std;

SymbolTable::SymbolTable()  
{
    textCursor = TEXT_SEGMENT_START;
    dataCursor = DATA_SEGMENT_START;
    
    currentSegment = TEXT;
}
  
void SymbolTable::addSymbol(const string& name, uint32_t address)  
{
    if (symbols.find(name) != symbols.end()) 
    {
        cerr << "Warning!! Symbol '" << name << "' already defined. Overwriting previous value." << endl;
    }
    symbols[name] = address;
}

uint32_t SymbolTable::getSymbolAddress(const string& name) const
{   
    //checking for symbols
    if (symbols.find(name) == symbols.end())
    {
        cerr << "Error!! Symbol '" << name << "' not found in symbol table." << endl;
        return 0; 
    }
    return symbols.at(name);
}          

bool SymbolTable::hasSymbol(const string& name) const
{
    return symbols.find(name) != symbols.end();
}       

void SymbolTable::setCurrentSegment(MemorySegment segment)      
{
    currentSegment = segment;
}

MemorySegment SymbolTable::getCurrentSegment() const 
{
    return currentSegment;
}

uint32_t SymbolTable::getCurrentAddress() const {
    switch (currentSegment) {
        case TEXT:
            return textCursor;
        case DATA:
            return dataCursor;
        case HEAP:
            return HEAP_SEGMENT_START; 
        case STACK:
            return STACK_SEGMENT_START; 
        default:
            return 0;
    }
}

void SymbolTable::incrementAddress(uint32_t increment) {
    switch (currentSegment) {
        case TEXT:
            textCursor += increment;
            break;
        case DATA:
            dataCursor += increment;
            break;
        // heap and stack addresses not handled by the assembler
        case HEAP:
        case STACK:
            cerr << "Warning!! Attempting to increment address in heap or stack segment." << endl;  //throw error
            break;
    }
}

void SymbolTable::resetCursors() {
    textCursor = TEXT_SEGMENT_START;
    dataCursor = DATA_SEGMENT_START;
}

void SymbolTable::printSymbolTable() const {
    // Print the symbol table for debugging
    cout << "Symbol Table:" << endl;
    cout << "-------------" << endl;
    cout << left << setw(20) << "Symbol" << "Address" << endl;
    cout << "-------------------------------" << endl;
    
    for (const auto& entry : symbols) {
        cout << left << setw(20) << entry.first 
                  << "0x" << hex << setw(8) << setfill('0') << right
                  << entry.second << dec << setfill(' ') << endl;
    }
    
    cout << "-------------------------------" << endl;
    cout << "Text Cursor: 0x" << hex << setw(8) << setfill('0') << textCursor << endl;
    cout << "Data Cursor: 0x" << hex << setw(8) << setfill('0') << dataCursor << endl;
    cout << dec << setfill(' ');
}
