//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_GAMECOMMON_HPP
#define KRAFTON_GAMECOMMON_HPP
#pragma once

#include "Shared.hpp"
#include <cmath>
#include <random>

namespace CoinCollector {

/**
 * Common game logic shared between client and server
 */
class GameCommon {
public:
    /**
     * Apply input to player state and integrate physics
     * This MUST be deterministic for client-side prediction to work
     */
    static void applyInput(PlayerState& player, const InputState& input, float dt) {
        Vec2 acceleration(0.0f, 0.0f);

        if (input.up) acceleration.y -= 1.0f;
        if (input.down) acceleration.y += 1.0f;
        if (input.left) acceleration.x -= 1.0f;
        if (input.right) acceleration.x += 1.0f;

        // Normalize diagonal movement
        if (acceleration.lengthSquared() > 0.0f) {
            acceleration = acceleration.normalized();
        }

        // Simple velocity integration
        player.velocity = acceleration * MAX_PLAYER_SPEED;

        // Update position
        player.position = player.position + player.velocity * dt;

        // Clamp to world bounds
        clampPosition(player.position);
    }

    /**
     * Clamp position to world boundaries
     */
    static void clampPosition(Vec2& pos) {
        if (pos.x < PLAYER_RADIUS) pos.x = PLAYER_RADIUS;
        if (pos.x > WORLD_WIDTH - PLAYER_RADIUS) pos.x = WORLD_WIDTH - PLAYER_RADIUS;
        if (pos.y < PLAYER_RADIUS) pos.y = PLAYER_RADIUS;
        if (pos.y > WORLD_HEIGHT - PLAYER_RADIUS) pos.y = WORLD_HEIGHT - PLAYER_RADIUS;
    }

    /**
     * Check collision between player and coin
     */
    static bool checkCollision(const Vec2& playerPos, const Vec2& coinPos) {
        float distSq = (playerPos - coinPos).lengthSquared();
        float radiusSum = PLAYER_RADIUS + COIN_RADIUS;
        return distSq < (radiusSum * radiusSum);
    }

    /**
     * Generate random position for coin spawn
     */
    static Vec2 randomCoinPosition() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> distX(
            COIN_RADIUS, WORLD_WIDTH - COIN_RADIUS
        );
        static std::uniform_real_distribution<float> distY(
            COIN_RADIUS, WORLD_HEIGHT - COIN_RADIUS
        );

        return Vec2(distX(gen), distY(gen));
    }

    /**
     * Linear interpolation between two positions
     */
    static Vec2 lerp(const Vec2& a, const Vec2& b, float t) {
        return a + (b - a) * t;
    }
};

} // namespace CoinCollector
#endif //KRAFTON_GAMECOMMON_HPP