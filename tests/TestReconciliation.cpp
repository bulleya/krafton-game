//
// Created by bansal3112 on 29/11/25.
//

#include "../include/Shared.hpp"
#include "../include/GameCommon.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace CoinCollector;

void testDeterministicPhysics() {
    std::cout << "Test: Deterministic physics application..." << std::endl;

    // Apply same input multiple times, should get same result
    InputState input;
    input.right = true;

    PlayerState player1(1, Vec2(100.0f, 100.0f));
    PlayerState player2(2, Vec2(100.0f, 100.0f));

    float dt = FIXED_DT;

    GameCommon::applyInput(player1, input, dt);
    GameCommon::applyInput(player2, input, dt);

    assert(std::abs(player1.position.x - player2.position.x) < 0.001f);
    assert(std::abs(player1.position.y - player2.position.y) < 0.001f);

    std::cout << "  PASSED" << std::endl;
}

void testInputReplay() {
    std::cout << "Test: Input sequence replay..." << std::endl;

    // Simulate client prediction and server reconciliation
    PlayerState clientState(1, Vec2(100.0f, 100.0f));
    PlayerState serverState(1, Vec2(100.0f, 100.0f));

    float dt = FIXED_DT;

    // Client applies 3 inputs
    InputState input1; input1.right = true;
    InputState input2; input2.right = true;
    InputState input3; input3.up = true;

    GameCommon::applyInput(clientState, input1, dt);
    GameCommon::applyInput(clientState, input2, dt);
    GameCommon::applyInput(clientState, input3, dt);

    // Server processes first 2 inputs
    GameCommon::applyInput(serverState, input1, dt);
    GameCommon::applyInput(serverState, input2, dt);

    // Client reconciles: rewind to server state, replay input3
    PlayerState reconciledState = serverState;
    GameCommon::applyInput(reconciledState, input3, dt);

    // Reconciled state should match original client prediction
    assert(std::abs(clientState.position.x - reconciledState.position.x) < 0.001f);
    assert(std::abs(clientState.position.y - reconciledState.position.y) < 0.001f);

    std::cout << "  PASSED" << std::endl;
}

void testBoundaryClamp() {
    std::cout << "Test: Position boundary clamping..." << std::endl;

    Vec2 pos(WORLD_WIDTH + 100.0f, -50.0f);
    GameCommon::clampPosition(pos);

    assert(pos.x <= WORLD_WIDTH - PLAYER_RADIUS);
    assert(pos.y >= PLAYER_RADIUS);

    std::cout << "  PASSED" << std::endl;
}

int main() {
    std::cout << "=== Reconciliation Tests ===" << std::endl;

    testDeterministicPhysics();
    testInputReplay();
    testBoundaryClamp();

    std::cout << "\nAll reconciliation tests passed!" << std::endl;
    return 0;
}