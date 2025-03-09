#ifndef PARSER_H
#define PARSER_H
#include <string>
#include <vector>
using namespace std;

// defining the different types of instructions
enum InstructionType {
    R_TYPE,
    I_TYPE,
    S_TYPE,
    SB_TYPE,
    U_TYPE,
    UJ_TYPE,
    DIRECTIVE,
    LABEL,
    UNKNOWN
};

// defining the structure of an instruction
struct Instruction {
    string label;
    string opcode; 
    vector<std::string> operands;
    string comment;
    InstructionType type;      
    int lineNumber;  
    bool hasLabel; 
};

// defining the functions that will be used in the parser
bool isDirective(const string& token);
InstructionType getInstructionType(const string& opcode);
Instruction parseLine(const string& line, int lineNumber);
vector<Instruction> parseFile(const string& filename);
string trimString(const string& str);
vector<string> splitString(const string& str, char delimiter);
string removeComments(const string& line);
string refine(const string& line);

#endif 
