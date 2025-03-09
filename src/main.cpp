#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include "parser.h"
#include "encoder.h"
#include "symboltable.h"

int main() {
    string inputFile = "input.asm";
    string outputFile = "output.mc";
    SymbolTable symbolTable;

    // -------------------------- FIRST PASS -------------------------
    cout << "\nFirst pass : Building symbol table..." << endl;

    vector<Instruction> instructions = parseFile(inputFile);                     // parse the input file (done in parser.cpp)
    if (instructions.empty()) {
        cerr << "Error: No instructions found or file could not be opened." << endl;
        return 1;
    }

    symbolTable.setCurrentSegment(TEXT);                                          // start with text segment (default)

    // now we go to each instruction, check its type and update the symbol table accordingly
    for (const auto& instr : instructions) {
        if (instr.type!=DIRECTIVE) {
            symbolTable.setCurrentSegment(TEXT);
        }

        if (instr.hasLabel && instr.type!=DIRECTIVE) {
            symbolTable.addSymbol(instr.label, symbolTable.getCurrentAddress());  // add label to symbol table
        }
        
        if (instr.type == DIRECTIVE) {
            if (instr.opcode == ".text") {              
                symbolTable.setCurrentSegment(TEXT);    // switch to the text segment
            } else if (instr.opcode == ".data") {       // switch to the data segment
                symbolTable.setCurrentSegment(DATA);
            } 
            else{
                symbolTable.setCurrentSegment(DATA);
                if (instr.hasLabel) {
                    symbolTable.addSymbol(instr.label, symbolTable.getCurrentAddress());     // add label to symbol table
                }

                // increment address of data cursor based on directive
                if (instr.opcode == ".byte") {
                    symbolTable.incrementAddress(instr.operands.size());
                } else if (instr.opcode == ".half") {
                    symbolTable.incrementAddress(2 * instr.operands.size());
                } else if (instr.opcode == ".word") {
                    symbolTable.incrementAddress(4 * instr.operands.size());
                } else if (instr.opcode == ".dword") {
                    symbolTable.incrementAddress(8 * instr.operands.size());
                } else if (instr.opcode == ".asciiz") {
                    string str = instr.operands[0];
                    if (str.size() >= 2 && str.front() == '"' && str.back() == '"') str = str.substr(1, str.size() - 2);    // remove double quotes
                    
                    symbolTable.incrementAddress(str.size() + 1);     // +1 for null terminator
                }
            }
        } else if (instr.type != LABEL) {
            symbolTable.incrementAddress(4);         // if the instruction is not a label or directive, increment address of text cursor by 4
        }
    }

    // print the final Symbol Table after completion of first pass
    cout<<endl;
    symbolTable.printSymbolTable();

    // -------------------------- SECOND PASS -------------------------
    cout << "\nSecond pass : Generating machine code..." << endl;

    // reset the data and text cursors to the beginning and reset the segment to text
    symbolTable.resetCursors();
    symbolTable.setCurrentSegment(TEXT);

    ofstream outFile(outputFile);          // open the output file
    if (!outFile.is_open()) {
        cerr << "Error: Output file " << outputFile << " could not be opened" << endl;
        return 1;
    }
    stringstream memoryStream;             // stream to store the memory contents

    // instruction for termination
    Instruction instr;
    instr.type = UNKNOWN;
    instr.opcode = "terminate";
    instr.operands = {};
    instr.label = "";
    instr.hasLabel = false;
    instr.lineNumber = 0;
    instructions.push_back(instr);

    // now we go to each instruction, check its type and generate the machine code accordingly
    for (const auto& instr : instructions) {
        uint32_t currentAddress = symbolTable.getCurrentAddress(); // get the current address

        if (instr.type == DIRECTIVE) {
            if (instr.opcode == ".text") {
                symbolTable.setCurrentSegment(TEXT); 
            } else if (instr.opcode == ".data") {
                symbolTable.setCurrentSegment(DATA); 
            } else if (instr.opcode == ".byte" || instr.opcode == ".half" || instr.opcode == ".word" || instr.opcode == ".dword" || instr.opcode == ".asciiz") {
                symbolTable.setCurrentSegment(DATA); 
                currentAddress = symbolTable.getCurrentAddress(); 
                vector<uint8_t> data = encodeDirective(instr);               // returns a vector of bytes containing the data to be added to the data memory

                // write the data to the output file
                for (size_t i = 0; i < data.size(); i++) { 
                    memoryStream << intToHex(currentAddress + i) << " 0x";
                    memoryStream << hex << right <<setw(2) << setfill('0') << static_cast<int>(data[i]);
                    memoryStream << endl;
                }
                symbolTable.incrementAddress(data.size());                   // increment the address of the data cursor
            }
        }
        else if (instr.type != LABEL && instr.type != UNKNOWN) {             // if the instruction is a normal text instruction
            symbolTable.setCurrentSegment(TEXT);                             // switch to the text segment

            currentAddress = symbolTable.getCurrentAddress(); 
            uint32_t machineCode = encodeInstruction(instr, currentAddress, symbolTable);         // generate the machine code for the instruction (done in encoder.cpp)
            string encodingComment = generateEncodingComment(instr, machineCode);                 // generate the encoding comment for the instruction (done in encoder.cpp)

            string outputLine = formatOutputLine(currentAddress, machineCode, instr, encodingComment); // format the output line according to the format
            outFile << outputLine << endl; 
            
            symbolTable.incrementAddress(4); // increment the address of the text cursor
        }
        else if (instr.type == UNKNOWN && instr.opcode == "terminate") {
            symbolTable.setCurrentSegment(TEXT); 
            currentAddress = symbolTable.getCurrentAddress(); 
            uint32_t machineCode = -1;                                         // generate the machine code for the instruction (done in encoder.cpp)
            string encodingComment = "Terminate";                              // generate the encoding comment for the instruction (done in encoder.cpp)
            outFile << formatOutputLine(currentAddress, machineCode, instr, encodingComment) << endl;    // write the output line to the output file
            break;
        }
    }
    
    outFile << endl << memoryStream.str(); // write the data memory contents to the output file
    outFile.close();
    cout << "Output successfully written to " << outputFile << endl << endl;
    return 0;
}
