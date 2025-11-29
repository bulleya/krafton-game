//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_LAGSIMULATOR_HPP
#define KRAFTON_LAGSIMULATOR_HPP
#pragma once

#include "Shared.hpp"
#include <deque>
#include <mutex>
#include <chrono>

namespace CoinCollector {

/**
 * LatencyBuffer - Simulates network latency by buffering items
 * and only releasing them after a specified delay.
 *
 * Thread-safe implementation for use in network threads.
 */
template <typename T>
class LatencyBuffer {
public:
    explicit LatencyBuffer(int latencyMs = SIMULATED_LATENCY_MS)
        : latency_(std::chrono::milliseconds(latencyMs)) {}

    /**
     * Add an item to the buffer with a release time of now + latency
     */
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        Item bufferedItem;
        bufferedItem.data = item;
        bufferedItem.releaseTime = std::chrono::steady_clock::now() + latency_;
        buffer_.push_back(bufferedItem);
    }

    /**
     * Try to pop an item if one is ready (release time has passed)
     * Returns true if an item was popped, false otherwise
     */
    bool popReady(T& out) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (buffer_.empty()) {
            return false;
        }

        auto now = std::chrono::steady_clock::now();
        if (buffer_.front().releaseTime <= now) {
            out = std::move(buffer_.front().data);
            buffer_.pop_front();
            return true;
        }

        return false;
    }

    /**
     * Change the simulated latency (affects future items only)
     */
    void setLatency(int latencyMs) {
        std::lock_guard<std::mutex> lock(mutex_);
        latency_ = std::chrono::milliseconds(latencyMs);
    }

    /**
     * Get the current buffer size (for debugging/monitoring)
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return buffer_.size();
    }

    /**
     * Clear all buffered items
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_.clear();
    }

private:
    struct Item {
        T data;
        TimePoint releaseTime;
    };

    std::deque<Item> buffer_;
    mutable std::mutex mutex_;
    std::chrono::milliseconds latency_;
};

} // namespace CoinCollector
#endif //KRAFTON_LAGSIMULATOR_HPP