#include "symboltable.h"
#include <iostream>
#include <iomanip>

using namespace std;

SymbolTable::SymbolTable()  // constructor function
{
    textCursor = TEXT_SEGMENT_START;
    dataCursor = DATA_SEGMENT_START;
    
    currentSegment = TEXT;
}
  
void SymbolTable::addSymbol(const string& name, uint32_t address)  // function to add symbol
{
    if (symbols.find(name) != symbols.end()) 
    {
        cerr << "Warning!! Symbol '" << name << "' already defined. Overwriting previous value." << endl;
    }
    symbols[name] = address;
}

uint32_t SymbolTable::getSymbolAddress(const string& name) // get symbol address
{   
    //checking for symbols
    if (symbols.find(name) == symbols.end())
    {
        cerr << "Error!! Symbol '" << name << "' not found in symbol table." << endl;
        return 0; 
    }
    return symbols.at(name);
}          

bool SymbolTable::hasSymbol(const string& name)        // check if the symbol is there in table

void SymbolTable::setCurrentSegment(MemorySegment segment)      // change current segment

MemorySegment SymbolTable::getCurrentSegment()      // returns current segment

uint32_t SymbolTable::getCurrentAddress() {
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
            cerr << "Warning: Attempting to increment address in heap or stack segment." << endl;  //throw error
            break;
    }
}

void SymbolTable::resetCursors() {
    textCursor = TEXT_SEGMENT_START;
    dataCursor = DATA_SEGMENT_START;
}

void SymbolTable::printSymbolTable()          // print sumbol table
