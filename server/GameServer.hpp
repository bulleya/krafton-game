//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_GAMESERVER_HPP
#define KRAFTON_GAMESERVER_HPP


#pragma once
#include "Shared.hpp"
#include <cstdint>
#include <memory>
#include <vector>
#include <atomic>

#include "ServerNetwork.hpp"
#include "ServerPlayer.hpp"

namespace CoinCollector {
    class ServerNetwork;
    class ServerPlayer;

    class GameServer {
    public:
        explicit GameServer(uint16_t port);
        ~GameServer();

        bool start();
        void run(std::atomic<bool>& running);
        void stop();

    private:
        void gameLoop();
        void processInputs();
        void updatePhysics(float dt);
        void checkCollisions();
        void broadcastWorldState();
        void spawnCoins();

        uint16_t port_;
        uint32_t currentTick_;
        std::unique_ptr<ServerNetwork> network_;
        std::vector<ServerPlayer*> players_;
        std::vector<CoinState> coins_;
        TimePoint lastBroadcast_;
    };

} // namespace CoinCollector


#endif //KRAFTON_GAMESERVER_HPP