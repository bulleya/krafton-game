//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_SERVERPLAYER_HPP
#define KRAFTON_SERVERPLAYER_HPP


#pragma once
#include <vector>
#include <cstdint>

#include "LagSimulator.hpp"
#include "Shared.hpp"


namespace CoinCollector {

    struct InputPacket {
        SequenceID sequenceId;
        InputState input;
    };

    class ServerPlayer {
    public:
        ServerPlayer(PlayerID id, SocketType socket);

        PlayerID getId() const { return state_.id; }
        SocketType getSocket() const { return socket_; }
        PlayerState& getState() { return state_; }
        const PlayerState& getState() const { return state_; }

        void appendReceiveBuffer(const uint8_t* data, size_t size);
        void processPackets();
        bool popInput(InputPacket& out);

        void setLastProcessedSeq(SequenceID seq) { lastProcessedSeq_ = seq; }
        SequenceID getLastProcessedSeq() const { return lastProcessedSeq_; }

    private:
        PlayerState state_;
        SocketType socket_;
        std::vector<uint8_t> receiveBuffer_;
        LatencyBuffer<InputPacket> inputBuffer_;
        SequenceID lastProcessedSeq_;
    };

} // namespace CoinCollector

#endif //KRAFTON_SERVERPLAYER_HPP