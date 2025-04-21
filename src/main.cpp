#include <iostream>
#include <cstdlib> 
#include <thread>  
#include <chrono>
#include <unistd.h>
#include <wx/wx.h>
#ifdef _WIN32
    #include <conio.h>  
    #include <windows.h>
    #define SLEEP(ms) Sleep(ms)
#else
    #include <termios.h>
    #include <fcntl.h>
    #define SLEEP(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms))
#endif
#include "simulator.h"
#include "generate_mc.h"

using namespace std;

char getKeyPress() {
    #ifdef _WIN32
        if (_kbhit()) return _getch();
    #else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        int ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    #endif
        return 0;
}

void displayPipelineConfig(const Simulator& simulator) {
    cout << "\n===== Pipeline Configuration =====\n";
    cout << "Knob1 (Pipelining):  " << (simulator.enable_pipelining ? "Enabled" : "Disabled") << endl;
    cout << "Knob2 (Data Forwarding): " << (simulator.enable_data_forwarding ? "Enabled" : "Disabled") << endl;
    cout << "Knob3 (Register Printing): " << (simulator.print_registers_each_cycle ? "Enabled" : "Disabled") << endl;
    cout << "Knob4 (Pipeline State): " << (simulator.print_pipeline_registers ? "Enabled" : "Disabled") << endl;
    cout << "Knob5 (Instruction Tracing): " << (simulator.trace_instruction ? "Instruction #" + to_string(simulator.trace_instruction_num) : "Disabled") << endl;
    cout << "Knob6 (Branch Predictor Printing): " << (simulator.print_branch_prediction ? "Enabled" : "Disabled") << endl;
    cout << "==================================\n";
}


void configurePipelineKnobs(Simulator& simulator) {
    cout << "===== Pipeline Knob Setup =====\n";
    char input;

    // Knob1: Enable Pipelining
    while (true) {
        cout << "Enable Pipelining (Knob1)? (y/n): ";
        cin >> input;
        if (input == 'y' || input == 'Y' || input == 'n' || input == 'N') break;
        cout << "Invalid input. Please enter y or n.\n";
    }
    bool pipelining = (input == 'y' || input == 'Y');
    simulator.setKnob1(pipelining);

    if (!pipelining) {
        // If pipelining is off, only allow Knob3
        cout << "Pipelining is OFF. Skipping Knobs 2, 4, 5, 6.\n";

        simulator.setKnob2(false);
        simulator.setKnob4(false);
        simulator.setKnob5(-1);
        simulator.setKnob6(false);

        // Validate Knob3 input
        while (true) {
            cout << "Enable Register Printing Each Cycle (Knob3)? (y/n): ";
            cin >> input;
            if (input == 'y' || input == 'Y' || input == 'n' || input == 'N') {
                simulator.setKnob3(input == 'y' || input == 'Y');
                break;
            } else {
                cout << "Invalid input. Please enter y or n.\n";
            }
        }
        return;
    }

    // Knob2: Data Forwarding
    while (true) {
        cout << "Enable Data Forwarding (Knob2)? (y/n): ";
        cin >> input;
        if (input == 'y' || input == 'Y' || input == 'n' || input == 'N') {
            simulator.setKnob2(input == 'y' || input == 'Y');
            break;
        } else {
            cout << "Invalid input. Please enter y or n.\n";
        }
    }

    // Knob3: Register Printing
    while (true) {
        cout << "Enable Register Printing Each Cycle (Knob3)? (y/n): ";
        cin >> input;
        if (input == 'y' || input == 'Y' || input == 'n' || input == 'N') {
            simulator.setKnob3(input == 'y' || input == 'Y');
            break;
        } else {
            cout << "Invalid input. Please enter y or n.\n";
        }
    }

    // Knob4: Pipeline State Printing
    bool knob4_enabled = false;
    while (true) {
        cout << "Enable Pipeline State Printing (Knob4)? (y/n): ";
        cin >> input;
        if (input == 'y' || input == 'Y' || input == 'n' || input == 'N') {
            knob4_enabled = (input == 'y' || input == 'Y');
            simulator.setKnob4(knob4_enabled);
            break;
        } else {
            cout << "Invalid input. Please enter y or n.\n";
        }
    }

    // Knob5: Instruction Tracing
    if (knob4_enabled) {
        cout << "Instruction Tracing (Knob5) is disabled because Knob4 is enabled.\n";
        simulator.setKnob5(-1);
    } else {
        while (true) {
            cout << "Enable Instruction Tracing (Knob5)? (y/n): ";
            cin >> input;
            if (input == 'y' || input == 'Y') {
                int instr_num;
                cout << "Enter PC to trace (-1 to disable): ";
                cin >> instr_num;
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid number. Please enter a valid PC .\n";
                    continue;
                }
                simulator.setKnob5(instr_num);
                break;
            } else if (input == 'n' || input == 'N') {
                simulator.setKnob5(-1);
                break;
            } else {
                cout << "Invalid input. Please enter y or n.\n";
            }
        }
    }

    // Knob6: Branch Prediction
    while (true) {
        cout << "Enable Branch Prediction Printing (Knob6)? (y/n): ";
        cin >> input;
        if (input == 'y' || input == 'Y' || input == 'n' || input == 'N') {
            simulator.setKnob6(input == 'y' || input == 'Y');
            break;
        } else {
            cout << "Invalid input. Please enter y or n.\n";
        }
    }
}


void runConsole(Simulator& simulator) {
    bool running = true;

    displayPipelineConfig(simulator);

    while (running) {
        cout << "\n------------------- MENU -------------------" << endl;
        cout << "[R] Run | [S] Step | [E] Exit | [V] View Registers | [M] View Memory" << endl;
        cout << "---------------------------------------------" << endl;
        cout << "Press a key to choose an option... ";
        
        while (true) {
            char choice = getKeyPress();
            if (choice) {
                choice = toupper(choice);
                cout << choice << endl;
                
                switch (choice) {
                    // Run simulation
                    case 'R':
                        cout << "\nRunning simulation...\n";
                        simulator.run();
                        break;
                        
                    // Execute one instruction
                    case 'S':
                        cout << "\nExecuting one cycle...\n";
                        simulator.step();
                        break;
                        
                    // Exit simulation
                    case 'E':
                        cout << "\nExiting simulation...\n";
                        running = false;
                        return;
                        
                    // View register states
                    case 'V':
                        cout << "\nRegister States:\n";
                        simulator.printRegisters();
                        break;
                        
                    // View memory contents
                    case 'M': {
                        uint32_t start=0, end=0;
                        cout << "Enter Start Address: ";
                        cin >> hex >> start;
                        cout << "Enter End Address: ";
                        cin >> hex >> end;
                        
                        if (cin.fail() || start > end || start < 0 || end > 0xFFFFFFFF) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "Invalid input. Please enter a valid memory range.\n";
                        }
                        else {
                            cout << "Enter Format (h for hex, d for decimal, a for ascii): ";
                            char format;
                            cin >> format;
                            simulator.printMemory(start, end, format);
                        }
                        break;
                    }             
                    default:
                        cout << "Invalid choice! Press R, S, E, V, M, P.\n";
                        break;
                }
                break;
            }
            SLEEP(10);
        }
    }
}

int main(int argc, char** argv) {
    cout << "\n[Phase 1] Generating machine code...\n";
    if (generateMC()){
        cerr << "Error: Failed to generate machine code.\n";
        return 1;
    } else {
        cout << "Machine code generated successfully.\n";
    }

    cout << "\n[Phase 2 / 3] Initializing RISC-V Simulator...\n";
    Simulator simulator;

    if (!simulator.loadProgram("output.mc")) {
        cerr << "Error: Failed to load machine code.\n";
        return 1;
    }

    if (argc > 1 && string(argv[1]) == "--gui") {
        simulator.enable_pipelining = false;
        simulator.enable_data_forwarding = false;
        simulator.print_registers_each_cycle = false;
        simulator.print_pipeline_registers = false;
        simulator.trace_instruction = false;
        simulator.trace_instruction_num = -1;
        simulator.enable_branch_prediction = false;
        simulator.print_branch_prediction = false;
        wxEntryStart(argc, argv);
        wxTheApp->CallOnInit();
        wxTheApp->OnRun();
        wxEntryCleanup();
    } else {
        configurePipelineKnobs(simulator);  
        runConsole(simulator);              
    }
    return 0;
}
