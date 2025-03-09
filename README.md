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
- [Notes](#notes)

---

## Project Overview
This project is a **32-bit RISC-V assembler** that translates assembly language instructions into machine code, similar to the **Venus assembler**. It processes an input `.asm` file and generates an output `.mc` file with encoded machine instructions. The assembler follows the **RISC-V 32-bit Instruction Set Architecture (ISA)** and supports **31 core instructions** across different formats (R, I, S, SB, U, and UJ).

The generated `.mc` file includes memory addresses, encoded instructions, and comments for easier debugging.

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
  `.text, .data, .byte, .half, .word, .dword, .asciz`  
- **Symbol Table Management**: Tracks labels and memory locations.  
- **Two-Pass Assembly Process**: Ensures correct label resolution.  
- **Venus-style Output Format**: Provides clear and structured machine code output.  

---

## File Structure
```
📂 Project Root
│── 📄 main.cpp            # Main program handling file processing and instruction encoding
│── 📄 parser.cpp          # Parses assembly instructions and extracts components
│── 📄 parser.h            # Header file for instruction parsing
│── 📄 encoder.cpp         # Encodes instructions into machine code
│── 📄 encoder.h           # Header file for instruction encoding
│── 📄 symboltable.cpp     # Manages symbol table (labels, memory addresses)
│── 📄 symboltable.h       # Header file for the symbol table
│── 📄 test1.asm           # Sample RISC-V assembly program
│── 📄 output.mc           # Machine code output file
│── 📄 README.md           # Project documentation
```

---

## Input and Output Format

### **Input Format (`test1.asm`)**
The `.asm` file contains **one instruction per line**:
```
add x1, x2, x3
andi x5, x6, 10
```

### **Output Format (`output.mc`)**
Each line follows the format:
```
<memory_address> <machine_code> , <assembly_instruction> # <binary_encoding>
```
**Example:**
```
0x0 0x003100B3 , add x1,x2,x3 # 0110011-000-0000000-00001-00010-00011-NULL
0x4 0x00A37293 , andi x5,x6,10 # 0010011-111-NULL-00101-00110-000000001010
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
Use **G++** to compile the assembler:
```sh
g++ -o assembler main.cpp parser.cpp encoder.cpp symboltable.cpp
```

### **To Run:**
```sh
./assembler
```
This will generate an `output.mc` file containing the assembled machine code.

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
- **Gitansh Bansal**
- **Mohakjot Dhiman**
- **Shaurya Anant**

---

## Notes
- **Pseudo-instructions are not supported**.
- **Floating-point operations are not included**.
- Labels are resolved using a **two-pass approach**.
- The assembler assumes **aligned memory access** for instructions and data.

---



