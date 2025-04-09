#include "wxRISCVSimulator.h"
#include <wx/wx.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/filedlg.h>
#include <fstream>
#include <sstream>
#include <iomanip>

// Implement application
IMPLEMENT_APP_NO_MAIN(wxRISCVSimulatorApp)

BEGIN_EVENT_TABLE(wxRISCVSimulatorFrame, wxFrame)
    EVT_MENU(ID_LOAD_PROGRAM, wxRISCVSimulatorFrame::OnLoadProgram)
    EVT_MENU(wxID_EXIT, wxRISCVSimulatorFrame::OnExit)
    EVT_BUTTON(ID_STEP, wxRISCVSimulatorFrame::OnStep)
    EVT_BUTTON(ID_RUN, wxRISCVSimulatorFrame::OnRun)
    EVT_BUTTON(ID_RESET, wxRISCVSimulatorFrame::OnReset)
    EVT_BUTTON(ID_LOAD_MEMORY, wxRISCVSimulatorFrame::OnLoadMemory)
    EVT_RADIOBUTTON(ID_HEX_FORMAT, wxRISCVSimulatorFrame::OnRegisterFormatChanged)
    EVT_RADIOBUTTON(ID_DEC_FORMAT, wxRISCVSimulatorFrame::OnRegisterFormatChanged)
END_EVENT_TABLE()

bool wxRISCVSimulatorApp::OnInit() {
    wxRISCVSimulatorFrame* frame = new wxRISCVSimulatorFrame("RISC-V Simulator");
    frame->Show(true);
    return true;
}

// Modify part of the constructor in wxRISCVSimulatorFrame.cpp where register view is created
wxRISCVSimulatorFrame::wxRISCVSimulatorFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1000, 700)),
      m_currentInstruction(-1), m_registerFormat(FORMAT_HEX) {

    this->Maximize(true);
    
    // Create menu bar
    CreateMenuBar();
    
    // Create main splitter window
    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    
    // Create left panel for control buttons and instruction list
    wxPanel* leftPanel = new wxPanel(m_splitter, wxID_ANY);
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    
    // Add control buttons at the top of instruction list
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    m_stepBtn = new wxButton(leftPanel, ID_STEP, "Step");
    m_runBtn = new wxButton(leftPanel, ID_RUN, "Run");
    m_resetBtn = new wxButton(leftPanel, ID_RESET, "Reset");
    
    buttonSizer->Add(m_stepBtn, 1, wxALL, 5);
    buttonSizer->Add(m_runBtn, 1, wxALL, 5);
    buttonSizer->Add(m_resetBtn, 1, wxALL, 5);
    
    leftSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
    
    // Create instruction list
    CreateInstructionList(leftPanel);
    leftSizer->Add(m_instructionList, 1, wxEXPAND | wxALL, 5);
    leftPanel->SetSizer(leftSizer);
    
    // Create notebook for registers and memory on the right
    m_notebook = new wxNotebook(m_splitter, wxID_ANY);
    
    // Create combined register and memory view
    CreateCombinedRegMemView();
    
    // Create log console
    CreateLogConsole();
    
    // Split the window
    m_splitter->SplitVertically(leftPanel, m_notebook, 750);

    // Set up the simulator
    m_simulator.reset();
    
    // Try to load the default program if it exists
    LoadProgram("output.mc");
}


void wxRISCVSimulatorFrame::CreateLogConsole() {
    // Create log panel
    wxPanel* logPanel = new wxPanel(m_notebook);
    wxBoxSizer* logSizer = new wxBoxSizer(wxVERTICAL);
    
    // Add title
    wxStaticText* logTitle = new wxStaticText(logPanel, wxID_ANY, "Log Console");
    logTitle->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    logSizer->Add(logTitle, 0, wxALL, 5);
    
    // Create log text control
    m_logConsole = new wxTextCtrl(logPanel, wxID_ANY, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
    m_logConsole->SetFont(monoFont);
    
    logSizer->Add(m_logConsole, 1, wxEXPAND | wxALL, 5);
    logPanel->SetSizer(logSizer);
    
    // Add the log panel to the notebook
    m_notebook->AddPage(logPanel, "Log Console");

    new wxStreamToTextRedirector(m_logConsole);
}

wxRISCVSimulatorFrame::~wxRISCVSimulatorFrame() {
    // Cleanup resources if needed
}

void wxRISCVSimulatorFrame::CreateMenuBar() {
    m_menuBar = new wxMenuBar();
    
    // File menu
    m_fileMenu = new wxMenu();
    m_fileMenu->Append(ID_LOAD_PROGRAM, "&Load Program\tCtrl-O", "Load a machine code file");
    m_fileMenu->AppendSeparator();
    m_fileMenu->Append(wxID_EXIT);
    
    // Simulation menu
    m_simulationMenu = new wxMenu();
    m_simulationMenu->Append(ID_STEP, "&Step\tF7", "Execute one instruction");
    m_simulationMenu->Append(ID_RUN, "&Run\tF5", "Run the program until completion");
    m_simulationMenu->Append(ID_RESET, "&Reset\tF6", "Reset the simulator");
    
    // Add menus to the menu bar
    m_menuBar->Append(m_fileMenu, "&File");
    m_menuBar->Append(m_simulationMenu, "&Simulation");
    
    SetMenuBar(m_menuBar);
}

void wxRISCVSimulatorFrame::CreateInstructionList(wxWindow* parent) {
    m_instructionList = new wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
                                      wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES);
    
    // Add columns
    m_instructionList->InsertColumn(0, "PC", wxLIST_FORMAT_LEFT, 175);
    m_instructionList->InsertColumn(1, "Machine Code", wxLIST_FORMAT_LEFT, 175);
    m_instructionList->InsertColumn(2, "Instruction", wxLIST_FORMAT_LEFT, 350);

    // Ensure only 3 columns
    while (m_instructionList->GetColumnCount() > 3) {
        m_instructionList->DeleteColumn(3);
    }

    m_instructionList->SetFont(monoFont);

}

void wxRISCVSimulatorFrame::CreateCombinedRegMemView() {
    // Create a panel to hold both register and memory views
    wxPanel* combinedPanel = new wxPanel(m_notebook);
    wxBoxSizer* combinedSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Create register panel
    wxPanel* registerPanel = new wxPanel(combinedPanel);
    wxBoxSizer* registerSizer = new wxBoxSizer(wxVERTICAL);
    
    // Add register format radio buttons
    wxBoxSizer* formatSizer = new wxBoxSizer(wxHORIZONTAL);
    formatSizer->Add(new wxStaticText(registerPanel, wxID_ANY, "Register Format:"), 
                    0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    
    m_regHexFormatRB = new wxRadioButton(registerPanel, ID_HEX_FORMAT, "Hexadecimal", 
                                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_regDecFormatRB = new wxRadioButton(registerPanel, ID_DEC_FORMAT, "Decimal");
    
    m_regHexFormatRB->SetValue(true);
    
    formatSizer->Add(m_regHexFormatRB, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    formatSizer->Add(m_regDecFormatRB, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    
    registerSizer->Add(formatSizer, 0, wxEXPAND | wxALL, 5);
    
    // Create panel for general registers
    wxPanel* genRegPanel = new wxPanel(registerPanel, wxID_ANY);
    wxBoxSizer* genRegSizer = new wxBoxSizer(wxVERTICAL);
    
    // Add title
    wxStaticText* genRegTitle = new wxStaticText(genRegPanel, wxID_ANY, "General Purpose Registers");
    genRegTitle->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    genRegSizer->Add(genRegTitle, 0, wxALL, 5);
    
    // Create general register list
    m_genRegisterList = new wxListCtrl(genRegPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
                                      wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES);
    
    // Add columns to general register list
    m_genRegisterList->InsertColumn(0, "Register", wxLIST_FORMAT_LEFT, 150);
    m_genRegisterList->InsertColumn(1, "Value", wxLIST_FORMAT_LEFT, 175);
    
    genRegSizer->Add(m_genRegisterList, 1, wxEXPAND | wxALL, 5);
    genRegPanel->SetSizer(genRegSizer);
    
    // Add general register panel to register sizer
    registerSizer->Add(genRegPanel, 1, wxEXPAND | wxALL, 5);
    
    // Create panel for special and temporary registers
    wxPanel* tempRegPanel = new wxPanel(registerPanel, wxID_ANY);
    wxBoxSizer* tempRegSizer = new wxBoxSizer(wxVERTICAL);
    
    // Add title
    wxStaticText* tempRegTitle = new wxStaticText(tempRegPanel, wxID_ANY, "Special and Temporary Registers");
    tempRegTitle->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    tempRegSizer->Add(tempRegTitle, 0, wxALL, 5);
    
    // Create temporary register list
    m_tempRegisterList = new wxListCtrl(tempRegPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
                                       wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES);
    
    // Add columns to temporary register list
    m_tempRegisterList->InsertColumn(0, "Register", wxLIST_FORMAT_LEFT, 150);
    m_tempRegisterList->InsertColumn(1, "Value", wxLIST_FORMAT_LEFT, 175);
    
    tempRegSizer->Add(m_tempRegisterList, 1, wxEXPAND | wxALL, 5);
    tempRegPanel->SetSizer(tempRegSizer);
    
    // Add special register panel to register sizer
    registerSizer->Add(tempRegPanel, 1, wxEXPAND | wxALL, 5);
    
    // Set sizer for register panel
    registerPanel->SetSizer(registerSizer);
    
    // Create memory panel
    wxPanel* memoryPanel = new wxPanel(combinedPanel);
    wxBoxSizer* memorySizer = new wxBoxSizer(wxVERTICAL);
    
    // Create controls for memory range selection
    wxBoxSizer* rangeSelectionSizer = new wxBoxSizer(wxHORIZONTAL);
    
    rangeSelectionSizer->Add(new wxStaticText(memoryPanel, wxID_ANY, "Start Address (hex):"), 
                            0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_startAddrCtrl = new wxTextCtrl(memoryPanel, wxID_ANY, "0x00000000", 
                                    wxDefaultPosition, wxSize(100, -1));
    rangeSelectionSizer->Add(m_startAddrCtrl, 0, wxALL, 5);
    
    rangeSelectionSizer->Add(new wxStaticText(memoryPanel, wxID_ANY, "End Address (hex):"), 
                            0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_endAddrCtrl = new wxTextCtrl(memoryPanel, wxID_ANY, "0x00000020", 
                                  wxDefaultPosition, wxSize(100, -1));
    rangeSelectionSizer->Add(m_endAddrCtrl, 0, wxALL, 5);
    
    memorySizer->Add(rangeSelectionSizer, 0, wxEXPAND | wxALL, 5);
    
    // Format options
    wxBoxSizer* formatSizer2 = new wxBoxSizer(wxHORIZONTAL);
    formatSizer2->Add(new wxStaticText(memoryPanel, wxID_ANY, "Display Format:"), 
                     0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    
    m_hexFormatRB = new wxRadioButton(memoryPanel, wxID_ANY, "Hex", 
                                     wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_decFormatRB = new wxRadioButton(memoryPanel, wxID_ANY, "Decimal");
    m_asciiFormatRB = new wxRadioButton(memoryPanel, wxID_ANY, "ASCII");
    
    m_hexFormatRB->SetValue(true);
    
    formatSizer2->Add(m_hexFormatRB, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    formatSizer2->Add(m_decFormatRB, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    formatSizer2->Add(m_asciiFormatRB, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    
    // Added the Load Memory button here
    m_loadMemBtn = new wxButton(memoryPanel, ID_LOAD_MEMORY, "Load Memory");
    formatSizer2->Add(m_loadMemBtn, 0, wxALL, 5);
    
    memorySizer->Add(formatSizer2, 0, wxEXPAND | wxALL, 5);
    
    // Create memory list control
    m_memoryList = new wxListCtrl(memoryPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
                                 wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES);
    
    // Add columns to memory list
    m_memoryList->InsertColumn(0, "Address", wxLIST_FORMAT_LEFT, 150);
    m_memoryList->InsertColumn(1, "Byte 3", wxLIST_FORMAT_LEFT, 100);
    m_memoryList->InsertColumn(2, "Byte 2", wxLIST_FORMAT_LEFT, 100);
    m_memoryList->InsertColumn(3, "Byte 1", wxLIST_FORMAT_LEFT, 100);
    m_memoryList->InsertColumn(4, "Byte 0", wxLIST_FORMAT_LEFT, 100);

    m_memoryList->SetFont(monoFont);
    
    memorySizer->Add(m_memoryList, 1, wxEXPAND | wxALL, 5);
    
    memoryPanel->SetSizer(memorySizer);
    
    // Set fonts
    m_genRegisterList->SetFont(monoFont);
    m_tempRegisterList->SetFont(monoFont);
    
    // Add both panels to the combined sizer
    combinedSizer->Add(registerPanel, 1, wxEXPAND | wxALL, 5);
    combinedSizer->Add(memoryPanel, 1, wxEXPAND | wxALL, 5);
    
    combinedPanel->SetSizer(combinedSizer);
    
    // Add the combined panel to the notebook
    m_notebook->AddPage(combinedPanel, "Registers and Memory");
}


bool wxRISCVSimulatorFrame::LoadProgram(const wxString& filename) {
    // Reset simulator
    m_simulator.reset();
    
    // Load machine code into the simulator
    if (!m_simulator.loadProgram(filename.ToStdString())) {
        wxMessageBox("Failed to load program into simulator", "Error", wxICON_ERROR);
        return false;
    }
    
    // Parse machine code file to populate instruction list
    ParseMachineCodeFile(filename);
    
    // Update the UI
    UpdateInstructionList();
    UpdateRegisterView();
    UpdateMemoryView();
    
    m_currentInstruction = 0;
    
    return true;
}

void wxRISCVSimulatorFrame::ParseMachineCodeFile(const wxString& filename) {
    m_instructions.clear();
    
    std::ifstream file(filename.ToStdString());
    if (!file.is_open()) {
        wxMessageBox("Could not open file: " + filename, "Error", wxICON_ERROR);
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;
        
        // Look for machine code lines with comment format
        std::istringstream iss(line);
        std::string addrStr, codeStr, instrStr, commentStr;
        
        // Parse address
        if (!(iss >> addrStr)) continue;
        
        // Parse machine code
        if (!(iss >> codeStr)) continue;
        
        // Try to extract instruction and comment
        std::string remaining;
        std::getline(iss, remaining);
        
        size_t commentPos = remaining.find('#');
        if (commentPos != std::string::npos) {
            instrStr = remaining.substr(2, commentPos-2);
            commentStr = remaining.substr(commentPos + 1);
        } else {
            instrStr = remaining;
        }
        
        // Convert address and machine code to integers
        uint32_t address = 0, machineCode = 0;
        try {
            // Remove 0x prefix if present
            if (addrStr.substr(0, 2) == "0x") addrStr = addrStr.substr(2);
            if (codeStr.substr(0, 2) == "0x") codeStr = codeStr.substr(2);
            
            address = std::stoul(addrStr, nullptr, 16);
            machineCode = std::stoul(codeStr, nullptr, 16);
        } catch (const std::exception& e) {
            // Skip malformed lines
            continue;
        }
        
        // Add to our instruction list
        if (machineCode==-1 ) break;
        m_instructions.push_back(InstructionListItem(address, machineCode, instrStr));
    }
    
    file.close();
}

void wxRISCVSimulatorFrame::UpdateInstructionList() {
    m_instructionList->DeleteAllItems();
    
    for (size_t i = 0; i < m_instructions.size(); i++) {
        const auto& instr = m_instructions[i];
        
        // Format address and machine code as hex strings
        std::stringstream addrSS, codeSS;
        addrSS << "0x" << std::hex << std::setw(8) << std::setfill('0') << instr.address;
        codeSS << "0x" << std::hex << std::setw(8) << std::setfill('0') << instr.machineCode;
        
        long idx = m_instructionList->InsertItem(i, addrSS.str());
        m_instructionList->SetItem(idx, 1, codeSS.str());
        m_instructionList->SetItem(idx, 2, instr.instruction);
        
        // Highlight current instruction
        if (static_cast<int>(i) == m_currentInstruction) {
            m_instructionList->SetItemBackgroundColour(idx, wxColour(50,50, 230)); // Light blue
        }
    }
    
    // Ensure current instruction is visible
    if (m_currentInstruction >= 0 && m_currentInstruction < static_cast<int>(m_instructions.size())) {
        m_instructionList->EnsureVisible(m_currentInstruction);
    }
}

void wxRISCVSimulatorFrame::UpdateRegisterView() {
    // Clear both register lists
    m_genRegisterList->DeleteAllItems();
    m_tempRegisterList->DeleteAllItems();
    
    // Add general purpose registers to the left list
    for (int i = 0; i < 32; i++) {
        int32_t value = m_simulator.getRegisterState().getGen(i);
        
        wxString valueStr;
        if (m_registerFormat == FORMAT_HEX) {
            std::stringstream ss;
            ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << value;
            valueStr = ss.str();
        } else {
            valueStr = wxString::Format("%d", value);
        }
        
        long idx = m_genRegisterList->InsertItem(i, wxString::Format("x%d", i));
        m_genRegisterList->SetItem(idx, 1, valueStr);
    }
    
    // Add special and temporary registers to the right list
    int idx = 0;
    
    // PC
    {
        uint32_t pc = m_simulator.getRegisterState().getPC();
        wxString valueStr;
        if (m_registerFormat == FORMAT_HEX) {
            std::stringstream ss;
            ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << pc;
            valueStr = ss.str();
        } else {
            valueStr = wxString::Format("%u", pc);
        }
        
        long itemIdx = m_tempRegisterList->InsertItem(idx++, "PC");
        m_tempRegisterList->SetItem(itemIdx, 1, valueStr);
    }
    
    // IR
    {
        uint32_t ir = m_simulator.getRegisterState().getIR();
        wxString valueStr;
        if (m_registerFormat == FORMAT_HEX) {
            std::stringstream ss;
            ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << ir;
            valueStr = ss.str();
        } else {
            valueStr = wxString::Format("%u", ir);
        }
        
        long itemIdx = m_tempRegisterList->InsertItem(idx++, "IR");
        m_tempRegisterList->SetItem(itemIdx, 1, valueStr);
    }
    
    // Add temporary registers
    const std::vector<std::string> tempRegs = {"RM", "RA", "RB", "RY", "RZ", "MAR", "MDR", "IMM", "PC_TEMP"};
    for (const auto& regName : tempRegs) {
        int32_t value = m_simulator.getRegisterState().getTemp(regName);
        
        wxString valueStr;
        if (m_registerFormat == FORMAT_HEX) {
            std::stringstream ss;
            ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << value;
            valueStr = ss.str();
        } else {
            valueStr = wxString::Format("%d", value);
        }
        
        long itemIdx = m_tempRegisterList->InsertItem(idx++, regName);
        m_tempRegisterList->SetItem(itemIdx, 1, valueStr);
    }
}

void wxRISCVSimulatorFrame::UpdateMemoryView() {
    m_memoryList->DeleteAllItems();
    
    // Get address range
    wxString startAddrStr = m_startAddrCtrl->GetValue();
    wxString endAddrStr = m_endAddrCtrl->GetValue();
    
    // Remove 0x prefix if present
    if (startAddrStr.StartsWith("0x")) startAddrStr = startAddrStr.Mid(2);
    if (endAddrStr.StartsWith("0x")) endAddrStr = endAddrStr.Mid(2);
    
    uint32_t startAddr = 0, endAddr = 0;
    
    try {
        startAddr = std::stoul(startAddrStr.ToStdString(), nullptr, 16);
        endAddr = std::stoul(endAddrStr.ToStdString(), nullptr, 16);
    } catch (const std::exception& e) {
        wxMessageBox("Invalid address format. Please use hexadecimal.", "Error", wxICON_ERROR);
        return;
    }
    
    if (startAddr > endAddr) {
        wxMessageBox("Start address must be less than or equal to end address.", "Error", wxICON_ERROR);
        return;
    }
    
    // Get the format
    char format = 'h'; // Default to hex
    if (m_decFormatRB->GetValue()) format = 'd';
    else if (m_asciiFormatRB->GetValue()) format = 'a';
    
    // Fetch memory data
    int idx = 0;
    
    // Align to 4-byte boundaries
    startAddr = startAddr & ~0x3;
    endAddr = (endAddr + 3) & ~0x3;
    
    // Iterate through memory in word chunks
    for (uint32_t addr = startAddr; addr < endAddr; addr += 4) {
        std::stringstream addrSS;
        addrSS << "0x" << std::hex << std::setw(8) << std::setfill('0') << addr;
        
        long itemIdx = m_memoryList->InsertItem(idx++, addrSS.str());
        
        // Display each byte
        for (int i = 3; i >= 0; i--) {
            uint8_t byte = m_simulator.getMemory().readByte(addr + i);
            
            wxString byteStr;
            if (format == 'h') {
                byteStr = wxString::Format("0x%02x", byte);
            } else if (format == 'd') {
                byteStr = wxString::Format("%d", byte);
            } else if (format == 'a') {
                if (byte >= 32 && byte <= 126) {
                    byteStr = wxString::Format("'%c'", byte);
                } else {
                    byteStr = wxString::Format("0x%02x", byte);
                }
            }
            
            m_memoryList->SetItem(itemIdx, 4 - i, byteStr);
        }
    }
}

void wxRISCVSimulatorFrame::Step() {
    if (m_currentInstruction < 0 || m_currentInstruction >= static_cast<int>(m_instructions.size())) {
        wxMessageBox("No program loaded or reached end of program.", "Step", wxICON_INFORMATION);
        return;
    }
    
    // Execute one instruction
    m_simulator.step();
    
    // Update UI
    m_currentInstruction = m_simulator.getRegisterState().getPC() / 4;
    UpdateInstructionList();
    UpdateRegisterView();
    UpdateMemoryView();
    
    // Check if we've reached the end
    if (!m_simulator.isRunning()) {
        wxMessageBox("Program execution complete.", "Step", wxICON_INFORMATION);
    }
}

void wxRISCVSimulatorFrame::Run() {
    if (m_currentInstruction < 0 || m_currentInstruction >= static_cast<int>(m_instructions.size())) {
        wxMessageBox("No program loaded or reached end of program.", "Run", wxICON_INFORMATION);
        return;
    }
    
    // Run until completion or error
    m_simulator.run();
    
    // Update UI
    m_currentInstruction = m_simulator.getRegisterState().getPC() / 4;
    UpdateInstructionList();
    UpdateRegisterView();
    UpdateMemoryView();
    
    wxMessageBox(wxString::Format("Program execution complete. Clock cycles: %d", m_simulator.getClock()), 
                "Run", wxICON_INFORMATION);
}

void wxRISCVSimulatorFrame::Reset() {
    m_simulator.reset();
    
    if (!m_simulator.loadProgram("output.mc")) {
        wxMessageBox("Failed to reload program into simulator", "Error", wxICON_ERROR);
        return;
    }
    
    m_currentInstruction = 0;
    UpdateInstructionList();
    UpdateRegisterView();
    UpdateMemoryView();
    
    wxMessageBox("Simulator reset.", "Reset", wxICON_INFORMATION);
}

void wxRISCVSimulatorFrame::OnRegisterFormatChanged(wxCommandEvent& event) {
    if (event.GetId() == ID_HEX_FORMAT) {
        m_registerFormat = FORMAT_HEX;
    } else if (event.GetId() == ID_DEC_FORMAT) {
        m_registerFormat = FORMAT_DEC;
    }
    
    // Update register view with the new format
    UpdateRegisterView();
}

// Event handlers
void wxRISCVSimulatorFrame::OnLoadProgram(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, "Open Machine Code File", "", "",
                               "Machine Code files (*.mc)|*.mc|All files (*.*)|*.*",
                               wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;
    
    LoadProgram(openFileDialog.GetPath());
}

void wxRISCVSimulatorFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void wxRISCVSimulatorFrame::OnStep(wxCommandEvent& event) {
    Step();
}

void wxRISCVSimulatorFrame::OnRun(wxCommandEvent& event) {
    Run();
}

void wxRISCVSimulatorFrame::OnReset(wxCommandEvent& event) {
    Reset();
}

void wxRISCVSimulatorFrame::OnLoadMemory(wxCommandEvent& event) {
    UpdateMemoryView();
}