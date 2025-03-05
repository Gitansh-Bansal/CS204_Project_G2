#include "parser.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>  
#include <regex>          // for handling instruction format

using namespace std;

static unordered_map<string, InstructionType> instructionTypeMap = {
    // R format
    {"add", R_TYPE}, {"and", R_TYPE}, {"or", R_TYPE}, {"sll", R_TYPE}, 
    {"slt", R_TYPE}, {"sra", R_TYPE}, {"srl", R_TYPE}, {"sub", R_TYPE}, 
    {"xor", R_TYPE}, {"mul", R_TYPE}, {"div", R_TYPE}, {"rem", R_TYPE},
    
    // I format
    {"addi", I_TYPE}, {"andi", I_TYPE}, {"ori", I_TYPE}, {"lb", I_TYPE}, 
    {"ld", I_TYPE}, {"lh", I_TYPE}, {"lw", I_TYPE}, {"jalr", I_TYPE},
    
    // S format
    {"sb", S_TYPE}, {"sw", S_TYPE}, {"sd", S_TYPE}, {"sh", S_TYPE},
    
    // SB format
    {"beq", SB_TYPE}, {"bne", SB_TYPE}, {"bge", SB_TYPE}, {"blt", SB_TYPE},
    
    // U format
    {"auipc", U_TYPE}, {"lui", U_TYPE},
    
    // UJ format
    {"jal", UJ_TYPE}
};

static unordered_map<string, bool> directiveMap = {
    {".text", true}, {".data", true}, {".byte", true}, {".half", true}, {".word", true}, {".dword", true}, {".asciz", true}
};

bool isDirective(const string& token);

InstructionType getInstructionType(const string& opcode);

string trimString(const string& str);

vector<string> splitString(const string& str, char delimiter);

string removeComments(const string& line);

Instruction parseLine(const string& line, int lineNumber);

vector<Instruction> parseFile(const string& filename);
