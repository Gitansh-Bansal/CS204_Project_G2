#include "symboltable.h"
#include <iostream>
#include <iomanip>

using namespace std;

SymbolTable::SymbolTable()  // constructor function
  
void SymbolTable::addSymbol(const string& name, uint32_t address)  // function to add symbol

uint32_t SymbolTable::getSymbolAddress(const string& name)          // get symbol address

bool SymbolTable::hasSymbol(const string& name)        // check if the symbol is there in table

void SymbolTable::setCurrentSegment(MemorySegment segment)      // change current segment

MemorySegment SymbolTable::getCurrentSegment()      // returns current segment

uint32_t SymbolTable::getCurrentAddress()      // current address in text/data

void SymbolTable::incrementAddress(uint32_t increment)      // increment the address

void SymbolTable::resetCursors()              // reset text / data cursor

void SymbolTable::printSymbolTable()          // print sumbol table
