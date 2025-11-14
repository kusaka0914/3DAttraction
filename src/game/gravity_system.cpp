#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "gravity_system.h"

void GravitySystem::updateGravityZones(GameState& gameState, float deltaTime) {
    for (auto& zone : gameState.gravityZones) {
        (void)zone; // 警告を回避
    }
    (void)deltaTime; // 警告を回避
}

