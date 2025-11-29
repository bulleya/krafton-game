cmake_minimum_required(VERSION 3.15)
project(CoinCollectorMP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic )
endif()

# Find SFML (required for client only)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Platform-specific socket libraries
if(WIN32)
    set(SOCKET_LIBS ws2_32)
else()
    set(SOCKET_LIBS pthread)
endif()

# Server executable
add_executable(GameServer
        server/ServerMain.cpp
        server/GameServer.cpp
        server/ServerNetwork.cpp
        server/ServerPlayer.cpp
)
target_link_libraries(GameServer ${SOCKET_LIBS})

# Client executable
add_executable(GameClient
        client/ClientMain.cpp
        client/GameClient.cpp
        client/ClientNetwork.cpp
        client/Prediction.cpp
        client/Interpolation.cpp
        client/Render.cpp
)
target_link_libraries(GameClient
        sfml-graphics
        sfml-window
        sfml-system
        ${SOCKET_LIBS}
)

# Tests
add_executable(TestInterpolation tests/TestInterpolation.cpp)
add_executable(TestReconciliation tests/TestReconciliation.cpp)

# Install targets
install(TARGETS GameServer GameClient DESTINATION bin)