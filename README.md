
# RISC-V Assembler and Simulator - CS204 Project

## Table of Contents
- [Project Overview](#project-overview)
- [Features](#features)
- [File Structure](#file-structure)
- [Input and Output Format](#input-and-output-format)
- [Working Mechanism](#working-mechanism)
  - [Phase 1: Assembly Process](#phase-1-assembly-process)
  - [Phase 2: Instruction Execution (Simulation)](#phase-2-instruction-execution-simulation)
  - [Phase 3: Pipelined Execution](#phase-3-pipelined-execution)
- [Compilation and Execution](#compilation-and-execution)
- [Team Members](#team-members)

---

## Project Overview
This project implements a **32-bit RISC-V assembler and simulator** that translates assembly language instructions into **binary machine code** executable by a RISC-V processor. The assembler processes an input `.asm` file, converts instructions into machine-readable format using a **two-pass assembly mechanism**, and generates an output `.mc` file containing the encoded instructions along with their respective memory addresses and binary representations. The simulator executes the machine code by modeling the **five-stage instruction execution pipeline** (**Fetch, Decode, Execute, Memory Access, Writeback**), allowing users to run programs in **step-by-step mode** or **full execution mode** while tracking register updates, memory operations, and clock cycles. Additionally, a **Graphical User Interface (GUI)** using **wxWidgets** enhances usability by providing **real-time visualization** of instruction execution, register states, and memory contents, along with interactive controls for seamless debugging and program execution. In **Phase 3**, the simulator is enhanced to implement a **pipelined architecture** with realistic behavior including **data hazards, control hazards, data forwarding, pipeline stalling**, and **branch prediction**. 
---

## Features
- 🔧 **31 Supported RISC-V Instructions**
  - **R-type**: `add, and, or, sll, slt, sra, srl, sub, xor, mul, div, rem`  
  - **I-type**: `addi, andi, ori, lb, ld, lh, lw, jalr`  
  - **S-type**: `sb, sw, sd, sh`  
  - **SB-type**: `beq, bne, bge, blt`  
  - **U-type**: `auipc, lui`  
  - **UJ-type**: `jal` 
- 📜 **Assembler Directives** (`.text`, `.data`, `.byte`, etc.)
- 🧠 **Two-Pass Assembly**
- ❌ **Syntax and Semantic Error Reporting**
- ⚙️ **Pipeline Simulation with Enhanced Logic**
  - Pipeline registers between each stage
  - Data forwarding logic
  - Stall and flush mechanisms
  - Dynamic 1-bit branch prediction (BTB + PHT)
- 🎛️ **Execution Control Knobs**:
  - `Knob1`: Enable/disable pipelining  
  - `Knob2`: Enable/disable data forwarding  
  - `Knob3`: Print register file per cycle  
  - `Knob4`: Print all pipeline register info per cycle  
  - `Knob5`: Print pipeline info for specific instruction  
  - `Knob6`: Print branch prediction unit status
- 📊 **Statistics Tracking**:
  - Total cycles, CPI, instruction counts
  - Data/control hazards, stalls, mispredictions


---

## File Structure
```
📂 Project Root  
├── 📂 include/               # Contains all header files  
│   ├── 📄 encoder.h          # Header file for instruction encoding  
│   ├── 📄 parser.h           # Header file for instruction parsing  
│   ├── 📄 symboltable.h      # Header file for symbol table management  
│   ├── 📄 simulator.h        # Header file for the simulator  
│   ├── 📄 branch_predictor.h # Header file for branch prediction logic
│   ├── 📄 generate_mc.h      # Header file for machine code generation
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
|   ├── 📄 branch_predictor.cpp # Implements branch prediction logic
│   ├── 📄 generate_mc.cpp      # Generates machine code from assembly instruction s
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

### Phase 3: Pipelined Execution
Phase 3 simulates realistic pipelined execution of RISC-V instructions, improving performance modeling and realism.

#### Key Enhancements:
- **Pipeline Registers**: Introduced between each stage to store intermediate instruction states.
- **Data Hazards**: Managed through stalling and data forwarding based on Knob2 setting.
- **Control Hazards**: Handled using **1-bit dynamic branch predictor** with Branch Target Buffer (BTB) and Pattern History Table (PHT).
- **Flushing**: Incorrectly predicted branches are flushed and re-fetched correctly.
- **Stalls**: Introduced when data hazards occur, with stall cycles counted and reported.
- **Branch Prediction**: Implemented using a 1-bit predictor with BTB and PHT, allowing for dynamic prediction of branch instructions.  

#### Statistics Printed:
- Cycle count, instruction count, CPI
- Data/control instruction counts
- Hazard counts (data/control)
- Branch mispredictions
- Stall statistics (total, due to data/control hazards)



---


## Compilation and Execution

### **To Compile using direct command:**
```sh
g++ -std=c++17 `wx-config --cxxflags --libs` -o simulator.exe src/*.cpp -Iinclude
```

### **To compile using Makefile**
```sh
make
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