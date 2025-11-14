#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "cannon_system.h"
#include <iostream>
#include <cmath>

void CannonSystem::updateCannons(GameState& gameState, float deltaTime) {
    for (auto& cannon : gameState.cannons) {
        if (cannon.cooldownTimer > 0.0f) {
            cannon.cooldownTimer -= deltaTime;
        }
    }
}

void CannonSystem::checkCannonCollision(GameState& gameState, const glm::vec3& playerPosition, const glm::vec3& playerSize) {
    for (auto& cannon : gameState.cannons) {
        if (!cannon.isActive || cannon.cooldownTimer > 0.0f) {
            continue;
        }
        
        glm::vec3 cannonMin = cannon.position - cannon.size * 0.5f;
        glm::vec3 cannonMax = cannon.position + cannon.size * 0.5f;
        glm::vec3 playerMin = playerPosition - playerSize * 0.5f;
        glm::vec3 playerMax = playerPosition + playerSize * 0.5f;
        
        if (playerMax.x > cannonMin.x && playerMin.x < cannonMax.x &&
            playerMax.y > cannonMin.y && playerMin.y < cannonMax.y &&
            playerMax.z > cannonMin.z && playerMin.z < cannonMax.z) {
            
            if (!cannon.hasPlayerInside) {
                cannon.hasPlayerInside = true;
                
                gameState.player.position = cannon.position;
                gameState.player.velocity = cannon.launchDirection;
                
                cannon.cooldownTimer = cannon.cooldownTime;
            }
        } else {
            cannon.hasPlayerInside = false;
        }
    }
}

