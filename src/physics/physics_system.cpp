#include "physics_system.h"
#include "../app/game_constants.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

// 重力反転エリア内にいるかチェック
bool PhysicsSystem::isPlayerInGravityZone(const GameState& gameState, const glm::vec3& playerPos, glm::vec3& gravityDirection) {
    for (const auto& zone : gameState.gravityZones) {
        if (!zone.isActive) continue;
        
        float distance = glm::length(playerPos - zone.position);
        if (distance <= zone.radius) {
            gravityDirection = zone.gravityDirection;
            return true;
        }
    }
    return false;
}

// 水平方向の足場衝突判定
bool PhysicsSystem::checkPlatformCollisionHorizontal(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (const auto& platform : gameState.platforms) {
        // プラットフォームが存在するかチェック（消えた足場は無視）
        if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
        
        // 水平方向の衝突判定のみ（Y軸は少し余裕を持たせる）
        glm::vec3 platformMin = platform.position - platform.size * GameConstants::PhysicsCalculationConstants::PLATFORM_HALF_SIZE_MULTIPLIER;
        glm::vec3 platformMax = platform.position + platform.size * GameConstants::PhysicsCalculationConstants::PLATFORM_HALF_SIZE_MULTIPLIER;
        glm::vec3 playerMin = playerPos - playerSize * GameConstants::PhysicsCalculationConstants::PLAYER_HALF_SIZE_MULTIPLIER;
        glm::vec3 playerMax = playerPos + playerSize * GameConstants::PhysicsCalculationConstants::PLAYER_HALF_SIZE_MULTIPLIER;
        
        // プレイヤーが足場の上にいる場合は水平移動を許可（通常重力）
        // 厳密に判定：プレイヤーが実際に足場の上に乗っているかチェック
        bool onTopSurface = (std::abs(playerMin.y - platformMax.y) < GameConstants::PhysicsConstants::PLATFORM_SURFACE_TOLERANCE) && 
                           (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                            playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        if (onTopSurface) {
            continue; // 足場の上にいる場合は水平衝突を無視
        }
        
        // プレイヤーが足場の下面にいる場合も水平移動を許可（重力反転時）
        // 厳密に判定：プレイヤーが実際に足場の下面に乗っているかチェック
        bool onBottomSurface = (std::abs(playerMax.y - platformMin.y) < GameConstants::PhysicsConstants::PLATFORM_SURFACE_TOLERANCE) && 
                              (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                               playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        if (onBottomSurface) {
            continue; // 足場の下面にいる場合も水平衝突を無視
        }
        
        // プレイヤーが足場の側面高さ範囲にいる場合のみ水平衝突をチェック
        if (playerMax.y > platformMin.y - GameConstants::PhysicsConstants::COLLISION_TOLERANCE && 
            playerMin.y < platformMax.y + GameConstants::PhysicsConstants::COLLISION_TOLERANCE) {
            if (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                playerMax.z >= platformMin.z && playerMin.z <= platformMax.z) {
                return true;
            }
        }
    }
    return false;
}

// 重力方向を考慮した足場衝突をチェック（回転対応）
GameState::Platform* PhysicsSystem::checkPlatformCollisionWithGravity(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection) {
    for (auto& platform : gameState.platforms) {
        // プラットフォームが存在するかチェック
        if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
        
        // 重力方向に応じて衝突判定を行う
        if (isPlayerOnPlatformWithGravity(platform, playerPos, playerSize, gravityDirection)) {
            return &platform;
        }
    }
    return nullptr;
}

// 垂直方向の足場衝突をチェック（回転対応）
GameState::Platform* PhysicsSystem::checkPlatformCollisionVertical(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (auto& platform : gameState.platforms) {
        // プラットフォームが存在するかチェック
        if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
        
        // 回転した足場の当たり判定を使用
        if (isPlayerOnRotatedPlatform(platform, playerPos, playerSize)) {
            return &platform;
        }
    }
    return nullptr;
}

// 重力方向を考慮した足場衝突判定
bool PhysicsSystem::isPlayerOnPlatformWithGravity(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection) {
    // 重力方向が上向き（反転）の場合
    if (gravityDirection.y > GameConstants::PhysicsConstants::GRAVITY_DIRECTION_THRESHOLD) {
        // 足場の下面に衝突判定
        glm::vec3 platformMin = platform.position - platform.size * GameConstants::PhysicsCalculationConstants::PLATFORM_HALF_SIZE_MULTIPLIER;
        glm::vec3 platformMax = platform.position + platform.size * GameConstants::PhysicsCalculationConstants::PLATFORM_HALF_SIZE_MULTIPLIER;
        glm::vec3 playerMin = playerPos - playerSize * GameConstants::PhysicsCalculationConstants::PLAYER_HALF_SIZE_MULTIPLIER;
        glm::vec3 playerMax = playerPos + playerSize * GameConstants::PhysicsCalculationConstants::PLAYER_HALF_SIZE_MULTIPLIER;
        
        // 水平方向に重なっており、プレイヤーが足場の下面付近にいる
        bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                 playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (horizontalOverlap && 
            std::abs(playerMax.y - platformMin.y) < GameConstants::PhysicsCalculationConstants::PLATFORM_COLLISION_DISTANCE) { // 足場の下面からの判定
            return true;
        }
    } else {
        // 通常の重力（下向き）の場合 - 上面に衝突判定
        if (isPlayerOnRotatedPlatform(platform, playerPos, playerSize)) {
            return true;
        }
    }
    return false;
}

// 重力方向を考慮した足場判定（移動用）
bool PhysicsSystem::isPlayerOnPlatformWithGravityForMovement(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection) {
    // 重力方向が上向き（反転）の場合
    if (gravityDirection.y > GameConstants::PhysicsConstants::GRAVITY_DIRECTION_THRESHOLD) {
        // 足場の下面に衝突判定
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        // 水平方向に重なっており、プレイヤーが足場の下面付近にいる
        bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                 playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (horizontalOverlap && 
            std::abs(playerMax.y - platformMin.y) < 0.5f) { // 足場の下面から0.5以内（より緩い判定）
            return true;
        }
    } else {
        // 通常の重力（下向き）の場合 - 上面に衝突判定
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        // 水平方向に重なっており、プレイヤーが足場の上面付近にいる
        bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                 playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (horizontalOverlap && 
            std::abs(playerMin.y - platformMax.y) < 0.2f) { // 足場の上面から0.2以内
            return true;
        }
    }
    return false;
}

// プレイヤーが足場の上に立っているかチェック（ジャンプ判定用）
bool PhysicsSystem::isPlayerOnPlatform(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (const auto& platform : gameState.platforms) {
        if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
        
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        // 水平方向に重なっており、プレイヤーが足場の上面付近にいる
        bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                 playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (horizontalOverlap && 
            std::abs(playerMin.y - platformMax.y) < 0.1f) { // 足場の上面から0.1以内
            return true;
        }
    }
    return false;
}

// 回転した足場の当たり判定
bool PhysicsSystem::isPlayerOnRotatedPlatform(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    if (!platform.isRotating) {
        // 通常の当たり判定
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                 playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        return horizontalOverlap && 
               playerMin.y <= platformMax.y + 0.1f && 
               playerMin.y >= platformMax.y - 0.5f;
    } else {
        // 回転した足場の当たり判定
        glm::vec3 topCorners[4];
        getRotatedPlatformTopCorners(platform, topCorners);
        
        // プレイヤーの足元位置
        glm::vec3 playerFoot = playerPos - glm::vec3(0, playerSize.y * 0.5f, 0);
        
        // 上面の平面方程式を計算（3点から法線ベクトルを求める）
        glm::vec3 v1 = topCorners[1] - topCorners[0];
        glm::vec3 v2 = topCorners[2] - topCorners[0];
        glm::vec3 normal = glm::normalize(glm::cross(v1, v2));
        
        // プレイヤーが上面の範囲内にいるかチェック（2D投影）
        // 簡易的に、4つの角を結ぶ四角形の内部にいるかチェック
        glm::vec2 playerPos2D = glm::vec2(playerFoot.x, playerFoot.z);
        glm::vec2 corners2D[4] = {
            glm::vec2(topCorners[0].x, topCorners[0].z),
            glm::vec2(topCorners[1].x, topCorners[1].z),
            glm::vec2(topCorners[2].x, topCorners[2].z),
            glm::vec2(topCorners[3].x, topCorners[3].z)
        };
        
        // 点が四角形内部にあるかチェック（外積を使用）
        bool inside = true;
        for (int i = 0; i < 4; i++) {
            int next = (i + 1) % 4;
            glm::vec2 edge = corners2D[next] - corners2D[i];
            glm::vec2 toPlayer = playerPos2D - corners2D[i];
            float cross = edge.x * toPlayer.y - edge.y * toPlayer.x;
            if (cross < 0) {
                inside = false;
                break;
            }
        }
        
        if (!inside) return false;
        
        // プレイヤーが上面の高さ付近にいるかチェック
        // 平面上の点の高さを計算
        float d = -glm::dot(normal, topCorners[0]);
        float surfaceY = -(normal.x * playerFoot.x + normal.z * playerFoot.z + d) / normal.y;
        
        return playerFoot.y <= surfaceY + 0.1f && playerFoot.y >= surfaceY - 0.5f;
    }
}

// 点を軸周りに回転
glm::vec3 PhysicsSystem::rotatePointAroundAxis(const glm::vec3& point, const glm::vec3& axis, float angle, const glm::vec3& center) {
    // 点を中心に移動
    glm::vec3 translated = point - center;
    
    // 回転行列を作成
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    
    // 回転を適用
    glm::vec4 rotated = rotationMatrix * glm::vec4(translated, 1.0f);
    
    // 中心座標を戻す
    return glm::vec3(rotated) + center;
}

// 回転した足場の8つの角の座標を計算
void PhysicsSystem::getRotatedPlatformCorners(const GameState::Platform& platform, glm::vec3 corners[8]) {
    glm::vec3 halfSize = platform.size * 0.5f;
    
    // 回転前の8つの角の座標
    glm::vec3 localCorners[8] = {
        glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z),
        glm::vec3( halfSize.x, -halfSize.y, -halfSize.z),
        glm::vec3( halfSize.x,  halfSize.y, -halfSize.z),
        glm::vec3(-halfSize.x,  halfSize.y, -halfSize.z),
        glm::vec3(-halfSize.x, -halfSize.y,  halfSize.z),
        glm::vec3( halfSize.x, -halfSize.y,  halfSize.z),
        glm::vec3( halfSize.x,  halfSize.y,  halfSize.z),
        glm::vec3(-halfSize.x,  halfSize.y,  halfSize.z)
    };
    
    // 回転を適用
    for (int i = 0; i < 8; i++) {
        if (platform.isRotating) {
            corners[i] = rotatePointAroundAxis(
                platform.position + localCorners[i],
                platform.rotationAxis,
                platform.rotationAngle,
                platform.position
            );
        } else {
            corners[i] = platform.position + localCorners[i];
        }
    }
}

// 回転した足場の上面の4つの角を取得
void PhysicsSystem::getRotatedPlatformTopCorners(const GameState::Platform& platform, glm::vec3 topCorners[4]) {
    glm::vec3 corners[8];
    getRotatedPlatformCorners(platform, corners);
    
    // 上面の4つの角（Y座標が大きい方）
    topCorners[0] = corners[3]; // 左後上
    topCorners[1] = corners[2]; // 右後上
    topCorners[2] = corners[6]; // 右前上
    topCorners[3] = corners[7]; // 左前上
}

// 回転した足場の下面の4つの角を取得
void PhysicsSystem::getRotatedPlatformBottomCorners(const GameState::Platform& platform, glm::vec3 bottomCorners[4]) {
    glm::vec3 corners[8];
    getRotatedPlatformCorners(platform, corners);
    
    // 下面の4つの角（Y座標が小さい方）
    bottomCorners[0] = corners[0]; // 左後下
    bottomCorners[1] = corners[1]; // 右後下
    bottomCorners[2] = corners[5]; // 右前下
    bottomCorners[3] = corners[4]; // 左前下
}

