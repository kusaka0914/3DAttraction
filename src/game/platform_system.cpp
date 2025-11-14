#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "platform_system.h"
#include <variant>
#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

void PlatformSystem::update(float deltaTime, const glm::vec3& playerPos) {
    for (auto& platform : platforms) {
        std::visit(overloaded{
            [this, deltaTime](StaticPlatform& p) { updateStaticPlatform(p, deltaTime); },
            [this, deltaTime](MovingPlatform& p) { updateMovingPlatform(p, deltaTime); },
            [this, deltaTime](RotatingPlatform& p) { updateRotatingPlatform(p, deltaTime); },
            [this, deltaTime](PatrollingPlatform& p) { updatePatrollingPlatform(p, deltaTime); },
            [this, deltaTime](TeleportPlatform& p) { updateTeleportPlatform(p, deltaTime); },
            [this, deltaTime](JumpPad& p) { updateJumpPad(p, deltaTime); },
            [this, deltaTime](CycleDisappearingPlatform& p) { updateCycleDisappearingPlatform(p, deltaTime); },
            [this, deltaTime](DisappearingPlatform& p) { updateDisappearingPlatform(p, deltaTime); },
            [this, deltaTime, &playerPos](FlyingPlatform& p) { updateFlyingPlatform(p, deltaTime, playerPos); }
        }, platform);
    }
}

std::pair<PlatformVariant*, int> PlatformSystem::checkCollisionWithIndex(const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (int i = 0; i < platforms.size(); i++) {
        auto& platform = platforms[i];
        bool collision = std::visit(overloaded{
            [this, &playerPos, &playerSize](const RotatingPlatform& p) {
                return checkCollisionWithRotatingPlatform(p, playerPos, playerSize);
            },
            [this, &playerPos, &playerSize](const auto& p) {
                return checkCollisionWithBase(p, playerPos, playerSize);
            }
        }, platform);
        
        if (collision) {
            return {&platform, i};
        }
    }
    return {nullptr, -1};
}

std::vector<glm::vec3> PlatformSystem::getPositions() const {
    std::vector<glm::vec3> positions;
    positions.reserve(platforms.size());
    
    for (const auto& platform : platforms) {
        std::visit([&positions](const auto& p) {
            positions.push_back(p.position);
        }, platform);
    }
    
    return positions;
}

std::vector<glm::vec3> PlatformSystem::getSizes() const {
    std::vector<glm::vec3> sizes;
    sizes.reserve(platforms.size());
    
    for (const auto& platform : platforms) {
        std::visit([&sizes](const auto& p) {
            sizes.push_back(p.size);
        }, platform);
    }
    
    return sizes;
}

std::vector<glm::vec3> PlatformSystem::getColors() const {
    std::vector<glm::vec3> colors;
    colors.reserve(platforms.size());
    
    for (const auto& platform : platforms) {
        std::visit([&colors](const auto& p) {
            colors.push_back(p.color);
        }, platform);
    }
    
    return colors;
}

std::vector<bool> PlatformSystem::getVisibility() const {
    std::vector<bool> visibility;
    visibility.reserve(platforms.size());
    
    for (const auto& platform : platforms) {
        std::visit([&visibility](const auto& p) {
            visibility.push_back(p.isVisible);
        }, platform);
    }
    
    return visibility;
}

std::vector<bool> PlatformSystem::getIsRotating() const {
    std::vector<bool> isRotating;
    isRotating.reserve(platforms.size());
    
    for (const auto& platform : platforms) {
        std::visit([&isRotating](const auto& p) {
            if constexpr (std::is_same_v<std::decay_t<decltype(p)>, RotatingPlatform>) {
                isRotating.push_back(true);
            } else {
                isRotating.push_back(false);
            }
        }, platform);
    }
    
    return isRotating;
}

std::vector<float> PlatformSystem::getRotationAngles() const {
    std::vector<float> rotationAngles;
    rotationAngles.reserve(platforms.size());
    
    for (const auto& platform : platforms) {
        std::visit([&rotationAngles](const auto& p) {
            if constexpr (std::is_same_v<std::decay_t<decltype(p)>, RotatingPlatform>) {
                rotationAngles.push_back(p.rotationAngle);
            } else {
                rotationAngles.push_back(0.0f);
            }
        }, platform);
    }
    
    return rotationAngles;
}

std::vector<glm::vec3> PlatformSystem::getRotationAxes() const {
    std::vector<glm::vec3> rotationAxes;
    rotationAxes.reserve(platforms.size());
    
    for (const auto& platform : platforms) {
        std::visit([&rotationAxes](const auto& p) {
            if constexpr (std::is_same_v<std::decay_t<decltype(p)>, RotatingPlatform>) {
                rotationAxes.push_back(p.rotationAxis);
            } else {
                rotationAxes.push_back(glm::vec3(1, 0, 0));
            }
        }, platform);
    }
    
    return rotationAxes;
}

std::vector<float> PlatformSystem::getBlinkAlphas() const {
    std::vector<float> blinkAlphas;
    blinkAlphas.reserve(platforms.size());
    
    for (const auto& platform : platforms) {
        std::visit([&blinkAlphas](const auto& p) {
            if constexpr (std::is_same_v<std::decay_t<decltype(p)>, CycleDisappearingPlatform>) {
                blinkAlphas.push_back(p.blinkAlpha);
            } else {
                blinkAlphas.push_back(1.0f);
            }
        }, platform);
    }
    
    return blinkAlphas;
}

std::vector<std::string> PlatformSystem::getPlatformTypes() const {
    std::vector<std::string> platformTypes;
    platformTypes.reserve(platforms.size());
    
    for (const auto& platform : platforms) {
        std::visit([&platformTypes](const auto& p) {
            using T = std::decay_t<decltype(p)>;
            if constexpr (std::is_same_v<T, StaticPlatform>) {
                platformTypes.push_back("static");
            } else if constexpr (std::is_same_v<T, MovingPlatform>) {
                platformTypes.push_back("moving");
            } else if constexpr (std::is_same_v<T, RotatingPlatform>) {
                platformTypes.push_back("rotating");
            } else if constexpr (std::is_same_v<T, PatrollingPlatform>) {
                platformTypes.push_back("patrolling");
            } else if constexpr (std::is_same_v<T, TeleportPlatform>) {
                platformTypes.push_back("teleport");
            } else if constexpr (std::is_same_v<T, JumpPad>) {
                platformTypes.push_back("jumppad");
            } else if constexpr (std::is_same_v<T, CycleDisappearingPlatform>) {
                platformTypes.push_back("cycle_disappearing");
            } else if constexpr (std::is_same_v<T, DisappearingPlatform>) {
                platformTypes.push_back("disappearing");
            } else if constexpr (std::is_same_v<T, FlyingPlatform>) {
                platformTypes.push_back("flying");
            } else {
                platformTypes.push_back("unknown");
            }
        }, platform);
    }
    
    return platformTypes;
}

void PlatformSystem::updateStaticPlatform(StaticPlatform& platform, float deltaTime) {
    (void)platform;
    (void)deltaTime;
}

void PlatformSystem::updateMovingPlatform(MovingPlatform& platform, float deltaTime) {
    platform.previousPosition = platform.position;
    
    if (platform.hasPlayerOnBoard) {
        platform.moveTimer += deltaTime;
        
        glm::vec3 targetPos = platform.moveTargetPosition;
        glm::vec3 startPos = platform.originalPosition;
        
        float distance = glm::length(targetPos - startPos);
        float timeToTarget = distance / platform.moveSpeed;
        
        float t = platform.moveTimer / timeToTarget;
        
        if (t >= 1.0f) {
            platform.position = targetPos;
            platform.moveTimer = 0.0f;
            platform.returnToOriginal = true;
        } else {
            platform.position = glm::mix(startPos, targetPos, t);
        }
    } else if (platform.returnToOriginal) {
        platform.moveTimer += deltaTime;
        
        glm::vec3 targetPos = platform.originalPosition;
        glm::vec3 startPos = platform.moveTargetPosition;
        
        float distance = glm::length(targetPos - startPos);
        float timeToTarget = distance / platform.moveSpeed;
        
        float t = platform.moveTimer / timeToTarget;
        
        if (t >= 1.0f) {
            platform.position = targetPos;
            platform.moveTimer = 0.0f;
            platform.returnToOriginal = false;
        } else {
            platform.position = glm::mix(startPos, targetPos, t);
        }
    }
}

void PlatformSystem::updateRotatingPlatform(RotatingPlatform& platform, float deltaTime) {
    platform.rotationAngle += platform.rotationSpeed * deltaTime;
    if (platform.rotationAngle >= 360.0f) {
        platform.rotationAngle -= 360.0f;
    }
}

void PlatformSystem::updatePatrollingPlatform(PatrollingPlatform& platform, float deltaTime) {
    if (platform.patrolPoints.empty()) return;
    
    platform.previousPosition = platform.position;
    
    platform.patrolTimer += deltaTime;
    
    int currentIndex = platform.currentPatrolIndex;
    int nextIndex = (currentIndex + 1) % platform.patrolPoints.size();
    
    glm::vec3 currentPoint = platform.patrolPoints[currentIndex];
    glm::vec3 nextPoint = platform.patrolPoints[nextIndex];
    
    float distance = glm::length(nextPoint - currentPoint);
    float timeToNext = distance / platform.patrolSpeed;
    
    float t = platform.patrolTimer / timeToNext;
    if (t >= 1.0f) {
        platform.currentPatrolIndex = nextIndex;
        platform.patrolTimer = 0.0f;
        platform.position = nextPoint;
    } else {
        platform.position = glm::mix(currentPoint, nextPoint, t);
    }
}

void PlatformSystem::updateTeleportPlatform(TeleportPlatform& platform, float deltaTime) {
    if (platform.cooldownTimer > 0.0f) {
        platform.cooldownTimer -= deltaTime;
    }
}

void PlatformSystem::updateJumpPad(JumpPad& platform, float deltaTime) {
    (void)platform;
    (void)deltaTime;
}

void PlatformSystem::updateCycleDisappearingPlatform(CycleDisappearingPlatform& platform, float deltaTime) {
    platform.cycleTimer += deltaTime;
    
    if (platform.cycleTimer >= platform.cycleTime) {
        platform.cycleTimer -= platform.cycleTime;
    }
    
    float blinkStartTime = platform.visibleTime - platform.blinkTime;
    
    if (platform.cycleTimer < blinkStartTime) {
        platform.isCurrentlyVisible = true;
        platform.isBlinking = false;
        platform.blinkAlpha = 1.0f;
        platform.size = platform.originalSize;
    } else if (platform.cycleTimer < platform.visibleTime) {
        platform.isCurrentlyVisible = true;
        platform.isBlinking = true;
        platform.blinkTimer += deltaTime;
        
        float blinkCycle = 0.2f;
        float blinkProgress = std::fmod(platform.blinkTimer, blinkCycle);
        if (blinkProgress < blinkCycle * 0.5f) {
            platform.blinkAlpha = 1.0f;
        } else {
            platform.blinkAlpha = 0.3f;
        }
        
        platform.size = platform.originalSize;
    } else {
        platform.isCurrentlyVisible = false;
        platform.isBlinking = false;
        platform.blinkAlpha = 0.0f;
        platform.size = glm::vec3(0, 0, 0);
    }
}

void PlatformSystem::updateDisappearingPlatform(DisappearingPlatform& platform, float deltaTime) {
    platform.disappearTimer += deltaTime;
    if (platform.disappearTimer > 3.0f) { // 3秒後に消える
        platform.size = glm::vec3(0, 0, 0);
    }
}

void PlatformSystem::updateFlyingPlatform(FlyingPlatform& platform, float deltaTime, const glm::vec3& playerPos) {
    float distanceToTarget = glm::length(playerPos - platform.targetPosition);
    
    if (!platform.hasSpawned && distanceToTarget <= platform.detectionRange) {
        platform.hasSpawned = true;
        platform.isFlying = true;
        platform.isVisible = true;
        platform.position = platform.spawnPosition;
        platform.size = glm::vec3(3, 0.5, 3); // 元のサイズに戻す
    }
    
    if (platform.isFlying) {
        glm::vec3 targetPos;
        if (platform.isReturning) {
            targetPos = platform.spawnPosition;
        } else {
            targetPos = platform.targetPosition;
        }
        
        glm::vec3 direction = glm::normalize(targetPos - platform.position);
        float distanceToTarget = glm::length(targetPos - platform.position);
        
        if (distanceToTarget > 0.5f) {
            platform.position += direction * platform.flySpeed * deltaTime;
        } else {
            platform.isFlying = false;
            platform.position = targetPos;
            
            if (platform.isReturning) {
                platform.isReturning = false;
                platform.hasSpawned = false;
                platform.isVisible = false;
                platform.size = glm::vec3(0, 0, 0);
            }
        }
    }
    
    if (!platform.isFlying && platform.hasSpawned && !platform.isReturning) {
        float distanceToTarget = glm::length(playerPos - platform.targetPosition);
        if (distanceToTarget > 5.0f) { // プレイヤーが5単位以上離れたら
            platform.isReturning = true;
            platform.isFlying = true;
        }
    }
}

void PlatformSystem::resetMovingPlatformFlags() {
    for (auto& platform : platforms) {
        std::visit(overloaded{
            [](auto& p) {}, // 他のタイプは何もしない
            [](MovingPlatform& p) {
                p.hasPlayerOnBoard = false;
            }
        }, platform);
    }
}

bool PlatformSystem::checkCollisionWithBase(const BasePlatform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    if (!platform.isVisible) return false;
    
    glm::vec3 platformMin = platform.position - platform.size * 0.5f;
    glm::vec3 platformMax = platform.position + platform.size * 0.5f;
    glm::vec3 playerMin = playerPos - playerSize * 0.5f;
    glm::vec3 playerMax = playerPos + playerSize * 0.5f;
    
    return (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
            playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
            playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
}

bool PlatformSystem::checkCollisionWithRotatingPlatform(const RotatingPlatform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    if (!platform.isVisible) return false;
    
    glm::vec3 halfSize = platform.size * 0.5f;
    
    if (glm::length(platform.rotationAxis - glm::vec3(0, 1, 0)) < 0.1f) {
        float angle = glm::radians(platform.rotationAngle);
        float cosAngle = cos(angle);
        float sinAngle = sin(angle);
        
        glm::vec3 localPlayerPos = playerPos - platform.position;
        
        float newX = localPlayerPos.x * cosAngle + localPlayerPos.z * sinAngle;
        float newZ = -localPlayerPos.x * sinAngle + localPlayerPos.z * cosAngle;
        
        glm::vec3 unrotatedPlayerPos = platform.position + glm::vec3(newX, localPlayerPos.y, newZ);
        
        glm::vec3 platformMin = platform.position - halfSize;
        glm::vec3 platformMax = platform.position + halfSize;
        glm::vec3 playerMin = unrotatedPlayerPos - playerSize * 0.5f;
        glm::vec3 playerMax = unrotatedPlayerPos + playerSize * 0.5f;
        
        return (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
                playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
    } else if (glm::length(platform.rotationAxis - glm::vec3(1, 0, 0)) < 0.1f) {
        float angle = glm::radians(platform.rotationAngle);
        float cosAngle = cos(angle);
        float sinAngle = sin(angle);
        
        glm::vec3 localPlayerPos = playerPos - platform.position;
        
        float newY = localPlayerPos.y * cosAngle - localPlayerPos.z * sinAngle;
        float newZ = localPlayerPos.y * sinAngle + localPlayerPos.z * cosAngle;
        
        glm::vec3 unrotatedPlayerPos = platform.position + glm::vec3(localPlayerPos.x, newY, newZ);
        
        glm::vec3 platformMin = platform.position - halfSize;
        glm::vec3 platformMax = platform.position + halfSize;
        glm::vec3 playerMin = unrotatedPlayerPos - playerSize * 0.5f;
        glm::vec3 playerMax = unrotatedPlayerPos + playerSize * 0.5f;
        
        return (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
                playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
    } else {
        glm::vec3 localPlayerPos = playerPos - platform.position;
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-platform.rotationAngle), platform.rotationAxis);
        glm::vec4 rotatedPlayerPos = rotationMatrix * glm::vec4(localPlayerPos, 1.0f);
        glm::vec3 unrotatedPlayerPos = glm::vec3(rotatedPlayerPos) + platform.position;
        
        glm::vec3 platformMin = platform.position - halfSize;
        glm::vec3 platformMax = platform.position + halfSize;
        glm::vec3 playerMin = unrotatedPlayerPos - playerSize * 0.5f;
        glm::vec3 playerMax = unrotatedPlayerPos + playerSize * 0.5f;
        
        return (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
                playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
    }
}
