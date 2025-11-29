//
// Created by bansal3112 on 29/11/25.
//

#include "GameClient.hpp"
#include "Shared.hpp"
#include <iostream>
#include <cstdint>

int main(int argc, char* argv[]) {
    using namespace CoinCollector;

    std::string serverHost = "127.0.0.1";
    uint16_t serverPort = SERVER_PORT;

    if (argc > 1) {
        serverHost = argv[1];
    }
    if (argc > 2) {
        serverPort = static_cast<uint16_t>(std::atoi(argv[2]));
    }

    std::cout << "=== Coin Collector Multiplayer Client ===" << std::endl;
    std::cout << "Connecting to: " << serverHost << ":" << serverPort << std::endl;
    std::cout << "Simulated Latency: " << SIMULATED_LATENCY_MS << " ms" << std::endl;
    std::cout << "Controls: Arrow Keys or WASD" << std::endl;
    std::cout << "==========================================" << std::endl;

    try {
        GameClient client(serverHost, serverPort);

        if (!client.connect()) {
            std::cerr << "Failed to connect to server" << std::endl;
            return 1;
        }

        std::cout << "Connected to server successfully" << std::endl;

        // Main client loop (handles input, prediction, reconciliation, rendering)
        client.run();

        client.disconnect();
        std::cout << "Disconnected from server" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}