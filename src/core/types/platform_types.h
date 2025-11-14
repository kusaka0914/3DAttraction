/**
 * @file platform_types.h
 * @brief プラットフォームタイプ定義
 * @details 各種プラットフォームタイプ（静的、移動、回転、テレポートなど）の構造体を定義します。
 */
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <vector>
#include <variant>
#include <glm/glm.hpp>

// 基底Platform構造体
struct BasePlatform {
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    bool isVisible = true;
    
    BasePlatform(const glm::vec3& pos, const glm::vec3& siz, const glm::vec3& col)
        : position(pos), size(siz), color(col) {}
};

// 静的足場
struct StaticPlatform : public BasePlatform {
    StaticPlatform(const glm::vec3& pos, const glm::vec3& siz, const glm::vec3& col)
        : BasePlatform(pos, siz, col) {}
};

// 移動足場
struct MovingPlatform : public BasePlatform {
    glm::vec3 moveTargetPosition;
    float moveSpeed;
    bool hasPlayerOnBoard = false;
    glm::vec3 originalPosition;
    bool returnToOriginal = false;
    float moveTimer = 0.0f;
    glm::vec3 previousPosition;
    
    MovingPlatform(const glm::vec3& pos, const glm::vec3& siz, const glm::vec3& col,
                   const glm::vec3& target, float speed)
        : BasePlatform(pos, siz, col), moveTargetPosition(target), moveSpeed(speed), 
          originalPosition(pos), previousPosition(pos) {}
};

// 回転足場
struct RotatingPlatform : public BasePlatform {
    glm::vec3 rotationAxis;
    float rotationSpeed;
    float rotationAngle = 0.0f;
    
    RotatingPlatform(const glm::vec3& pos, const glm::vec3& siz, const glm::vec3& col,
                     const glm::vec3& axis, float speed)
        : BasePlatform(pos, siz, col), rotationAxis(axis), rotationSpeed(speed) {}
};

// 巡回足場
struct PatrollingPlatform : public BasePlatform {
    std::vector<glm::vec3> patrolPoints;
    float patrolSpeed;
    int currentPatrolIndex = 0;
    float patrolTimer = 0.0f;
    glm::vec3 previousPosition;
    
    PatrollingPlatform(const glm::vec3& pos, const glm::vec3& siz, const glm::vec3& col,
                       const std::vector<glm::vec3>& points, float speed)
        : BasePlatform(pos, siz, col), patrolPoints(points), patrolSpeed(speed), 
          previousPosition(pos) {}
};

// テレポート足場
struct TeleportPlatform : public BasePlatform {
    glm::vec3 teleportDestination;
    float cooldown;
    bool hasTeleported = false;
    float cooldownTimer = 0.0f;
    
    TeleportPlatform(const glm::vec3& pos, const glm::vec3& siz, const glm::vec3& col,
                     const glm::vec3& destination, float cd = 2.0f)
        : BasePlatform(pos, siz, col), teleportDestination(destination), cooldown(cd) {}
};

// ジャンプパッド
struct JumpPad : public BasePlatform {
    float jumpPower;
    
    JumpPad(const glm::vec3& pos, const glm::vec3& siz, const glm::vec3& col, float power = 15.0f)
        : BasePlatform(pos, siz, col), jumpPower(power) {}
};

// 周期的に消える足場
struct CycleDisappearingPlatform : public BasePlatform {
    float cycleTime;
    float visibleTime;
    float blinkTime;
    float cycleTimer;
    float blinkTimer = 0.0f;
    bool isCurrentlyVisible;
    bool isBlinking = false;
    float blinkAlpha = 1.0f;
    glm::vec3 originalSize;
    
    CycleDisappearingPlatform(const glm::vec3& pos, const glm::vec3& siz, const glm::vec3& col,
                              float cycle, float visible, float blink, float initial = 0.0f)
        : BasePlatform(pos, siz, col), cycleTime(cycle), visibleTime(visible), 
          blinkTime(blink), cycleTimer(cycle - initial), isCurrentlyVisible(initial < visible),
          originalSize(siz) {}
};

// 消える足場
struct DisappearingPlatform : public BasePlatform {
    float disappearTimer = 0.0f;
    
    DisappearingPlatform(const glm::vec3& pos, const glm::vec3& siz, const glm::vec3& col)
        : BasePlatform(pos, siz, col) {}
};

// 飛んでくる足場
struct FlyingPlatform : public BasePlatform {
    glm::vec3 spawnPosition;
    glm::vec3 targetPosition;
    float flySpeed;
    float detectionRange;
    bool isFlying = false;
    bool hasSpawned = false;
    bool isReturning = false;
    
    FlyingPlatform(const glm::vec3& pos, const glm::vec3& siz, const glm::vec3& col,
                   const glm::vec3& spawn, const glm::vec3& target, float speed, float range)
        : BasePlatform(pos, siz, col), spawnPosition(spawn), targetPosition(target), 
          flySpeed(speed), detectionRange(range) {
        isVisible = false; // 初期状態では見えない
        size = glm::vec3(0, 0, 0); // 初期状態ではサイズを0にする
    }
};

// 全ての足場タイプを管理
using PlatformVariant = std::variant<
    StaticPlatform,
    MovingPlatform,
    RotatingPlatform,
    PatrollingPlatform,
    TeleportPlatform,
    JumpPad,
    CycleDisappearingPlatform,
    DisappearingPlatform,
    FlyingPlatform
>;
