//
// Created by bansal3112 on 29/11/25.
//

#include "GameClient.hpp"
#include "ClientNetwork.hpp"
#include "Render.hpp"
#include "GameProtocol.hpp"
#include "Prediction.hpp"
#include "Interpolation.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <chrono>
#include <thread>
#include <iostream>

namespace CoinCollector {

GameClient::GameClient(const std::string& serverHost, uint16_t serverPort)
    : serverHost_(serverHost), serverPort_(serverPort), myPlayerId_(0),
      lastReceivedTick_(0) {

    network_ = std::make_unique<ClientNetwork>(serverHost, serverPort);
    renderer_ = std::make_unique<Renderer>();

    localPlayer_.id = 0;
    localPlayer_.position = Vec2(WORLD_WIDTH / 2, WORLD_HEIGHT / 2);
}

GameClient::~GameClient() {
    disconnect();
}

bool GameClient::connect() {
    if (!network_->connect()) {
        return false;
    }

    // Send handshake
    ByteBuffer handshake = GameProtocol::serializeHandshake(0);
    network_->send(handshake);
    std::cout << "Waiting for Player ID..." << std::endl;
    for(int i=0; i<100; i++) {
        network_->update();
        PlayerID id = network_->getPlayerId();
        if (id != 0) {
            myPlayerId_ = id; // <--- SUCCESS! Update the GameClient ID
            std::cout << "GameClient initialized with ID: " << myPlayerId_ << std::endl;
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cerr << "Timed out waiting for Player ID" << std::endl;
    return false;
}

void GameClient::run() {
    auto lastTime = std::chrono::steady_clock::now();
    float accumulator = 0.0f;

    while (renderer_->isOpen()) {
        auto currentTime = std::chrono::steady_clock::now();
        float frameTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        if (frameTime > 0.25f) frameTime = 0.25f;
        accumulator += frameTime;

        // Process events
        sf::Event event;
        while (renderer_->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                renderer_->close();
            }
        }

        // Fixed timestep for game logic
        while (accumulator >= FIXED_DT) {
            processInput();
            updatePrediction(FIXED_DT);
            network_->update();

            // Process incoming world state
            WorldStatePacket worldState;
            if (network_->popWorldState(worldState)) {
                lastReceivedTick_ = worldState.tick;

                // Find local player in world state
                for (const auto& player : worldState.players) {
                    if (player.id == myPlayerId_) {
                        // Reconcile with server
                        localPlayer_.score = player.score;
                        prediction_.reconcile(player, worldState.tick,
                                             localPlayer_, FIXED_DT);
                    } else {
                        // Add to interpolation
                        interpolation_.addSnapshot(player, worldState.tick);
                    }
                }

                coins_ = worldState.coins;
            }

            updateInterpolation();
            accumulator -= FIXED_DT;
        }

        // Render with interpolation alpha
        float alpha = accumulator / FIXED_DT;
        render(alpha);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void GameClient::disconnect() {
    if (network_) {
        network_->disconnect();
    }
}

void GameClient::processInput() {
    if (!renderer_->hasFocus()) {
        return;
    }
    InputState newInput;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        newInput.up = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        newInput.down = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        newInput.left = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        newInput.right = true;
    }

        currentInput_ = newInput;

        // Apply prediction and send to server
        SequenceID seq = prediction_.applyInput(localPlayer_, currentInput_, FIXED_DT);

        ByteBuffer packet = GameProtocol::serializeInput(seq, currentInput_);
        network_->send(packet);

}

void GameClient::updatePrediction(float dt) {
    (void)dt;
    // Prediction already applied in processInput
}

void GameClient::updateInterpolation() {
    remotePlayers_.clear();

    // Get interpolated positions for all remote players
    for (const auto& player : interpolation_.getAllPlayers()) {
        PlayerState interpolated;
        if (interpolation_.getInterpolatedState(player, interpolated)) {
            remotePlayers_.push_back(interpolated);
        }
    }
}

void GameClient::render(float alpha) {
    (void)alpha;
    renderer_->clear();

    // Draw local player (green)
    renderer_->drawPlayer(localPlayer_, sf::Color::Green, true);

    // Draw remote players (red)
    for (const auto& player : remotePlayers_) {
        renderer_->drawPlayer(player, sf::Color::Red, false);
    }

    // Draw coins (yellow)
    for (const auto& coin : coins_) {
        if (coin.active) {
            renderer_->drawCoin(coin);
        }
    }

    // Draw HUD
    renderer_->drawHUD(localPlayer_, lastReceivedTick_);

    renderer_->display();
}

} // namespace CoinCollector