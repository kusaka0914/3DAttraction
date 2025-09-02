#include "cannon_system.h"
#include <iostream>
#include <cmath>

// 大砲の更新
void CannonSystem::updateCannons(GameState& gameState, float deltaTime) {
    for (auto& cannon : gameState.cannons) {
        // クールダウンタイマーの更新
        if (cannon.cooldownTimer > 0.0f) {
            cannon.cooldownTimer -= deltaTime;
        }
    }
}

// 大砲との衝突判定
void CannonSystem::checkCannonCollision(GameState& gameState, const glm::vec3& playerPosition, const glm::vec3& playerSize) {
    for (auto& cannon : gameState.cannons) {
        if (!cannon.isActive || cannon.cooldownTimer > 0.0f) {
            continue;
        }
        
        // プレイヤーと大砲の衝突判定
        glm::vec3 cannonMin = cannon.position - cannon.size * 0.5f;
        glm::vec3 cannonMax = cannon.position + cannon.size * 0.5f;
        glm::vec3 playerMin = playerPosition - playerSize * 0.5f;
        glm::vec3 playerMax = playerPosition + playerSize * 0.5f;
        
        if (playerMax.x > cannonMin.x && playerMin.x < cannonMax.x &&
            playerMax.y > cannonMin.y && playerMin.y < cannonMax.y &&
            playerMax.z > cannonMin.z && playerMin.z < cannonMax.z) {
            
            // プレイヤーが大砲の中にいる
            if (!cannon.hasPlayerInside) {
                cannon.hasPlayerInside = true;
                
                // プレイヤーを発射
                gameState.playerPosition = cannon.position;
                gameState.playerVelocity = cannon.launchDirection;
                
                // クールダウン開始
                cannon.cooldownTimer = cannon.cooldownTime;
            }
        } else {
            // プレイヤーが大砲から出た
            cannon.hasPlayerInside = false;
        }
    }
}

