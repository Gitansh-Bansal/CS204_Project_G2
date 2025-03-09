# RISC-V Assembler - CS204 Project

## Table of Contents
- [Project Overview](#risc-v-assembler---cs204-project)
- [Features](#features)
- [File Structure](#file-structure)
- [Input and Output Format](#input-and-output-format)
- [Working Mechanism](#working-mechanism)
- [Compilation and Execution](#compilation-and-execution)
- [Memory Layout](#memory-layout)
- [Team Members](#team-members)

---

## Project Overview
This project implements a **32-bit RISC-V assembler** that translates assembly language instructions into **binary machine code** executable by a RISC-V processor. The assembler processes an input `.asm` file, converts instructions into machine-readable format using a **two-pass assembly mechanism**, and generates an output `.mc` file containing the encoded instructions along with their respective memory addresses and binary representations. The assembler supports **31 core RISC-V instructions** across multiple formats (R, I, S, SB, U, and UJ) and includes support for essential **assembler directives** for data management.

Adhering to the **RISC-V 32-bit Instruction Set Architecture (ISA)**, the assembler efficiently manages labels through a **Symbol Table**, ensuring accurate instruction encoding and memory address resolution. The structured output format enhances code debugging and analysis. This project provides a comprehensive understanding of the assembly process and serves as an essential tool for bridging high-level assembly code and low-level machine execution.

---

## Features
- **Supports 31 RISC-V Instructions**  
  - **R-type**: `add, and, or, sll, slt, sra, srl, sub, xor, mul, div, rem`  
  - **I-type**: `addi, andi, ori, lb, ld, lh, lw, jalr`  
  - **S-type**: `sb, sw, sd, sh`  
  - **SB-type**: `beq, bne, bge, blt`  
  - **U-type**: `auipc, lui`  
  - **UJ-type**: `jal`  
- **Assembler Directives Support**:  
  `.text, .data, .byte, .half, .word, .dword, .asciiz`  
- **Symbol Table Management**: Tracks and keeps record of labels and memory locations. Also displaying final symbol table. 
- **Two-Pass Assembly Process**: Ensures correct label resolution.
- **Error Reporting**: Detects and Reports Syntactic and Semantic Errors in the input (Incorrect Instruction Formats, Immediate Overflows, Undefined Labels, Invalid Registers, etc.).
- **Conprehensive File Structure**: Ensures proper understandability and and modularity of the code.
  
---

## File Structure
```
📂 Project Root  
├── 📂 include/            # Contains all header files  
│   ├── 📄 encoder.h       # Header file for instruction encoding  
│   ├── 📄 parser.h        # Header file for instruction parsing  
│   ├── 📄 symboltable.h   # Header file for symbol table management  
│  
├── 📂 src/                # Contains all source files  
│   ├── 📄 main.cpp        # Main program handling file processing and instruction encoding  
│   ├── 📄 encoder.cpp     # Encodes instructions into machine code  
│   ├── 📄 parser.cpp      # Parses assembly instructions and extracts components  
│   ├── 📄 symboltable.cpp # Manages symbol table (labels, memory addresses)  
│  
├── 📄 input.asm           # Sample RISC-V assembly program  
├── 📄 output.mc           # Machine code output file  
├── 📄 README.md           # Project documentation  

```

---

## Input and Output Format

#### **Input Format (`input.asm`):**
The `.asm` file contains **one instruction per line**:
```
.data
arr: .byte 13, 25, 57
.text
add x1, x2, x3
andi x5, x6, 10
```

#### **Output Format (`output.mc`):**
The program will give the following `.mc` file in Output:
```
0x00000000 0x003100b3 , add x1,x2,x3 # 0110011-000-0000000-00001-00010-00011-NULL
0x00000004 0x00a37293 , andi x5,x6,10 # 0010011-111-NULL-00101-00110-000000001010
0x00000008 0xffffffff , terminate  # Terminate

0x10000000 0x0d
0x10000001 0x19
0x10000002 0x39

```

---

## Working Mechanism

1. **First Pass (Symbol Table Construction)**
   - Parses `.asm` file to extract labels and their corresponding memory addresses.
   - Stores them in a **symbol table**.

2. **Second Pass (Machine Code Generation)**
   - Converts each instruction into **binary machine code**.
   - Resolves labels using the **symbol table**.
   - Outputs the result to **`output.mc`**.

---

## Compilation and Execution

### **To Compile:**
Use **G++** to compile the assembler in root directory:
```sh
g++ -o assembler src/*.cpp -I include
```

### **To Run:**
```sh
./assembler
```
This will generate an `output.mc` file containing the assembled machine code corresponding to the Assembly Code in `input.asm` file.

---

## Memory Layout
The assembler follows a predefined memory layout:
| Segment  | Start Address |
|----------|--------------|
| **Text** (Code) | `0x00000000` |
| **Data** (Static Data) | `0x10000000` |
| **Heap** (Dynamic Memory) | `0x10008000` |
| **Stack** (Function Calls) | `0x7FFFFFDC` |

---

## Team Members
- **Gitansh Bansal (2023MCB1294)**
- **Mohakjot Dhiman (2023MCB1302)**
- **Shaurya Anant (2023CSB1313)**

---



