#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>

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
    
    // 分離された足場データ（platform_types.hで定義）
    #include "platform_types.h"
    
    // 後方互換性のための古いPlatform構造体（段階的に移行予定）
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
    
    // アイテムシステム
    struct Item {
        glm::vec3 position;
        glm::vec3 size;
        glm::vec3 color;
        bool isCollected;
        int itemId;  // アイテムの識別ID
        float rotationAngle;  // 回転角度（視覚効果用）
        float bobHeight;  // 上下の揺れ（視覚効果用）
        float bobTimer;  // 揺れのタイマー
    };
    
    std::vector<Item> items;
    int collectedItems = 0;  // 収集したアイテム数
    int requiredItems = 3;   // ステージクリアに必要なアイテム数
    
    // チェックポイントシステム
    glm::vec3 lastCheckpoint = glm::vec3(0, 30.0f, 0);  // 最後のチェックポイント位置
    int lastCheckpointItemId = -1;  // 最後のチェックポイントのアイテムID
    
    // デコ（道路マーカーや草パッチ）
    std::vector<glm::vec3> roadMarkers; // 細いグレーの目印（簡易道路表現）
    std::vector<glm::vec3> grassDecos;  // 小さな緑ブロック
    
    int score = 0;
    float gameTime = 0.0f;
    int currentStage = 0;  // 初期ステージをステージ選択フィールドに変更
    
    // 制限時間システム
    float timeLimit = 20.0f;        // 制限時間（秒）
    float remainingTime = 20.0f;    // 残り時間（秒）
    int earnedStars = 0;            // 今回獲得した星の数（0-3）
    float clearTime = 0.0f;         // クリア時間（秒）
    bool isTimeUp = false;          // 時間切れフラグ
    bool isStageCompleted = false;  // ステージ完了フラグ
    
    // 星数管理システム
    std::map<int, int> stageStars;  // ステージ別の最高星数記録 {ステージ番号, 星数}
    int totalStars = 0;             // トータル星数
    
    // ステージクリアUI状態
    bool showStageClearUI = false;
    float stageClearTimer = 0.0f;
    bool stageClearConfirmed = false;
    
    // 速度制御システム
    float timeScale = 1.0f;        // 時間倍率（1.0 = 通常速度、2.0 = 2倍速、3.0 = 3倍速）
    int timeScaleLevel = 0;        // 速度レベル（0 = 1倍、1 = 2倍、2 = 3倍）
    
    // 残機システム
    int lives = 6;                 // 残機数（最大6個）
    bool isGameOver = false;       // ゲームオーバーフラグ
    
    // チュートリアルシステム
    bool showTutorial = false;     // チュートリアル表示フラグ
    bool tutorialShown = false;    // チュートリアルが既に表示されたかどうか
    
    // ゴール後の移動制限
    bool isGoalReached = false;    // ゴール到達フラグ（移動制限用）
};

// ゲーム状態の初期化関数
void initializeGameState(GameState& gameState);
