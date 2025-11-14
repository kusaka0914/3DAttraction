#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "physics_system.h"
#include "../core/constants/game_constants.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

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

bool PhysicsSystem::checkPlatformCollisionHorizontal(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (const auto& platform : gameState.platforms) {
        if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
        
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        bool basicCollision = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                              playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
                              playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (!basicCollision) continue;
        
        bool onTopSurface = (std::abs(playerMin.y - platformMax.y) < 0.1f) && 
                           (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                            playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (!onTopSurface) {
            return true; // 水平衝突あり → 移動を阻止
        }
    }
    return false; // 水平衝突なし → 移動を許可
}

GameState::Platform* PhysicsSystem::checkPlatformCollisionWithGravity(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection) {
    for (auto& platform : gameState.platforms) {
        if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
        
        if (isPlayerOnPlatformWithGravity(platform, playerPos, playerSize, gravityDirection)) {
            return &platform;
        }
    }
    return nullptr;
}

GameState::Platform* PhysicsSystem::checkPlatformCollisionVertical(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (auto& platform : gameState.platforms) {
        if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
        
        if (isPlayerOnRotatedPlatform(platform, playerPos, playerSize)) {
            return &platform;
        }
    }
    return nullptr;
}

bool PhysicsSystem::isPlayerOnPlatformWithGravity(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection) {
    if (gravityDirection.y > GameConstants::PhysicsConstants::GRAVITY_DIRECTION_THRESHOLD) {
        glm::vec3 platformMin = platform.position - platform.size * GameConstants::PhysicsCalculationConstants::PLATFORM_HALF_SIZE_MULTIPLIER;
        glm::vec3 platformMax = platform.position + platform.size * GameConstants::PhysicsCalculationConstants::PLATFORM_HALF_SIZE_MULTIPLIER;
        glm::vec3 playerMin = playerPos - playerSize * GameConstants::PhysicsCalculationConstants::PLAYER_HALF_SIZE_MULTIPLIER;
        glm::vec3 playerMax = playerPos + playerSize * GameConstants::PhysicsCalculationConstants::PLAYER_HALF_SIZE_MULTIPLIER;
        
        bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                 playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (horizontalOverlap && 
            std::abs(playerMax.y - platformMin.y) < GameConstants::PhysicsCalculationConstants::PLATFORM_COLLISION_DISTANCE) { // 足場の下面からの判定
            return true;
        }
    } else {
        if (isPlayerOnRotatedPlatform(platform, playerPos, playerSize)) {
            return true;
        }
    }
    return false;
}

bool PhysicsSystem::isPlayerOnPlatformWithGravityForMovement(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection) {
    if (gravityDirection.y > GameConstants::PhysicsConstants::GRAVITY_DIRECTION_THRESHOLD) {
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                 playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (horizontalOverlap && 
            std::abs(playerMax.y - platformMin.y) < 0.5f) { // 足場の下面から0.5以内（より緩い判定）
            return true;
        }
    } else {
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                 playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (horizontalOverlap && 
            std::abs(playerMin.y - platformMax.y) < 0.2f) { // 足場の上面から0.2以内
            return true;
        }
    }
    return false;
}

bool PhysicsSystem::isPlayerOnPlatform(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (const auto& platform : gameState.platforms) {
        if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
        
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                 playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (horizontalOverlap && 
            std::abs(playerMin.y - platformMax.y) < 0.1f) { // 足場の上面から0.1以内
            return true;
        }
    }
    return false;
}

bool PhysicsSystem::isPlayerOnRotatedPlatform(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    if (!platform.isRotating) {
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
        glm::vec3 topCorners[4];
        getRotatedPlatformTopCorners(platform, topCorners);
        
        glm::vec3 playerFoot = playerPos - glm::vec3(0, playerSize.y * 0.5f, 0);
        
        glm::vec3 v1 = topCorners[1] - topCorners[0];
        glm::vec3 v2 = topCorners[2] - topCorners[0];
        glm::vec3 normal = glm::normalize(glm::cross(v1, v2));
        
        glm::vec2 playerPos2D = glm::vec2(playerFoot.x, playerFoot.z);
        glm::vec2 corners2D[4] = {
            glm::vec2(topCorners[0].x, topCorners[0].z),
            glm::vec2(topCorners[1].x, topCorners[1].z),
            glm::vec2(topCorners[2].x, topCorners[2].z),
            glm::vec2(topCorners[3].x, topCorners[3].z)
        };
        
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
        
        float d = -glm::dot(normal, topCorners[0]);
        float surfaceY = -(normal.x * playerFoot.x + normal.z * playerFoot.z + d) / normal.y;
        
        return playerFoot.y <= surfaceY + 0.1f && playerFoot.y >= surfaceY - 0.5f;
    }
}

glm::vec3 PhysicsSystem::rotatePointAroundAxis(const glm::vec3& point, const glm::vec3& axis, float angle, const glm::vec3& center) {
    glm::vec3 translated = point - center;
    
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    
    glm::vec4 rotated = rotationMatrix * glm::vec4(translated, 1.0f);
    
    return glm::vec3(rotated) + center;
}

void PhysicsSystem::getRotatedPlatformCorners(const GameState::Platform& platform, glm::vec3 corners[8]) {
    glm::vec3 halfSize = platform.size * 0.5f;
    
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

void PhysicsSystem::getRotatedPlatformTopCorners(const GameState::Platform& platform, glm::vec3 topCorners[4]) {
    glm::vec3 corners[8];
    getRotatedPlatformCorners(platform, corners);
    
    topCorners[0] = corners[3]; // 左後上
    topCorners[1] = corners[2]; // 右後上
    topCorners[2] = corners[6]; // 右前上
    topCorners[3] = corners[7]; // 左前上
}

void PhysicsSystem::getRotatedPlatformBottomCorners(const GameState::Platform& platform, glm::vec3 bottomCorners[4]) {
    glm::vec3 corners[8];
    getRotatedPlatformCorners(platform, corners);
    
    bottomCorners[0] = corners[0]; // 左後下
    bottomCorners[1] = corners[1]; // 右後下
    bottomCorners[2] = corners[5]; // 右前下
    bottomCorners[3] = corners[4]; // 左前下
}

