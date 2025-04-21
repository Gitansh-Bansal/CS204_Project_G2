// branch_predictor.cpp
#include "branch_predictor.h"
#include <iostream>
#include <iomanip>

BranchPredictor::BranchPredictor()
    : total_predictions(0),
      correct_predictions(0),
      branch_mispredictions(0) {
    reset();
}

void BranchPredictor::reset() {
    pht.clear();
    btb.clear();
    total_predictions = 0;
    correct_predictions = 0;
    branch_mispredictions = 0;
}

bool BranchPredictor::predict(uint32_t pc) {
    total_predictions++;
    
    // Align PC to 4-byte boundary
    uint32_t aligned_pc = pc & 0xFFFFFFFC;
    
    // Check if we have a prediction for this PC
    auto it = pht.find(aligned_pc);
    if (it == pht.end()) {
        // No prediction yet, default to not taken
        pht[aligned_pc] = false;
        return false;
    }
    
    // Return the stored prediction
    return it->second;
}

uint32_t BranchPredictor::get_target(uint32_t pc) {
    // Align PC to 4-byte boundary
    uint32_t aligned_pc = pc & 0xFFFFFFFC;
    
    // Check if we have a target for this PC
    auto it = btb.find(aligned_pc);
    if (it == btb.end()) {
        // No target yet, default to PC+4
        return pc + 4;
    }
    
    // Return the stored target
    return it->second;
}

bool BranchPredictor::was_predicted_taken(uint32_t pc) {
    // Align PC to 4-byte boundary
    uint32_t aligned_pc = pc & 0xFFFFFFFC;
    
    // Check if we have a prediction for this PC
    auto it = pht.find(aligned_pc);
    if (it == pht.end()) {
        // No prediction yet, default to not taken
        return false;
    }
    
    // Return the stored prediction
    return it->second;
}

void BranchPredictor::update(uint32_t pc, uint32_t target, bool taken) {
    // Align PC to 4-byte boundary
    uint32_t aligned_pc = pc & 0xFFFFFFFC;
    
    // Update BTB with the target address
    btb[aligned_pc] = target;
    
    // Check if prediction was correct
    bool predicted_taken = was_predicted_taken(aligned_pc);
    if (predicted_taken == taken) {
        correct_predictions++;
    } else {
        branch_mispredictions++;
    }
    
    // Update prediction - for one-bit predictor, just set to the last outcome
    pht[aligned_pc] = taken;
}

void BranchPredictor::print_state() const {
    std::cout << "Branch Predictor State:" << std::endl;
    std::cout << "======================" << std::endl;
    
    // Print PHT entries
    std::cout << "Pattern History Table (PHT):" << std::endl;
    std::cout << "  PC\t\tPrediction" << std::endl;
    for (const auto& entry : pht) {
        std::cout << "  0x" << std::hex << std::setw(8) << std::setfill('0') << entry.first
                  << "\t" << (entry.second ? "Taken" : "Not Taken") << std::dec << std::endl;
    }
    
    // Print BTB entries
    std::cout << "Branch Target Buffer (BTB):" << std::endl;
    std::cout << "  PC\t\tTarget" << std::endl;
    for (const auto& entry : btb) {
        std::cout << "  0x" << std::hex << std::setw(8) << std::setfill('0') << entry.first
                  << "\t0x" << std::setw(8) << std::setfill('0') << entry.second << std::dec << std::endl;
    }
    
    // Print statistics
    std::cout << "Statistics:" << std::endl;
    std::cout << "  Total Predictions: " << total_predictions << std::endl;
    std::cout << "  Correct Predictions: " << correct_predictions << std::endl;
    std::cout << "  Branch Mispredictions: " << branch_mispredictions << std::endl;
    std::cout << "  Accuracy: " << std::fixed << std::setprecision(2) 
              << (total_predictions > 0 ? 
                  (static_cast<double>(correct_predictions) / total_predictions * 100.0) : 0.0)
              << "%" << std::endl;
}
