#pragma once

#include "../game/game_state.h"

// 物理システム
class PhysicsSystem {
public:
    // 重力反転エリアのチェック
    static bool isPlayerInGravityZone(const GameState& gameState, const glm::vec3& playerPos, glm::vec3& gravityDirection);
    
    // 足場衝突判定
    static bool checkPlatformCollisionHorizontal(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize);
    static GameState::Platform* checkPlatformCollisionWithGravity(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection);
    static GameState::Platform* checkPlatformCollisionVertical(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize);
    
    // 足場判定（重力方向考慮）
    static bool isPlayerOnPlatformWithGravity(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection);
    static bool isPlayerOnPlatformWithGravityForMovement(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection);
    static bool isPlayerOnPlatform(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize);
    
    // 回転足場の衝突判定
    static bool isPlayerOnRotatedPlatform(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize);
    static glm::vec3 rotatePointAroundAxis(const glm::vec3& point, const glm::vec3& axis, float angle, const glm::vec3& center);
    static void getRotatedPlatformCorners(const GameState::Platform& platform, glm::vec3 corners[8]);
    static void getRotatedPlatformTopCorners(const GameState::Platform& platform, glm::vec3 topCorners[4]);
    static void getRotatedPlatformBottomCorners(const GameState::Platform& platform, glm::vec3 bottomCorners[4]);
};

