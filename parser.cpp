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
 
bool isDirective(const string& token);      // check if the string is in directiveMap

InstructionType getInstructionType(const string& opcode);       // return the instruction type from opcode

string trimString(const string& str);       // remove any leading or trailing whitespaces

vector<string> splitString(const string& str){
    vector<string> tokens;
    std::istringstream ss(str);
    string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}  // split the string by given delimiter

string removeComments(const string& line)
// remove comments from line
{
    size_t pos = line.find('#');
    if (pos != string::npos) {
        return line.substr(0, pos);
    }
    return line;
}      

void removeCommas(string& line){               // remove commas from line
    std::replace(line.begin(), line.end(), ',', ' ');
}

Instruction parseLine(const string& line, int lineNumber) {
    Instruction instruction;
    removeCommas(line);
    instruction.lineNumber = lineNumber;
    instruction.hasLabel = false;

    line = trimString(removeComments(line));        // remove comments, leading and trailing whitespaces

    if (line.empty()) {
        instruction.type = UNKNOWN;
        return instruction;
    }

    size_t labelEnd = line.find(':');               // check if the line has a label
    if(labelEnd != string::nops) {
        instruction.label = trimstring(line.substr(0,labelEnd));
        instruction.hasLabel = true;
        line = trimString(line.substr(labelEnd+1));
    }

    if (line.empty()) {
        instruction.type = LABEL;
        return instruction;
    }

    size_t firstSpace = line.find(' ');
    instruction.opcode = trimString(line.substr(0, firstSpace));
    instruction.type = getInstructionType(instruction.opcode);

    operandsStr = trimString(line.substr(firstSpace+1));


    if (instruction.opcode == ".asciiz") {
        regex stringPattern("\"([^\"]*)\"");        // string enclosed in double quotes
        smatch match;
        if (regex_search(operandsStr, match, stringPattern)) {
            instruction.operands.push_back(match[0]);
        }   
    } else {
        //operandsStr = regex_replace(s, regex(",\\s*"), " ");
        instruction.operands = splitString(operandsStr, ' ');
    }

    return instruction;
}

vector<Instruction> parseFile(const string& filename) {
    vector<Instruction> instructions;
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open file : " << filename << endl;
        return instructions;
    }

    string line;
    int lineNumber = 0;

    while(getline(file, line)) {
        Instruction instr = parseLine(line, lineNumber);
        if (instr.type != UNKNOWN || instr.hasLabel) {
            instructions.push_back(instr);
        }
        lineNumber++;
    }

    file.close();
    return instructions;
}
