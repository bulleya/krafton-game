# Multiplayer Coin Collector

**Author:** Divyanshu Bansal  
**Entry Number:** 2021CS50603

## Overview
This project is a real-time multiplayer game developed in C++ using SFML for rendering and raw sockets (BSD/WinSock) for networking. It demonstrates an authoritative server architecture capable of handling significant network latency (200ms) through client-side prediction, server reconciliation, and entity interpolation.

## Core Features
* **Authoritative Server:** All game logic (physics, collisions, scoring) is calculated on the server to prevent cheating.
* **Client-Side Prediction:** Local player input is applied immediately for responsive movement, then verified against server state.
* **Server Reconciliation:** If the client diverges from the server (due to lag or collision), the client snaps back to the correct authoritative state.
* **Entity Interpolation:** Remote players are rendered with a 50-100ms buffer to ensure smooth movement despite low tick rates or packet jitter.
* **Lag Simulation:** Configurable artificial latency to demonstrate network resilience.

## Dependencies
* **C++ Compiler:** GCC/Clang (Linux) or MSVC (Windows) supporting C++17.
* **CMake:** Version 3.15 or higher.
* **SFML:** Version 2.5+ (Graphics, Window, System modules).

## Build Instructions

### Linux (Ubuntu/Debian)
1. Install dependencies:
   ```bash
   sudo apt-get install libsfml-dev cmake g++
   ```
2. Compile the project:
   ```bash
   mkdir build && cd build
   cmake ..
   make -j4
   ```

### Windows
1. Ensure SFML is installed and accessible to CMake.
2. Open the project directory in Visual Studio or use the command line:
   ```cmd
   mkdir build && cd build
   cmake ..
   cmake --build . --config Release
   ```

## How to Run

You need to run the server first, then connect one or more clients.

### 1. Start the Server
Run the server executable. It listens on port 8888 by default.
```bash
./GameServer 8888
```

### 2. Start the Client
Run the client executable. You must provide the IP and Port.
```bash
./GameClient 127.0.0.1 8888
```
*Note: To simulate a multiplayer scenario locally, open a second terminal and run another instance of the client.*

## Configuration (Latency)
The network simulation settings can be modified in `include/Shared.hpp` before compiling:
* `SIMULATED_LATENCY_MS`: Artificial delay added to packets (Default: 200 for assignment requirements).
* `INTERPOLATION_DELAY_MS`: Buffering time for remote entities (Default: 100).
* `TICK_RATE`: Server logic update rate (Default: 60Hz).

## Architecture Details

### Protocol
Communication uses binary packets serialized in `GameProtocol.hpp`.
* **Handshake:** Assigns a unique Player ID upon connection.
* **Input Packet:** Client sends boolean state of WASD/Arrows per tick.
* **World State:** Server sends a snapshot of all player positions, velocities, scores, and active coins.

### Network Flow
1. **Input:** Client captures input → applies locally (prediction) → sends to Server.
2. **Processing:** Server receives input → validates physics → resolves collisions → updates score.
3. **Broadcast:** Server broadcasts the authoritative World State to all clients.
4. **Correction:**
    * **Local Player:** Client compares Server state with history. If a mismatch is found (prediction error), it snaps to Server state and replays subsequent inputs.
    * **Remote Players:** Client stores snapshots in a buffer and linearly interpolates positions based on the render timestamp.

## Controls
* **Movement:** WASD or Arrow Keys.
* **Goal:** Collect yellow coins to increase score.
* **Exit:** Close the window or press `Ctrl+C` in the terminal.