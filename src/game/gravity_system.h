#pragma once

#include "game_state.h"

// 重力システム
class GravitySystem {
public:
    // 重力反転エリアの更新
    static void updateGravityZones(GameState& gameState, float deltaTime);
};

