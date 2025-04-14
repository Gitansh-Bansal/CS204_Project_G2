#ifndef CONTROL_UNIT_H
#define CONTROL_UNIT_H

#include <queue>
#include <vector>
#include <cstdint>

struct FetchControl {
    bool stall=false;
    bool flush;
};

struct DecodeControl {
    uint32_t opcode;
    bool stall=false;
    bool branch;
    bool jump;
};

struct ExecuteControl {
    bool stall = false;
    uint32_t aluOp;
    bool useImm;
    bool branch;
    bool aluSrc;
    bool jump;
};

struct MemoryControl {
    bool stall = false;
    bool memRead;
    bool memWrite;
    uint32_t memWidth; // 1=byte, 2=half, 4=word
    bool signExtend;
};

struct WriteBackControl {
    bool stall = false;
    bool regWrite;
    bool memToReg;
};

class ControlUnit {
private:
    std::queue<FetchControl> fetchQueue;
    std::queue<DecodeControl> decodeQueue;
    std::queue<ExecuteControl> executeQueue;
    std::queue<MemoryControl> memoryQueue;
    std::queue<WriteBackControl> writeBackQueue;
    
    
public:
    ControlUnit();
    

    bool isFetchEmpty() const;
    bool isDecodeEmpty() const;
    bool isExecuteEmpty() const;
    bool isMemoryEmpty() const;
    bool isWriteBackEmpty() const;
    
    // Get control signals for each stage
    FetchControl getFetchControl();
    DecodeControl getDecodeControl();
    ExecuteControl getExecuteControl();
    MemoryControl getMemoryControl();
    WriteBackControl getWriteBackControl();

    void addFetchControl(FetchControl fetchCtrl);
    void addDecodeControl(DecodeControl decodeCtrl);
    void addExecuteControl(ExecuteControl exeCtrl);
    void addMemoryControl(MemoryControl memCtrl);
    void addWriteBackControl(WriteBackControl wbCtrl);

    void addFetchStall();
    void addDecodeStall();
    void addExecuteStall();
    void addMemoryStall();
    void addWriteBackStall();

    
    // Handle stalls and flushes
    void flushPipeline();
    
    // Check if any stage has pending instructions
    bool hasPendingInstructions() const;
    
    // Clear all queues
    void reset();
};

#endif // CONTROL_UNIT_H
