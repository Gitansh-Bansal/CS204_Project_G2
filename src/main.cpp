#include <iostream>
#include <cstdlib> 
#include <thread>  
#include <chrono>
#include <unistd.h>
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

bool generateMachineCode() {
    cout << "\n[Phase 1] Generating Machine Code..." << endl;
    
    #ifdef _WIN32
        int status = system("generate_mc.exe"); 
    #else
        int status = system("./generate_mc.exe"); 
    #endif

    if (status != 0) {
        cerr << "Error: Machine code generation failed!" << endl;
        return false;
    }
    cout << "Machine code successfully generated in 'output.mc'.\n" << endl;
    return true;
}

void runConsole(Simulator& simulator) {
    bool running = true;

    while (running) {
        cout << "\n------------------- MENU -------------------" << endl;
        cout << "[R] Run  |  [S] Step  |  [E] Exit  |  [V] View Registers  |  [M] View Memory" << endl;
        cout << "--------------------------------------------" << endl;
        cout << "Press a key to choose an option... ";

        while (true) {
            char choice = getKeyPress();
            if (choice) {
                choice = toupper(choice);
                cout << choice << endl;
                switch (choice) 
                {
                    case 'R':
                        cout << "\nRunning simulation...\n";
                        simulator.run();
                        break;

                    case 'S':
                        cout << "\nExecuting one instruction...\n";
                        simulator.step();
                        break;

                    case 'E':
                        cout << "\nExiting simulation...\n";
                        running = false;
                        return;

                    case 'V':
                        cout << "\nRegister States:\n";
                        simulator.printRegisters();
                        break;

                    case 'M': 
                    {
                        uint32_t start=0, end=0;
                   
                        cout << "Enter Start Address: ";
                        cin >> hex >> start;
                        cout << "Enter End Address: ";
                        cin >> hex >> end;
                
                        if (cin.fail() || start > end || start < 0 || end > 0xFFFFFFFF){  
                            cin.clear();  
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');  
                            cout << "Invalid input. Please enter a valid memory range.\n";
                        } 
                        else {
                            cout<< "Enter Format (h for hex, d for decimal, a for ascii): ";
                            char format;
                            cin >> format;
                            simulator.printMemory(start, end, format);
                        }
                        break;
                    }
                    default:
                        cout << "Invalid choice! Press R, S, E, V, or M.\n";
                }
                break; 
            }
            SLEEP(10);
        }
    }
}

int main(int argc, char** argv) {
    if (!generateMachineCode()) return 1; 

    cout << "\n[Phase 2] Initializing RISC-V Simulator...\n" << endl;
    Simulator simulator;
    
    if (!simulator.loadProgram("output.mc")) {
        cerr << "Error: Failed to load machine code into memory." << endl;
        return 1;
    }

    runConsole(simulator);
    return 0;
}
