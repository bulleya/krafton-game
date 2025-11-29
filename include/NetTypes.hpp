//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_NETTYPES_HPP
#define KRAFTON_NETTYPES_HPP
#pragma once

#include "Shared.hpp"
#include <vector>
#include <cstring>

namespace CoinCollector {

// Packet types
enum class PacketType : uint8_t {
    Handshake = 1,
    Input = 2,
    WorldState = 3,
    Event = 4,
    Ping = 5,
    Pong = 6
};

// Base packet header (6 bytes)
struct PacketHeader {
    PacketType type;
    uint32_t sequenceId;
    uint16_t payloadSize;

    PacketHeader() : type(PacketType::Handshake), sequenceId(0), payloadSize(0) {}
    PacketHeader(PacketType t, uint32_t seq, uint16_t size)
        : type(t), sequenceId(seq), payloadSize(size) {}
};

// Lightweight byte buffer for serialization
class ByteBuffer {
public:
    ByteBuffer() { data_.reserve(1024); }
    explicit ByteBuffer(size_t capacity) { data_.reserve(capacity); }
    explicit ByteBuffer(const std::vector<uint8_t>& buf) : data_(buf), readPos_(0) {}

    // Write methods
    void writeUint8(uint8_t value) {
        data_.push_back(value);
    }

    void writeUint16(uint16_t value) {
        data_.push_back(static_cast<uint8_t>(value & 0xFF));
        data_.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    }

    void writeUint32(uint32_t value) {
        data_.push_back(static_cast<uint8_t>(value & 0xFF));
        data_.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        data_.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        data_.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    }

    void writeFloat(float value) {
        uint32_t temp;
        std::memcpy(&temp, &value, sizeof(float));
        writeUint32(temp);
    }

    void writeBool(bool value) {
        writeUint8(value ? 1 : 0);
    }

    // Read methods
    uint8_t readUint8() {
        if (readPos_ + 1 > data_.size()) return 0;
        return data_[readPos_++];
    }

    uint16_t readUint16() {
        if (readPos_ + 2 > data_.size()) return 0;
        uint16_t value = data_[readPos_] | (data_[readPos_ + 1] << 8);
        readPos_ += 2;
        return value;
    }

    uint32_t readUint32() {
        if (readPos_ + 4 > data_.size()) return 0;
        uint32_t value = data_[readPos_] | (data_[readPos_ + 1] << 8) |
                         (data_[readPos_ + 2] << 16) | (data_[readPos_ + 3] << 24);
        readPos_ += 4;
        return value;
    }

    float readFloat() {
        uint32_t temp = readUint32();
        float value;
        std::memcpy(&value, &temp, sizeof(float));
        return value;
    }

    bool readBool() {
        return readUint8() != 0;
    }

    const uint8_t* data() const { return data_.data(); }
    size_t size() const { return data_.size(); }
    void clear() { data_.clear(); readPos_ = 0; }
    size_t remaining() const { return data_.size() - readPos_; }

private:
    std::vector<uint8_t> data_;
    size_t readPos_ = 0;
};

} // namespace CoinCollector
#endif //KRAFTON_NETTYPES_HPP