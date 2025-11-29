//
// Created by bansal3112 on 29/11/25.
//

#include "../include/Shared.hpp"
#include "../include/GameCommon.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace CoinCollector;

void testLerpBasic() {
    std::cout << "Test: Basic linear interpolation..." << std::endl;

    Vec2 a(0.0f, 0.0f);
    Vec2 b(100.0f, 100.0f);

    Vec2 result1 = GameCommon::lerp(a, b, 0.0f);
    assert(std::abs(result1.x - 0.0f) < 0.001f);
    assert(std::abs(result1.y - 0.0f) < 0.001f);

    Vec2 result2 = GameCommon::lerp(a, b, 0.5f);
    assert(std::abs(result2.x - 50.0f) < 0.001f);
    assert(std::abs(result2.y - 50.0f) < 0.001f);

    Vec2 result3 = GameCommon::lerp(a, b, 1.0f);
    assert(std::abs(result3.x - 100.0f) < 0.001f);
    assert(std::abs(result3.y - 100.0f) < 0.001f);

    std::cout << "  PASSED" << std::endl;
}

void testLerpNegative() {
    std::cout << "Test: Interpolation with negative coordinates..." << std::endl;

    Vec2 a(-50.0f, -50.0f);
    Vec2 b(50.0f, 50.0f);

    Vec2 result = GameCommon::lerp(a, b, 0.5f);
    assert(std::abs(result.x - 0.0f) < 0.001f);
    assert(std::abs(result.y - 0.0f) < 0.001f);

    std::cout << "  PASSED" << std::endl;
}

void testLerpSamePoints() {
    std::cout << "Test: Interpolation between same points..." << std::endl;

    Vec2 a(42.0f, 42.0f);
    Vec2 b(42.0f, 42.0f);

    Vec2 result = GameCommon::lerp(a, b, 0.7f);
    assert(std::abs(result.x - 42.0f) < 0.001f);
    assert(std::abs(result.y - 42.0f) < 0.001f);

    std::cout << "  PASSED" << std::endl;
}

int main() {
    std::cout << "=== Interpolation Tests ===" << std::endl;

    testLerpBasic();
    testLerpNegative();
    testLerpSamePoints();

    std::cout << "\nAll interpolation tests passed!" << std::endl;
    return 0;
}