#include "parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>  
#include <regex>          
using namespace std;

// unordered map for mapping the instructions to instruction type
static unordered_map<string, InstructionType> instructionTypeMap = {
    // R format
    {"add", R_TYPE}, {"and", R_TYPE}, {"or", R_TYPE}, {"sll", R_TYPE}, {"slt", R_TYPE}, {"sra", R_TYPE}, {"srl", R_TYPE}, {"sub", R_TYPE}, {"xor", R_TYPE}, {"mul", R_TYPE}, {"div", R_TYPE}, {"rem", R_TYPE},
    
    // I format
    {"addi", I_TYPE}, {"andi", I_TYPE}, {"ori", I_TYPE}, {"lb", I_TYPE}, {"ld", I_TYPE}, {"lh", I_TYPE}, {"lw", I_TYPE}, {"jalr", I_TYPE},
    
    // S format
    {"sb", S_TYPE}, {"sw", S_TYPE}, {"sd", S_TYPE}, {"sh", S_TYPE},
    
    // SB format
    {"beq", SB_TYPE}, {"bne", SB_TYPE}, {"bge", SB_TYPE}, {"blt", SB_TYPE},
    
    // U format
    {"auipc", U_TYPE}, {"lui", U_TYPE},
    
    // UJ format
    {"jal", UJ_TYPE}
};

// unordered map which maps assembly directives to true
static unordered_map<string, bool> directiveMap = {
    {".text", true}, {".data", true}, {".byte", true}, {".half", true}, {".word", true}, {".dword", true}, {".asciiz", true}
};
 
// Checks if the token is a valid directive
bool isDirective(const string& token){  
    return !token.empty() && token[0] == '.' && directiveMap.find(token) != directiveMap.end();     // check if the token starts with . and is a directive
}

// Returns the instruction type based on the opcode
// returns UNKNOWN if the instruction is not recognised
InstructionType getInstructionType(const string& opcode){       
    if (instructionTypeMap.find(opcode) != instructionTypeMap.end()) {
        return instructionTypeMap[opcode];
    }
    if (isDirective(opcode)) {
        return DIRECTIVE;
    }
    return UNKNOWN;
}

// Function to remove any leading or trailing whitespaces from the string
string trimString(const string& str) {
    auto start= find_if_not(str.begin(), str.end(), [](char c) { return isspace(c); });             // find the index of first non-whitespace character 
    auto end = find_if_not(str.rbegin(), str.rend(), [](char c) { return isspace(c); }).base();     // find the index of last non-whitespace character 

    if (start >= end) return "";    // returns empty string if start and end overlap

    return string(start, end);
}

// Tokenizes the string 
// returns a vector of tokens extracted from the string
vector<string> splitString(const string& str){ 
    vector<string> tokens;
    istringstream ss(str);          // input string stream for the given string
    string token;

    while (ss >> token) {
        tokens.push_back(token);    // push all the strings from the input stream to tokens vector
    }

    return tokens;
}

// Remove comments from a given line (string)
string removeComments(const string& line){      
    size_t pos = line.find('#');
    
    if (pos != string::npos) {
        return line.substr(0, pos);     // if there is a comment, return the substring excluding the comment
    }

    return line;                        // if there is no comment, return as it is
}      

// Function to remove all symbols like , ( ) from the 
string refine(const string& line){               
    string res=line;

    // replace the symbols with whitespaces
    replace(res.begin(), res.end(), ',', ' ');
    replace(res.begin(), res.end(), '(', ' ');
    replace(res.begin(), res.end(), ')', ' ');

    return res;
}

// Function to parse a line of assembly code
// returns an instance of Instruction struct
Instruction parseLine(const string& rawline, int lineNumber) {
    Instruction instruction;
    string line=refine(rawline);                    // replace the symbols , ( ) with whitespaces
    string line1=removeComments(rawline);           // remove the comments

    instruction.lineNumber = lineNumber;
    instruction.hasLabel = false;

    line = trimString(removeComments(line));        // remove comments, leading and trailing whitespaces

    // if line is empty after removing comments, return UNKNOWN type instruction
    if (line.empty()) {
        instruction.type = EMPTY;
        return instruction;
    }

    size_t labelEnd = line.find(':');               // check if the line has a label
    if(labelEnd != string::npos) {
        // extract and store the label for the instruction
        instruction.label = trimString(line.substr(0,labelEnd));    
        instruction.hasLabel = true;

        // remove the label from instruction and trim any whitespaces
        line = trimString(line.substr(labelEnd+1));
    }

    // if line only contains a label, return LABEL type instruction
    if (line.empty()) {
        instruction.type = LABEL;
        return instruction;
    }

    size_t firstSpace = line.find(' ');
    instruction.opcode = trimString(line.substr(0, firstSpace));    // substring till the first whitespace is the opcode
    instruction.type = getInstructionType(instruction.opcode);      // identify the instrcution type using opcode

    string operandsStr = trimString(line.substr(firstSpace+1));     // the remaining string after opcode consists of operands


    if (instruction.opcode == ".asciiz") {                          // for asciiz type data 
        regex stringPattern("\"([^\"]*)\"");                        // string enclosed in double quotes
        smatch match;
        if (regex_search(line1, match, stringPattern)) {
            instruction.operands.push_back(match[0]);               // if string matches the pattern, push it to oeprands vector
        }   
    } else {
        //operandsStr = regex_replace(s, regex(",\\s*"), " ");
        instruction.operands = splitString(operandsStr);            // for other data type directive, split the operands string 
    }

    return instruction;
}

// Function to parse the entire file of assembly code
// returns the vector of Instruction type elements
vector<Instruction> parseFile(const string& filename) {
    vector<Instruction> instructions;
    ifstream file(filename); 

    // If file does not open, display error
    if (!file.is_open()) {
        cout << "Error: Could not open file : " << filename << endl;
        return instructions;
    }

    string line;
    int lineNumber = 1;     // initialize lineNumber to 1                                             

    while(getline(file, line)) {
        Instruction instr = parseLine(line, lineNumber);                                // parse the assembly code line by line
        if (instr.type != EMPTY || instr.hasLabel) instructions.push_back(instr);     // push only valid instructions to the output vector
        //instructions.push_back(instr);     // push all instructions to the output vector
        lineNumber++;       // increment line numbers after reading each line
    }

    file.close();           // close the file
    return instructions;    // return the instructions vector
}
