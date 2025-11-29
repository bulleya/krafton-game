//
// Created by bansal3112 on 29/11/25.
//

#include "ServerNetwork.hpp"
#include "ServerPlayer.hpp"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <memory>
#include <utility>

#include "GameProtocol.hpp"

#ifndef _WIN32
    #include <arpa/inet.h>
    #include <netinet/tcp.h>
    #define closesocket close
#endif

namespace CoinCollector {

ServerNetwork::ServerNetwork(uint16_t port)
    : port_(port), listenSocket_(INVALID_SOCKET_VALUE),
      nextPlayerId_(1), outgoingBuffer_(SIMULATED_LATENCY_MS), running_(false) {
}

ServerNetwork::~ServerNetwork() {
    shutdown();
}

bool ServerNetwork::initialize() {
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[ServerNetwork] WSAStartup failed" << std::endl;
        return false;
    }
    #endif


    listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket_ == INVALID_SOCKET_VALUE) {
        std::cerr << "[ServerNetwork] Failed to create socket" << std::endl;
        return false;
    }

    // Set socket options
    int reuse = 1;
    setsockopt(listenSocket_, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    // Bind
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_);

    if (bind(listenSocket_, reinterpret_cast<sockaddr*>(&serverAddr),
             sizeof(serverAddr)) < 0) {
        std::cerr << "[ServerNetwork] Bind failed" << std::endl;
        return false;
    }

    // Listen
    if (listen(listenSocket_, 10) < 0) {
        std::cerr << "[ServerNetwork] Listen failed" << std::endl;
        return false;
    }

    // Set non-blocking
    if (!setNonBlocking(listenSocket_)) {
        std::cerr << "[ServerNetwork] Failed to set non-blocking" << std::endl;
        return false;
    }

    running_ = true;
    std::cout << "[ServerNetwork] Listening on port " << port_ << std::endl;
    return true;
}

void ServerNetwork::update() {
    acceptNewClients();
    receiveFromClients();
    sendToClients();
}

void ServerNetwork::shutdown() {
    running_ = false;

    players_.clear();

    if (listenSocket_ != INVALID_SOCKET_VALUE) {
        closesocket(listenSocket_);
        close(listenSocket_);
        listenSocket_ = INVALID_SOCKET_VALUE;
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

    // Replace the entire getPlayers function with this:
std::vector<ServerPlayer*> ServerNetwork::getPlayers() {
    std::vector<ServerPlayer*> result;
    result.reserve(players_.size());

    for (const auto& player : players_) {
        // Push the raw pointer. Do NOT make_unique.
        result.push_back(player.get());
    }
    return result;
}

void ServerNetwork::broadcast(const ByteBuffer& data) {
    OutgoingPacket packet;
    packet.data = data;
    packet.targetId = 0; // Broadcast
    outgoingBuffer_.push(packet);
}

void ServerNetwork::send(PlayerID playerId, const ByteBuffer& data) {
    OutgoingPacket packet;
    packet.data = data;
    packet.targetId = playerId;
    outgoingBuffer_.push(packet);
}

void ServerNetwork::acceptNewClients() {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);

    SocketType clientSocket = accept(listenSocket_,
        reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);

    if (clientSocket != INVALID_SOCKET_VALUE) {
        setNonBlocking(clientSocket);
        setTcpNoDelay(clientSocket);

        PlayerID newId = nextPlayerId_++;
        auto newPlayer = std::make_unique<ServerPlayer>(newId, clientSocket);

        // Random spawn position
        float randX = 50.0f + static_cast<float>(std::rand() % static_cast<int>(WORLD_WIDTH - 100));

        // Generate random Y between 50 and WORLD_HEIGHT - 50
        float randY = 50.0f + static_cast<float>(std::rand() % static_cast<int>(WORLD_HEIGHT - 100));

        newPlayer->getState().position = Vec2(randX, randY);

        players_.push_back(std::move(newPlayer));

        std::cout << "[ServerNetwork] Client connected: " << newId << std::endl;
        ByteBuffer welcomePacket = GameProtocol::serializeHandshakeResponse(0, newId);
        send(newId, welcomePacket);
    }
}

void ServerNetwork::receiveFromClients() {
    for (auto it = players_.begin(); it != players_.end();) {
        auto& player = *it;

        uint8_t buffer[1024];
        int received = recv(player->getSocket(),
                           reinterpret_cast<char*>(buffer), sizeof(buffer), 0);

        if (received > 0) {
            player->appendReceiveBuffer(buffer, received);
            player->processPackets();
            ++it;
        } else if (received == 0 ||
                   (received < 0 && errno != EWOULDBLOCK && errno != EAGAIN)) {
            std::cout << "[ServerNetwork] Client disconnected: "
                      << player->getId() << std::endl;
            it = players_.erase(it);
        } else {
            ++it;
        }
    }
}

void ServerNetwork::sendToClients() {
    OutgoingPacket packet;
    while (outgoingBuffer_.popReady(packet)) {
        if (packet.targetId == 0) {
            // Broadcast to all
            for (auto& player : players_) {
                ::send(player->getSocket(),
                      reinterpret_cast<const char*>(packet.data.data()),
                      packet.data.size(), 0);
            }
        } else {
            // Send to specific player
            for (auto& player : players_) {
                if (player->getId() == packet.targetId) {
                    ::send(player->getSocket(),
                          reinterpret_cast<const char*>(packet.data.data()),
                          packet.data.size(), 0);
                    break;
                }
            }
        }
    }
}

bool ServerNetwork::setNonBlocking(SocketType socket) {
#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket(socket, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(socket, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}

bool ServerNetwork::setTcpNoDelay(SocketType socket) {
    int flag = 1;
    return setsockopt(socket, IPPROTO_TCP, TCP_NODELAY,
                     reinterpret_cast<const char*>(&flag), sizeof(flag)) == 0;
}

} // namespace CoinCollector