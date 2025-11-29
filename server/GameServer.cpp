//
// Created by bansal3112 on 29/11/25.
//

#include "GameServer.hpp"
#include "GameCommon.hpp"
#include <iostream>
#include <thread>

#include "GameProtocol.hpp"


namespace CoinCollector {

GameServer::GameServer(uint16_t port)
    : port_(port), currentTick_(0), lastBroadcast_(std::chrono::steady_clock::now()) {
    network_ = std::make_unique<ServerNetwork>(port);
}

GameServer::~GameServer() {
    stop();
}

bool GameServer::start() {
    if (!network_->initialize()) {
        return false;
    }

    // Spawn initial coins
    spawnCoins();

    return true;
}

void GameServer::run(std::atomic<bool>& running) {
    auto lastTime = std::chrono::steady_clock::now();
    float accumulator = 0.0f;

    while (running) {
        auto currentTime = std::chrono::steady_clock::now();
        float frameTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Cap frame time to prevent spiral of death
        if (frameTime > 0.25f) frameTime = 0.25f;

        accumulator += frameTime;

        // Fixed timestep update
        while (accumulator >= FIXED_DT) {
            gameLoop();
            accumulator -= FIXED_DT;
            currentTick_++;
        }

        // Small sleep to prevent CPU spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void GameServer::stop() {
    if (network_) {
        network_->shutdown();
    }
}

void GameServer::gameLoop() {
    // Update network (accept new clients, receive packets)
    network_->update();

    // Get connected players
    players_ = network_->getPlayers();

    // Process client inputs
    processInputs();

    // Update physics
    updatePhysics(FIXED_DT);

    // Check collisions
    checkCollisions();

    // Broadcast world state (every 3 ticks = 20Hz)
    if (currentTick_ % 3 == 0) {
        broadcastWorldState();
    }
}

void GameServer::processInputs() {
    for (auto& player : players_) {
        InputPacket input;
        if (player->popInput(input)) {
            // Apply input with validation
            GameCommon::applyInput(player->getState(), input.input, FIXED_DT);

            // Update last processed sequence
            player->setLastProcessedSeq(input.sequenceId);
        }
    }
}

void GameServer::updatePhysics(float dt) {
    (void)dt;
    // Physics already applied in processInputs
    // Additional physics updates would go here
}

void GameServer::checkCollisions() {
    for (auto& player : players_) {
        PlayerState& playerState = player->getState();

        for (auto& coin : coins_) {
            if (!coin.active) continue;

            if (GameCommon::checkCollision(playerState.position, coin.position)) {
                // Player collected coin
                playerState.score++;
                coin.active = false;

                // Respawn coin at random position
                coin.position = GameCommon::randomCoinPosition();
                coin.active = true;

                std::cout << "[Server] Player " << playerState.id
                          << " collected coin. Score: " << playerState.score << std::endl;
            }
        }
    }
}

void GameServer::broadcastWorldState() {
    // Build player states
    std::vector<PlayerState> playerStates;
    for (const auto& player : players_) {
        playerStates.push_back(player->getState());
    }

    // Serialize world state
    ByteBuffer buffer = GameProtocol::serializeWorldState(
        currentTick_,
        currentTick_,
        playerStates,
        coins_
    );

    // Broadcast to all clients through latency buffer
    network_->broadcast(buffer);
}

void GameServer::spawnCoins() {
    coins_.clear();
    coins_.reserve(MAX_COINS);

    for (int i = 0; i < MAX_COINS; ++i) {
        CoinState coin;
        coin.id = i;
        coin.position = GameCommon::randomCoinPosition();
        coin.active = true;
        coins_.push_back(coin);
    }

    std::cout << "[Server] Spawned " << MAX_COINS << " coins" << std::endl;
}

} // namespace CoinCollector