//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_CLIENTNETWORK_HPP
#define KRAFTON_CLIENTNETWORK_HPP


#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "NetTypes.hpp"
#include "Shared.hpp"
#include "LagSimulator.hpp"


typedef int SocketType;

namespace CoinCollector {

    struct WorldStatePacket {
        uint32_t tick;
        std::vector<PlayerState> players;
        std::vector<CoinState> coins;
    };

    class ClientNetwork {
    public:
        ClientNetwork(const std::string& host, uint16_t port);
        ~ClientNetwork();

        bool connect();
        void disconnect();
        void update();

        void send(const ByteBuffer& data);
        bool popWorldState(WorldStatePacket& out);

        PlayerID getPlayerId() const { return assignedPlayerId_; }

    private:
        void receive();
        void processPackets();
        bool setNonBlocking();
        bool setTcpNoDelay();

        std::string host_;
        uint16_t port_;
        SocketType socket_;

        std::vector<uint8_t> receiveBuffer_;
        LatencyBuffer<ByteBuffer> outgoingBuffer_;
        LatencyBuffer<WorldStatePacket> incomingWorldStates_;

        bool connected_;

        PlayerID assignedPlayerId_ = 0;
    };

} // namespace CoinCollector

#endif //KRAFTON_CLIENTNETWORK_HPP