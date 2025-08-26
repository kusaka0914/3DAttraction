#pragma once

#include "game_state.h"

// 大砲システム
class CannonSystem {
public:
    // 大砲の更新
    static void updateCannons(GameState& gameState, float deltaTime);
    
    // 衝突判定
    static void checkCannonCollision(GameState& gameState, const glm::vec3& playerPosition, const glm::vec3& playerSize);
};

