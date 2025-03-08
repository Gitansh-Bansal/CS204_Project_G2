#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "parser.h"
#include "encoder.h"
#include "symboltable.h"

int main() {
    string inputFile = "test1.asm";
    string outputFile = "output.mc";

    SymbolTable symbolTable;

    // first pass
    cout << "First pass : Building symbol table..." << endl;


    vector<Instruction> instructions = parseFile(inputFile);
    if (instructions.empty()) {
        cerr << "Error: No instructions found or file could not be opened." << endl;
        return 1;
    }

    symbolTable.setCurrentSegment(TEXT);

    for (const auto& instr : instructions) {
        if (instr.type!=DIRECTIVE) {
            symbolTable.setCurrentSegment(TEXT);
        }

        // add label to symbol table
        if (instr.hasLabel && instr.type!=DIRECTIVE) {
            symbolTable.addSymbol(instr.label, symbolTable.getCurrentAddress());
        }
        
        if (instr.type == DIRECTIVE) {
            if (instr.opcode == ".text") {              // start of text segment
                symbolTable.setCurrentSegment(TEXT);
            } else if (instr.opcode == ".data") {       // start of data segment
                symbolTable.setCurrentSegment(DATA);
            } 
            else{
                symbolTable.setCurrentSegment(DATA);
                if (instr.hasLabel) symbolTable.addSymbol(instr.label, symbolTable.getCurrentAddress());

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
                    
                    symbolTable.incrementAddress(str.size() + 1); // +1 for null terminator
                }
            }
        } else if (instr.type != LABEL) {
            symbolTable.incrementAddress(4);
        }
    }

    // symbolTable.printSymbolTable();

    // second pass
    cout << "Second pass : Generating machine code..." << endl;

    symbolTable.resetCursors();
    symbolTable.setCurrentSegment(TEXT);

    ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        cerr << "Error: Output file " << outputFile << " could not be opened" << endl;
        return 1;
    }

    for (const auto& instr : instructions) {
        uint32_t currentAddress = symbolTable.getCurrentAddress();

        if (instr.type == DIRECTIVE) {
            if (instr.opcode == ".text") {
                symbolTable.setCurrentSegment(TEXT);
            } else if (instr.opcode == ".data") {
                symbolTable.setCurrentSegment(DATA);
            } else if (instr.opcode == ".byte" || instr.opcode == ".half" || instr.opcode == ".word" || instr.opcode == ".dword" || instr.opcode == ".asciiz") {
                symbolTable.setCurrentSegment(DATA);
                currentAddress = symbolTable.getCurrentAddress();
                vector<uint8_t> data = encodeDirective(instr);
                
                // // write data bytes to output
                // for (size_t i = 0; i < data.size(); i++) {
                //     if (i % 4 == 0) {
                //         // Start a new line for each word
                //         if (i > 0) outFile << endl;
                //         outFile << intToHex(currentAddress + i) << " ";
                //     }
                //     outFile << hex << setw(2) << setfill('0') 
                //             << static_cast<int>(data[i]);
                // }

                for (size_t i = 0; i < data.size(); i++) { 
                    if (i % 4 == 0) {
                        if (i > 0) outFile << endl;
                        outFile << intToHex(currentAddress + i) << " ";
                    }
                    
                    char buffer[3];             // buffer to store hex representation
                    snprintf(buffer, sizeof(buffer), "%02X", static_cast<int>(data[i]));
                    outFile << buffer << " ";
                }
                
                outFile << " , " << instr.opcode;
                for (size_t i = 0; i < instr.operands.size(); i++) {
                    outFile << " " << instr.operands[i];
                    if (i < instr.operands.size() - 1) outFile << ",";
                }
                outFile << endl;
                
                symbolTable.incrementAddress(data.size());
            }
        } else if (instr.type != LABEL && instr.type != UNKNOWN) {
            symbolTable.setCurrentSegment(TEXT);
            currentAddress = symbolTable.getCurrentAddress();
            
            uint32_t machineCode = encodeInstruction(instr, currentAddress, symbolTable);
            
            string encodingComment = generateEncodingComment(instr, machineCode);
            
            string outputLine = formatOutputLine(currentAddress, machineCode, instr, encodingComment);
            outFile << outputLine << endl;
            
            symbolTable.incrementAddress(4);
        }
    }

    outFile.close();

    cout << "Output successfully written to " << outputFile << endl;
    return 0;
}