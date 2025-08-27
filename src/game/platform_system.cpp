#include "platform_system.h"
#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

void PlatformSystem::update(float deltaTime, const glm::vec3& playerPos) {
    for (auto& platform : platforms) {
        std::visit(overloaded{
            [this, deltaTime](GameState::StaticPlatform& p) { updateStaticPlatform(p, deltaTime); },
            [this, deltaTime](GameState::MovingPlatform& p) { updateMovingPlatform(p, deltaTime); },
            [this, deltaTime](GameState::RotatingPlatform& p) { updateRotatingPlatform(p, deltaTime); },
            [this, deltaTime](GameState::PatrollingPlatform& p) { updatePatrollingPlatform(p, deltaTime); },
            [this, deltaTime](GameState::TeleportPlatform& p) { updateTeleportPlatform(p, deltaTime); },
            [this, deltaTime](GameState::JumpPad& p) { updateJumpPad(p, deltaTime); },
            [this, deltaTime](GameState::CycleDisappearingPlatform& p) { updateCycleDisappearingPlatform(p, deltaTime); },
            [this, deltaTime](GameState::DisappearingPlatform& p) { updateDisappearingPlatform(p, deltaTime); },
            [this, deltaTime, &playerPos](GameState::FlyingPlatform& p) { updateFlyingPlatform(p, deltaTime, playerPos); }
        }, platform);
    }
}

GameState::PlatformVariant* PlatformSystem::checkCollision(const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (auto& platform : platforms) {
        bool collision = std::visit(overloaded{
            [this, &playerPos, &playerSize](const GameState::RotatingPlatform& p) {
                return checkCollisionWithRotatingPlatform(p, playerPos, playerSize);
            },
            [this, &playerPos, &playerSize](const auto& p) {
                return checkCollisionWithBase(p, playerPos, playerSize);
            }
        }, platform);
        
        if (collision) {
            return &platform;
        }
    }
    return nullptr;
}

GameState::PlatformVariant* PlatformSystem::getCurrentPlatform(const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (auto& platform : platforms) {
        bool collision = std::visit(overloaded{
            [this, &playerPos, &playerSize](const GameState::RotatingPlatform& p) {
                return checkCollisionWithRotatingPlatform(p, playerPos, playerSize);
            },
            [this, &playerPos, &playerSize](const auto& p) {
                return checkCollisionWithBase(p, playerPos, playerSize);
            }
        }, platform);
        
        if (collision) {
            return &platform;
        }
    }
    return nullptr;
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
            if constexpr (std::is_same_v<std::decay_t<decltype(p)>, GameState::RotatingPlatform>) {
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
            if constexpr (std::is_same_v<std::decay_t<decltype(p)>, GameState::RotatingPlatform>) {
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
            if constexpr (std::is_same_v<std::decay_t<decltype(p)>, GameState::RotatingPlatform>) {
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
            if constexpr (std::is_same_v<std::decay_t<decltype(p)>, GameState::CycleDisappearingPlatform>) {
                blinkAlphas.push_back(p.blinkAlpha);
            } else {
                blinkAlphas.push_back(1.0f);
            }
        }, platform);
    }
    
    return blinkAlphas;
}

// 古い設計との互換性のための静的メソッド
void PlatformSystem::updatePlatforms(GameState& gameState, float deltaTime) {
    // 古い設計の実装をここに追加
}

void PlatformSystem::checkTeleportPlatformCollision(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    // 古い設計の実装をここに追加
}

void PlatformSystem::checkMovingPlatformCollision(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    // 古い設計の実装をここに追加
}

// 各足場タイプの更新処理
void PlatformSystem::updateStaticPlatform(GameState::StaticPlatform& platform, float deltaTime) {
    // 静的足場は何もしない
    (void)platform;
    (void)deltaTime;
}

void PlatformSystem::updateMovingPlatform(GameState::MovingPlatform& platform, float deltaTime) {
    // 前フレームの位置を保存
    platform.previousPosition = platform.position;
    
    if (platform.hasPlayerOnBoard) {
        platform.moveTimer += deltaTime;
        
        // 目標位置と開始位置を決定
        glm::vec3 targetPos = platform.moveTargetPosition;
        glm::vec3 startPos = platform.originalPosition;
        
        // 2点間の距離を計算
        float distance = glm::length(targetPos - startPos);
        float timeToTarget = distance / platform.moveSpeed;
        
        // 現在のタイマーで補間位置を計算
        float t = platform.moveTimer / timeToTarget;
        
        if (t >= 1.0f) {
            // 目標に到達
            platform.position = targetPos;
            platform.moveTimer = 0.0f;
            platform.returnToOriginal = true;
        } else {
            // 2点間を線形補間で滑らかに移動
            platform.position = glm::mix(startPos, targetPos, t);
        }
    } else if (platform.returnToOriginal) {
        // 元の位置に戻る
        platform.moveTimer += deltaTime;
        
        glm::vec3 targetPos = platform.originalPosition;
        glm::vec3 startPos = platform.moveTargetPosition;
        
        float distance = glm::length(targetPos - startPos);
        float timeToTarget = distance / platform.moveSpeed;
        
        float t = platform.moveTimer / timeToTarget;
        
        if (t >= 1.0f) {
            // 元の位置に到達
            platform.position = targetPos;
            platform.moveTimer = 0.0f;
            platform.returnToOriginal = false;
        } else {
            // 元の位置に向かって移動
            platform.position = glm::mix(startPos, targetPos, t);
        }
    }
}

void PlatformSystem::updateRotatingPlatform(GameState::RotatingPlatform& platform, float deltaTime) {
    platform.rotationAngle += platform.rotationSpeed * deltaTime;
    // 360度を超えたら0度に戻す
    if (platform.rotationAngle >= 360.0f) {
        platform.rotationAngle -= 360.0f;
    }
}

void PlatformSystem::updatePatrollingPlatform(GameState::PatrollingPlatform& platform, float deltaTime) {
    if (platform.patrolPoints.empty()) return;
    
    // 前フレームの位置を保存
    platform.previousPosition = platform.position;
    
    platform.patrolTimer += deltaTime;
    
    // 現在の目標点と次の目標点を取得
    int currentIndex = platform.currentPatrolIndex;
    int nextIndex = (currentIndex + 1) % platform.patrolPoints.size();
    
    glm::vec3 currentPoint = platform.patrolPoints[currentIndex];
    glm::vec3 nextPoint = platform.patrolPoints[nextIndex];
    
    // 2点間の距離を計算
    float distance = glm::length(nextPoint - currentPoint);
    float timeToNext = distance / platform.patrolSpeed;
    
    // 現在のタイマーで補間位置を計算
    float t = platform.patrolTimer / timeToNext;
    if (t >= 1.0f) {
        // 次の点に到達
        platform.currentPatrolIndex = nextIndex;
        platform.patrolTimer = 0.0f;
        platform.position = nextPoint;
    } else {
        // 2点間を線形補間
        platform.position = glm::mix(currentPoint, nextPoint, t);
    }
}

void PlatformSystem::updateTeleportPlatform(GameState::TeleportPlatform& platform, float deltaTime) {
    if (platform.cooldownTimer > 0.0f) {
        platform.cooldownTimer -= deltaTime;
    }
}

void PlatformSystem::updateJumpPad(GameState::JumpPad& platform, float deltaTime) {
    // ジャンプパッドは何もしない
    (void)platform;
    (void)deltaTime;
}

void PlatformSystem::updateCycleDisappearingPlatform(GameState::CycleDisappearingPlatform& platform, float deltaTime) {
    platform.cycleTimer += deltaTime;
    
    // サイクル時間を超えたらリセット
    if (platform.cycleTimer >= platform.cycleTime) {
        platform.cycleTimer -= platform.cycleTime;
    }
    
    // 点滅開始タイミングを計算
    float blinkStartTime = platform.visibleTime - platform.blinkTime;
    
    if (platform.cycleTimer < blinkStartTime) {
        // 完全に見える時間
        platform.isCurrentlyVisible = true;
        platform.isBlinking = false;
        platform.blinkAlpha = 1.0f;
        platform.size = platform.originalSize;
    } else if (platform.cycleTimer < platform.visibleTime) {
        // 点滅時間
        platform.isCurrentlyVisible = true;
        platform.isBlinking = true;
        platform.blinkTimer += deltaTime;
        
        // 点滅の透明度を計算（0.2秒周期で点滅）
        float blinkCycle = 0.2f;
        float blinkProgress = std::fmod(platform.blinkTimer, blinkCycle);
        if (blinkProgress < blinkCycle * 0.5f) {
            platform.blinkAlpha = 1.0f;
        } else {
            platform.blinkAlpha = 0.3f;
        }
        
        platform.size = platform.originalSize;
    } else {
        // 完全に消える時間
        platform.isCurrentlyVisible = false;
        platform.isBlinking = false;
        platform.blinkAlpha = 0.0f;
        platform.size = glm::vec3(0, 0, 0);
    }
}

void PlatformSystem::updateDisappearingPlatform(GameState::DisappearingPlatform& platform, float deltaTime) {
    platform.disappearTimer += deltaTime;
    if (platform.disappearTimer > 3.0f) { // 3秒後に消える
        platform.size = glm::vec3(0, 0, 0);
    }
}

void PlatformSystem::updateFlyingPlatform(GameState::FlyingPlatform& platform, float deltaTime, const glm::vec3& playerPos) {
    // プレイヤーとターゲット位置との距離をチェック
    float distanceToTarget = glm::length(playerPos - platform.targetPosition);
    
    // プレイヤーが検知範囲内に入ったら発動
    if (!platform.hasSpawned && distanceToTarget <= platform.detectionRange) {
        platform.hasSpawned = true;
        platform.isFlying = true;
        platform.isVisible = true;
        platform.position = platform.spawnPosition;
        platform.size = glm::vec3(3, 0.5, 3); // 元のサイズに戻す
    }
    
    // 飛行ロジックの実装
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
    
    // プレイヤーが目標位置から離れたら戻る処理
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
            [](GameState::MovingPlatform& p) {
                p.hasPlayerOnBoard = false;
            }
        }, platform);
    }
}

bool PlatformSystem::checkCollisionWithBase(const GameState::BasePlatform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    if (!platform.isVisible) return false;
    
    glm::vec3 platformMin = platform.position - platform.size * 0.5f;
    glm::vec3 platformMax = platform.position + platform.size * 0.5f;
    glm::vec3 playerMin = playerPos - playerSize * 0.5f;
    glm::vec3 playerMax = playerPos + playerSize * 0.5f;
    
    // 衝突判定
    return (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
            playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
            playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
}

bool PlatformSystem::checkCollisionWithRotatingPlatform(const GameState::RotatingPlatform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    if (!platform.isVisible) return false;
    
    // 回転した足場の当たり判定
    glm::vec3 halfSize = platform.size * 0.5f;
    
    // Y軸回転の場合の特別な処理
    if (glm::length(platform.rotationAxis - glm::vec3(0, 1, 0)) < 0.1f) {
        // Y軸回転の場合、XZ平面での回転のみを考慮
        float angle = glm::radians(platform.rotationAngle);
        float cosAngle = cos(angle);
        float sinAngle = sin(angle);
        
        // プレイヤーの位置を足場のローカル座標系に変換
        glm::vec3 localPlayerPos = playerPos - platform.position;
        
        // XZ平面での逆回転
        float newX = localPlayerPos.x * cosAngle + localPlayerPos.z * sinAngle;
        float newZ = -localPlayerPos.x * sinAngle + localPlayerPos.z * cosAngle;
        
        glm::vec3 unrotatedPlayerPos = platform.position + glm::vec3(newX, localPlayerPos.y, newZ);
        
        // 回転を考慮しない通常の当たり判定
        glm::vec3 platformMin = platform.position - halfSize;
        glm::vec3 platformMax = platform.position + halfSize;
        glm::vec3 playerMin = unrotatedPlayerPos - playerSize * 0.5f;
        glm::vec3 playerMax = unrotatedPlayerPos + playerSize * 0.5f;
        
        // 衝突判定
        return (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
                playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
    } else if (glm::length(platform.rotationAxis - glm::vec3(1, 0, 0)) < 0.1f) {
        // X軸回転（縦回転）の場合、YZ平面での回転のみを考慮
        float angle = glm::radians(platform.rotationAngle);
        float cosAngle = cos(angle);
        float sinAngle = sin(angle);
        
        // プレイヤーの位置を足場のローカル座標系に変換
        glm::vec3 localPlayerPos = playerPos - platform.position;
        
        // YZ平面での逆回転
        float newY = localPlayerPos.y * cosAngle - localPlayerPos.z * sinAngle;
        float newZ = localPlayerPos.y * sinAngle + localPlayerPos.z * cosAngle;
        
        glm::vec3 unrotatedPlayerPos = platform.position + glm::vec3(localPlayerPos.x, newY, newZ);
        
        // 回転を考慮しない通常の当たり判定
        glm::vec3 platformMin = platform.position - halfSize;
        glm::vec3 platformMax = platform.position + halfSize;
        glm::vec3 playerMin = unrotatedPlayerPos - playerSize * 0.5f;
        glm::vec3 playerMax = unrotatedPlayerPos + playerSize * 0.5f;
        
        // 衝突判定
        return (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
                playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
    } else {
        // その他の軸回転の場合、一般的な回転行列を使用
        glm::vec3 localPlayerPos = playerPos - platform.position;
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-platform.rotationAngle), platform.rotationAxis);
        glm::vec4 rotatedPlayerPos = rotationMatrix * glm::vec4(localPlayerPos, 1.0f);
        glm::vec3 unrotatedPlayerPos = glm::vec3(rotatedPlayerPos) + platform.position;
        
        // 回転を考慮しない通常の当たり判定
        glm::vec3 platformMin = platform.position - halfSize;
        glm::vec3 platformMax = platform.position + halfSize;
        glm::vec3 playerMin = unrotatedPlayerPos - playerSize * 0.5f;
        glm::vec3 playerMax = unrotatedPlayerPos + playerSize * 0.5f;
        
        // 衝突判定
        return (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
                playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
    }
}
