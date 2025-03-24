# RISC-V Assembler and Simulator - CS204 Project

## Table of Contents
- [Project Overview](#risc-v-assembler-and-simulator---cs204-project)
- [Features](#features)
- [File Structure](#file-structure)
- [Input and Output Format](#input-and-output-format)
- [Working Mechanism](#working-mechanism)
- [Compilation and Execution](#compilation-and-execution)
- [Memory Layout](#memory-layout)
- [Team Members](#team-members)

---


## Project Overview  
This project implements a **32-bit RISC-V assembler and simulator** that translates assembly language instructions into **binary machine code** executable by a RISC-V processor. The assembler processes an input `.asm` file, converts instructions into machine-readable format using a **two-pass assembly mechanism**, and generates an output `.mc` file containing the encoded instructions along with their respective memory addresses and binary representations. The simulator executes the machine code by modeling the **five-stage instruction execution pipeline** (**Fetch, Decode, Execute, Memory Access, Writeback**), allowing users to run programs in **step-by-step mode** or **full execution mode** while tracking register updates, memory operations, and clock cycles. Additionally, a **Graphical User Interface (GUI)** using **wxWidgets** enhances usability by providing **real-time visualization** of instruction execution, register states, and memory contents, along with interactive controls for seamless debugging and program execution.

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
- **Symbol Table Management**: Tracks and keeps record of labels and memory locations. Also displays the final symbol table.
- **Two-Pass Assembly Process**: Ensures correct label resolution.
- **Error Reporting**: Detects and reports syntactic and semantic errors in the input.
- **Five-Stage Pipeline Simulation**: Implements **Fetch, Decode, Execute, Memory Access, Writeback** stages.
- **Step-by-Step Execution:** Run one instruction at a time for debugging.
- **Full Execution Mode:** Run the entire program automatically.
- **Register & Memory Inspection:** View values in registers and memory at any point.
- **Clock Cycle Tracking:** Displays the number of cycles taken for execution.
- **GUI-Based Simulation:**  
  - **Real-Time Visualization:** A wxWidgets-based graphical interface shows register updates, memory changes, console.  
  - **Interactive Controls:** Users can step through execution, run through whole program and inspect memory, registers visually.  
  - **Instruction Highlighting:** The GUI highlights the currently executing instruction. 

---

## File Structure
```
📂 Project Root  
├── 📂 include/               # Contains all header files  
│   ├── 📄 encoder.h          # Header file for instruction encoding  
│   ├── 📄 parser.h           # Header file for instruction parsing  
│   ├── 📄 symboltable.h      # Header file for symbol table management  
│   ├── 📄 simulator.h        # Header file for the simulator  
│   ├── 📄 memory.h           # Header file for memory management  
│   ├── 📄 register_state.h   # Header file for register state  
│   ├── 📄 wxRISCVSimulator.h # Header file for wxWidgets GUI
│  
├── 📂 src/                     # Contains all source files  
│   ├── 📄 main.cpp             # Main program handling file processing and execution  
│   ├── 📄 encoder.cpp          # Encodes instructions into machine code  
│   ├── 📄 parser.cpp           # Parses assembly instructions and extracts components  
│   ├── 📄 symboltable.cpp      # Manages symbol table  
│   ├── 📄 simulator.cpp        # Implements five-stage pipeline execution  
│   ├── 📄 memory.cpp           # Implements memory operations  
│   ├── 📄 register_state.cpp   # Implements register operations  
│   ├── 📄 wxRISCVSimulator.cpp # Implements wxWidgets GUI for visualization  
│  
├── 📄 input.asm           # Sample RISC-V assembly program  
├── 📄 output.mc           # Machine code output file  
├── 📄 README.md           # Project documentation  
```

---

## Input and Output Format for Phase 1

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
The program will generate the following `.mc` file as output:
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

### **Phase 1: Assembly Process**
1. **First Pass (Symbol Table Construction)**
   - Parses `.asm` file to extract labels and their corresponding memory addresses.
   - Stores them in a **symbol table**.

2. **Second Pass (Machine Code Generation)**
   - Converts each instruction into **binary machine code**.
   - Resolves labels using the **symbol table**.
   - Outputs the result to **`output.mc`**.

### **Phase 2: Instruction Execution (Simulation)**
The simulator reads the machine code from `output.mc` and executes instructions in **five pipeline stages**:
1. **Fetch** - Reads the instruction from memory using the Program Counter (PC).
2. **Decode** - Identifies the instruction type and extracts operands.
3. **Execute** - Performs computations such as arithmetic, logic, and branching.
4. **Memory Access** - Reads/writes data for load/store instructions.
5. **Register Update** - Writes back results to registers.


---

## Compilation and Execution

### **To Compile:**
```sh
g++ -std=c++17 `wx-config --cxxflags --libs` -o simulator.exe src/*.cpp -Iinclude
```

### **To Run:**
```sh
./simulator.exe     
```

---

## Team Members
- **Gitansh Bansal (2023MCB1294)**
- **Mohakjot Dhiman (2023MCB1302)**
- **Shaurya Anant (2023CSB1313)**

---

