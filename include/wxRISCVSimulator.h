#ifndef WXRISCV_SIMULATOR_H
#define WXRISCV_SIMULATOR_H

#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/spinctrl.h>
#include <wx/radiobut.h>
#include <vector>
#include <string>
#include "simulator.h"

wxFont monoFont(16, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

// Define register display format enum
enum RegisterFormat {
    FORMAT_HEX,
    FORMAT_DEC
};

// Instruction list item class
class InstructionListItem {
public:
    uint32_t address;
    uint32_t machineCode;
    std::string instruction;
    std::string comment;

    InstructionListItem(uint32_t addr, uint32_t code, const std::string& instr)
        : address(addr), machineCode(code), instruction(instr) {}
};

// Main application class
class wxRISCVSimulatorApp : public wxApp {
public:
    virtual bool OnInit();
};

// Main frame class
class wxRISCVSimulatorFrame : public wxFrame {
public:
    wxRISCVSimulatorFrame(const wxString& title);
    virtual ~wxRISCVSimulatorFrame();

private:
wxTextCtrl* m_logConsole;

wxListCtrl* m_genRegisterList;    // For general purpose registers
wxListCtrl* m_tempRegisterList;   // For temporary registers

    wxTextCtrl* m_consoleOutput;
    
    // Method to update console output
    void UpdateConsoleOutput();
    wxPanel* m_instrPanel;
    wxRadioButton* m_regHexFormatRB;
    wxRadioButton* m_regDecFormatRB;
    
    // Register format tracking
    RegisterFormat m_registerFormat;

    // GUI components
    wxSplitterWindow* m_splitter;
    wxNotebook* m_notebook;
    wxListCtrl* m_instructionList;
    // wxListCtrl* m_registerList;
    wxPanel* m_memoryPanel;
    wxListCtrl* m_memoryList;
    wxTextCtrl* m_startAddrCtrl;
    wxTextCtrl* m_endAddrCtrl;
    wxRadioButton* m_hexFormatRB;
    wxRadioButton* m_decFormatRB;
    wxRadioButton* m_asciiFormatRB;
    
    // Menu items
    wxMenuBar* m_menuBar;
    wxMenu* m_fileMenu;
    wxMenu* m_simulationMenu;
    
    // Buttons
    wxButton* m_stepBtn;
    wxButton* m_runBtn;
    wxButton* m_resetBtn;
    wxButton* m_loadMemBtn;
    
    // Simulator
    Simulator m_simulator;
    std::vector<InstructionListItem> m_instructions;
    int m_currentInstruction;
    
    // Methods for initialization
    void CreateMenuBar();
    void CreateInstructionList(wxWindow* parent);
    //void CreateRegisterView();
    //void CreateMemoryView();
    void CreateLogConsole();
    void CreateCombinedRegMemView();
    
    // Methods for functionality
    bool LoadProgram(const wxString& filename);
    void ParseMachineCodeFile(const wxString& filename);
    void UpdateInstructionList();
    void UpdateRegisterView();
    void UpdateMemoryView();
    void Step();
    void Run();
    void Reset();
    
    // Event handlers
    void OnLoadProgram(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnStep(wxCommandEvent& event);
    void OnRun(wxCommandEvent& event);
    void OnReset(wxCommandEvent& event);
    void OnLoadMemory(wxCommandEvent& event);
    void OnRegisterFormatChanged(wxCommandEvent& event);
    
    // ID values for controls
    enum {
        ID_LOAD_PROGRAM = wxID_HIGHEST + 1,
        ID_STEP,
        ID_RUN,
        ID_RESET,
        ID_LOAD_MEMORY,
        ID_HEX_FORMAT,
        ID_DEC_FORMAT
    };
    
    DECLARE_EVENT_TABLE()
};

#endif // WXRISCV_SIMULATOR_H