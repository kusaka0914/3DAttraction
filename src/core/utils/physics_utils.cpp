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
        gameState.player.position.y = platformPosition.y - platformSize.y * 0.5f - playerSize.y * 0.5f;
    } else {
        gameState.player.position.y = platformPosition.y + platformSize.y * 0.5f + playerSize.y * 0.5f;
    }
    gameState.player.velocity.y = 0.0f;
}

float calculateGravityStrength(float baseGravity, float deltaTime, float timeScale, 
                              const glm::vec3& gravityDirection, GameState& gameState) {
    float gravityStrength = baseGravity * deltaTime;
    
    if (timeScale > 1.0f) {
        gravityStrength *= timeScale * GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_TIME_SCALE;
    }
    if(gameState.progress.currentStage==0){
        gravityStrength *= GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_STAGE_0;
    }
    
    if (gravityDirection.y > GameConstants::PhysicsConstants::GRAVITY_DIRECTION_THRESHOLD) {
        gravityStrength *= GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_INVERTED;
    }
    
    if (gameState.skills.isBurstJumpActive && !gameState.skills.hasUsedBurstJump) {
        gravityStrength *= GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_BURST_JUMP;
    }
    
    return gravityStrength;
}

} // namespace PhysicsUtils
