// branch_predictor.h
#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <unordered_map>
#include <cstdint>

class BranchPredictor {
private:
    // Pattern History Table (PHT) - maps PC to prediction bit
    std::unordered_map<uint32_t, bool> pht;
    
    // Branch Target Buffer (BTB) - maps PC to target address
    std::unordered_map<uint32_t, uint32_t> btb;
    
    // Statistics
    uint32_t total_predictions;
    uint32_t correct_predictions;
    uint32_t branch_mispredictions;

public:
    BranchPredictor();
    
    // Reset the predictor
    void reset();
    
    // Predict whether a branch will be taken
    bool predict(uint32_t pc);
    
    // Get the predicted target address
    uint32_t get_target(uint32_t pc);
    
    // Check if the branch was predicted as taken
    bool was_predicted_taken(uint32_t pc);
    
    // Update the predictor with the actual branch outcome
    void update(uint32_t pc, uint32_t target, bool taken);
    
    // Print the state of the branch predictor
    void print_state() const;
    
    // Get statistics
    uint32_t get_total_predictions() const { return total_predictions; }
    uint32_t get_correct_predictions() const { return correct_predictions; }
    uint32_t get_branch_mispredictions() const { return branch_mispredictions; }
};

#endif // BRANCH_PREDICTOR_H
