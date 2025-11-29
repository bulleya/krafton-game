//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_GAMEPROTOCOL_HPP
#define KRAFTON_GAMEPROTOCOL_HPP
#pragma once

#include "NetTypes.hpp"
#include "Shared.hpp"

namespace CoinCollector {

/**
 * Game Protocol - Serialization and deserialization for all packet types
 */
class GameProtocol {
public:
    // Serialize header
    static void serializeHeader(ByteBuffer& buffer, const PacketHeader& header) {
        buffer.writeUint8(static_cast<uint8_t>(header.type));
        buffer.writeUint32(header.sequenceId);
        buffer.writeUint16(header.payloadSize);
    }

    // Deserialize header
    static PacketHeader deserializeHeader(ByteBuffer& buffer) {
        PacketHeader header;
        header.type = static_cast<PacketType>(buffer.readUint8());
        header.sequenceId = buffer.readUint32();
        header.payloadSize = buffer.readUint16();
        return header;
    }

    // Serialize handshake packet (client -> server)
    static ByteBuffer serializeHandshake(SequenceID seq) {
        ByteBuffer buffer;
        PacketHeader header(PacketType::Handshake, seq, 0);
        serializeHeader(buffer, header);
        return buffer;
    }

    // Serialize input packet
    static ByteBuffer serializeInput(SequenceID seq, const InputState& input) {
        ByteBuffer buffer;
        PacketHeader header(PacketType::Input, seq, 4); // 4 bools = 4 bytes
        serializeHeader(buffer, header);

        buffer.writeBool(input.up);
        buffer.writeBool(input.down);
        buffer.writeBool(input.left);
        buffer.writeBool(input.right);

        return buffer;
    }

    // Deserialize input packet
    static InputState deserializeInput(ByteBuffer& buffer) {
        InputState input;
        input.up = buffer.readBool();
        input.down = buffer.readBool();
        input.left = buffer.readBool();
        input.right = buffer.readBool();
        return input;
    }
    static ByteBuffer serializeHandshakeResponse(SequenceID seq, PlayerID playerId) {
        ByteBuffer buffer;
        // Payload: 4 bytes for PlayerID (uint32_t)
        PacketHeader header(PacketType::Handshake, seq, sizeof(uint32_t));
        serializeHeader(buffer, header);

        buffer.writeUint32(playerId); // Write the ID into the packet
        return buffer;
    }

    // NEW: Deserialize handshake response to extract the ID
    static PlayerID deserializeHandshakeResponse(ByteBuffer& buffer) {
        return buffer.readUint32();
    }

    // Serialize world state packet
    static ByteBuffer serializeWorldState(
        SequenceID seq,
        uint32_t tick,
        const std::vector<PlayerState>& players,
        const std::vector<CoinState>& coins
    ) {
        ByteBuffer buffer;

        // Calculate payload size
        uint16_t payloadSize = 4; // tick number
        payloadSize += 1; // player count
        payloadSize += players.size() * (4 + 8 + 8 + 4); // id + pos + vel + score
        payloadSize += 1; // coin count
        payloadSize += coins.size() * (4 + 8 + 1); // id + pos + active

        PacketHeader header(PacketType::WorldState, seq, payloadSize);
        serializeHeader(buffer, header);

        // Tick number
        buffer.writeUint32(tick);

        // Players
        buffer.writeUint8(static_cast<uint8_t>(players.size()));
        for (const auto& player : players) {
            buffer.writeUint32(player.id);
            buffer.writeFloat(player.position.x);
            buffer.writeFloat(player.position.y);
            buffer.writeFloat(player.velocity.x);
            buffer.writeFloat(player.velocity.y);
            buffer.writeUint32(player.score);
        }

        // Coins
        buffer.writeUint8(static_cast<uint8_t>(coins.size()));
        for (const auto& coin : coins) {
            buffer.writeUint32(coin.id);
            buffer.writeFloat(coin.position.x);
            buffer.writeFloat(coin.position.y);
            buffer.writeBool(coin.active);
        }

        return buffer;
    }

    // Deserialize world state packet
    static bool deserializeWorldState(
        ByteBuffer& buffer,
        uint32_t& tick,
        std::vector<PlayerState>& players,
        std::vector<CoinState>& coins
    ) {
        players.clear();
        coins.clear();

        // Tick number
        tick = buffer.readUint32();

        // Players
        uint8_t playerCount = buffer.readUint8();
        players.reserve(playerCount);

        for (uint8_t i = 0; i < playerCount; ++i) {
            PlayerState player;
            player.id = buffer.readUint32();
            player.position.x = buffer.readFloat();
            player.position.y = buffer.readFloat();
            player.velocity.x = buffer.readFloat();
            player.velocity.y = buffer.readFloat();
            player.score = buffer.readUint32();
            players.push_back(player);
        }

        // Coins
        uint8_t coinCount = buffer.readUint8();
        coins.reserve(coinCount);

        for (uint8_t i = 0; i < coinCount; ++i) {
            CoinState coin;
            coin.id = buffer.readUint32();
            coin.position.x = buffer.readFloat();
            coin.position.y = buffer.readFloat();
            coin.active = buffer.readBool();
            coins.push_back(coin);
        }

        return true;
    }
};

} // namespace CoinCollector
#endif //KRAFTON_GAMEPROTOCOL_HPP