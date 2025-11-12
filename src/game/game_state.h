#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <vector>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include "../core/types/platform_types.h"

struct GameState {
    // プレイヤー
    glm::vec3 playerPosition = glm::vec3(0, 30.0f, 0);
    glm::vec3 playerVelocity = glm::vec3(0, 0, 0);
    glm::vec3 playerColor = glm::vec3(1, 0.8f, 0.9f);
    bool isMovingBackward = false;
    bool isShowingFrontTexture = false;  // Sキーを押した後、他の移動キーを押すまでtrue

    // 現在の足場
    int currentPlatformIndex = -1;
    
    // ゴール
    glm::vec3 goalPosition = glm::vec3(0, 2.0f, 20);
    glm::vec3 goalColor = glm::vec3(1, 1, 0);
    bool gameWon = false;
    
    // 足場
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
        float cycleTime;          
        float visibleTime;        
        float cycleTimer;         
        bool isCurrentlyVisible;  
        glm::vec3 originalSize;    
        float blinkTime;           
        float blinkTimer;          
        bool isBlinking;          
        float blinkAlpha;         
        
        // プレイヤー接近で飛んでくる足場
        bool isFlyingPlatform;    
        glm::vec3 spawnPosition;  
        glm::vec3 targetPosition; 
        float flySpeed;           
        bool isFlying;            
        bool hasSpawned;          
        float detectionRange;     
        bool isReturning;         
        
        // テレポート機能
        bool isTeleportPlatform;  
        glm::vec3 teleportDestination;
        bool hasTeleported;       
        float teleportCooldown;   
        float teleportCooldownTimer; 
        
        // 移動足場機能
        bool isMovingPlatform;    
        glm::vec3 moveTargetPosition; 
        bool hasPlayerOnBoard;    
        glm::vec3 originalPosition;
        bool returnToOriginal;    
    };
    
    // 大砲システム
    struct Cannon {
        glm::vec3 position;    
        glm::vec3 size;        
        glm::vec3 color;       
        glm::vec3 targetPosition;
        float power;
        bool isActive;
        bool hasPlayerInside;
        float cooldownTimer;
        float cooldownTime;
        glm::vec3 launchDirection;
    };
    
    std::vector<Cannon> cannons;
    std::vector<Platform> platforms;

    // カメラ制御
    float cameraYaw = 90.0f;
    float cameraPitch = 0.0f;
    float cameraDistance = 30.0f;
    bool firstMouse = true;
    float lastMouseX = 640.0f;
    float lastMouseY = 360.0f;
    
    // カメラモード制御
    bool isFirstPersonView = false;
    bool isFirstPersonMode = false;
    
    // 難易度モード制御
    bool isEasyMode = false;

    // 重力反転エリア
    struct GravityZone {
        glm::vec3 position;
        glm::vec3 size;
        glm::vec3 gravityDirection;
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
        bool isToggle;
        std::vector<int> targetPlatformIndices;
        std::vector<bool> targetStates;
        float pressTimer;
        float cooldownTimer;
        bool isMultiSwitch;
        int multiSwitchGroup;
    };
    
    std::vector<Switch> switches;
    
    // アイテムシステム
    struct Item {
        glm::vec3 position;
        glm::vec3 size;
        glm::vec3 color;
        bool isCollected;
        int itemId;
        float rotationAngle;
        float bobHeight;
        float bobTimer;
    };
    
    std::vector<Item> items;
    int collectedItems = 0;
    int requiredItems = 3;
    
    // チェックポイントシステム
    glm::vec3 lastCheckpoint = glm::vec3(0, 30.0f, 0);
    int lastCheckpointItemId = -1;
    
    float gameTime = 0.0f;
    int currentStage = 0;
    
    // 制限時間システム
    float timeLimit = 20.0f;
    float remainingTime = 20.0f;
    int earnedStars = 0;
    float clearTime = 0.0f;
    bool isTimeUp = false;
    bool isStageCompleted = false;
    
    // 星数管理システム
    std::map<int, int> stageStars;
    int totalStars = 100;
    std::map<int, bool> unlockedStages;
    
    // アイテム管理システム
    int earnedItems = 0;
    int totalItems = 3;
    
    // ステージクリアUI状態
    bool showStageClearUI = false;
    float stageClearTimer = 0.0f;
    bool stageClearConfirmed = false;
    
    // エンディング関連状態
    bool showStaffRoll = false;
    bool showEndingMessage = false;
    float staffRollTimer = 0.0f;
    float endingMessageTimer = 0.0f;
    bool isEndingSequence = false;
    
    // ステージ解放確認UI状態
    bool showUnlockConfirmUI = false;
    int unlockTargetStage = 0;
    int unlockRequiredStars = 0;
    
    // 星不足警告UI状態
    bool showStarInsufficientUI = false;
    int insufficientTargetStage = 0;
    int insufficientRequiredStars = 0;
    
    // ワープ機能説明UI状態
    bool showWarpTutorialUI = false;
    int warpTutorialStage = 0;
    bool blockEnterUntilReleased = false; // ワープUIで直前のENTERを無視するためのフラグ
    
    // 操作アシストUI状態
    bool showStageSelectionAssist = false;
    int assistTargetStage = 0;
    
    // 初回ステージ0入場チュートリアルUI状態
    bool showStage0Tutorial = true;
    
    // EASYモード説明UI状態
    bool showEasyModeExplanationUI = false;
    
    // モード選択UI状態
    bool showModeSelectionUI = false;
    int modeSelectionTargetStage = 0;  // モード選択UIで選択されたステージ番号
    // カウントダウン時の時間設定フラグ
    bool timeLimitApplied = false;
    
    // タイムアタックモード
    bool isTimeAttackMode = false;
    float currentTimeAttackTime = 0.0f;  // 現在のタイムアタック経過時間
    std::map<int, float> timeAttackRecords;  // ステージ番号 -> ベストタイム（秒）
    bool isNewRecord = false;  // 新記録フラグ
    float timeAttackStartTime = 0.0f;  // タイムアタック開始時刻（カウントダウン終了時）
    
    // リプレイシステム
    struct ReplayFrame {
        float timestamp;           // カウントダウン終了からの経過時間（秒）
        glm::vec3 playerPosition;  // プレイヤー位置
        glm::vec3 playerVelocity;  // プレイヤー速度
        std::vector<bool> itemCollectedStates;  // アイテムの収集状態
    };
    
    struct ReplayData {
        int stageNumber;
        float clearTime;
        std::vector<ReplayFrame> frames;
        std::string recordedDate;
        float frameRate;  // 記録間隔（秒）
    };
    
    // リプレイ記録用
    bool isRecordingReplay = false;
    std::vector<ReplayFrame> replayBuffer;
    float replayRecordTimer = 0.0f;
    const float REPLAY_RECORD_INTERVAL = 0.1f; // 0.1秒間隔で記録
    
    // リプレイ再生用
    bool isReplayMode = false;
    ReplayData currentReplay;
    float replayPlaybackTime = 0.0f;
    bool isReplayPaused = false;
    float replayPlaybackSpeed = 1.0f;  // 再生速度（0.3x, 0.5x, 1.0x）
    
    // 速度制御システム
    float timeScale = 1.0f;
    int timeScaleLevel = 0;
    
    // 残機システム
    int lives = 6;
    bool isGameOver = false;
    float gameOverTimer = 0.0f;
    
    // ゴール後の移動制限
    bool isGoalReached = false;
    
    // Ready画面システム
    bool showReadyScreen = false;
    bool readyScreenShown = false;
    int readyScreenSpeedLevel = 0;
    bool isCountdownActive = false;
    float countdownTimer = 3.0f;
    
    // お助けモード用
    bool canDoubleJump = true;
    glm::vec3 lastPlatformPosition = glm::vec3(0, 0, 0);
    int lastPlatformIndex = -1;
    bool isTrackingPlatform = false;
    
    // 二段ジャンプスキル用
    bool hasDoubleJumpSkill = true;
    int doubleJumpMaxUses = 3;
    int doubleJumpRemainingUses = 3;
    
    // ハートフエールスキル用
    bool hasHeartFeelSkill = false;
    int heartFeelMaxUses = 3;
    int heartFeelRemainingUses = 3;
    
    // フリーカメラスキル用
    bool hasFreeCameraSkill = false;
    bool isFreeCameraActive = false;
    float freeCameraDuration = 20.0f;
    float freeCameraTimer = 0.0f;
    int freeCameraMaxUses = 3;
    int freeCameraRemainingUses = 3;
    float freeCameraYaw = 90.0f;
    float freeCameraPitch = 0.0f;
    
    // バーストジャンプスキル用
    bool hasBurstJumpSkill = true;
    bool isBurstJumpActive = false;
    bool hasUsedBurstJump = false;
    bool isInBurstJumpAir = false;
    float burstJumpDelayTimer = 0.0f;
    int burstJumpMaxUses = 3;
    int burstJumpRemainingUses = 3;
    
    // 時間停止スキル用
    bool hasTimeStopSkill = false;
    bool isTimeStopped = false;
    float timeStopDuration = 5.0f;
    float timeStopTimer = 0.0f;
    int timeStopMaxUses = 3;
    int timeStopRemainingUses = 3;
    
    // チュートリアル専用ステージ用
    bool isTutorialStage = false;
    int tutorialStep = 0;
    bool tutorialStepCompleted = false;
    glm::vec3 tutorialStartPosition = glm::vec3(0, 0, 0);
    float tutorialRequiredDistance = 5.0f;
    bool showTutorialUI = false;
    std::string tutorialMessage = "";
    bool tutorialInputEnabled = false;
    
    // 音声管理
    bool audioEnabled = true;
    std::string currentBGM = "";
    bool bgmPlaying = false;
};

// ゲーム状態の初期化関数
void initializeGameState(GameState& gameState);