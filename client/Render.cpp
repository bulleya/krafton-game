//
// Created by bansal3112 on 29/11/25.
//

#include "Render.hpp"

#include <iostream>
#include <sstream>
#include <memory>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "Shared.hpp"

namespace CoinCollector {

Renderer::Renderer() : fontLoaded_(false) {
    window_ = std::make_unique<sf::RenderWindow>(
        sf::VideoMode(static_cast<unsigned int>(WORLD_WIDTH),
                     static_cast<unsigned int>(WORLD_HEIGHT)),
        "Coin Collector Multiplayer"
    );
    window_->setFramerateLimit(144);

    // Try to load default font (may not work on all systems)
    fontLoaded_ = font_.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") ||
                  font_.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");
}

Renderer::~Renderer() {
    if (window_) {
        window_->close();
    }
}

bool Renderer::isOpen() const {
    return window_ && window_->isOpen();
}

bool Renderer::pollEvent(sf::Event& event) {
    return window_->pollEvent(event);
}

void Renderer::close() {
    if (window_) {
        window_->close();
    }
}

void Renderer::clear() {
    window_->clear(sf::Color(30, 30, 30));
}

void Renderer::display() {
    window_->display();
}

bool Renderer::hasFocus() const {
    return window_->hasFocus();
}

void Renderer::drawPlayer(const PlayerState& player, const sf::Color& color, bool isLocal) {
    // 1. Draw the Player Circle (Existing Code)
    sf::CircleShape circle(PLAYER_RADIUS);
    circle.setOrigin(PLAYER_RADIUS, PLAYER_RADIUS);
    circle.setPosition(player.position.x, player.position.y);
    circle.setFillColor(color);

    if (isLocal) {
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(2);
    }

    window_->draw(circle);

    // 2. Draw the ID Text (New Code)
    if (fontLoaded_) {
        sf::Text idText;
        idText.setFont(font_);
        idText.setString(std::to_string(player.id));
        idText.setCharacterSize(14); // Readable size
        idText.setFillColor(sf::Color::White);

        // Make text black for remote players (red background) for better contrast
        if (!isLocal) {
            idText.setFillColor(sf::Color::Black); // Black text on Red player
        } else {
            idText.setFillColor(sf::Color::Black); // Black text on Green player
        }

        // Center the text on the player
        sf::FloatRect textBounds = idText.getLocalBounds();
        idText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                         textBounds.top + textBounds.height / 2.0f);
        idText.setPosition(player.position.x, player.position.y);

        window_->draw(idText);
    }
}

void Renderer::drawCoin(const CoinState& coin) {
    sf::CircleShape circle(COIN_RADIUS);
    circle.setPosition(coin.position.x - COIN_RADIUS,
                       coin.position.y - COIN_RADIUS);
    circle.setFillColor(sf::Color::Yellow);
    window_->draw(circle);
}

void Renderer::drawHUD(const PlayerState& localPlayer, uint32_t tick) {
    if (!fontLoaded_) return;

    std::ostringstream oss;
    oss << "Score: " << localPlayer.score << "\n";
    oss << "Tick: " << tick << "\n";
    oss << "Latency: " << SIMULATED_LATENCY_MS << "ms (simulated)";

    sf::Text text(oss.str(), font_, 16);
    text.setPosition(10, 10);
    text.setFillColor(sf::Color::White);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(1);

    window_->draw(text);
}

} // namespace CoinCollector