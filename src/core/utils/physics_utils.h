#pragma once

#include <glm/glm.hpp>
#include "../constants/game_constants.h"

// 前方宣言
struct GameState;

namespace PhysicsUtils {

// 重力方向に応じたプレイヤー位置調整
void adjustPlayerPositionForGravity(GameState& gameState, const glm::vec3& platformPosition, 
                                   const glm::vec3& platformSize, const glm::vec3& playerSize, 
                                   const glm::vec3& gravityDirection);

// 速度倍率に応じた重力強度を計算
float calculateGravityStrength(float baseGravity, float deltaTime, float timeScale, 
                              const glm::vec3& gravityDirection, GameState& gameState);

} // namespace PhysicsUtils
