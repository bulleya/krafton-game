//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_RENDER_HPP
#define KRAFTON_RENDER_HPP

#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include "Shared.hpp"


namespace CoinCollector {

    class Renderer{
    public:
        Renderer();
        ~Renderer();

        bool isOpen() const;
        bool pollEvent(sf::Event& event);
        void close();

        void clear();
        void display();

        bool hasFocus() const;

        void drawPlayer(const PlayerState& player, const sf::Color& color, bool isLocal);
        void drawCoin(const CoinState& coin);
        void drawHUD(const PlayerState& localPlayer, uint32_t tick);

    private:
        std::unique_ptr<sf::RenderWindow> window_;
        sf::Font font_;
        bool fontLoaded_;
    };

} // namespace CoinCollector

#endif //KRAFTON_RENDER_HPP