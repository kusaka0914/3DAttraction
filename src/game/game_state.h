/**
 * @file game_state.h
 * @brief ゲーム全体の状態を管理する構造体
 * @details 各サブシステムの状態（プレイヤー、カメラ、アイテム、スキル、進行状況、リプレイ、UI）を統合的に管理します。
 */
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <vector>
#include <map>
#include <set>
#include <string>
#include <glm/glm.hpp>
#include "../core/types/platform_types.h"
#include "player_state.h"
#include "camera_state.h"
#include "item_state.h"
#include "skill_state.h"
#include "game_progress_state.h"
#include "replay_state.h"
#include "ui_state.h"

struct EditorState;

/**
 * @brief ゲーム全体の状態を管理する構造体
 * @details 各サブシステムの状態を統合的に管理します。
 * PlayerState、CameraState、ItemState、SkillState、GameProgressState、
 * ReplayState、UIStateなどのサブシステム状態を保持します。
 */
struct GameState {
    // 分割された状態構造体
    PlayerState player;
    CameraState camera;
    ItemState items;
    SkillState skills;
    GameProgressState progress;
    ReplayState replay;
    UIState ui;
    
    
    struct Platform {
        glm::vec3 position;
        glm::vec3 size;
        glm::vec3 color;
        bool isMoving;
        glm::vec3 moveDirection;
        float moveSpeed;  // 移動速度（単位: ワールド座標/秒）
        float moveRange;  // 移動範囲（単位: ワールド座標）
        float moveTimer;  // 移動タイマー（単位: 秒）
        bool isDisappearing;
        float disappearTimer;  // 消失タイマー（単位: 秒）
        bool isJumpPad;
        bool isRotating;
        glm::vec3 rotationAxis;
        float rotationSpeed;  // 回転速度（単位: 度/秒）
        float rotationAngle;  // 回転角度（単位: 度）
        bool isPatrolling;
        std::vector<glm::vec3> patrolPoints;
        int currentPatrolIndex;  // 現在の巡回ポイントインデックス（-1の場合は未開始）
        float patrolSpeed;  // 巡回速度（単位: ワールド座標/秒）
        float patrolTimer;  // 巡回タイマー（単位: 秒）
        bool isCycleDisappearing;
        float cycleTime;  // サイクル時間（単位: 秒）
        float visibleTime;  // 表示時間（単位: 秒）
        float cycleTimer;  // サイクルタイマー（単位: 秒）
        bool isCurrentlyVisible;
        glm::vec3 originalSize;
        float blinkTime;  // 点滅時間（単位: 秒）
        float blinkTimer;  // 点滅タイマー（単位: 秒）
        bool isBlinking;
        float blinkAlpha;  // 点滅アルファ値（0.0～1.0）
        bool isFlyingPlatform;
        glm::vec3 spawnPosition;
        glm::vec3 targetPosition;
        float flySpeed;  // 飛行速度（単位: ワールド座標/秒）
        bool isFlying;
        bool hasSpawned;
        float detectionRange;  // 検出範囲（単位: ワールド座標）
        bool isReturning;
        bool isTeleportPlatform;
        glm::vec3 teleportDestination;
        bool hasTeleported;
        float teleportCooldown;  // テレポートクールダウン時間（単位: 秒）
        float teleportCooldownTimer;  // テレポートクールダウンタイマー（単位: 秒）
        bool isMovingPlatform;
        glm::vec3 moveTargetPosition;
        bool hasPlayerOnBoard;
        glm::vec3 originalPosition;
        bool returnToOriginal;
    };
    
    struct Cannon {
        glm::vec3 position;
        glm::vec3 size;
        glm::vec3 color;
        glm::vec3 targetPosition;
        float power;  // 発射威力（単位: ワールド座標/秒）
        bool isActive;
        bool hasPlayerInside;
        float cooldownTimer;  // クールダウンタイマー（単位: 秒）
        float cooldownTime;  // クールダウン時間（単位: 秒）
        glm::vec3 launchDirection;
    };
    
    struct GravityZone {
        glm::vec3 position;
        glm::vec3 size;
        glm::vec3 gravityDirection;
        float radius;  // 重力ゾーンの半径（単位: ワールド座標）
        bool isActive;
    };
    
    struct Switch {
        glm::vec3 position;
        glm::vec3 size;
        glm::vec3 color;
        bool isPressed;
        bool isToggle;
        std::vector<int> targetPlatformIndices;
        std::vector<bool> targetStates;
        float pressTimer;  // 押下タイマー（単位: 秒）
        float cooldownTimer;  // クールダウンタイマー（単位: 秒）
        bool isMultiSwitch;
        int multiSwitchGroup;
    };
    
    std::vector<Cannon> cannons;
    std::vector<Platform> platforms;
    std::vector<GravityZone> gravityZones;
    std::vector<Switch> switches;
    
    bool audioEnabled = true;
    std::string currentBGM = "";
    bool bgmPlaying = false;
    
    EditorState* editorState = nullptr;
    
};

/**
 * @brief ゲーム状態を初期化する
 * @details プラットフォーム、重力ゾーン、スイッチ、大砲などの
 * ゲーム要素をクリアし、UI状態とプレイヤー状態を初期値に設定します。
 * 
 * @param gameState 初期化するゲーム状態
 */
void initializeGameState(GameState& gameState);
