//
// Created by bansal3112 on 29/11/25.
//

#ifndef KRAFTON_SHARED_HPP
#define KRAFTON_SHARED_HPP
#pragma once

#include <cstdint>
#include <chrono>
#include <cmath>
#ifdef _WIN32
    #include <winsock2.h>
    typedef SOCKET SocketType;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>  // Fixes inet_pton
#include <unistd.h>     // Fixes close
#include <fcntl.h>      // Fixes fcntl
#define closesocket close
typedef int SocketType;
#define INVALID_SOCKET_VALUE -1
#define SOCKET_ERROR -1
#endif

namespace CoinCollector {

// Game constants
constexpr uint16_t SERVER_PORT = 8888;
constexpr float WORLD_WIDTH = 950.0f;
constexpr float WORLD_HEIGHT = 550.0f;
constexpr float PLAYER_RADIUS = 20.0f;
constexpr float COIN_RADIUS = 15.0f;
constexpr float MAX_PLAYER_SPEED = 300.0f; // pixels per second
constexpr int MAX_COINS = 10;
constexpr int TICK_RATE = 60; // Hz
constexpr float FIXED_DT = 1.0f / TICK_RATE;
constexpr int SIMULATED_LATENCY_MS = 200;
constexpr int INTERPOLATION_DELAY_MS = 100;

// Type aliases
using PlayerID = uint32_t;
using SequenceID = uint32_t;
using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::steady_clock::duration;

// 2D Vector
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator-(const Vec2& other) const {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2 operator*(float scalar) const {
        return Vec2(x * scalar, y * scalar);
    }

    float lengthSquared() const {
        return x * x + y * y;
    }

    float length() const {
        return std::sqrt(lengthSquared());
    }

    Vec2 normalized() const {
        float len = length();
        if (len > 0.0f) {
            return Vec2(x / len, y / len);
        }
        return Vec2(0.0f, 0.0f);
    }
};

// Player input state
struct InputState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;

    bool operator==(const InputState& other) const {
        return up == other.up && down == other.down &&
               left == other.left && right == other.right;
    }

    bool operator!=(const InputState& other) const {
        return !(*this == other);
    }

    bool hasMovement() const {
        return up || down || left || right;
    }
};

// Player state
struct PlayerState {
    PlayerID id = 0;
    Vec2 position;
    Vec2 velocity;
    uint32_t score = 0;

    PlayerState() = default;
    PlayerState(PlayerID id_, Vec2 pos) : id(id_), position(pos) {}
};

// Coin state
struct CoinState {
    uint32_t id = 0;
    Vec2 position;
    bool active = false;

    CoinState() = default;
    CoinState(uint32_t id_, Vec2 pos, bool active_)
        : id(id_), position(pos), active(active_) {}
};

} // namespace CoinCollector
#endif //KRAFTON_SHARED_HPP