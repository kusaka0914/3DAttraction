#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "physics_utils.h"
#include "../../game/game_state.h"

namespace PhysicsUtils {

void adjustPlayerPositionForGravity(GameState& gameState, const glm::vec3& platformPosition, 
                                   const glm::vec3& platformSize, const glm::vec3& playerSize, 
                                   const glm::vec3& gravityDirection) {
    if (gravityDirection.y > 0.5f) {
        // 重力反転時：足場の下面に配置
        gameState.playerPosition.y = platformPosition.y - platformSize.y * 0.5f - playerSize.y * 0.5f;
    } else {
        // 通常重力：足場の上面に配置
        gameState.playerPosition.y = platformPosition.y + platformSize.y * 0.5f + playerSize.y * 0.5f;
    }
    gameState.playerVelocity.y = 0.0f;
}

float calculateGravityStrength(float baseGravity, float deltaTime, float timeScale, 
                              const glm::vec3& gravityDirection, GameState& gameState) {
    // 基本重力強度
    float gravityStrength = baseGravity * deltaTime;
    
    // 速度倍率に応じて重力を増強（地面につくスピードを上げる）
    if (timeScale > 1.0f) {
        // 速度倍率の2乗に比例して重力を増強（より明確な効果）
        // 2倍速で4倍の重力、3倍速で9倍の重力
        gravityStrength *= timeScale * GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_TIME_SCALE;
    }
    if(gameState.currentStage==0){
        gravityStrength *= GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_STAGE_0;
    }
    
    // 重力反転時は70%の強度
    if (gravityDirection.y > GameConstants::PhysicsConstants::GRAVITY_DIRECTION_THRESHOLD) {
        gravityStrength *= GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_INVERTED;
    }
    
    // バーストジャンプ中は重力を半分にする
    if (gameState.isBurstJumpActive && !gameState.hasUsedBurstJump) {
        gravityStrength *= GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_BURST_JUMP;
    }
    
    return gravityStrength;
}

} // namespace PhysicsUtils
