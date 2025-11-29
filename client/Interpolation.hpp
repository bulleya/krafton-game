//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_INTERPOLATION_HPP
#define KRAFTON_INTERPOLATION_HPP
#pragma once

#include "Shared.hpp"
#include "GameCommon.hpp"
#include <map>
#include <deque>
#include <chrono>

namespace CoinCollector {

/**
 * Entity interpolation for remote players
 *
 * Maintains a buffer of snapshots and interpolates between them
 * with a small delay to ensure smooth movement.
 */
class InterpolationEngine {
public:
    struct Snapshot {
        PlayerState state;
        TimePoint timestamp;
        uint32_t tick;
    };

    InterpolationEngine()
        : interpolationDelay_(std::chrono::milliseconds(INTERPOLATION_DELAY_MS)) {}

    /**
     * Add a new snapshot for a player
     */
    void addSnapshot(const PlayerState& state, uint32_t tick) {
        Snapshot snapshot;
        snapshot.state = state;
        snapshot.timestamp = std::chrono::steady_clock::now();
        snapshot.tick = tick;

        auto& buffer = snapshots_[state.id];
        buffer.push_back(snapshot);

        // Keep buffer size reasonable (1 second worth at 60Hz = 60 snapshots)
        if (buffer.size() > 60) {
            buffer.pop_front();
        }
    }

    /**
     * Get interpolated position for a player
     *
     * @param playerId The player to interpolate
     * @param outState Output interpolated state
     * @return true if interpolation was successful
     */
    bool getInterpolatedState(PlayerID playerId, PlayerState& outState) {
        auto it = snapshots_.find(playerId);
        if (it == snapshots_.end() || it->second.size() < 2) {
            return false; // Need at least 2 snapshots to interpolate
        }

        auto& buffer = it->second;
        auto renderTime = std::chrono::steady_clock::now() - interpolationDelay_;

        // Find two snapshots surrounding the render time
        Snapshot* before = nullptr;
        Snapshot* after = nullptr;

        for (size_t i = 0; i < buffer.size() - 1; ++i) {
            if (buffer[i].timestamp <= renderTime && buffer[i + 1].timestamp >= renderTime) {
                before = &buffer[i];
                after = &buffer[i + 1];
                break;
            }
        }

        if (!before || !after) {
            // Render time is outside buffer range, use latest
            outState = buffer.back().state;
            return true;
        }

        // Interpolate between the two snapshots
        auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
            after->timestamp - before->timestamp
        ).count();

        auto timeFromBefore = std::chrono::duration_cast<std::chrono::milliseconds>(
            renderTime - before->timestamp
        ).count();

        float alpha = (timeDiff > 0) ? (static_cast<float>(timeFromBefore) / timeDiff) : 0.0f;
        alpha = std::max(0.0f, std::min(1.0f, alpha)); // Clamp to [0, 1]

        // Interpolate position
        outState = before->state;
        outState.position = GameCommon::lerp(before->state.position, after->state.position, alpha);
        outState.score = after->state.score; // Don't interpolate score

        return true;
    }

    std::vector<PlayerID> getAllPlayers() const {
        std::vector<PlayerID> ids;
        // CHANGE 'buffer' TO 'snapshots_'
        for (const auto& pair : snapshots_) {
            ids.push_back(pair.first);
        }
        return ids;
    }

    /**
     * Remove all snapshots for a player (when they disconnect)
     */
    void removePlayer(PlayerID playerId) {
        snapshots_.erase(playerId);
    }

    /**
     * Clear all snapshots
     */
    void clear() {
        snapshots_.clear();
    }

    /**
     * Get buffer size for a player (debugging)
     */
    size_t getBufferSize(PlayerID playerId) const {
        auto it = snapshots_.find(playerId);
        return (it != snapshots_.end()) ? it->second.size() : 0;
    }

private:
    std::map<PlayerID, std::deque<Snapshot>> snapshots_;
    std::chrono::milliseconds interpolationDelay_;
};

} // namespace CoinCollector
#endif //KRAFTON_INTERPOLATION_HPP