//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_PREDICTION_HPP
#define KRAFTON_PREDICTION_HPP


#pragma once

#include "Shared.hpp"
#include "GameCommon.hpp"
#include <deque>
#include <iostream>
#include <algorithm>

namespace CoinCollector {

/**
 * Client-side prediction with server reconciliation
 *
 * Maintains a history of inputs and predicted states.
 * When server state arrives, reconciles and replays inputs if necessary.
 */
class PredictionEngine {
public:
    struct HistoryEntry {
        SequenceID sequenceId;
        InputState input;
        PlayerState predictedState;
    };

    PredictionEngine() : nextSequenceId_(1) {}

    /**
     * Apply input with client-side prediction
     * Returns the sequence ID assigned to this input
     */
    SequenceID applyInput(PlayerState& localPlayer, const InputState& input, float dt) {
        SequenceID seqId = nextSequenceId_++;

        // Apply input to local state (prediction)
        GameCommon::applyInput(localPlayer, input, dt);

        // Store in history for reconciliation
        HistoryEntry entry;
        entry.sequenceId = seqId;
        entry.input = input;
        entry.predictedState = localPlayer;

        history_.push_back(entry);

        // Limit history size (keep last 2 seconds worth at 60Hz = 120 entries)
        if (history_.size() > 120) {
            history_.pop_front();
        }

        return seqId;
    }

    /**
     * Reconcile with authoritative server state
     *
     * @param serverState The authoritative state from server
     * @param lastProcessedSeq The last sequence ID processed by server
     * @param currentPlayer The current local player state (will be updated)
     * @param dt Fixed timestep for replay
     */
    void reconcile(
        const PlayerState& serverState,
        SequenceID lastProcessedSeq,
        PlayerState& currentPlayer,
        float dt
    ) {
        // Remove acknowledged inputs from history
        while (!history_.empty() && history_.front().sequenceId <= lastProcessedSeq) {
            history_.pop_front();
        }

        // Check if we need to reconcile
        if (history_.empty()) {
            // Server is caught up, just use server state
            currentPlayer = serverState;
            return;
        }

        // Find the history entry for lastProcessedSeq
        auto it = std::find_if(history_.begin(), history_.end(),
            [lastProcessedSeq](const HistoryEntry& e) {
                return e.sequenceId == lastProcessedSeq;
            });

        // Calculate prediction error
        Vec2 predictedPos = (it != history_.end()) ? it->predictedState.position : currentPlayer.position;
        Vec2 serverPos = serverState.position;
        float errorMagnitude = (predictedPos - serverPos).length();

        const float RECONCILIATION_THRESHOLD = 5.0f; // pixels

        if (errorMagnitude > RECONCILIATION_THRESHOLD) {
            // Significant mismatch - need to reconcile
            std::cout << "[Reconciliation] Error: " << errorMagnitude
                      << "px, replaying " << history_.size() << " inputs" << std::endl;

            // Start from server state
            currentPlayer = serverState;

            // Replay all unacknowledged inputs
            for (const auto& entry : history_) {
                GameCommon::applyInput(currentPlayer, entry.input, dt);
            }
        } else {
            // Small error or no error - keep prediction
            // This avoids visible snapping for minor discrepancies
        }
    }

    void clear() {
        history_.clear();
        nextSequenceId_ = 1;
    }

    size_t historySize() const { return history_.size(); }

private:
    std::deque<HistoryEntry> history_;
    SequenceID nextSequenceId_;
};

} // namespace CoinCollector

#endif //KRAFTON_PREDICTION_HPP