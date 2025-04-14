#include "control_unit.h"

ControlUnit::ControlUnit() {
    ControlUnit::reset();
}

void ControlUnit::reset() {
    FetchControl firstFetch;
    while (!fetchQueue.empty()) fetchQueue.pop();
    fetchQueue.push(firstFetch);
    while (!decodeQueue.empty()) decodeQueue.pop();
    addDecodeStall();
    while (!executeQueue.empty()) executeQueue.pop();
    addExecuteStall();
    addExecuteStall();
    while (!memoryQueue.empty()) memoryQueue.pop();
    addMemoryStall();
    addMemoryStall();
    addMemoryStall();
    while (!writeBackQueue.empty()) writeBackQueue.pop();
    addWriteBackStall();
    addWriteBackStall();
    addWriteBackStall();
    addWriteBackStall();
}

bool ControlUnit::isFetchEmpty() const {
    return fetchQueue.empty();
}
bool ControlUnit::isDecodeEmpty() const {
    return decodeQueue.empty();
}
bool ControlUnit::isExecuteEmpty() const {
    return executeQueue.empty();
}
bool ControlUnit::isMemoryEmpty() const {
    return memoryQueue.empty();
}
bool ControlUnit::isWriteBackEmpty() const {
    return writeBackQueue.empty();
}

FetchControl ControlUnit::getFetchControl() {
    if (fetchQueue.empty()) {
        return {false, false}; 
    }
    
    FetchControl control = fetchQueue.front();
    fetchQueue.pop();
    return control;
}

DecodeControl ControlUnit::getDecodeControl() {
    if (decodeQueue.empty()) {
        DecodeControl emptyControl = {0};
        return emptyControl;
    }
    
    DecodeControl control = decodeQueue.front();
    decodeQueue.pop();
    return control;
}

ExecuteControl ControlUnit::getExecuteControl() {
    if (executeQueue.empty()) {
        ExecuteControl emptyControl = {0};
        return emptyControl;
    }
    
    ExecuteControl control = executeQueue.front();
    executeQueue.pop();
    return control;
}

MemoryControl ControlUnit::getMemoryControl() {
    if (memoryQueue.empty()) {
        MemoryControl emptyControl = {0};
        return emptyControl;
    }
    
    MemoryControl control = memoryQueue.front();
    memoryQueue.pop();
    return control;
}

WriteBackControl ControlUnit::getWriteBackControl() {
    if (writeBackQueue.empty()) {
        WriteBackControl emptyControl = {0};
        return emptyControl;
    }
    
    WriteBackControl control = writeBackQueue.front();
    writeBackQueue.pop();
    return control;
}

void ControlUnit::addFetchControl(FetchControl fetchCtrl) {
    fetchQueue.push(fetchCtrl);
}

void ControlUnit::addDecodeControl(DecodeControl decodeCtrl) {
    decodeQueue.push(decodeCtrl);
}

void ControlUnit::addExecuteControl(ExecuteControl exeCtrl) {
    executeQueue.push(exeCtrl);
}

void ControlUnit::addMemoryControl(MemoryControl memCtrl) {
    memoryQueue.push(memCtrl);
}

void ControlUnit::addWriteBackControl(WriteBackControl wbCtrl) {
    writeBackQueue.push(wbCtrl);
}

void ControlUnit::addFetchStall() {
    FetchControl fetchCtrl = {true, false}; // Stall, no flush
    fetchQueue.push(fetchCtrl);
}

void ControlUnit::addDecodeStall() {
    DecodeControl decodeCtrl = {true};
    decodeQueue.push(decodeCtrl);
}

void ControlUnit::addExecuteStall() {
    ExecuteControl exeCtrl = {true}; // Stall
    executeQueue.push(exeCtrl);
}

void ControlUnit::addMemoryStall() {
    MemoryControl memCtrl = {true}; // Stall
    memoryQueue.push(memCtrl);
}

void ControlUnit::addWriteBackStall() {
    WriteBackControl wbCtrl = {true}; // Stall
    writeBackQueue.push(wbCtrl);
}


void ControlUnit::flushPipeline() {
    // Insert a flush signal
    FetchControl fetchCtrl = {false, true}; // No stall, flush
    
    // Insert NOPs into the pipeline
    DecodeControl decodeCtrl = {0}; 
    ExecuteControl exeCtrl = {0}; 
    MemoryControl memCtrl = {0}; 
    WriteBackControl wbCtrl = {0}; 
    
    fetchQueue.push(fetchCtrl);
    decodeQueue.push(decodeCtrl);
    executeQueue.push(exeCtrl);
    memoryQueue.push(memCtrl);
    writeBackQueue.push(wbCtrl);
}

bool ControlUnit::hasPendingInstructions() const {
    return !fetchQueue.empty() || !decodeQueue.empty() || 
           !executeQueue.empty() || !memoryQueue.empty() || 
           !writeBackQueue.empty();
}
