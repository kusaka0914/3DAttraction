#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../gfx/opengl_renderer.h"

// =========================
//  カービィの能力タイプ
// =========================
enum class KirbyAbility {
    None,
    Fire,
    Ice,
    Electric,
    Sword,
    Hammer
};

// =========================
//  敵の種類
// =========================
enum class EnemyType {
    WaddleDee,
    WaddleDoo,
    BrontoBurt,
    Scarfy
};

// =========================
//  敵の構造体
// =========================
struct Enemy {
    glm::vec3 position;
    glm::vec3 velocity;
    EnemyType type;
    KirbyAbility ability;
    bool isAlive;
    float health;
    float size;
    
    // 視野システム
    float viewRange;        // 視野範囲
    float viewAngle;        // 視野角度（度）
    glm::vec3 viewDirection; // 現在向いている方向
    bool isChasing;         // プレイヤーを追いかけているか
    glm::vec3 initialPosition; // 初期位置（パトロール用）
    float patrolTimer;      // パトロールタイマー
};

// =========================
//  ゲーム状態
// =========================
struct GameState {
    // カービィ（プレイヤー）
    glm::vec3 playerPosition = glm::vec3(0, 30.0f, 0); // 空中に配置
    glm::vec3 playerVelocity = glm::vec3(0, 0, 0);
    glm::vec3 playerColor = glm::vec3(1, 0.8f, 0.9f); // ピンク色
    bool isFloating = false;
    bool isInflated = false;
    bool isSucking = false;
    KirbyAbility currentAbility = KirbyAbility::None;
    float abilityTimer = 0.0f;
    float floatTimer = 0.0f;
    float suckTimer = 0.0f;
    int floatCount = 0; // 浮遊回数カウンター
    int currentPlatformIndex = -1; // 現在乗っている足場のインデックス
    
    // レンダリング準備状態
    bool renderingReady = true;  // 即座に準備完了
    float initializationTimer = 0.0f;
    float initializationDelay = 0.0f;  // 遅延なし

    // 敵
    std::vector<Enemy> enemies;
    
    // ステージ要素
    glm::vec3 goalPosition = glm::vec3(0, 2.0f, 20); // 空中に配置
    glm::vec3 goalColor = glm::vec3(1, 1, 0);
    bool gameWon = false;
    
    // 空中アクション用の足場データ
    struct Platform {
        glm::vec3 position;
        glm::vec3 size;
        glm::vec3 color;
        bool isMoving;
        glm::vec3 moveDirection;
        float moveSpeed;
        float moveRange;
        float moveTimer;
        bool isDisappearing;
        float disappearTimer;
        bool isJumpPad;
        
        // 回転機能
        bool isRotating;
        glm::vec3 rotationAxis;
        float rotationSpeed;
        float rotationAngle;
        
        // 巡回機能
        bool isPatrolling;
        std::vector<glm::vec3> patrolPoints;
        int currentPatrolIndex;
        float patrolSpeed;
        float patrolTimer;
        
        // 周期的な消失機能
        bool isCycleDisappearing;
        float cycleTime;           // 1サイクルの時間
        float visibleTime;         // 見える時間
        float cycleTimer;          // サイクルタイマー
        bool isCurrentlyVisible;   // 現在見えているか
        glm::vec3 originalSize;    // 元のサイズ（復元用）
        float blinkTime;           // 点滅時間
        float blinkTimer;          // 点滅タイマー
        bool isBlinking;           // 点滅中かどうか
        float blinkAlpha;          // 点滅の透明度（1.0=完全に見える、0.0=透明）
        
        // プレイヤー接近で飛んでくる足場
        bool isFlyingPlatform;     // 飛んでくる足場かどうか
        glm::vec3 spawnPosition;   // 出現位置（遠くの空）
        glm::vec3 targetPosition;  // 目標位置（プレイヤー近く）
        float flySpeed;            // 飛行速度
        bool isFlying;             // 飛行中かどうか
        bool hasSpawned;           // 出現済みかどうか
        float detectionRange;      // プレイヤー検知範囲
        bool isReturning;          // 元の場所に戻り中かどうか
        
        // テレポート機能
        bool isTeleportPlatform;   // テレポート足場かどうか
        glm::vec3 teleportDestination; // テレポート先の位置
        bool hasTeleported;        // テレポート済みかどうか
        float teleportCooldown;    // テレポートのクールダウン時間
        float teleportCooldownTimer; // テレポートのクールダウンタイマー
        
        // 移動足場機能
        bool isMovingPlatform;     // 移動足場かどうか
        glm::vec3 moveTargetPosition; // 移動先の位置
        bool hasPlayerOnBoard;     // プレイヤーが乗っているかどうか
        glm::vec3 originalPosition; // 元の位置
        bool returnToOriginal;     // 元の位置に戻るかどうか
    };
    
    // 大砲システム
    struct Cannon {
        glm::vec3 position;        // 大砲の位置
        glm::vec3 size;           // 大砲のサイズ
        glm::vec3 color;          // 大砲の色
        glm::vec3 targetPosition; // 発射先の位置
        float power;              // 発射パワー
        bool isActive;            // 大砲が有効かどうか
        bool hasPlayerInside;     // プレイヤーが大砲の中にいるか
        float cooldownTimer;      // クールダウンタイマー
        float cooldownTime;       // クールダウン時間
        glm::vec3 launchDirection; // 発射方向（計算済み）
    };
    
    std::vector<Cannon> cannons;  // 大砲のリスト
    
    std::vector<Platform> platforms;
    float terrainScale = 1.0f;

    // カメラ制御（動的：今回は固定寄り）
    float cameraYaw = 0.0f;
    float cameraPitch = 90.0f; 
    float cameraDistance = 30.0f;
    bool firstMouse = true;
    float lastMouseX = 640.0f;
    float lastMouseY = 360.0f;

    // 重力反転エリア
    struct GravityZone {
        glm::vec3 position;
        glm::vec3 size;
        glm::vec3 gravityDirection;  // 重力の方向
        float radius;
        bool isActive;
    };
    
    std::vector<GravityZone> gravityZones;
    
    // スイッチシステム
    struct Switch {
        glm::vec3 position;
        glm::vec3 size;
        glm::vec3 color;
        bool isPressed;
        bool isToggle;  // トグルスイッチかどうか
        std::vector<int> targetPlatformIndices;  // 操作する足場のインデックス
        std::vector<bool> targetStates;  // 目標状態（true=出現、false=消失）
        float pressTimer;  // 押されている時間
        float cooldownTimer;  // クールダウン時間
        bool isMultiSwitch;  // 複数スイッチの一部かどうか
        int multiSwitchGroup;  // 複数スイッチのグループID
    };
    
    std::vector<Switch> switches;
    
    // デコ（道路マーカーや草パッチ）
    std::vector<glm::vec3> roadMarkers; // 細いグレーの目印（簡易道路表現）
    std::vector<glm::vec3> grassDecos;  // 小さな緑ブロック
    
    int score = 0;
    float gameTime = 0.0f;
    int currentStage = 2;
};

// ------------------------------------------------------
// マウス・スクロール（今回は未使用だが一応残す）
// ------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    GameState* gameState = static_cast<GameState*>(glfwGetWindowUserPointer(window));
    if (gameState->firstMouse) {
        gameState->lastMouseX = xpos;
        gameState->lastMouseY = ypos;
        gameState->firstMouse = false;
    }
    float xoffset = float(xpos - gameState->lastMouseX);
    float yoffset = float(gameState->lastMouseY - ypos);
    gameState->lastMouseX = float(xpos);
    gameState->lastMouseY = float(ypos);
    
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    gameState->cameraYaw += xoffset;
    gameState->cameraPitch += yoffset;
    gameState->cameraPitch = std::max(-89.0f, std::min(89.0f, gameState->cameraPitch));
}

void scroll_callback(GLFWwindow* window, double, double yoffset) {
    GameState* gameState = static_cast<GameState*>(glfwGetWindowUserPointer(window));
    gameState->cameraDistance -= float(yoffset);
    gameState->cameraDistance = std::max(1.0f, std::min(50.0f, gameState->cameraDistance));
}

// ------------------------------------------------------
// 空中アクション用の足場生成
// ------------------------------------------------------
void generatePlatforms(GameState& gameState) {
    gameState.platforms.clear();
    
    // マリオサンシャイン風の一直線アスレチックコース
    // 手前から奥へ一直線に続く足場を配置
    
    // スタート足場（緑）
    GameState::Platform startPlatform;
    startPlatform.position = glm::vec3(0, 5, -25);
    startPlatform.size = glm::vec3(4, 1, 4);
    startPlatform.color = glm::vec3(0.2f, 0.8f, 0.2f);
    startPlatform.isMoving = false;
    startPlatform.isDisappearing = false;
    startPlatform.moveDirection = glm::vec3(0, 0, 0);
    startPlatform.moveSpeed = 0.0f;
    startPlatform.moveRange = 0.0f;
    startPlatform.moveTimer = 0.0f;
    startPlatform.disappearTimer = 0.0f;
    startPlatform.isJumpPad = false;
    startPlatform.isRotating = false;
    startPlatform.rotationAxis = glm::vec3(0, 1, 0);
    startPlatform.rotationSpeed = 0.0f;
    startPlatform.rotationAngle = 0.0f;
    startPlatform.isPatrolling = false;
    startPlatform.patrolPoints = {};
    startPlatform.currentPatrolIndex = 0;
    startPlatform.patrolSpeed = 0.0f;
    startPlatform.patrolTimer = 0.0f;
    startPlatform.isCycleDisappearing = false;
    startPlatform.cycleTime = 0.0f;
    startPlatform.visibleTime = 0.0f;
    startPlatform.cycleTimer = 0.0f;
    startPlatform.isCurrentlyVisible = true;
    startPlatform.originalSize = startPlatform.size;
    startPlatform.blinkTime = 0.0f;
    startPlatform.blinkTimer = 0.0f;
    startPlatform.isBlinking = false;
    startPlatform.blinkAlpha = 1.0f;
    startPlatform.isFlyingPlatform = false;
    gameState.platforms.push_back(startPlatform);
    
    // 回転する足場（スタート地点の左）
    GameState::Platform rotatingPlatform;
    rotatingPlatform.position = glm::vec3(-8, 5, -25);
    rotatingPlatform.size = glm::vec3(3, 1, 3);
    rotatingPlatform.color = glm::vec3(0.8f, 0.2f, 0.8f); // 紫色
    rotatingPlatform.isMoving = false;
    rotatingPlatform.isDisappearing = false;
    rotatingPlatform.moveDirection = glm::vec3(0, 0, 0);
    rotatingPlatform.moveSpeed = 0.0f;
    rotatingPlatform.moveRange = 0.0f;
    rotatingPlatform.moveTimer = 0.0f;
    rotatingPlatform.disappearTimer = 0.0f;
    rotatingPlatform.isJumpPad = false;
    rotatingPlatform.isRotating = true;
    rotatingPlatform.rotationAxis = glm::vec3(1, 0, 0); // X軸周りに回転（縦方向）
    rotatingPlatform.rotationSpeed = 90.0f; // 90度/秒
    rotatingPlatform.rotationAngle = 0.0f;
    rotatingPlatform.isPatrolling = false;
    rotatingPlatform.patrolPoints = {};
    rotatingPlatform.currentPatrolIndex = 0;
    rotatingPlatform.patrolSpeed = 0.0f;
    rotatingPlatform.patrolTimer = 0.0f;
    rotatingPlatform.isCycleDisappearing = false;
    rotatingPlatform.cycleTime = 0.0f;
    rotatingPlatform.visibleTime = 0.0f;
    rotatingPlatform.cycleTimer = 0.0f;
    rotatingPlatform.isCurrentlyVisible = true;
    rotatingPlatform.originalSize = rotatingPlatform.size;
    rotatingPlatform.blinkTime = 0.0f;
    rotatingPlatform.blinkTimer = 0.0f;
    rotatingPlatform.isBlinking = false;
    rotatingPlatform.blinkAlpha = 1.0f;
    rotatingPlatform.isFlyingPlatform = false;
    gameState.platforms.push_back(rotatingPlatform);
    
    // 第1セクション：基本的なジャンプ
    GameState::Platform platform1;
    platform1.position = glm::vec3(0, 5, -20);
    platform1.size = glm::vec3(3, 1, 3);
    platform1.color = glm::vec3(0.2f, 0.6f, 1.0f);
    platform1.isMoving = false;
    platform1.isDisappearing = false;
    platform1.moveDirection = glm::vec3(0, 0, 0);
    platform1.moveSpeed = 0.0f;
    platform1.moveRange = 0.0f;
    platform1.moveTimer = 0.0f;
    platform1.disappearTimer = 0.0f;
    platform1.isJumpPad = false;
    platform1.isRotating = false;
    platform1.rotationAxis = glm::vec3(0, 1, 0);
    platform1.rotationSpeed = 0.0f;
    platform1.rotationAngle = 0.0f;
    platform1.isPatrolling = false;
    platform1.patrolPoints = {};
    platform1.currentPatrolIndex = 0;
    platform1.patrolSpeed = 0.0f;
    platform1.patrolTimer = 0.0f;
    platform1.isCycleDisappearing = false;
    platform1.cycleTime = 0.0f;
    platform1.visibleTime = 0.0f;
    platform1.cycleTimer = 0.0f;
    platform1.isCurrentlyVisible = true;
    platform1.originalSize = platform1.size;
    platform1.blinkTime = 0.0f;
    platform1.blinkTimer = 0.0f;
    platform1.isBlinking = false;
    platform1.blinkAlpha = 1.0f;
    platform1.isFlyingPlatform = false;
    gameState.platforms.push_back(platform1);
    
    GameState::Platform platform2;
    platform2.position = glm::vec3(0, 5, -15);
    platform2.size = glm::vec3(3, 1, 3);
    platform2.color = glm::vec3(0.2f, 0.6f, 1.0f);
    platform2.isMoving = false;
    platform2.isDisappearing = false;
    platform2.moveDirection = glm::vec3(0, 0, 0);
    platform2.moveSpeed = 0.0f;
    platform2.moveRange = 0.0f;
    platform2.moveTimer = 0.0f;
    platform2.disappearTimer = 0.0f;
    platform2.isJumpPad = false;
    platform2.isRotating = false;
    platform2.rotationAxis = glm::vec3(0, 1, 0);
    platform2.rotationSpeed = 0.0f;
    platform2.rotationAngle = 0.0f;
    platform2.isPatrolling = false;
    platform2.patrolPoints = {};
    platform2.currentPatrolIndex = 0;
    platform2.patrolSpeed = 0.0f;
    platform2.patrolTimer = 0.0f;
    platform2.isCycleDisappearing = false;
    platform2.cycleTime = 0.0f;
    platform2.visibleTime = 0.0f;
    platform2.cycleTimer = 0.0f;
    platform2.isCurrentlyVisible = true;
    platform2.originalSize = platform2.size;
    platform2.blinkTime = 0.0f;
    platform2.blinkTimer = 0.0f;
    platform2.isBlinking = false;
    platform2.blinkAlpha = 1.0f;
    platform2.isFlyingPlatform = false;
    gameState.platforms.push_back(platform2);
    
    // 第2セクション：少し高いジャンプ
    GameState::Platform platform3;
    platform3.position = glm::vec3(0, 7, -10);
    platform3.size = glm::vec3(3, 1, 3);
    platform3.color = glm::vec3(0.2f, 0.6f, 1.0f);
    platform3.isMoving = false;
    platform3.isDisappearing = false;
    platform3.moveDirection = glm::vec3(0, 0, 0);
    platform3.moveSpeed = 0.0f;
    platform3.moveRange = 0.0f;
    platform3.moveTimer = 0.0f;
    platform3.disappearTimer = 0.0f;
    platform3.isJumpPad = false;
    platform3.isRotating = false;
    platform3.rotationAxis = glm::vec3(0, 1, 0);
    platform3.rotationSpeed = 0.0f;
    platform3.rotationAngle = 0.0f;
    platform3.isPatrolling = false;
    platform3.patrolPoints = {};
    platform3.currentPatrolIndex = 0;
    platform3.patrolSpeed = 0.0f;
    platform3.patrolTimer = 0.0f;
    platform3.isCycleDisappearing = false;
    platform3.cycleTime = 0.0f;
    platform3.visibleTime = 0.0f;
    platform3.cycleTimer = 0.0f;
    platform3.isCurrentlyVisible = true;
    platform3.originalSize = platform3.size;
    platform3.blinkTime = 0.0f;
    platform3.blinkTimer = 0.0f;
    platform3.isBlinking = false;
    platform3.blinkAlpha = 1.0f;
    platform3.isFlyingPlatform = false;
    gameState.platforms.push_back(platform3);
    
    GameState::Platform platform4;
    platform4.position = glm::vec3(0, 7, -5);
    platform4.size = glm::vec3(3, 1, 3);
    platform4.color = glm::vec3(0.2f, 0.6f, 1.0f);
    platform4.isMoving = false;
    platform4.isDisappearing = false;
    platform4.moveDirection = glm::vec3(0, 0, 0);
    platform4.moveSpeed = 0.0f;
    platform4.moveRange = 0.0f;
    platform4.moveTimer = 0.0f;
    platform4.disappearTimer = 0.0f;
    platform4.isJumpPad = false;
    platform4.isRotating = false;
    platform4.rotationAxis = glm::vec3(0, 1, 0);
    platform4.rotationSpeed = 0.0f;
    platform4.rotationAngle = 0.0f;
    platform4.isPatrolling = false;
    platform4.patrolPoints = {};
    platform4.currentPatrolIndex = 0;
    platform4.patrolSpeed = 0.0f;
    platform4.patrolTimer = 0.0f;
    platform4.isCycleDisappearing = false;
    platform4.cycleTime = 0.0f;
    platform4.visibleTime = 0.0f;
    platform4.cycleTimer = 0.0f;
    platform4.isCurrentlyVisible = true;
    platform4.originalSize = platform4.size;
    platform4.blinkTime = 0.0f;
    platform4.blinkTimer = 0.0f;
    platform4.isBlinking = false;
    platform4.blinkAlpha = 1.0f;
    platform4.isFlyingPlatform = false;
    gameState.platforms.push_back(platform4);
    
    // 第3セクション：移動足場（既存の移動足場は削除）
    
    // 第4セクション：消える足場
    GameState::Platform disappearingPlatform1;
    disappearingPlatform1.position = glm::vec3(0, 11, 10);
    disappearingPlatform1.size = glm::vec3(2, 1, 2);
    disappearingPlatform1.color = glm::vec3(1.0f, 0.2f, 0.2f);
    disappearingPlatform1.isMoving = false;
    disappearingPlatform1.isDisappearing = true;
    disappearingPlatform1.moveDirection = glm::vec3(0, 0, 0);
    disappearingPlatform1.moveSpeed = 0.0f;
    disappearingPlatform1.moveRange = 0.0f;
    disappearingPlatform1.moveTimer = 0.0f;
    disappearingPlatform1.disappearTimer = 0.0f;
    disappearingPlatform1.isJumpPad = false;
    disappearingPlatform1.isRotating = false;
    disappearingPlatform1.rotationAxis = glm::vec3(0, 1, 0);
    disappearingPlatform1.rotationSpeed = 0.0f;
    disappearingPlatform1.rotationAngle = 0.0f;
    disappearingPlatform1.isPatrolling = false;
    disappearingPlatform1.patrolPoints = {};
    disappearingPlatform1.currentPatrolIndex = 0;
    disappearingPlatform1.patrolSpeed = 0.0f;
    disappearingPlatform1.patrolTimer = 0.0f;
    disappearingPlatform1.isCycleDisappearing = false;
    disappearingPlatform1.cycleTime = 0.0f;
    disappearingPlatform1.visibleTime = 0.0f;
    disappearingPlatform1.cycleTimer = 0.0f;
    disappearingPlatform1.isCurrentlyVisible = true;
    disappearingPlatform1.originalSize = disappearingPlatform1.size;
    disappearingPlatform1.blinkTime = 0.0f;
    disappearingPlatform1.blinkTimer = 0.0f;
    disappearingPlatform1.isBlinking = false;
    disappearingPlatform1.blinkAlpha = 1.0f;
    gameState.platforms.push_back(disappearingPlatform1);
    
    GameState::Platform disappearingPlatform2;
    disappearingPlatform2.position = glm::vec3(0, 11, 15);
    disappearingPlatform2.size = glm::vec3(2, 1, 2);
    disappearingPlatform2.color = glm::vec3(1.0f, 0.2f, 0.2f);
    disappearingPlatform2.isMoving = false;
    disappearingPlatform2.isDisappearing = true;
    disappearingPlatform2.moveDirection = glm::vec3(0, 0, 0);
    disappearingPlatform2.moveSpeed = 0.0f;
    disappearingPlatform2.moveRange = 0.0f;
    disappearingPlatform2.moveTimer = 0.0f;
    disappearingPlatform2.disappearTimer = 0.0f;
    disappearingPlatform2.isJumpPad = false;
    disappearingPlatform2.isRotating = false;
    disappearingPlatform2.rotationAxis = glm::vec3(0, 1, 0);
    disappearingPlatform2.rotationSpeed = 0.0f;
    disappearingPlatform2.rotationAngle = 0.0f;
    disappearingPlatform2.isPatrolling = false;
    disappearingPlatform2.patrolPoints = {};
    disappearingPlatform2.currentPatrolIndex = 0;
    disappearingPlatform2.patrolSpeed = 0.0f;
    disappearingPlatform2.patrolTimer = 0.0f;
    disappearingPlatform2.isCycleDisappearing = false;
    disappearingPlatform2.cycleTime = 0.0f;
    disappearingPlatform2.visibleTime = 0.0f;
    disappearingPlatform2.cycleTimer = 0.0f;
    disappearingPlatform2.isCurrentlyVisible = true;
    disappearingPlatform2.originalSize = disappearingPlatform2.size;
    disappearingPlatform2.blinkTime = 0.0f;
    disappearingPlatform2.blinkTimer = 0.0f;
    disappearingPlatform2.isBlinking = false;
    disappearingPlatform2.blinkAlpha = 1.0f;
    gameState.platforms.push_back(disappearingPlatform2);
    
    // 第5セクション：ジャンプパッド
    GameState::Platform jumpPad;
    jumpPad.position = glm::vec3(0, 13, 20);
    jumpPad.size = glm::vec3(2.5f, 1, 2.5f);
    jumpPad.color = glm::vec3(0.0f, 0.8f, 1.0f);
    jumpPad.isMoving = false;
    jumpPad.isDisappearing = false;
    jumpPad.moveDirection = glm::vec3(0, 0, 0);
    jumpPad.moveSpeed = 0.0f;
    jumpPad.moveRange = 0.0f;
    jumpPad.moveTimer = 0.0f;
    jumpPad.disappearTimer = 0.0f;
    jumpPad.isJumpPad = true;
    jumpPad.isRotating = false;
    jumpPad.rotationAxis = glm::vec3(0, 1, 0);
    jumpPad.rotationSpeed = 0.0f;
    jumpPad.rotationAngle = 0.0f;
    jumpPad.isPatrolling = false;
    jumpPad.patrolPoints = {};
    jumpPad.currentPatrolIndex = 0;
    jumpPad.patrolSpeed = 0.0f;
    jumpPad.patrolTimer = 0.0f;
    jumpPad.isCycleDisappearing = false;
    jumpPad.cycleTime = 0.0f;
    jumpPad.visibleTime = 0.0f;
    jumpPad.cycleTimer = 0.0f;
    jumpPad.isCurrentlyVisible = true;
    jumpPad.originalSize = jumpPad.size;
    jumpPad.blinkTime = 0.0f;
    jumpPad.blinkTimer = 0.0f;
    jumpPad.isBlinking = false;
    jumpPad.blinkAlpha = 1.0f;
    gameState.platforms.push_back(jumpPad);
    
    // ゴール足場（黄色）
    GameState::Platform goalPlatform;
    goalPlatform.position = glm::vec3(0, 15, 25);
    goalPlatform.size = glm::vec3(4, 1, 4);
    goalPlatform.color = glm::vec3(1.0f, 1.0f, 0.0f);
    goalPlatform.isMoving = false;
    goalPlatform.isDisappearing = false;
    goalPlatform.moveDirection = glm::vec3(0, 0, 0);
    goalPlatform.moveSpeed = 0.0f;
    goalPlatform.moveRange = 0.0f;
    goalPlatform.moveTimer = 0.0f;
    goalPlatform.disappearTimer = 0.0f;
    goalPlatform.isJumpPad = false;
    goalPlatform.isRotating = false;
    goalPlatform.rotationAxis = glm::vec3(0, 1, 0);
    goalPlatform.rotationSpeed = 0.0f;
    goalPlatform.rotationAngle = 0.0f;
    goalPlatform.isPatrolling = false;
    goalPlatform.patrolPoints = {};
    goalPlatform.currentPatrolIndex = 0;
    goalPlatform.patrolSpeed = 0.0f;
    goalPlatform.patrolTimer = 0.0f;
    goalPlatform.isCycleDisappearing = false;
    goalPlatform.cycleTime = 0.0f;
    goalPlatform.visibleTime = 0.0f;
    goalPlatform.cycleTimer = 0.0f;
    goalPlatform.isCurrentlyVisible = true;
    goalPlatform.originalSize = goalPlatform.size;
    goalPlatform.blinkTime = 0.0f;
    goalPlatform.blinkTimer = 0.0f;
    goalPlatform.isBlinking = false;
    goalPlatform.blinkAlpha = 1.0f;
    gameState.platforms.push_back(goalPlatform);
    
    // 巡回する足場（スタート地点の右）
    GameState::Platform patrollingPlatform;
    patrollingPlatform.position = glm::vec3(8, 5, -25);
    patrollingPlatform.size = glm::vec3(2.5f, 1, 2.5f);
    patrollingPlatform.color = glm::vec3(0.2f, 0.8f, 0.8f); // 水色
    patrollingPlatform.isMoving = false;
    patrollingPlatform.isDisappearing = false;
    patrollingPlatform.moveDirection = glm::vec3(0, 0, 0);
    patrollingPlatform.moveSpeed = 0.0f;
    patrollingPlatform.moveRange = 0.0f;
    patrollingPlatform.moveTimer = 0.0f;
    patrollingPlatform.disappearTimer = 0.0f;
    patrollingPlatform.isJumpPad = false;
    patrollingPlatform.isRotating = false;
    patrollingPlatform.rotationAxis = glm::vec3(0, 1, 0);
    patrollingPlatform.rotationSpeed = 0.0f;
    patrollingPlatform.rotationAngle = 0.0f;
    patrollingPlatform.isPatrolling = true;
    patrollingPlatform.patrolPoints = {
        glm::vec3(8, 5, -25),   // スタート位置
        glm::vec3(12, 7, -20),  // 右上
        glm::vec3(8, 9, -15),   // 上
        glm::vec3(4, 7, -20),   // 左上
        glm::vec3(8, 5, -25)    // スタート位置に戻る
    };
    patrollingPlatform.currentPatrolIndex = 0;
    patrollingPlatform.patrolSpeed = 2.0f; // 2秒で1点移動
    patrollingPlatform.patrolTimer = 0.0f;
    patrollingPlatform.isCycleDisappearing = false;
    patrollingPlatform.cycleTime = 0.0f;
    patrollingPlatform.visibleTime = 0.0f;
    patrollingPlatform.cycleTimer = 0.0f;
    patrollingPlatform.isCurrentlyVisible = true;
    patrollingPlatform.originalSize = patrollingPlatform.size;
    patrollingPlatform.blinkTime = 0.0f;
    patrollingPlatform.blinkTimer = 0.0f;
    patrollingPlatform.isBlinking = false;
    patrollingPlatform.blinkAlpha = 1.0f;
    gameState.platforms.push_back(patrollingPlatform);
    
    // 周期的に消える足場（スタート地点の後ろ）
    GameState::Platform cycleDisappearingPlatform;
    cycleDisappearingPlatform.position = glm::vec3(0, 5, -30);
    cycleDisappearingPlatform.size = glm::vec3(3, 1, 3);
    cycleDisappearingPlatform.color = glm::vec3(1.0f, 0.5f, 0.0f); // オレンジ色
    cycleDisappearingPlatform.isMoving = false;
    cycleDisappearingPlatform.isDisappearing = false;
    cycleDisappearingPlatform.moveDirection = glm::vec3(0, 0, 0);
    cycleDisappearingPlatform.moveSpeed = 0.0f;
    cycleDisappearingPlatform.moveRange = 0.0f;
    cycleDisappearingPlatform.moveTimer = 0.0f;
    cycleDisappearingPlatform.disappearTimer = 0.0f;
    cycleDisappearingPlatform.isJumpPad = false;
    cycleDisappearingPlatform.isRotating = false;
    cycleDisappearingPlatform.rotationAxis = glm::vec3(0, 1, 0);
    cycleDisappearingPlatform.rotationSpeed = 0.0f;
    cycleDisappearingPlatform.rotationAngle = 0.0f;
    cycleDisappearingPlatform.isPatrolling = false;
    cycleDisappearingPlatform.patrolPoints = {};
    cycleDisappearingPlatform.currentPatrolIndex = 0;
    cycleDisappearingPlatform.patrolSpeed = 0.0f;
    cycleDisappearingPlatform.patrolTimer = 0.0f;
    cycleDisappearingPlatform.isCycleDisappearing = true;
    cycleDisappearingPlatform.cycleTime = 4.0f;      // 4秒で1サイクル
    cycleDisappearingPlatform.visibleTime = 2.0f;    // 2秒見える、2秒消える（点滅時間含む）
    cycleDisappearingPlatform.cycleTimer = 0.0f;  // 常に見える状態で開始
    cycleDisappearingPlatform.isCurrentlyVisible = true;
    cycleDisappearingPlatform.originalSize = cycleDisappearingPlatform.size;
    cycleDisappearingPlatform.blinkTime = 1.0f;      // 1秒間点滅
    cycleDisappearingPlatform.blinkTimer = 0.0f;
    cycleDisappearingPlatform.isBlinking = false;
    cycleDisappearingPlatform.blinkAlpha = 1.0f;
    // 初期状態を明示的に設定
    cycleDisappearingPlatform.size = cycleDisappearingPlatform.originalSize;
    cycleDisappearingPlatform.isFlyingPlatform = false;
    gameState.platforms.push_back(cycleDisappearingPlatform);
    
    // 重力反転エリア内の足場（天井に配置）
    GameState::Platform invertedPlatform1;
    invertedPlatform1.position = glm::vec3(0, 12, -20);  // 重力反転エリアの天井付近
    invertedPlatform1.size = glm::vec3(3, 1, 3);
    invertedPlatform1.color = glm::vec3(0.2f, 0.8f, 0.2f); // 緑色
    invertedPlatform1.isMoving = false;
    invertedPlatform1.isDisappearing = false;
    invertedPlatform1.moveDirection = glm::vec3(0, 0, 0);
    invertedPlatform1.moveSpeed = 0.0f;
    invertedPlatform1.moveRange = 0.0f;
    invertedPlatform1.moveTimer = 0.0f;
    invertedPlatform1.disappearTimer = 0.0f;
    invertedPlatform1.isJumpPad = false;
    invertedPlatform1.isRotating = false;
    invertedPlatform1.rotationAxis = glm::vec3(0, 1, 0);
    invertedPlatform1.rotationSpeed = 0.0f;
    invertedPlatform1.rotationAngle = 0.0f;
    invertedPlatform1.isPatrolling = false;
    invertedPlatform1.patrolPoints = {};
    invertedPlatform1.currentPatrolIndex = 0;
    invertedPlatform1.patrolSpeed = 0.0f;
    invertedPlatform1.patrolTimer = 0.0f;
    invertedPlatform1.isCycleDisappearing = false;
    invertedPlatform1.cycleTime = 0.0f;
    invertedPlatform1.visibleTime = 0.0f;
    invertedPlatform1.cycleTimer = 0.0f;
    invertedPlatform1.isCurrentlyVisible = true;
    invertedPlatform1.originalSize = invertedPlatform1.size;
    invertedPlatform1.blinkTime = 0.0f;
    invertedPlatform1.blinkTimer = 0.0f;
    invertedPlatform1.isBlinking = false;
    invertedPlatform1.blinkAlpha = 1.0f;
    invertedPlatform1.isFlyingPlatform = false;
    gameState.platforms.push_back(invertedPlatform1);
    
    GameState::Platform invertedPlatform2;
    invertedPlatform2.position = glm::vec3(0, 15, -18);  // さらに上の足場
    invertedPlatform2.size = glm::vec3(2.5f, 1, 2.5f);
    invertedPlatform2.color = glm::vec3(0.8f, 0.2f, 0.8f); // 紫色
    invertedPlatform2.isMoving = false;
    invertedPlatform2.isDisappearing = false;
    invertedPlatform2.moveDirection = glm::vec3(0, 0, 0);
    invertedPlatform2.moveSpeed = 0.0f;
    invertedPlatform2.moveRange = 0.0f;
    invertedPlatform2.moveTimer = 0.0f;
    invertedPlatform2.disappearTimer = 0.0f;
    invertedPlatform2.isJumpPad = false;
    invertedPlatform2.isRotating = false;
    invertedPlatform2.rotationAxis = glm::vec3(0, 1, 0);
    invertedPlatform2.rotationSpeed = 0.0f;
    invertedPlatform2.rotationAngle = 0.0f;
    invertedPlatform2.isPatrolling = false;
    invertedPlatform2.patrolPoints = {};
    invertedPlatform2.currentPatrolIndex = 0;
    invertedPlatform2.patrolSpeed = 0.0f;
    invertedPlatform2.patrolTimer = 0.0f;
    invertedPlatform2.isCycleDisappearing = false;
    invertedPlatform2.cycleTime = 0.0f;
    invertedPlatform2.visibleTime = 0.0f;
    invertedPlatform2.cycleTimer = 0.0f;
    invertedPlatform2.isCurrentlyVisible = true;
    invertedPlatform2.originalSize = invertedPlatform2.size;
    invertedPlatform2.blinkTime = 0.0f;
    invertedPlatform2.blinkTimer = 0.0f;
    invertedPlatform2.isBlinking = false;
    invertedPlatform2.blinkAlpha = 1.0f;
    invertedPlatform2.isFlyingPlatform = false;
    gameState.platforms.push_back(invertedPlatform2);
    
    // プレイヤー接近で飛んでくる足場
    GameState::Platform flyingPlatform;
    flyingPlatform.position = glm::vec3(0, 15, -40);  // 初期位置（遠くの空）
    flyingPlatform.size = glm::vec3(3, 0.5, 3);
    flyingPlatform.color = glm::vec3(0.8f, 0.2f, 0.8f);  // 紫色
    flyingPlatform.isMoving = false;
    flyingPlatform.isDisappearing = false;
    flyingPlatform.isRotating = false;
    flyingPlatform.isPatrolling = false;
    flyingPlatform.isCycleDisappearing = false;
    flyingPlatform.isCurrentlyVisible = true;
    flyingPlatform.originalSize = flyingPlatform.size;
    flyingPlatform.blinkTime = 0.0f;
    flyingPlatform.blinkTimer = 0.0f;
    flyingPlatform.isBlinking = false;
    flyingPlatform.blinkAlpha = 1.0f;
    
    // 飛んでくる足場の設定
    flyingPlatform.isFlyingPlatform = true;
    flyingPlatform.spawnPosition = glm::vec3(20, 12, -25);  // 出現位置（スタート地点の左の空）
    flyingPlatform.targetPosition = glm::vec3(3.5, 5, -25);   // 目標位置（スタート地点の左）
    flyingPlatform.flySpeed = 80.0f;  // 飛行速度
    flyingPlatform.isFlying = false;
    flyingPlatform.hasSpawned = false;
    flyingPlatform.detectionRange = 3.0f;  // プレイヤー検知範囲（スタート地点の左端）
    flyingPlatform.isReturning = false;
    gameState.platforms.push_back(flyingPlatform);
    
    // 順次出現・消失する足場（一直線上に配置）
    for (int i = 0; i < 8; i++) {
        GameState::Platform cyclePlatform;
        cyclePlatform.position = glm::vec3(0, 5, 5 + i * 4); // 一直線上に4単位間隔で配置
        cyclePlatform.size = glm::vec3(3, 1, 3);
        cyclePlatform.color = glm::vec3(0.2f, 0.8f, 0.8f); // シアン色
        cyclePlatform.isMoving = false;
        cyclePlatform.isDisappearing = false;
        cyclePlatform.moveDirection = glm::vec3(0, 0, 0);
        cyclePlatform.moveSpeed = 0.0f;
        cyclePlatform.moveRange = 0.0f;
        cyclePlatform.moveTimer = 0.0f;
        cyclePlatform.disappearTimer = 0.0f;
        cyclePlatform.isJumpPad = false;
        cyclePlatform.isRotating = false;
        cyclePlatform.rotationAxis = glm::vec3(0, 1, 0);
        cyclePlatform.rotationSpeed = 0.0f;
        cyclePlatform.rotationAngle = 0.0f;
        cyclePlatform.isPatrolling = false;
        cyclePlatform.patrolPoints = {};
        cyclePlatform.currentPatrolIndex = 0;
        cyclePlatform.patrolSpeed = 0.0f;
        cyclePlatform.patrolTimer = 0.0f;
        cyclePlatform.isCycleDisappearing = true; // サイクル消失を有効化
        cyclePlatform.cycleTime = 8.0f; // 8秒周期（ゆっくり）
        cyclePlatform.visibleTime = 4.0f; // 4秒間表示
        cyclePlatform.cycleTimer = i * 1.0f; // 1秒ずつずらして開始（ピッ、ピッと順次）
        cyclePlatform.isCurrentlyVisible = false; // 初期状態は非表示
        cyclePlatform.originalSize = cyclePlatform.size;
        cyclePlatform.blinkTime = 0.5f; // 点滅時間を短く
        cyclePlatform.blinkTimer = 0.0f;
        cyclePlatform.isBlinking = false;
        cyclePlatform.blinkAlpha = 1.0f;
        cyclePlatform.isFlyingPlatform = false;
        gameState.platforms.push_back(cyclePlatform);
    }
    
    // テレポート足場を追加
    GameState::Platform teleportPlatform1;
    teleportPlatform1.position = glm::vec3(10, 5, -15); // テレポート足場の位置
    teleportPlatform1.size = glm::vec3(3, 1, 3);
    teleportPlatform1.color = glm::vec3(1.0f, 0.5f, 0.0f); // オレンジ色
    teleportPlatform1.isMoving = false;
    teleportPlatform1.isDisappearing = false;
    teleportPlatform1.moveDirection = glm::vec3(0, 0, 0);
    teleportPlatform1.moveSpeed = 0.0f;
    teleportPlatform1.moveRange = 0.0f;
    teleportPlatform1.moveTimer = 0.0f;
    teleportPlatform1.disappearTimer = 0.0f;
    teleportPlatform1.isJumpPad = false;
    teleportPlatform1.isRotating = false;
    teleportPlatform1.rotationAxis = glm::vec3(0, 1, 0);
    teleportPlatform1.rotationSpeed = 0.0f;
    teleportPlatform1.rotationAngle = 0.0f;
    teleportPlatform1.isPatrolling = false;
    teleportPlatform1.patrolPoints = {};
    teleportPlatform1.currentPatrolIndex = 0;
    teleportPlatform1.patrolSpeed = 0.0f;
    teleportPlatform1.patrolTimer = 0.0f;
    teleportPlatform1.isCycleDisappearing = false;
    teleportPlatform1.cycleTime = 0.0f;
    teleportPlatform1.visibleTime = 0.0f;
    teleportPlatform1.cycleTimer = 0.0f;
    teleportPlatform1.isCurrentlyVisible = true;
    teleportPlatform1.originalSize = teleportPlatform1.size;
    teleportPlatform1.blinkTime = 0.0f;
    teleportPlatform1.blinkTimer = 0.0f;
    teleportPlatform1.isBlinking = false;
    teleportPlatform1.blinkAlpha = 1.0f;
    teleportPlatform1.isFlyingPlatform = false;
    teleportPlatform1.isTeleportPlatform = true; // テレポート足場を有効化
    teleportPlatform1.teleportDestination = glm::vec3(0, 6.0f, -25.0f); // スタート地点にテレポート
    teleportPlatform1.hasTeleported = false;
    teleportPlatform1.teleportCooldown = 2.0f; // 2秒のクールダウン
    teleportPlatform1.teleportCooldownTimer = 0.0f;
    gameState.platforms.push_back(teleportPlatform1);
    
    // 2つ目のテレポート足場（別の場所からスタート地点へ）
    GameState::Platform teleportPlatform2;
    teleportPlatform2.position = glm::vec3(-10, 8, 10); // 別の場所の位置
    teleportPlatform2.size = glm::vec3(3, 1, 3);
    teleportPlatform2.color = glm::vec3(0.0f, 0.5f, 1.0f); // 青色
    teleportPlatform2.isMoving = false;
    teleportPlatform2.isDisappearing = false;
    teleportPlatform2.moveDirection = glm::vec3(0, 0, 0);
    teleportPlatform2.moveSpeed = 0.0f;
    teleportPlatform2.moveRange = 0.0f;
    teleportPlatform2.moveTimer = 0.0f;
    teleportPlatform2.disappearTimer = 0.0f;
    teleportPlatform2.isJumpPad = false;
    teleportPlatform2.isRotating = false;
    teleportPlatform2.rotationAxis = glm::vec3(0, 1, 0);
    teleportPlatform2.rotationSpeed = 0.0f;
    teleportPlatform2.rotationAngle = 0.0f;
    teleportPlatform2.isPatrolling = false;
    teleportPlatform2.patrolPoints = {};
    teleportPlatform2.currentPatrolIndex = 0;
    teleportPlatform2.patrolSpeed = 0.0f;
    teleportPlatform2.patrolTimer = 0.0f;
    teleportPlatform2.isCycleDisappearing = false;
    teleportPlatform2.cycleTime = 0.0f;
    teleportPlatform2.visibleTime = 0.0f;
    teleportPlatform2.cycleTimer = 0.0f;
    teleportPlatform2.isCurrentlyVisible = true;
    teleportPlatform2.originalSize = teleportPlatform2.size;
    teleportPlatform2.blinkTime = 0.0f;
    teleportPlatform2.blinkTimer = 0.0f;
    teleportPlatform2.isBlinking = false;
    teleportPlatform2.blinkAlpha = 1.0f;
    teleportPlatform2.isFlyingPlatform = false;
    teleportPlatform2.isTeleportPlatform = true; // テレポート足場を有効化
    teleportPlatform2.teleportDestination = glm::vec3(0, 6.0f, -25.0f); // スタート地点にテレポート
    teleportPlatform2.hasTeleported = false;
    teleportPlatform2.teleportCooldown = 2.0f; // 2秒のクールダウン
    teleportPlatform2.teleportCooldownTimer = 0.0f;
    gameState.platforms.push_back(teleportPlatform2);
    
    // 3つ目のテレポート足場（スタート地点から別の場所へ）
    GameState::Platform teleportPlatform3;
    teleportPlatform3.position = glm::vec3(5, 6.5, -25); // スタート地点の右側
    teleportPlatform3.size = glm::vec3(1.5, 0.5, 1.5);
    teleportPlatform3.color = glm::vec3(1.0f, 0.0f, 1.0f); // マゼンタ色
    teleportPlatform3.isMoving = false;
    teleportPlatform3.isDisappearing = false;
    teleportPlatform3.moveDirection = glm::vec3(0, 0, 0);
    teleportPlatform3.moveSpeed = 0.0f;
    teleportPlatform3.moveRange = 0.0f;
    teleportPlatform3.moveTimer = 0.0f;
    teleportPlatform3.disappearTimer = 0.0f;
    teleportPlatform3.isJumpPad = false;
    teleportPlatform3.isRotating = false;
    teleportPlatform3.rotationAxis = glm::vec3(0, 1, 0);
    teleportPlatform3.rotationSpeed = 0.0f;
    teleportPlatform3.rotationAngle = 0.0f;
    teleportPlatform3.isPatrolling = false;
    teleportPlatform3.patrolPoints = {};
    teleportPlatform3.currentPatrolIndex = 0;
    teleportPlatform3.patrolSpeed = 0.0f;
    teleportPlatform3.patrolTimer = 0.0f;
    teleportPlatform3.isCycleDisappearing = false;
    teleportPlatform3.cycleTime = 0.0f;
    teleportPlatform3.visibleTime = 0.0f;
    teleportPlatform3.cycleTimer = 0.0f;
    teleportPlatform3.isCurrentlyVisible = true;
    teleportPlatform3.originalSize = teleportPlatform3.size;
    teleportPlatform3.blinkTime = 0.0f;
    teleportPlatform3.blinkTimer = 0.0f;
    teleportPlatform3.isBlinking = false;
    teleportPlatform3.blinkAlpha = 1.0f;
    teleportPlatform3.isFlyingPlatform = false;
    teleportPlatform3.isTeleportPlatform = true; // テレポート足場を有効化
    teleportPlatform3.teleportDestination = glm::vec3(15, 5, -10); // 別の場所にテレポート
    teleportPlatform3.hasTeleported = false;
    teleportPlatform3.teleportCooldown = 2.0f; // 2秒のクールダウン
    teleportPlatform3.teleportCooldownTimer = 0.0f;
    gameState.platforms.push_back(teleportPlatform3);
    
    // 移動足場を追加
    GameState::Platform movingPlatform1;
    movingPlatform1.position = glm::vec3(-5, 5, -10); // 初期位置
    movingPlatform1.size = glm::vec3(3, 1, 3);
    movingPlatform1.color = glm::vec3(0.8f, 0.4f, 0.8f); // 紫がかった色
    movingPlatform1.isMoving = false;
    movingPlatform1.isDisappearing = false;
    movingPlatform1.moveDirection = glm::vec3(0, 0, 0);
    movingPlatform1.moveSpeed = 0.0f;
    movingPlatform1.moveRange = 0.0f;
    movingPlatform1.moveTimer = 0.0f;
    movingPlatform1.disappearTimer = 0.0f;
    movingPlatform1.isJumpPad = false;
    movingPlatform1.isRotating = false;
    movingPlatform1.rotationAxis = glm::vec3(0, 1, 0);
    movingPlatform1.rotationSpeed = 0.0f;
    movingPlatform1.rotationAngle = 0.0f;
    movingPlatform1.isPatrolling = false;
    movingPlatform1.patrolPoints = {};
    movingPlatform1.currentPatrolIndex = 0;
    movingPlatform1.patrolSpeed = 0.0f;
    movingPlatform1.patrolTimer = 0.0f;
    movingPlatform1.isCycleDisappearing = false;
    movingPlatform1.cycleTime = 0.0f;
    movingPlatform1.visibleTime = 0.0f;
    movingPlatform1.cycleTimer = 0.0f;
    movingPlatform1.isCurrentlyVisible = true;
    movingPlatform1.originalSize = movingPlatform1.size;
    movingPlatform1.blinkTime = 0.0f;
    movingPlatform1.blinkTimer = 0.0f;
    movingPlatform1.isBlinking = false;
    movingPlatform1.blinkAlpha = 1.0f;
    movingPlatform1.isFlyingPlatform = false;
    movingPlatform1.isTeleportPlatform = false;
    movingPlatform1.teleportDestination = glm::vec3(0, 0, 0);
    movingPlatform1.hasTeleported = false;
    movingPlatform1.teleportCooldown = 0.0f;
    movingPlatform1.teleportCooldownTimer = 0.0f;
    movingPlatform1.isMovingPlatform = true; // 移動足場を有効化
    movingPlatform1.moveTargetPosition = glm::vec3(5, 8, -10); // 移動先（高い位置）
    movingPlatform1.moveSpeed = 2.0f; // 移動速度
    movingPlatform1.isMoving = false;
    movingPlatform1.hasPlayerOnBoard = false;
    movingPlatform1.originalPosition = glm::vec3(-5, 5, -10); // 元の位置を明示的に設定
    movingPlatform1.returnToOriginal = false;
    movingPlatform1.moveTimer = 0.0f; // 移動タイマーを初期化
    gameState.platforms.push_back(movingPlatform1);
    
    // デバッグ情報：移動足場1の生成確認
    printf("Moving platform 1 created: Position(%.1f, %.1f, %.1f) Target(%.1f, %.1f, %.1f) Speed: %.1f\n", 
           movingPlatform1.position.x, movingPlatform1.position.y, movingPlatform1.position.z,
           movingPlatform1.moveTargetPosition.x, movingPlatform1.moveTargetPosition.y, movingPlatform1.moveTargetPosition.z,
           movingPlatform1.moveSpeed);
    
    // 2つ目の移動足場（往復移動）
    GameState::Platform movingPlatform2;
    movingPlatform2.position = glm::vec3(15, 5, -5); // 初期位置
    movingPlatform2.size = glm::vec3(3, 1, 3);
    movingPlatform2.color = glm::vec3(0.4f, 0.8f, 0.4f); // 緑がかった色
    movingPlatform2.isMoving = false;
    movingPlatform2.isDisappearing = false;
    movingPlatform2.moveDirection = glm::vec3(0, 0, 0);
    movingPlatform2.moveSpeed = 2.0f; // 移動速度を修正
    movingPlatform2.moveRange = 0.0f;
    movingPlatform2.moveTimer = 0.0f;
    movingPlatform2.disappearTimer = 0.0f;
    movingPlatform2.isJumpPad = false;
    movingPlatform2.isRotating = false;
    movingPlatform2.rotationAxis = glm::vec3(0, 1, 0);
    movingPlatform2.rotationSpeed = 0.0f;
    movingPlatform2.rotationAngle = 0.0f;
    movingPlatform2.isPatrolling = false;
    movingPlatform2.patrolPoints = {};
    movingPlatform2.currentPatrolIndex = 0;
    movingPlatform2.patrolSpeed = 0.0f;
    movingPlatform2.patrolTimer = 0.0f;
    movingPlatform2.isCycleDisappearing = false;
    movingPlatform2.cycleTime = 0.0f;
    movingPlatform2.visibleTime = 0.0f;
    movingPlatform2.cycleTimer = 0.0f;
    movingPlatform2.isCurrentlyVisible = true;
    movingPlatform2.originalSize = movingPlatform2.size;
    movingPlatform2.blinkTime = 0.0f;
    movingPlatform2.blinkTimer = 0.0f;
    movingPlatform2.isBlinking = false;
    movingPlatform2.blinkAlpha = 1.0f;
    movingPlatform2.isFlyingPlatform = false;
    movingPlatform2.isTeleportPlatform = false;
    movingPlatform2.teleportDestination = glm::vec3(0, 0, 0);
    movingPlatform2.hasTeleported = false;
    movingPlatform2.teleportCooldown = 0.0f;
    movingPlatform2.teleportCooldownTimer = 0.0f;
    movingPlatform2.isMovingPlatform = true; // 移動足場を有効化
    movingPlatform2.moveTargetPosition = glm::vec3(25, 5, -5); // 移動先（右側）
    movingPlatform2.moveSpeed = 2.0f; // 移動速度
    movingPlatform2.isMoving = false;
    movingPlatform2.hasPlayerOnBoard = false;
    movingPlatform2.originalPosition = glm::vec3(15, 5, -5); // 元の位置を明示的に設定
    movingPlatform2.returnToOriginal = false;
    movingPlatform2.moveTimer = 0.0f; // 移動タイマーを初期化
    gameState.platforms.push_back(movingPlatform2);
    
    // デバッグ情報：移動足場2の生成確認
    printf("Moving platform 2 created: Position(%.1f, %.1f, %.1f) Target(%.1f, %.1f, %.1f) Speed: %.1f\n", 
           movingPlatform2.position.x, movingPlatform2.position.y, movingPlatform2.position.z,
           movingPlatform2.moveTargetPosition.x, movingPlatform2.moveTargetPosition.y, movingPlatform2.moveTargetPosition.z,
           movingPlatform2.moveSpeed);
}

void generateGravityZones(GameState& gameState) {
    gameState.gravityZones.clear();
    
    // 重力反転エリア（プレイヤーの初期地点の目の前）
    GameState::GravityZone gravityZone;
    gravityZone.position = glm::vec3(0, 8, -20);  // 青いブロックの位置付近
    gravityZone.size = glm::vec3(6, 2, 6);        // エリアのサイズ
    gravityZone.gravityDirection = glm::vec3(0, 1, 0);  // 上向きの重力
    gravityZone.radius = 3.0f;                    // 影響範囲
    gravityZone.isActive = true;
    gameState.gravityZones.push_back(gravityZone);
}

void generateCannons(GameState& gameState) {
    gameState.cannons.clear();
    
    // 大砲1（スタート地点から遠くの足場へ）
    GameState::Cannon cannon1;
    cannon1.position = glm::vec3(5, 5, -15);
    cannon1.size = glm::vec3(2, 2, 2);
    cannon1.color = glm::vec3(0.8f, 0.4f, 0.2f); // オレンジ色
    cannon1.targetPosition = glm::vec3(20, 8, -5);
    cannon1.power = 15.0f;
    cannon1.isActive = true;
    cannon1.hasPlayerInside = false;
    cannon1.cooldownTimer = 0.0f;
    cannon1.cooldownTime = 2.0f; // 2秒のクールダウン
    
    // 発射方向を計算
    glm::vec3 direction = glm::normalize(cannon1.targetPosition - cannon1.position);
    float distance = glm::length(cannon1.targetPosition - cannon1.position);
    
    // 物理計算で必要な初速度を計算（重力を考慮）
    float gravity = 9.8f;
    float timeToTarget = sqrt(2.0f * distance / gravity);
    cannon1.launchDirection = direction * cannon1.power;
    cannon1.launchDirection.y = (cannon1.targetPosition.y - cannon1.position.y) / timeToTarget + 0.5f * gravity * timeToTarget;
    
    gameState.cannons.push_back(cannon1);
    
    // 大砲2（重力反転エリアから高い足場へ）
    GameState::Cannon cannon2;
    cannon2.position = glm::vec3(-10, 10, -10);
    cannon2.size = glm::vec3(2, 2, 2);
    cannon2.color = glm::vec3(0.2f, 0.6f, 0.8f); // 青色
    cannon2.targetPosition = glm::vec3(0, 15, 0);
    cannon2.power = 12.0f;
    cannon2.isActive = true;
    cannon2.hasPlayerInside = false;
    cannon2.cooldownTimer = 0.0f;
    cannon2.cooldownTime = 3.0f; // 3秒のクールダウン
    
    // 発射方向を計算
    direction = glm::normalize(cannon2.targetPosition - cannon2.position);
    distance = glm::length(cannon2.targetPosition - cannon2.position);
    timeToTarget = sqrt(2.0f * distance / gravity);
    cannon2.launchDirection = direction * cannon2.power;
    cannon2.launchDirection.y = (cannon2.targetPosition.y - cannon2.position.y) / timeToTarget + 0.5f * gravity * timeToTarget;
    
    gameState.cannons.push_back(cannon2);
    
    printf("Cannons generated: %zu cannons\n", gameState.cannons.size());
}

void generateSwitches(GameState& gameState) {
    gameState.switches.clear();
    
    // 単体スイッチ1（赤いスイッチ）- スタート足場の上
    GameState::Switch singleSwitch1;
    singleSwitch1.position = glm::vec3(-1, 6.5, -25); // スタート足場(0, 5, -25)の上
    singleSwitch1.size = glm::vec3(1, 0.5, 1);
    singleSwitch1.color = glm::vec3(1.0f, 0.2f, 0.2f); // 赤色
    singleSwitch1.isPressed = false;
    singleSwitch1.isToggle = true;
    singleSwitch1.targetPlatformIndices = {0}; // スタート足場を操作
    singleSwitch1.targetStates = {true}; // 出現させる
    singleSwitch1.pressTimer = 0.0f;
    singleSwitch1.cooldownTimer = 0.0f;
    singleSwitch1.isMultiSwitch = false;
    singleSwitch1.multiSwitchGroup = -1;
    gameState.switches.push_back(singleSwitch1);
    
    // 単体スイッチ2（青いスイッチ）- 回転足場の上
    GameState::Switch singleSwitch2;
    singleSwitch2.position = glm::vec3(-8, 6.5, -25); // 回転足場(-8, 5, -25)の上
    singleSwitch2.size = glm::vec3(1, 0.5, 1);
    singleSwitch2.color = glm::vec3(0.2f, 0.2f, 1.0f); // 青色
    singleSwitch2.isPressed = false;
    singleSwitch2.isToggle = true;
    singleSwitch2.targetPlatformIndices = {1}; // 回転足場を操作
    singleSwitch2.targetStates = {true}; // 出現させる
    singleSwitch2.pressTimer = 0.0f;
    singleSwitch2.cooldownTimer = 0.0f;
    singleSwitch2.isMultiSwitch = false;
    singleSwitch2.multiSwitchGroup = -1;
    gameState.switches.push_back(singleSwitch2);
    
    // 複数スイッチ1（緑のスイッチ）- platform2の上
    GameState::Switch multiSwitch1;
    multiSwitch1.position = glm::vec3(-1, 6.5, -15); // platform2(0, 5, -15)の上
    multiSwitch1.size = glm::vec3(1, 0.5, 1);
    multiSwitch1.color = glm::vec3(0.2f, 1.0f, 0.2f); // 緑色
    multiSwitch1.isPressed = false;
    multiSwitch1.isToggle = false;
    multiSwitch1.targetPlatformIndices = {}; // 複数スイッチなので空
    multiSwitch1.targetStates = {};
    multiSwitch1.pressTimer = 0.0f;
    multiSwitch1.cooldownTimer = 0.0f;
    multiSwitch1.isMultiSwitch = true;
    multiSwitch1.multiSwitchGroup = 0;
    gameState.switches.push_back(multiSwitch1);
    
    // 複数スイッチ2（黄色のスイッチ）- platform3の上
    GameState::Switch multiSwitch2;
    multiSwitch2.position = glm::vec3(-1, 8.5, -10); // platform3(0, 7, -10)の上
    multiSwitch2.size = glm::vec3(1, 0.5, 1);
    multiSwitch2.color = glm::vec3(1.0f, 1.0f, 0.2f); // 黄色
    multiSwitch2.isPressed = false;
    multiSwitch2.isToggle = false;
    multiSwitch2.targetPlatformIndices = {}; // 複数スイッチなので空
    multiSwitch2.targetStates = {};
    multiSwitch2.pressTimer = 0.0f;
    multiSwitch2.cooldownTimer = 0.0f;
    multiSwitch2.isMultiSwitch = true;
    multiSwitch2.multiSwitchGroup = 0;
    gameState.switches.push_back(multiSwitch2);
}

void generateTerrain(GameState& gameState) {
    // 空中アクション用の足場を生成
    generatePlatforms(gameState);
    // 重力反転エリアを生成
    generateGravityZones(gameState);
    // スイッチを生成
    generateSwitches(gameState);
    // 大砲を生成
    generateCannons(gameState);
}

// ------------------------------------------------------
// 空中アクション用の足場更新
// ------------------------------------------------------
void updatePlatforms(GameState& gameState, float deltaTime) {
    for (auto& platform : gameState.platforms) {
        // 移動する足場の更新
        if (platform.isMoving) {
            platform.moveTimer += deltaTime;
            
            // 往復移動（線形）
            float cycle = platform.moveTimer * platform.moveSpeed;
            float normalizedPosition = std::fmod(cycle, 4.0f); // 4秒周期
            
            float moveOffset;
            if (normalizedPosition <= 1.0f) {
                // 0→1: 正方向へ移動
                moveOffset = normalizedPosition * platform.moveRange;
            } else if (normalizedPosition <= 2.0f) {
                // 1→2: 正方向の最大位置で停止
                moveOffset = platform.moveRange;
            } else if (normalizedPosition <= 3.0f) {
                // 2→3: 負方向へ移動
                moveOffset = platform.moveRange * (3.0f - normalizedPosition);
            } else {
                // 3→4: 負方向の最大位置で停止
                moveOffset = 0.0f;
            }
            
            // 初期位置からのオフセットを計算
            static std::vector<glm::vec3> initialPositions;
            if (initialPositions.empty()) {
                // 初期位置を記録（最初の1回のみ）
                for (const auto& p : gameState.platforms) {
                    initialPositions.push_back(p.position);
                }
            }
            
            // 対応する初期位置を取得
            size_t platformIndex = &platform - &gameState.platforms[0];
            if (platformIndex < initialPositions.size()) {
                glm::vec3 initialPos = initialPositions[platformIndex];
                platform.position = initialPos + platform.moveDirection * moveOffset;
            }
        }
        
        // 消える足場の更新
        if (platform.isDisappearing) {
            platform.disappearTimer += deltaTime;
            if (platform.disappearTimer > 3.0f) { // 3秒後に消える
                platform.size = glm::vec3(0, 0, 0); // サイズを0にして見えなくする
            }
        }
        
        // 回転する足場の更新
        if (platform.isRotating) {
            platform.rotationAngle += platform.rotationSpeed * deltaTime;
            // 360度を超えたら0度に戻す
            if (platform.rotationAngle >= 360.0f) {
                platform.rotationAngle -= 360.0f;
            }
        }
        
        // 巡回する足場の更新
        if (platform.isPatrolling && !platform.patrolPoints.empty()) {
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
        
        // 周期的に消える足場の更新
        if (platform.isCycleDisappearing) {
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
                // 元のサイズに戻す
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
                    // 見える時間（0.1秒）
                    platform.blinkAlpha = 1.0f;
                } else {
                    // 薄くなる時間（0.1秒）
                    platform.blinkAlpha = 0.3f;
                }
                
                // 元のサイズを維持（当たり判定は残す）
                platform.size = platform.originalSize;
            } else {
                // 完全に消える時間
                platform.isCurrentlyVisible = false;
                platform.isBlinking = false;
                platform.blinkAlpha = 0.0f;
                // サイズを0にして見えなくする
                platform.size = glm::vec3(0, 0, 0);
            }
        }
        
        // 移動足場の更新（滑らかな移動）
        if (platform.isMovingPlatform) {
            // デバッグ情報：移動足場の状態確認（頻度を下げる）
            static int debugCounter = 0;
            debugCounter++;
            if (debugCounter % 120 == 0) { // 2秒に1回程度
                printf("Moving platform debug: isMoving=%d, hasPlayerOnBoard=%d, Position(%.1f, %.1f, %.1f)\n", 
                       platform.isMoving, platform.hasPlayerOnBoard,
                       platform.position.x, platform.position.y, platform.position.z);
            }
            
            if (platform.isMoving) {
                // 移動タイマーの更新
                platform.moveTimer += deltaTime;
                
                // 目標位置と開始位置を決定（片道移動）
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
                    
                    // 移動停止（往復移動なし）
                    platform.isMoving = false;
                } else {
                    // 2点間を線形補間で滑らかに移動
                    platform.position = glm::mix(startPos, targetPos, t);
                }
            }
        }
        
        // プレイヤー接近で飛んでくる足場の更新
        if (platform.isFlyingPlatform) {
            // プレイヤーと目標位置との距離を計算（目標位置近くで検知）
            float distanceToTarget = glm::length(platform.targetPosition - gameState.playerPosition);
            
            if (!platform.hasSpawned && distanceToTarget <= platform.detectionRange) {
                // プレイヤーが検知範囲内に入ったら出現開始
                platform.hasSpawned = true;
                platform.isFlying = true;
                platform.isReturning = false;
                platform.position = platform.spawnPosition;  // 出現位置に配置
                platform.isCurrentlyVisible = true;
                platform.size = platform.originalSize;
            } else if (platform.hasSpawned && !platform.isFlying && !platform.isReturning && distanceToTarget > platform.detectionRange) {
                // プレイヤーが検知範囲から出たら戻り開始
                platform.isReturning = true;
                platform.isFlying = true;
            }
            
            if (platform.isFlying) {
                glm::vec3 targetPos;
                if (platform.isReturning) {
                    // 元の出現位置に向かって戻る
                    targetPos = platform.spawnPosition;
                } else {
                    // 目標位置に向かって飛行
                    targetPos = platform.targetPosition;
                }
                
                glm::vec3 direction = glm::normalize(targetPos - platform.position);
                float distanceToTarget = glm::length(targetPos - platform.position);
                
                if (distanceToTarget > 0.5f) {
                    // まだ目標に到達していない場合
                    platform.position += direction * platform.flySpeed * deltaTime;
                } else {
                    // 目標に到達した場合
                    platform.isFlying = false;
                    platform.position = targetPos;
                    
                    if (platform.isReturning) {
                        // 戻り完了 - 初期状態にリセット
                        platform.isReturning = false;
                        platform.hasSpawned = false;
                        platform.isCurrentlyVisible = false;
                        platform.size = glm::vec3(0, 0, 0);  // 非表示
                    }
                }
            }
        }
        
        // テレポート足場の更新
        if (platform.isTeleportPlatform) {
            // クールダウンタイマーの更新
            if (platform.teleportCooldownTimer > 0.0f) {
                platform.teleportCooldownTimer -= deltaTime;
                
                // テレポート中は点滅効果
                platform.isBlinking = true;
                platform.blinkTimer += deltaTime;
                
                // 点滅の透明度を計算（0.3秒周期で点滅）
                float blinkCycle = 0.3f;
                float blinkProgress = std::fmod(platform.blinkTimer, blinkCycle);
                if (blinkProgress < blinkCycle * 0.5f) {
                    platform.blinkAlpha = 1.0f;
                } else {
                    platform.blinkAlpha = 0.3f;
                }
            } else {
                // クールダウン終了時
                platform.hasTeleported = false;
                platform.isBlinking = false;
                platform.blinkAlpha = 1.0f;
            }
        }
    }
}

// ------------------------------------------------------
// 重力反転エリアの更新
// ------------------------------------------------------
void updateGravityZones(GameState& gameState, float deltaTime) {
    // 重力反転エリアの更新（現在は静的だが、将来的に動的に変更可能）
    for (auto& zone : gameState.gravityZones) {
        // 将来的に重力方向を動的に変更する場合はここに実装
        // 例: 時間経過で重力方向が変わる、スイッチで切り替わるなど
    }
}

void updateCannons(GameState& gameState, float deltaTime) {
    for (auto& cannon : gameState.cannons) {
        // クールダウンタイマーの更新
        if (cannon.cooldownTimer > 0.0f) {
            cannon.cooldownTimer -= deltaTime;
        }
    }
}

void updateSwitches(GameState& gameState, float deltaTime) {
    // スイッチの更新
    for (auto& switch_obj : gameState.switches) {
        // クールダウンタイマーの更新
        if (switch_obj.cooldownTimer > 0.0f) {
            switch_obj.cooldownTimer -= deltaTime;
        }
        
        // 複数スイッチの処理
        if (switch_obj.isMultiSwitch) {
            // 同じグループのスイッチが全て押されているかチェック
            bool allPressed = true;
            for (const auto& otherSwitch : gameState.switches) {
                if (otherSwitch.isMultiSwitch && otherSwitch.multiSwitchGroup == switch_obj.multiSwitchGroup) {
                    if (!otherSwitch.isPressed) {
                        allPressed = false;
                        break;
                    }
                }
            }
            
            // 全て押されている場合、特別な足場を出現させる
            if (allPressed) {
                // 複数スイッチ用の特別な足場を出現（例：ゴールへの近道）
                // ここでは例として、新しい足場を動的に追加
            }
        }
    }
}

// ------------------------------------------------------
// 重力反転エリア内にいるかチェック
// ------------------------------------------------------
bool isPlayerInGravityZone(const GameState& gameState, const glm::vec3& playerPos, glm::vec3& gravityDirection) {
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

// スイッチとの衝突判定
// ------------------------------------------------------
bool checkSwitchCollision(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (auto& switch_obj : gameState.switches) {
        if (switch_obj.cooldownTimer > 0.0f) continue; // クールダウン中は無視
        
        glm::vec3 switchMin = switch_obj.position - switch_obj.size * 0.5f;
        glm::vec3 switchMax = switch_obj.position + switch_obj.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        // 衝突判定
        if (playerMax.x >= switchMin.x && playerMin.x <= switchMax.x &&
            playerMax.y >= switchMin.y && playerMin.y <= switchMax.y &&
            playerMax.z >= switchMin.z && playerMin.z <= switchMax.z) {
            
            // スイッチを押す
            if (!switch_obj.isPressed) {
                switch_obj.isPressed = true;
                switch_obj.pressTimer = 0.0f;
                switch_obj.cooldownTimer = 0.5f; // 0.5秒のクールダウン
                
                // 単体スイッチの処理
                if (!switch_obj.isMultiSwitch) {
                    for (size_t i = 0; i < switch_obj.targetPlatformIndices.size(); i++) {
                        int platformIndex = switch_obj.targetPlatformIndices[i];
                        if (platformIndex >= 0 && platformIndex < gameState.platforms.size()) {
                            bool targetState = switch_obj.targetStates[i];
                            if (switch_obj.isToggle) {
                                // トグルスイッチ：現在の状態を反転
                                if (targetState) {
                                    // 足場を出現させる
                                    gameState.platforms[platformIndex].size = gameState.platforms[platformIndex].originalSize;
                                } else {
                                    // 足場を消失させる
                                    gameState.platforms[platformIndex].size = glm::vec3(0, 0, 0);
                                }
                                // 目標状態を反転
                                switch_obj.targetStates[i] = !targetState;
                            } else {
                                // 通常スイッチ：指定された状態に設定
                                if (targetState) {
                                    gameState.platforms[platformIndex].size = gameState.platforms[platformIndex].originalSize;
                                } else {
                                    gameState.platforms[platformIndex].size = glm::vec3(0, 0, 0);
                                }
                            }
                        }
                    }
                }
            }
            return true;
        }
    }
    return false;
}

// テレポート足場との衝突判定
// ------------------------------------------------------
bool checkTeleportPlatformCollision(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (auto& platform : gameState.platforms) {
        if (!platform.isTeleportPlatform) continue;
        if (platform.teleportCooldownTimer > 0.0f) continue; // クールダウン中は無視
        
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        // 衝突判定
        if (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
            playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
            playerMax.z >= platformMin.z && playerMin.z <= platformMax.z) {
            
            // テレポート実行
            if (!platform.hasTeleported) {
                // デバッグ情報：どの足場がテレポートを実行したかを表示
                printf("Teleport triggered! From: (%.1f, %.1f, %.1f) To: (%.1f, %.1f, %.1f)\n", 
                       platform.position.x, platform.position.y, platform.position.z,
                       platform.teleportDestination.x, platform.teleportDestination.y, platform.teleportDestination.z);
                
                // プレイヤーをテレポート先に移動
                gameState.playerPosition = platform.teleportDestination;
                gameState.playerVelocity = glm::vec3(0, 0, 0); // 速度をリセット
                
                // テレポート状態を設定
                platform.hasTeleported = true;
                platform.teleportCooldownTimer = platform.teleportCooldown;
                
                return true;
            }
        }
    }
    return false;
}

// 大砲との衝突判定
// ------------------------------------------------------
void checkCannonCollision(GameState& gameState, const glm::vec3& playerPosition, const glm::vec3& playerSize) {
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
                printf("Player entered cannon! Launching to (%.1f, %.1f, %.1f)\n", 
                       cannon.targetPosition.x, cannon.targetPosition.y, cannon.targetPosition.z);
                
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

// 移動足場との衝突判定
// ------------------------------------------------------
bool checkMovingPlatformCollision(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (auto& platform : gameState.platforms) {
        if (!platform.isMovingPlatform) continue;
        
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        // 衝突判定
        if (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
            playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
            playerMax.z >= platformMin.z && playerMin.z <= platformMax.z) {
            
            // プレイヤーが移動足場に乗った
            if (!platform.hasPlayerOnBoard) {
                platform.hasPlayerOnBoard = true;
                platform.isMoving = true; // 移動開始
                platform.moveTimer = 0.0f; // 移動タイマーをリセット
                
                // デバッグ情報
                printf("Moving platform activated! Position: (%.1f, %.1f, %.1f) Target: (%.1f, %.1f, %.1f) Speed: %.1f\n", 
                       platform.position.x, platform.position.y, platform.position.z,
                       platform.moveTargetPosition.x, platform.moveTargetPosition.y, platform.moveTargetPosition.z,
                       platform.moveSpeed);
            }
            return true;
        } else {
            // プレイヤーが移動足場から離れた
            if (platform.hasPlayerOnBoard) {
                platform.hasPlayerOnBoard = false;
                platform.isMoving = false; // 移動停止
                printf("Moving platform deactivated! Position: (%.1f, %.1f, %.1f)\n", 
                       platform.position.x, platform.position.y, platform.position.z);
            }
        }
    }
    return false;
}

// ------------------------------------------------------
// 回転した足場の当たり判定用ヘルパー関数
// ------------------------------------------------------
glm::vec3 rotatePointAroundAxis(const glm::vec3& point, const glm::vec3& axis, float angle, const glm::vec3& center) {
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
void getRotatedPlatformCorners(const GameState::Platform& platform, glm::vec3 corners[8]) {
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
void getRotatedPlatformTopCorners(const GameState::Platform& platform, glm::vec3 topCorners[4]) {
    glm::vec3 corners[8];
    getRotatedPlatformCorners(platform, corners);
    
    // 上面の4つの角（Y座標が大きい方）
    topCorners[0] = corners[3]; // 左後上
    topCorners[1] = corners[2]; // 右後上
    topCorners[2] = corners[6]; // 右前上
    topCorners[3] = corners[7]; // 左前上
}

// 回転した足場の下面の4つの角を取得
void getRotatedPlatformBottomCorners(const GameState::Platform& platform, glm::vec3 bottomCorners[4]) {
    glm::vec3 corners[8];
    getRotatedPlatformCorners(platform, corners);
    
    // 下面の4つの角（Y座標が小さい方）
    bottomCorners[0] = corners[0]; // 左後下
    bottomCorners[1] = corners[1]; // 右後下
    bottomCorners[2] = corners[5]; // 右前下
    bottomCorners[3] = corners[4]; // 左前下
}

// プレイヤーが回転した足場の上にいるかをチェック
bool isPlayerOnRotatedPlatform(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize) {
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

// ------------------------------------------------------
// 空中アクション用の足場衝突判定（水平移動用）
// ------------------------------------------------------
bool checkPlatformCollisionHorizontal(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (const auto& platform : gameState.platforms) {
        // プラットフォームが存在するかチェック（消えた足場は無視）
        if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
        
        // 水平方向の衝突判定のみ（Y軸は少し余裕を持たせる）
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        // プレイヤーが足場の側面高さ範囲にいる場合のみ水平衝突をチェック（上面は除外）
        bool nearTopSurface = std::abs(playerMin.y - platformMax.y) < 0.15f;
        if (!nearTopSurface && playerMax.y > platformMin.y - 0.1f && playerMin.y < platformMax.y + 0.1f) {
            if (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                playerMax.z >= platformMin.z && playerMin.z <= platformMax.z) {
                return true;
            }
        }
    }
    return false;
}

// 重力方向を考慮した足場衝突判定（前方宣言）
bool isPlayerOnPlatformWithGravity(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection);

// 重力方向を考慮した足場判定（移動用）
bool isPlayerOnPlatformWithGravityForMovement(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection);

// 重力方向を考慮した足場衝突をチェック（回転対応）
GameState::Platform* checkPlatformCollisionWithGravity(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection) {
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

// 重力方向を考慮した足場衝突判定
bool isPlayerOnPlatformWithGravity(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection) {
    // 重力方向が上向き（反転）の場合
    if (gravityDirection.y > 0.5f) {
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
        if (isPlayerOnRotatedPlatform(platform, playerPos, playerSize)) {
            return true;
        }
    }
    return false;
}

// 重力方向を考慮した足場判定（移動用）
bool isPlayerOnPlatformWithGravityForMovement(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection) {
    // 重力方向が上向き（反転）の場合
    if (gravityDirection.y > 0.5f) {
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

// 垂直方向の足場衝突をチェック（回転対応）
GameState::Platform* checkPlatformCollisionVertical(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
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

// プレイヤーが足場の上に立っているかチェック（ジャンプ判定用）
bool isPlayerOnPlatform(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
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

// ------------------------------------------------------
// 敵を初期化
// ------------------------------------------------------
void initializeEnemies(GameState& gameState) {
    // 空中アクション用の敵配置
    Enemy waddleDee1;
    waddleDee1.position = glm::vec3(3, 8.0f, -15);
    waddleDee1.velocity = glm::vec3(0, 0, 0);
    waddleDee1.type = EnemyType::WaddleDee;
    waddleDee1.ability = KirbyAbility::None;
    waddleDee1.isAlive = true;
    waddleDee1.health = 1.0f;
    waddleDee1.size = 0.5f;
    gameState.enemies.push_back(waddleDee1);

    Enemy waddleDoo;
    waddleDoo.position = glm::vec3(-2, 10.0f, -5);
    waddleDoo.velocity = glm::vec3(0, 0, 0);
    waddleDoo.type = EnemyType::WaddleDoo;
    waddleDoo.ability = KirbyAbility::Fire;
    waddleDoo.isAlive = true;
    waddleDoo.health = 1.0f;
    waddleDoo.size = 0.6f;
    gameState.enemies.push_back(waddleDoo);

    Enemy brontoBurt;
    brontoBurt.position = glm::vec3(0, 15.0f, 15);
    brontoBurt.velocity = glm::vec3(0, 0, 0);
    brontoBurt.type = EnemyType::BrontoBurt;
    brontoBurt.ability = KirbyAbility::None;
    brontoBurt.isAlive = true;
    brontoBurt.health = 1.0f;
    brontoBurt.size = 0.4f;
    gameState.enemies.push_back(brontoBurt);
}

// ======================================================
//                        main
// ======================================================
int main() {
    
    // GLFW初期化
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // OpenGL 2.1設定
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    
    // ウィンドウ作成
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Vulkan3D - Portal Physics Puzzle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // OpenGLレンダラー初期化
    auto renderer = std::make_unique<gfx::OpenGLRenderer>();
    if (!renderer->initialize(window)) {
        std::cerr << "Failed to initialize OpenGL renderer" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // ゲーム状態
    GameState gameState;
    glfwSetWindowUserPointer(window, &gameState);
    
    // （必要なら）マウス設定
    // glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetScrollCallback(window, scroll_callback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 空中アクション用の足場を生成
    generateTerrain(gameState);

    // 空中アクション用のプレイヤーとゴール位置設定
    gameState.playerPosition = glm::vec3(0, 6.0f, -25.0f);  // スタート足場の上
    gameState.goalPosition   = glm::vec3(0, 16.0f, 25.0f);  // ゴール足場の上

    // プレイヤーとゴール配置完了

    // 敵を初期化（一時的にコメントアウト）
    // initializeEnemies(gameState);

    // ゲーム開始準備完了
    
    bool gameRunning = true;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastFrameTime = startTime;
    
    // --------------------------
    //         ゲームループ
    // --------------------------
    while (!glfwWindowShouldClose(window) && gameRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
        
        // deltaTimeの異常値を制限（フレームレート低下時の問題を防ぐ）
        deltaTime = std::min(deltaTime, 0.1f); // 最大100ms（10FPS相当）
        
        lastFrameTime = currentTime;
        
        gameState.gameTime = std::chrono::duration<float>(currentTime - startTime).count();
        
        // 足場更新システム
        updatePlatforms(gameState, deltaTime);
        // 重力反転エリア更新システム
        updateGravityZones(gameState, deltaTime);
        // スイッチ更新システム
        updateSwitches(gameState, deltaTime);
        // 大砲更新システム
        updateCannons(gameState, deltaTime);
        
        // 入力処理
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            gameRunning = false;
        }
        

        
        // ふんわり物理
        float moveSpeed = 6.0f;
        float airResistance = 0.98f;
        
        // 重力反転エリアのチェック
        glm::vec3 gravityDirection = glm::vec3(0, -1, 0); // デフォルトは下向き
        bool inGravityZone = isPlayerInGravityZone(gameState, gameState.playerPosition, gravityDirection);
        
        // 重力の適用
        float gravityStrength = 8.0f * deltaTime;
        // 重力反転時は少し弱くする
        if (gravityDirection.y > 0.5f) {
            gravityStrength *= 0.7f; // 重力反転時は70%の強度
        }
        glm::vec3 gravityForce = gravityDirection * gravityStrength;
        
        if (!gameState.isFloating) {
            gameState.playerVelocity += gravityForce;
            gameState.playerVelocity *= airResistance;
        }

        // 移動入力
        glm::vec3 moveDir(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir.z += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir.z -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir.x += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir.x -= 1.0f;

        if (glm::length(moveDir) > 0.0f) {
            moveDir = glm::normalize(moveDir);
            
            // 移動量を計算
            float moveDistance = moveSpeed * deltaTime;
            glm::vec3 newPosition = gameState.playerPosition;
            newPosition.x += moveDir.x * moveDistance;
            newPosition.z += moveDir.z * moveDistance;
            
            // 重力反転時は水平移動の制限を緩和
            if (gravityDirection.y > 0.5f) {
                // 重力反転時：水平移動を自由に許可
                gameState.playerPosition = newPosition;
            } else {
                // 通常時：水平移動での足場衝突をチェック
                glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
                if (!checkPlatformCollisionHorizontal(gameState, newPosition, playerSize)) {
                    gameState.playerPosition = newPosition;
                }
            }
        }

        // ジャンプと浮遊
        static bool spacePressed = false;
        bool spaceCurrentlyPressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

        if (spaceCurrentlyPressed && !spacePressed) {
            // 重力方向を考慮した足場判定
            glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
            bool onPlatform = false;
            
            // 重力方向に応じて足場判定を行う
            for (const auto& platform : gameState.platforms) {
                if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
                if (isPlayerOnPlatformWithGravityForMovement(platform, gameState.playerPosition, playerSize, gravityDirection)) {
                    onPlatform = true;
                    break;
                }
            }
            
            if (onPlatform) {
                // 重力方向に応じたジャンプ
                if (gravityDirection.y > 0.5f) {
                    gameState.playerVelocity.y = -8.0f; // 重力反転時は下向きにジャンプ
                } else {
                    gameState.playerVelocity.y = 8.0f; // 通常時は上向きにジャンプ
                }
                gameState.isFloating = false;
            } else if (!gameState.isFloating && gameState.floatCount < 2) { // 最大2回まで
                gameState.isFloating = true;
                gameState.floatTimer = 0.0f;
                if (gravityDirection.y > 0.5f) {
                    gameState.playerVelocity.y = -2.0f; // 重力反転時は下向きに浮遊
                } else {
                    gameState.playerVelocity.y = 2.0f; // 通常時は上向きに浮遊
                }
                gameState.floatCount++; // 浮遊回数を増加
            }
        }
        spacePressed = spaceCurrentlyPressed;

        if (gameState.isFloating) {
            gameState.floatTimer += deltaTime;
            if (gameState.floatTimer < 5.0f) {
                if (gravityDirection.y > 0.5f) {
                    // 重力反転時：下向きに浮遊
                    gameState.playerVelocity.y = glm::min(gameState.playerVelocity.y + 1.0f * deltaTime, 1.0f);
                    if (spaceCurrentlyPressed) {
                        gameState.playerVelocity.y = glm::max(gameState.playerVelocity.y - 3.0f * deltaTime, -2.0f);
                    }
                } else {
                    // 通常時：上向きに浮遊
                    gameState.playerVelocity.y = glm::max(gameState.playerVelocity.y - 1.0f * deltaTime, -1.0f);
                    if (spaceCurrentlyPressed) {
                        gameState.playerVelocity.y = glm::min(gameState.playerVelocity.y + 3.0f * deltaTime, 2.0f);
                    }
                }
            } else {
                gameState.isFloating = false;
            }
        }

        // 垂直位置更新
        gameState.playerPosition.y += gameState.playerVelocity.y * deltaTime;

        // 足場との衝突判定（重力方向を考慮）
        glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
        GameState::Platform* platform = checkPlatformCollisionWithGravity(gameState, gameState.playerPosition, playerSize, gravityDirection);
        

        
        if (platform != nullptr) {
            // 回転する足場の場合、急激な回転では衝突判定を無効化
            bool shouldIgnoreCollision = false;
            if (platform->isRotating) {
                float rotationRadians = glm::radians(platform->rotationAngle);
                float tiltAngle = std::abs(std::sin(rotationRadians));
                
                // 急激な傾斜（約60度以上）では衝突判定を無効化
                if (tiltAngle > 0.85f) {
                    shouldIgnoreCollision = true;
                }
            }
            
            if (!shouldIgnoreCollision) {
                // 回転した足場の正確な表面位置を計算
                if (platform->isRotating) {
                    glm::vec3 corners[4];
                    glm::vec3 playerFoot;
                    
                    if (gravityDirection.y > 0.5f) {
                        // 重力反転時：下面の角を使用
                        getRotatedPlatformBottomCorners(*platform, corners);
                        playerFoot = gameState.playerPosition + glm::vec3(0, playerSize.y * 0.5f, 0);
                    } else {
                        // 通常時：上面の角を使用
                        getRotatedPlatformTopCorners(*platform, corners);
                        playerFoot = gameState.playerPosition - glm::vec3(0, playerSize.y * 0.5f, 0);
                    }
                    
                    // 表面の平面方程式を計算
                    glm::vec3 v1 = corners[1] - corners[0];
                    glm::vec3 v2 = corners[2] - corners[0];
                    glm::vec3 normal = glm::normalize(glm::cross(v1, v2));
                    
                    // 平面上の点の高さを計算
                    float d = -glm::dot(normal, corners[0]);
                    float surfaceY = -(normal.x * playerFoot.x + normal.z * playerFoot.z + d) / normal.y;
                    
                    // 重力方向に応じてプレイヤーを表面に配置
                    if (gravityDirection.y > 0.5f) {
                        // 重力反転時：足場の下面に配置
                        gameState.playerPosition.y = surfaceY - playerSize.y * 0.5f;
                    } else {
                        // 通常時：足場の上面に配置
                        gameState.playerPosition.y = surfaceY + playerSize.y * 0.5f;
                    }
                } else {
                    // 重力方向に応じて足場の表面に配置
                    if (gravityDirection.y > 0.5f) {
                        // 重力反転時：足場の下面に配置
                        gameState.playerPosition.y = platform->position.y - platform->size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        // 通常時：足場の上面に配置
                        gameState.playerPosition.y = platform->position.y + platform->size.y * 0.5f + playerSize.y * 0.5f;
                    }
                }
                gameState.playerVelocity.y = 0.0f;
                gameState.isFloating = false;
                gameState.floatCount = 0; // 足場に着地したら浮遊回数をリセット
                
                // 現在の足場のインデックスを記録
                for (int i = 0; i < gameState.platforms.size(); i++) {
                    if (&gameState.platforms[i] == platform) {
                        gameState.currentPlatformIndex = i;
                        break;
                    }
                }
                
                // ジャンプパッドの場合
                if (platform->isJumpPad) {
                    gameState.playerVelocity.y = 15.0f; // 高いジャンプ
                    gameState.currentPlatformIndex = -1; // ジャンプしたら足場から離れる
                }
                
                // 消える足場の場合
                if (platform->isDisappearing && platform->disappearTimer == 0.0f) {
                    platform->disappearTimer = 0.01f; // タイマー開始
                }
            } else {
                gameState.currentPlatformIndex = -1; // 急激な回転で足場から離れる
            }
        } else {
            gameState.currentPlatformIndex = -1; // 足場から離れた
        }
        
        // 回転する足場の物理挙動（衝突判定の後、水平移動の前に適用）
        if (gameState.currentPlatformIndex >= 0 && gameState.currentPlatformIndex < gameState.platforms.size()) {
            auto& currentPlatform = gameState.platforms[gameState.currentPlatformIndex];
            
            if (currentPlatform.isRotating) {
                // 回転した足場の正確な法線ベクトルを計算
                glm::vec3 topCorners[4];
                getRotatedPlatformTopCorners(currentPlatform, topCorners);
                
                // 上面の法線ベクトルを計算
                glm::vec3 v1 = topCorners[1] - topCorners[0];
                glm::vec3 v2 = topCorners[2] - topCorners[0];
                glm::vec3 normal = glm::normalize(glm::cross(v1, v2));
                
                // 重力方向（下向き）
                glm::vec3 gravity = glm::vec3(0, -1, 0);
                
                // 法線ベクトルと重力ベクトルから滑り方向を計算
                glm::vec3 slideDirection = gravity - glm::dot(gravity, normal) * normal;
                slideDirection = glm::normalize(slideDirection);
                
                // 傾斜の強さを計算（法線ベクトルのY成分が小さいほど急傾斜）
                float slopeStrength = 1.0f - normal.y;
                
                // 滑り力を適用
                float slideForce = slopeStrength * 12.0f; // 傾斜力
                gameState.playerVelocity += slideDirection * slideForce * deltaTime;
                
                // 回転による遠心力
                float rotationRadians = glm::radians(currentPlatform.rotationAngle);
                float centrifugalForce = glm::radians(currentPlatform.rotationSpeed) * 4.0f;
                glm::vec3 playerRelativePos = gameState.playerPosition - currentPlatform.position;
                
                // X軸周りの回転の場合
                if (std::abs(currentPlatform.rotationAxis.x) > 0.5f) {
                    float distanceFromCenter = std::abs(playerRelativePos.z);
                    if (distanceFromCenter > currentPlatform.size.z * 0.2f) {
                        gameState.playerVelocity.z += centrifugalForce * deltaTime * (playerRelativePos.z > 0 ? 1.0f : -1.0f);
                    }
                }
                
                // 急激な傾斜では足場から離れやすくする
                if (slopeStrength > 0.6f) { // 約53度以上の傾斜
                    gameState.playerVelocity.y -= 6.0f * deltaTime; // 強い落下力
                }

            }
        }
        
        // 動く足場や巡回する足場に乗っている場合、足場と一緒に移動
        if (gameState.currentPlatformIndex >= 0 && gameState.currentPlatformIndex < gameState.platforms.size()) {
            auto& currentPlatform = gameState.platforms[gameState.currentPlatformIndex];
            
            // 移動足場に乗っている場合の処理
            if (currentPlatform.isMovingPlatform && currentPlatform.hasPlayerOnBoard) {
                // 移動足場の移動はupdatePlatformsで処理されるため、
                // ここではプレイヤーが足場の上に正しく配置されるようにする
                if (gravityDirection.y > 0.5f) {
                    // 重力反転時：足場の下面に配置
                    gameState.playerPosition.y = currentPlatform.position.y - currentPlatform.size.y * 0.5f - playerSize.y * 0.5f;
                } else {
                    // 通常時：足場の上面に配置
                    gameState.playerPosition.y = currentPlatform.position.y + currentPlatform.size.y * 0.5f + playerSize.y * 0.5f;
                }
            }
            
            // 移動する足場の場合
            if (currentPlatform.isMoving) {
                // 足場の実際の移動量を計算
                static std::vector<glm::vec3> lastPlatformPositions;
                if (lastPlatformPositions.size() != gameState.platforms.size()) {
                    lastPlatformPositions.resize(gameState.platforms.size());
                    for (int i = 0; i < gameState.platforms.size(); i++) {
                        lastPlatformPositions[i] = gameState.platforms[i].position;
                    }
                }
                
                glm::vec3 platformDelta = currentPlatform.position - lastPlatformPositions[gameState.currentPlatformIndex];
                gameState.playerPosition += platformDelta;
                lastPlatformPositions[gameState.currentPlatformIndex] = currentPlatform.position;
            }
            
            // 巡回する足場の場合
            if (currentPlatform.isPatrolling) {
                // 足場の実際の移動量を計算
                static std::vector<glm::vec3> lastPatrolPositions;
                if (lastPatrolPositions.size() != gameState.platforms.size()) {
                    lastPatrolPositions.resize(gameState.platforms.size());
                    for (int i = 0; i < gameState.platforms.size(); i++) {
                        lastPatrolPositions[i] = gameState.platforms[i].position;
                    }
                }
                
                glm::vec3 platformDelta = currentPlatform.position - lastPatrolPositions[gameState.currentPlatformIndex];
                gameState.playerPosition += platformDelta;
                lastPatrolPositions[gameState.currentPlatformIndex] = currentPlatform.position;
            }
            
                    // 足場表面への再配置（重力方向に応じて）
        if (gravityDirection.y > 0.5f) {
            // 重力反転時：足場の下面に再配置（少し離して配置）
            glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
            gameState.playerPosition.y = currentPlatform.position.y - currentPlatform.size.y * 0.5f - playerSize.y * 0.5f - 0.1f;
        } else {
            // 通常時：足場の上面に再配置
            glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
            gameState.playerPosition.y = currentPlatform.position.y + currentPlatform.size.y * 0.5f + playerSize.y * 0.5f;
        }
        }
        
        // スイッチとの衝突判定
        checkSwitchCollision(gameState, gameState.playerPosition, playerSize);
        
        // テレポート足場との衝突判定
        checkTeleportPlatformCollision(gameState, gameState.playerPosition, playerSize);
        
        // 移動足場との衝突判定
        checkMovingPlatformCollision(gameState, gameState.playerPosition, playerSize);
        
        // 大砲との衝突判定
        checkCannonCollision(gameState, gameState.playerPosition, playerSize);
        
        // 下限チェック（奈落に落ちた場合）
        if (gameState.playerPosition.y < -10.0f) {
            gameState.playerPosition = glm::vec3(0, 6.0f, -25.0f); // スタート地点にリセット
            gameState.playerVelocity = glm::vec3(0, 0, 0);
        }

        // 吸い込み
        float playerRadius = 0.5f;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            gameState.isSucking = true;
            gameState.suckTimer += deltaTime;

            // 敵の吸い込み（一時的にコメントアウト）
            /*
            for (auto& enemy : gameState.enemies) {
                if (!enemy.isAlive) continue;
                float distance = glm::length(gameState.playerPosition - enemy.position);
                if (distance < 3.0f) {
                    glm::vec3 suckDirection = glm::normalize(gameState.playerPosition - enemy.position);
                    enemy.velocity = suckDirection * 5.0f;
                    if (distance < 1.0f) {
                        enemy.isAlive = false;
                        if (enemy.ability != KirbyAbility::None) {
                            gameState.currentAbility = enemy.ability;
                            gameState.abilityTimer = 10.0f;
                        }
            gameState.score += 100;
                    }
                }
            }
            */
        } else {
            gameState.isSucking = false;
            gameState.suckTimer = 0.0f;
        }

        // 敵の更新（一時的にコメントアウト）
        /*
        for (auto& enemy : gameState.enemies) {
            if (!enemy.isAlive) continue;

            if (!gameState.isSucking) {
                glm::vec3 toPlayer = glm::normalize(gameState.playerPosition - enemy.position);
                enemy.velocity = toPlayer * 2.0f;
            }
            enemy.position += enemy.velocity * deltaTime;

            float enemyDistance = glm::length(gameState.playerPosition - enemy.position);
            if (enemyDistance < (playerRadius + enemy.size) && enemy.isAlive) {
                if (!gameState.isSucking) {
                    // ダメージ処理（サイレント）
                }
            }
        }
        */

        // 能力使用
        if (gameState.abilityTimer > 0.0f) {
            gameState.abilityTimer -= deltaTime;
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
                switch (gameState.currentAbility) {
                    case KirbyAbility::Fire:     /* 火炎攻撃 */ break;
                    case KirbyAbility::Ice:      /* 氷攻撃 */ break;
                    case KirbyAbility::Electric: /* 電気攻撃 */ break;
                    case KirbyAbility::Sword:    /* 剣攻撃 */ break;
                    case KirbyAbility::Hammer:   /* ハンマー攻撃 */ break;
                    default: break;
                }
            }
        } else {
            gameState.currentAbility = KirbyAbility::None;
        }
        
        // ゴール判定
        {
            float goalDistance = glm::length(gameState.playerPosition - gameState.goalPosition);
            if (goalDistance < 2.0f) {
                gameState.gameWon = true;
            }
        }

        // --------------------------
        //            描画
        // --------------------------
        renderer->beginFrame();
        
        // カメラ（見下ろし固定） - 高い位置に配置
        glm::vec3 cameraPos = gameState.playerPosition + glm::vec3(0, 2, -8);
        glm::vec3 cameraTarget = gameState.playerPosition;
        renderer->setCamera(cameraPos, cameraTarget);
        
        // デバッグ出力削除

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        renderer->setProjection(45.0f, (float)width / (float)height, 0.1f, 100.0f);
        
        // 足場のレンダリング（判定と一致する直方体）
        for (const auto& platform : gameState.platforms) {
            // 点滅床は特別な処理
            if (platform.isCycleDisappearing) {
                // 点滅床が完全に消えている場合はスキップ
                if (!platform.isCurrentlyVisible && !platform.isBlinking) continue;
            } else {
                // 通常の足場はサイズ0の場合スキップ
                if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
            }
            
            // 飛んでくる足場は飛行中のみ表示
            if (platform.isFlyingPlatform && !platform.isFlying && !platform.hasSpawned) {
                continue;
            }
            
            if (platform.isRotating) {
                // 回転する足場は回転機能付きレンダリング
                if (platform.isBlinking) {
                    renderer->renderRotatedBoxWithAlpha(platform.position, platform.color, platform.size, 
                                                      platform.rotationAxis, platform.rotationAngle, platform.blinkAlpha);
                } else {
                    renderer->renderRotatedBox(platform.position, platform.color, platform.size, 
                                             platform.rotationAxis, platform.rotationAngle);
                }
            } else {
                // 通常の足場は通常のレンダリング
                glm::vec3 renderSize = platform.size;
                // 点滅床の場合は元のサイズを使用
                if (platform.isCycleDisappearing && (platform.isBlinking || platform.isCurrentlyVisible)) {
                    renderSize = platform.originalSize;
                }
                
                if (platform.isBlinking) {
                    renderer->renderBoxWithAlpha(platform.position, platform.color, renderSize, platform.blinkAlpha);
                } else {
                    renderer->renderBox(platform.position, platform.color, renderSize);
                }
            }
        }

        // 重力反転エリアのレンダリング
        for (const auto& zone : gameState.gravityZones) {
            if (zone.isActive) {
                // 重力反転エリアを半透明の青い立方体で表示
                glm::vec3 zoneColor = glm::vec3(0.2f, 0.6f, 1.0f); // 青い色
                renderer->renderBoxWithAlpha(zone.position, zoneColor, zone.size, 0.3f); // 30%の透明度
            }
        }
        
        // スイッチのレンダリング
        for (const auto& switch_obj : gameState.switches) {
            glm::vec3 switchColor = switch_obj.color;
            if (switch_obj.isPressed) {
                // 押されている時は少し暗くする
                switchColor *= 0.7f;
            }
            renderer->renderBox(switch_obj.position, switchColor, switch_obj.size);
        }
        
        // 大砲のレンダリング
        for (const auto& cannon : gameState.cannons) {
            if (cannon.isActive) {
                // クールダウン中は色を暗くする
                glm::vec3 color = cannon.color;
                if (cannon.cooldownTimer > 0.0f) {
                    color *= 0.5f; // 半分の明度
                }
                renderer->renderBox(cannon.position, color, cannon.size);
            }
        }

        // 空中アクションなので地面デコは不要

        // カービィ（能力時は色変化）
        float kirbySize = gameState.isSucking ? 0.8f : 0.5f;
        glm::vec3 kirbyColor = gameState.playerColor;
        if (gameState.currentAbility != KirbyAbility::None) {
            switch (gameState.currentAbility) {
                case KirbyAbility::Fire:     kirbyColor = glm::vec3(1, 0.3f, 0); break;
                case KirbyAbility::Ice:      kirbyColor = glm::vec3(0.5f, 0.8f, 1); break;
                case KirbyAbility::Electric: kirbyColor = glm::vec3(1, 1, 0); break;
                case KirbyAbility::Sword:    kirbyColor = glm::vec3(0.8f, 0.8f, 0.8f); break;
                case KirbyAbility::Hammer:   kirbyColor = glm::vec3(0.6f, 0.3f, 0); break;
                default: break;
            }
        }
        renderer->renderCube(gameState.playerPosition, kirbyColor, kirbySize);

        // 敵（一時的にコメントアウト）
        /*
        for (const auto& enemy : gameState.enemies) {
            if (!enemy.isAlive) continue;
            glm::vec3 enemyColor;
            switch (enemy.type) {
                case EnemyType::WaddleDee: enemyColor = glm::vec3(0.8f, 0.6f, 0.4f); break;
                case EnemyType::WaddleDoo: enemyColor = glm::vec3(1, 0.3f, 0); break;
                case EnemyType::BrontoBurt: enemyColor = glm::vec3(0.3f, 0.7f, 1); break;
                case EnemyType::Scarfy:     enemyColor = glm::vec3(0.8f, 0.2f, 0.8f); break;
            }
            renderer->renderCube(enemy.position, enemyColor, enemy.size);
        }
        */

        // ゴール
        renderer->renderCube(gameState.goalPosition, gameState.goalColor, 1.5f);

                // UI
        renderer->renderText("Score: " + std::to_string(gameState.score), glm::vec2(10, 10), glm::vec3(1, 1, 1));
        renderer->renderText("Time: " + std::to_string((int)gameState.gameTime) + "s", glm::vec2(10, 30), glm::vec3(1, 1, 1));
        renderer->renderText("Float Count: " + std::to_string(gameState.floatCount) + "/2", glm::vec2(10, 50), glm::vec3(1, 1, 1));
        renderer->renderText("Player Pos: (" + std::to_string((int)gameState.playerPosition.x) + ", " + 
                           std::to_string((int)gameState.playerPosition.y) + ", " + 
                           std::to_string((int)gameState.playerPosition.z) + ")", glm::vec2(10, 110), glm::vec3(1, 1, 1));
        
        // 重力反転エリアの情報表示
        glm::vec3 uiGravityDirection = glm::vec3(0, -1, 0);
        bool uiInGravityZone = isPlayerInGravityZone(gameState, gameState.playerPosition, uiGravityDirection);
        if (uiInGravityZone) {
            renderer->renderText("GRAVITY INVERTED!", glm::vec2(10, 70), glm::vec3(0.2f, 0.6f, 1.0f));
            renderer->renderText("Gravity Y: " + std::to_string(uiGravityDirection.y), glm::vec2(10, 90), glm::vec3(0.2f, 0.6f, 1.0f));
        } else {
            renderer->renderText("Normal Gravity", glm::vec2(10, 70), glm::vec3(1.0f, 1.0f, 1.0f));
        }
        
        // スイッチの情報表示
        int pressedSwitches = 0;
        for (const auto& switch_obj : gameState.switches) {
            if (switch_obj.isPressed) pressedSwitches++;
        }
        renderer->renderText("Switches Pressed: " + std::to_string(pressedSwitches) + "/" + std::to_string(gameState.switches.size()), 
                           glm::vec2(10, 130), glm::vec3(1.0f, 1.0f, 1.0f));
        
        // テレポート足場の情報表示
        int teleportPlatforms = 0;
        for (const auto& platform : gameState.platforms) {
            if (platform.isTeleportPlatform) teleportPlatforms++;
        }
        renderer->renderText("Teleport Platforms: " + std::to_string(teleportPlatforms), 
                           glm::vec2(10, 150), glm::vec3(1.0f, 0.5f, 0.0f));
        
            // 移動足場の情報表示
    int movingPlatforms = 0;
    for (const auto& platform : gameState.platforms) {
        if (platform.isMovingPlatform) movingPlatforms++;
    }
    renderer->renderText("Moving Platforms: " + std::to_string(movingPlatforms),
                       glm::vec2(10, 170), glm::vec3(0.8f, 0.4f, 0.8f));
    
    // 大砲の情報表示
    renderer->renderText("Cannons: " + std::to_string(gameState.cannons.size()),
                       glm::vec2(10, 190), glm::vec3(0.8f, 0.4f, 0.2f));
        
        // 飛んでくる足場の情報表示
        for (const auto& platform : gameState.platforms) {
            if (platform.isFlyingPlatform) {
                float distanceToTarget = glm::length(platform.targetPosition - gameState.playerPosition);
                if (distanceToTarget <= platform.detectionRange && !platform.hasSpawned) {
                    renderer->renderText("Flying Platform Detected!", 
                                       glm::vec2(10, 80), glm::vec3(0.8f, 0.2f, 0.8f));
                    break;
                } else if (platform.isReturning) {
                    renderer->renderText("Platform Returning...", 
                                       glm::vec2(10, 80), glm::vec3(0.6f, 0.6f, 0.8f));
                    break;
                }
            }
        }

        if (gameState.currentAbility != KirbyAbility::None) {
            std::string abilityName;
            switch (gameState.currentAbility) {
                case KirbyAbility::Fire:     abilityName = "Fire"; break;
                case KirbyAbility::Ice:      abilityName = "Ice"; break;
                case KirbyAbility::Electric: abilityName = "Electric"; break;
                case KirbyAbility::Sword:    abilityName = "Sword"; break;
                case KirbyAbility::Hammer:   abilityName = "Hammer"; break;
                default: break;
            }
            renderer->renderText("Ability: " + abilityName + " (" + std::to_string((int)gameState.abilityTimer) + "s)",
                                 glm::vec2(10, 70), glm::vec3(1, 1, 0));
        }
        
        if (gameState.gameWon) {
            renderer->renderText("STAGE COMPLETE!", glm::vec2(640, 360), glm::vec3(1, 1, 0));
        }
        
        renderer->endFrame();
        
        // フレームレート制限
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        
        // ゲーム状態デバッグ出力削除
        
        glfwPollEvents();
    }
    
    // クリーンアップ
    renderer->cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    // ゲーム終了
    
    return 0;
}
