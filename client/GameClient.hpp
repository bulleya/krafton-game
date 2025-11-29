//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_GAMECLIENT_HPP
#define KRAFTON_GAMECLIENT_HPP


#pragma once
#include "Shared.hpp"
#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include "Prediction.hpp"
#include "Interpolation.hpp"

namespace CoinCollector {
    class ClientNetwork;
    class Renderer;
    class PredictionEngine;
    class InterpolationEngine;

    class GameClient {
    public:
        GameClient(const std::string& serverHost, uint16_t serverPort);
        ~GameClient();

        bool connect();
        void run();
        void disconnect();

    private:
        void processInput();
        void updatePrediction(float dt);
        void updateInterpolation();
        void render(float alpha);

        std::string serverHost_;
        uint16_t serverPort_;
        PlayerID myPlayerId_;

        std::unique_ptr<ClientNetwork> network_;
        std::unique_ptr<Renderer> renderer_;

        PredictionEngine prediction_;
        InterpolationEngine interpolation_;

        PlayerState localPlayer_;
        std::vector<PlayerState> remotePlayers_;
        std::vector<CoinState> coins_;

        InputState currentInput_;
        uint32_t lastReceivedTick_;
    };

} // namespace CoinCollector

#endif //KRAFTON_GAMECLIENT_HPP