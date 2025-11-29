//
// Created by bansal3112 on 29/11/25.
//

#include "GameServer.hpp"
#include "Shared.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <cstdint>
#include <ctime>
#include <cstdlib>

std::atomic<bool> g_running(true);

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutdown signal received..." << std::endl;
        g_running = false;
    }
}

int main(int argc, char* argv[]) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    using namespace CoinCollector;

    // Setup signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    uint16_t port = SERVER_PORT;
    if (argc > 1) {
        port = static_cast<uint16_t>(std::atoi(argv[1]));
    }

    std::cout << "=== Coin Collector Multiplayer Server ===" << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Tick Rate: " << TICK_RATE << " Hz" << std::endl;
    std::cout << "Simulated Latency: " << SIMULATED_LATENCY_MS << " ms" << std::endl;
    std::cout << "==========================================" << std::endl;

    try {
        GameServer server(port);

        if (!server.start()) {
            std::cerr << "Failed to start server" << std::endl;
            return 1;
        }

        std::cout << "Server started successfully" << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;

        // Main game loop
        server.run(g_running);

        std::cout << "Server shutting down..." << std::endl;
        server.stop();

    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

