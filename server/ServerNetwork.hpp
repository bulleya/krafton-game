//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_SERVERNETWORK_HPP
#define KRAFTON_SERVERNETWORK_HPP


#pragma once

#pragma once
#include <vector>
#include <memory>
#include <atomic>
#include <cstdint>

#include "LagSimulator.hpp"
#include "NetTypes.hpp"
#include "ServerPlayer.hpp"
#include "Shared.hpp"



typedef int SocketType;



namespace CoinCollector {

    struct OutgoingPacket {
        ByteBuffer data;
        PlayerID targetId; // 0 = broadcast
    };
    class ServerPlayer;

    class ServerNetwork {
    public:
        explicit ServerNetwork(uint16_t port);
        ~ServerNetwork();

        bool initialize();
        void update();
        void shutdown();

        std::vector<ServerPlayer*> getPlayers();
        void broadcast(const ByteBuffer& data);
        void send(PlayerID playerId, const ByteBuffer& data);

    private:
        void acceptNewClients();
        void receiveFromClients();
        void sendToClients();
        void disconnectClient(PlayerID playerId);
        bool setNonBlocking(SocketType socket);
        bool setTcpNoDelay(SocketType socket);

        uint16_t port_;
        SocketType listenSocket_;
        std::vector<std::unique_ptr<ServerPlayer>> players_;
        PlayerID nextPlayerId_;

        LatencyBuffer<OutgoingPacket> outgoingBuffer_;

        std::atomic<bool> running_;
    };

} // namespace CoinCollector

#endif //KRAFTON_SERVERNETWORK_HPP