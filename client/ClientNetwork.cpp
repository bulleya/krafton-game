//
// Created by bansal3112 on 29/11/25.
//

#include "ClientNetwork.hpp"
#include "GameProtocol.hpp"
#include "Shared.hpp"
#include <iostream>
#include <string>
#include <netinet/tcp.h>


namespace CoinCollector {

ClientNetwork::ClientNetwork(const std::string& host, uint16_t port)
    : host_(host), port_(port), socket_(INVALID_SOCKET_VALUE),
      outgoingBuffer_(SIMULATED_LATENCY_MS),
      incomingWorldStates_(SIMULATED_LATENCY_MS),
      connected_(false) {

    receiveBuffer_.reserve(8192);
}

ClientNetwork::~ClientNetwork() {
    disconnect();
}

bool ClientNetwork::connect() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[ClientNetwork] WSAStartup failed" << std::endl;
        return false;
    }
#endif
    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == INVALID_SOCKET_VALUE) {
        std::cerr << "[ClientNetwork] Failed to create socket" << std::endl;
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_);
    if (inet_pton(AF_INET, host_.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return false;
    }
    if (::connect(socket_, reinterpret_cast<sockaddr*>(&serverAddr),
                  sizeof(serverAddr)) < 0) {
        std::cerr << "[ClientNetwork] Connection failed" << std::endl;
        return false;
    }

    setNonBlocking();
    setTcpNoDelay();

    connected_ = true;
    std::cout << "[ClientNetwork] Connected to " << host_ << ":" << port_ << std::endl;
    return true;
}

void ClientNetwork::disconnect() {
    if (socket_ != INVALID_SOCKET_VALUE) {
        closesocket(socket_);
        close(socket_);
        socket_ = INVALID_SOCKET_VALUE;
    }
    connected_ = false;
#ifdef _WIN32
    WSACleanup();
#endif
}

void ClientNetwork::update() {
    if (!connected_) return;

    receive();
    processPackets();

    // Send buffered packets
    ByteBuffer packet;
    while (outgoingBuffer_.popReady(packet)) {
        ::send(socket_, reinterpret_cast<const char*>(packet.data()),
              packet.size(), 0);
    }
}

void ClientNetwork::send(const ByteBuffer& data) {
    outgoingBuffer_.push(data);
}

bool ClientNetwork::popWorldState(WorldStatePacket& out) {
    return incomingWorldStates_.popReady(out);
}

void ClientNetwork::receive() {
    uint8_t buffer[4096];
    int received = recv(socket_, reinterpret_cast<char*>(buffer),
                       sizeof(buffer), 0);

    if (received > 0) {
        receiveBuffer_.insert(receiveBuffer_.end(), buffer, buffer + received);
    } else if (received == 0) {
        std::cout << "[ClientNetwork] Server closed connection" << std::endl;
        connected_ = false;
    }
}

void ClientNetwork::processPackets() {
    while (receiveBuffer_.size() >= 7) {
        ByteBuffer headerBuf(std::vector<uint8_t>(
            receiveBuffer_.begin(), receiveBuffer_.begin() + 7));
        PacketHeader header = GameProtocol::deserializeHeader(headerBuf);

        size_t totalSize = 7 + header.payloadSize;
        if (receiveBuffer_.size() < totalSize) {
            break;
        }

        std::vector<uint8_t> packetData(
            receiveBuffer_.begin() + 7,
            receiveBuffer_.begin() + totalSize);
        ByteBuffer payloadBuf(packetData);
        if (header.type == PacketType::Handshake) {
            std::cout << "[ClientNetwork] RECEIVED HANDSHAKE PACKET!" << std::endl;
            // Verify the ID inside
            assignedPlayerId_ = GameProtocol::deserializeHandshakeResponse(payloadBuf);
            std::cout << "[ClientNetwork] Server assigned me ID: " << assignedPlayerId_ << std::endl;
        }

        if (header.type == PacketType::WorldState) {
            WorldStatePacket worldState;
            GameProtocol::deserializeWorldState(
                payloadBuf, worldState.tick,
                worldState.players, worldState.coins);

            incomingWorldStates_.push(worldState);
        }

        receiveBuffer_.erase(receiveBuffer_.begin(),
                            receiveBuffer_.begin() + totalSize);
    }
}

bool ClientNetwork::setNonBlocking() {
#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket(socket_, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(socket_, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(socket_, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}

bool ClientNetwork::setTcpNoDelay() {
    int flag = 1;
    return setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY,
                     reinterpret_cast<const char*>(&flag), sizeof(flag)) == 0;
}

} // namespace CoinCollector