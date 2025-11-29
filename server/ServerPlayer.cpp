//
// Created by bansal3112 on 29/11/25.
//

#include "ServerPlayer.hpp"

#include "GameProtocol.hpp"
#include "NetTypes.hpp"

namespace CoinCollector {

    ServerPlayer::ServerPlayer(PlayerID id, SocketType socket)
        : socket_(socket), inputBuffer_(SIMULATED_LATENCY_MS), lastProcessedSeq_(0) {
        state_.id = id;
        receiveBuffer_.reserve(4096);
    }

    void ServerPlayer::appendReceiveBuffer(const uint8_t* data, size_t size) {
        receiveBuffer_.insert(receiveBuffer_.end(), data, data + size);
    }

    void ServerPlayer::processPackets() {
        while (receiveBuffer_.size() >= 7) { // Minimum: header size
            // Parse header
            ByteBuffer headerBuf(std::vector<uint8_t>(
                receiveBuffer_.begin(), receiveBuffer_.begin() + 7));
            PacketHeader header = GameProtocol::deserializeHeader(headerBuf);

            size_t totalSize = 7 + header.payloadSize;
            if (receiveBuffer_.size() < totalSize) {
                break; // Wait for more data
            }

            // Extract full packet
            std::vector<uint8_t> packetData(
                receiveBuffer_.begin() + 7,
                receiveBuffer_.begin() + totalSize);
            ByteBuffer payloadBuf(packetData);

            // Process based on type
            if (header.type == PacketType::Input) {
                InputState input = GameProtocol::deserializeInput(payloadBuf);

                InputPacket inputPacket;
                inputPacket.sequenceId = header.sequenceId;
                inputPacket.input = input;

                // Push through latency buffer
                inputBuffer_.push(inputPacket);
            }

            // Remove processed packet from buffer
            receiveBuffer_.erase(receiveBuffer_.begin(),
                                receiveBuffer_.begin() + totalSize);
        }
    }

    bool ServerPlayer::popInput(InputPacket& out) {
        return inputBuffer_.popReady(out);
    }

} // namespace CoinCollector