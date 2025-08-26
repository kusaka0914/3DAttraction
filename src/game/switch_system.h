#pragma once

#include "game_state.h"

// スイッチシステム
class SwitchSystem {
public:
    // スイッチの更新
    static void updateSwitches(GameState& gameState, float deltaTime);
    
    // 衝突判定
    static bool checkSwitchCollision(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize);
};

