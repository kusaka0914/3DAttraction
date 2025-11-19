#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "platform_system.h"
#include "../core/constants/physics_constants.h"
#include <variant>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

void PlatformSystem::update(float deltaTime, const glm::vec3& playerPos, float absoluteTime, float timeScale) {
    // リプレイモードでは、absoluteTimeは既に各フレーム区間のtimeScaleを考慮した累積ゲーム時間
    // 通常モードでは、absoluteTime = -1.0fなので、deltaTimeを使用
    float actualTime = absoluteTime;
    // timeScaleパラメータは使用しない（リプレイモードでは既に考慮済み）
    
    for (auto& platform : platforms) {
        std::visit(overloaded{
            [this, deltaTime, actualTime, absoluteTime](StaticPlatform& p) { updateStaticPlatform(p, deltaTime); },
            [this, deltaTime, actualTime, absoluteTime, &playerPos](MovingPlatform& p) { 
                if (absoluteTime >= 0.0f) {
                    updateMovingPlatformFromTime(p, actualTime, playerPos, GameConstants::PLAYER_SIZE);
                } else {
                    updateMovingPlatform(p, deltaTime);
                }
            },
            [this, deltaTime, actualTime, absoluteTime](RotatingPlatform& p) { 
                if (absoluteTime >= 0.0f) {
                    updateRotatingPlatformFromTime(p, actualTime);
                } else {
                    updateRotatingPlatform(p, deltaTime);
                }
            },
            [this, deltaTime, actualTime, absoluteTime](PatrollingPlatform& p) { 
                if (absoluteTime >= 0.0f) {
                    updatePatrollingPlatformFromTime(p, actualTime);
                } else {
                    updatePatrollingPlatform(p, deltaTime);
                }
            },
            [this, deltaTime, actualTime, absoluteTime](TeleportPlatform& p) { updateTeleportPlatform(p, deltaTime); },
            [this, deltaTime, actualTime, absoluteTime](JumpPad& p) { updateJumpPad(p, deltaTime); },
            [this, deltaTime, actualTime, absoluteTime](CycleDisappearingPlatform& p) { 
                if (absoluteTime >= 0.0f) {
                    updateCycleDisappearingPlatformFromTime(p, actualTime);
                } else {
                    updateCycleDisappearingPlatform(p, deltaTime);
                }
            },
            [this, deltaTime, actualTime, absoluteTime](DisappearingPlatform& p) { updateDisappearingPlatform(p, deltaTime); },
            [this, deltaTime, actualTime, absoluteTime, &playerPos](FlyingPlatform& p) { updateFlyingPlatform(p, deltaTime, playerPos); }
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

void PlatformSystem::updateMovingPlatformFromTime(MovingPlatform& platform, float absoluteTime, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    platform.previousPosition = platform.position;
    
    // プレイヤーがプラットフォーム上にいるかどうかを判定
    glm::vec3 platformMin = platform.position - platform.size * 0.5f;
    glm::vec3 platformMax = platform.position + platform.size * 0.5f;
    glm::vec3 playerMin = playerPos - playerSize * 0.5f;
    glm::vec3 playerMax = playerPos + playerSize * 0.5f;
    
    bool playerOnBoard = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                          playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
                          playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
    
    // MovingPlatformはプレイヤーが乗った時に動き始める
    // リプレイモードでは、プレイヤーが乗っている間は動き続ける
    // 通常モードと同じロジックを再現するため、absoluteTimeから直接計算するのではなく、
    // プレイヤーが乗った時点からの経過時間を計算する必要がある
    // しかし、リプレイデータにはMovingPlatformの状態が記録されていないため、
    // プレイヤーの位置から推測する必要がある
    
    // 簡易的な実装：プレイヤーが乗っている場合は、absoluteTimeから直接計算
    // 通常モードでは、プレイヤーが乗った時にmoveTimerが0から始まるため、
    // リプレイモードでも同様に、プレイヤーが乗った時点を0として計算
    if (playerOnBoard || platform.hasPlayerOnBoard) {
        // 移動サイクルを計算
        float distanceToTarget = glm::length(platform.moveTargetPosition - platform.originalPosition);
        float timeToTarget = distanceToTarget / platform.moveSpeed;
        
        // プレイヤーが乗った時点からの経過時間を計算
        // absoluteTimeから、プレイヤーが最初に乗った時点を引く必要があるが、
        // リプレイデータには記録されていないため、簡易的にabsoluteTimeを使用
        // ただし、これは通常モードの動作と完全に一致しない可能性がある
        float moveTime = std::fmod(absoluteTime, timeToTarget * 2.0f);
        
        if (moveTime < timeToTarget) {
            // 目標位置へ移動中
            float t = moveTime / timeToTarget;
            t = std::clamp(t, 0.0f, 1.0f);
            platform.position = glm::mix(platform.originalPosition, platform.moveTargetPosition, t);
            platform.returnToOriginal = false;
            platform.moveTimer = moveTime;
        } else {
            // 元の位置へ戻る中
            float returnTime = moveTime - timeToTarget;
            float t = returnTime / timeToTarget;
            t = std::clamp(t, 0.0f, 1.0f);
            platform.position = glm::mix(platform.moveTargetPosition, platform.originalPosition, t);
            platform.returnToOriginal = true;
            platform.moveTimer = returnTime;
        }
        
        platform.hasPlayerOnBoard = true;
    } else {
        // プレイヤーが乗っていない場合は、元の位置に戻る
        if (platform.returnToOriginal) {
            float distanceToOriginal = glm::length(platform.originalPosition - platform.position);
            if (distanceToOriginal > 0.01f) {
                // まだ元の位置に戻っていない場合は、元の位置に戻す
                platform.position = platform.originalPosition;
            }
            platform.returnToOriginal = false;
            platform.moveTimer = 0.0f;
        }
        platform.hasPlayerOnBoard = false;
    }
}

void PlatformSystem::updateRotatingPlatform(RotatingPlatform& platform, float deltaTime) {
    platform.rotationAngle += platform.rotationSpeed * deltaTime;
    if (platform.rotationAngle >= 360.0f) {
        platform.rotationAngle -= 360.0f;
    }
}

void PlatformSystem::updateRotatingPlatformFromTime(RotatingPlatform& platform, float absoluteTime) {
    // 絶対時間から回転角度を直接計算
    platform.rotationAngle = std::fmod(platform.rotationSpeed * absoluteTime, 360.0f);
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

void PlatformSystem::updatePatrollingPlatformFromTime(PatrollingPlatform& platform, float absoluteTime) {
    if (platform.patrolPoints.empty()) return;
    
    platform.previousPosition = platform.position;
    
    // 絶対時間から巡回位置を直接計算
    // 全ポイント間の移動時間の合計を計算
    float totalCycleTime = 0.0f;
    for (size_t i = 0; i < platform.patrolPoints.size(); i++) {
        size_t nextI = (i + 1) % platform.patrolPoints.size();
        float distance = glm::length(platform.patrolPoints[nextI] - platform.patrolPoints[i]);
        totalCycleTime += distance / platform.patrolSpeed;
    }
    
    // サイクル内の時間を計算
    float cycleTime = std::fmod(absoluteTime, totalCycleTime);
    
    // 現在のセグメントを特定
    float accumulatedTime = 0.0f;
    for (size_t i = 0; i < platform.patrolPoints.size(); i++) {
        size_t nextI = (i + 1) % platform.patrolPoints.size();
        float distance = glm::length(platform.patrolPoints[nextI] - platform.patrolPoints[i]);
        float timeToNext = distance / platform.patrolSpeed;
        
        if (cycleTime <= accumulatedTime + timeToNext) {
            // このセグメント内
            platform.currentPatrolIndex = static_cast<int>(i);
            float t = (cycleTime - accumulatedTime) / timeToNext;
            t = std::clamp(t, 0.0f, 1.0f);
            platform.position = glm::mix(platform.patrolPoints[i], platform.patrolPoints[nextI], t);
            platform.patrolTimer = cycleTime - accumulatedTime;
            return;
        }
        
        accumulatedTime += timeToNext;
    }
    
    // 最後のポイントに到達
    platform.currentPatrolIndex = static_cast<int>(platform.patrolPoints.size() - 1);
    platform.position = platform.patrolPoints.back();
    platform.patrolTimer = 0.0f;
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

void PlatformSystem::updateCycleDisappearingPlatformFromTime(CycleDisappearingPlatform& platform, float absoluteTime) {
    // 絶対時間からサイクルタイマーを直接計算
    // コンストラクタでcycleTimer = cycleTime - initialと設定されている
    // 通常モードでは、cycleTimer += deltaTimeで累積更新される
    // リプレイモードでは、absoluteTime（各フレーム区間のtimeScaleを考慮した累積ゲーム時間）から直接計算
    // 
    // 通常モードでの動作：
    // - 初期値: cycleTimer = cycleTime - initial
    // - 更新: cycleTimer += deltaTime * timeScale
    // - リセット: cycleTimer >= cycleTime の時、cycleTimer -= cycleTime
    //
    // リプレイモードでの動作：
    // - absoluteTime = 0の時、cycleTimer = cycleTime - initialになるように計算
    // - つまり、cycleTimer = fmod(cycleTime - initial + absoluteTime, cycleTime)
    // 
    // リプレイ開始時点では、プラットフォームは初期化されたばかりなので、
    // platform.cycleTimer = cycleTime - initial になっている
    // この値から initial を逆算: initial = cycleTime - platform.cycleTimer
    float initial = platform.cycleTime - platform.cycleTimer;  // 初期オフセット（initial）
    // absoluteTime = 0の時、cycleTimer = cycleTime - initialになるように計算
    platform.cycleTimer = std::fmod(platform.cycleTime - initial + absoluteTime, platform.cycleTime);
    
    float blinkStartTime = platform.visibleTime - platform.blinkTime;
    
    // 通常モードでは、blinkTimerは点滅期間中に累積更新される
    // 点滅期間外では更新されないが、値は保持される
    // 次の点滅期間に入った時、前回の値から継続される
    //
    // リプレイモードでは、absoluteTimeから直接累積点滅時間を計算する必要がある
    // 累積点滅時間 = 経過した全サイクルでの点滅時間の合計 + 現在のサイクル内の点滅経過時間
    
    // 経過したサイクル数を計算
    float totalCycles = (absoluteTime + initial) / platform.cycleTime;
    int completedCycles = static_cast<int>(totalCycles);
    
    if (platform.cycleTimer < blinkStartTime) {
        // 点滅期間前：前回の点滅期間の終了時点での累積点滅時間を保持
        platform.isCurrentlyVisible = true;
        platform.isBlinking = false;
        platform.blinkAlpha = 1.0f;
        platform.size = platform.originalSize;
        
        // 前回の点滅期間の終了時点での累積点滅時間
        if (completedCycles > 0) {
            platform.blinkTimer = completedCycles * platform.blinkTime;
        } else {
            platform.blinkTimer = 0.0f;
        }
    } else if (platform.cycleTimer < platform.visibleTime) {
        // 点滅期間中：累積点滅時間を計算
        platform.isCurrentlyVisible = true;
        platform.isBlinking = true;
        
        // 現在のサイクル内での点滅経過時間
        float currentBlinkElapsed = platform.cycleTimer - blinkStartTime;
        // 累積点滅時間 = 完了したサイクル数 × 点滅時間 + 現在のサイクル内の点滅経過時間
        platform.blinkTimer = completedCycles * platform.blinkTime + currentBlinkElapsed;
        
        float blinkCycle = 0.2f;
        float blinkProgress = std::fmod(platform.blinkTimer, blinkCycle);
        if (blinkProgress < blinkCycle * 0.5f) {
            platform.blinkAlpha = 1.0f;
        } else {
            platform.blinkAlpha = 0.3f;
        }
        
        platform.size = platform.originalSize;
    } else {
        // 点滅期間後（消えている）：現在のサイクル内で点滅期間が終了している
        platform.isCurrentlyVisible = false;
        platform.isBlinking = false;
        platform.blinkAlpha = 0.0f;
        platform.size = glm::vec3(0, 0, 0);
        
        // 現在のサイクル内で点滅期間が終了しているので、完了したサイクル数 + 1
        platform.blinkTimer = (completedCycles + 1) * platform.blinkTime;
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
