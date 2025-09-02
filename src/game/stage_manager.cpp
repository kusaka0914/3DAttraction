#include "stage_manager.h"
#include "stage_generator.h"
#include "../app/game_constants.h"
#include <iostream>
#include <algorithm>
#include <tuple>

// 色の定数定義（GameConstants::Colorsを使用）
namespace Colors = GameConstants::Colors;

// 共通で使用する構造体を定義
struct ItemConfig {
    glm::vec3 position;
    glm::vec3 color;
    std::string description;
};

struct StaticConfig {
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    std::string description;
};

struct PatrolConfig {
    std::vector<glm::vec3> points;
    std::string description;
};

struct MovingConfig {
    glm::vec3 startPosition;
    glm::vec3 endPosition;
    glm::vec3 size;
    glm::vec3 color;
    float speed;
    std::string description;
};

struct CyclingDisappearingConfig {
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    float visibleTime;
    float invisibleTime;
    float initialTimer;
    std::string description;
};

StageManager::StageManager() : currentStage(1) {
    initializeStages();
}

// 共通のヘルパー関数
void initializeItems(GameState& gameState) {
    gameState.items.clear();
    gameState.collectedItems = 0;
}

void createItemsFromConfig(GameState& gameState, const std::vector<ItemConfig>& itemConfigs) {
    for (int i = 0; i < itemConfigs.size(); i++) {
        GameState::Item item;
        item.position = itemConfigs[i].position + glm::vec3(0, 1, 0);
        item.size = glm::vec3(0.5f, 0.5f, 0.5f);
        item.color = itemConfigs[i].color;
        item.isCollected = false;
        item.itemId = i + 1;
        item.rotationAngle = 0.0f;
        item.bobHeight = 0.0f;
        item.bobTimer = 0.0f;
        gameState.items.push_back(item);
        
        printf("Created %s at position (%.1f, %.1f, %.1f)\n", 
               itemConfigs[i].description.c_str(), 
               item.position.x, item.position.y, item.position.z);
    }
}

void initializeItemsWithConfig(GameState& gameState, const std::vector<ItemConfig>& itemConfigs) {
    initializeItems(gameState);
    createItemsFromConfig(gameState, itemConfigs);
}

void createItemPlatforms(PlatformSystem& platformSystem, const std::vector<ItemConfig>& itemConfigs) {
    for (int i = 0; i < itemConfigs.size(); i++) {
        platformSystem.addPlatform(GameState::StaticPlatform(
            itemConfigs[i].position, glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.6f, 1.0f)
        ));
    }
}

// 可変長のアイテム生成用のヘルパー関数
void createItems(GameState& gameState, PlatformSystem& platformSystem,
                std::initializer_list<std::tuple<std::tuple<float, float, float>, glm::vec3, std::string>> items) {
    std::vector<ItemConfig> itemConfigs;
    for (const auto& item : items) {
        const auto& pos = std::get<0>(item);
        itemConfigs.push_back({
            glm::vec3(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos)),
            std::get<1>(item),
            std::get<2>(item)
        });
    }
    initializeItemsWithConfig(gameState, itemConfigs);
    createItemPlatforms(platformSystem, itemConfigs);
}

// 可変長の静的プラットフォーム生成用のヘルパー関数
void createStaticPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, glm::vec3, std::string>> platforms) {
    for (const auto& platform : platforms) {
        const auto& pos = std::get<0>(platform);
        const auto& size = std::get<1>(platform);
        glm::vec3 position = glm::vec3(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        glm::vec3 color = std::get<2>(platform);
        std::string description = std::get<3>(platform);
        
        platformSystem.addPlatform(GameState::StaticPlatform(position, sizeVec, color));
        printf("Created %s at position (%.1f, %.1f, %.1f)\n", 
               description.c_str(), position.x, position.y, position.z);
    }
}

void createPatrolPlatforms(PlatformSystem& platformSystem, const std::vector<PatrolConfig>& patrolConfigs) {
    for (const auto& config : patrolConfigs) {
        platformSystem.addPlatform(GameState::PatrollingPlatform(
            config.points[0], glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.2f, 0.8f, 0.8f),
            config.points, 2.0f
        ));
        printf("Created %s with %zu patrol points\n", config.description.c_str(), config.points.size());
    }
}

// Flying-like overload: accept only paths and auto-generate descriptions
void createPatrolPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          const std::vector<std::vector<glm::vec3>>& paths,
                          const std::string& baseDescription = "巡回足場") {
    std::vector<PatrolConfig> patrolConfigs;
    patrolConfigs.reserve(paths.size());
    for (size_t i = 0; i < paths.size(); i++) {
        patrolConfigs.push_back({ paths[i], baseDescription + std::to_string(i + 1) });
    }
    createPatrolPlatforms(platformSystem, patrolConfigs);
}

// 可変長のパトロールプラットフォーム生成用のヘルパー関数
void createPatrolPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::vector<std::tuple<float, float, float>>, std::string>> patrols) {
    for (const auto& patrol : patrols) {
        const auto& points = std::get<0>(patrol);
        const std::string& description = std::get<1>(patrol);
        
        std::vector<glm::vec3> patrolPoints;
        for (const auto& point : points) {
            patrolPoints.push_back(glm::vec3(std::get<0>(point), std::get<1>(point), std::get<2>(point)));
        }
        
        platformSystem.addPlatform(GameState::PatrollingPlatform(
            patrolPoints[0], glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.2f, 0.8f, 0.8f),
            patrolPoints, 2.0f
        ));
        printf("Created %s with %zu patrol points\n", description.c_str(), patrolPoints.size());
    }
}

void createMovingPlatforms(PlatformSystem& platformSystem, const std::vector<MovingConfig>& movingConfigs) {
    for (const auto& config : movingConfigs) {
        platformSystem.addPlatform(GameState::MovingPlatform(
            config.startPosition, config.size, config.color,
            config.endPosition, config.speed
        ));
        printf("Created %s from (%.1f, %.1f, %.1f) to (%.1f, %.1f, %.1f)\n", 
               config.description.c_str(),
               config.startPosition.x, config.startPosition.y, config.startPosition.z,
               config.endPosition.x, config.endPosition.y, config.endPosition.z);
    }
}

// 可変長の移動プラットフォーム生成用のヘルパー関数
void createMovingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, glm::vec3, float, std::string>> platforms) {
    for (const auto& platform : platforms) {
        const auto& startPos = std::get<0>(platform);
        const auto& size = std::get<1>(platform);
        const auto& endPos = std::get<2>(platform);
        glm::vec3 color = std::get<3>(platform);
        float speed = std::get<4>(platform);
        std::string description = std::get<5>(platform);
        
        glm::vec3 startPosition = glm::vec3(std::get<0>(startPos), std::get<1>(startPos), std::get<2>(startPos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        glm::vec3 endPosition = glm::vec3(std::get<0>(endPos), std::get<1>(endPos), std::get<2>(endPos));
        
        platformSystem.addPlatform(GameState::MovingPlatform(
            startPosition, sizeVec, color,
            endPosition, speed
        ));
        printf("Created %s from (%.1f, %.1f, %.1f) to (%.1f, %.1f, %.1f) with speed %.1f\n",
               description.c_str(),
               startPosition.x, startPosition.y, startPosition.z,
               endPosition.x, endPosition.y, endPosition.z,
               speed);
    }
}

// 可変長のFlyingPlatform生成用のヘルパー関数
// ターゲット位置の配列から飛行足場を自動生成する関数
void createFlyingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          const std::vector<glm::vec3>& targetPositions, 
                          const glm::vec3& size, const glm::vec3& color, 
                          const std::string& baseDescription = "飛行足場") {
    for (size_t i = 0; i < targetPositions.size(); i++) {
        const auto& target = targetPositions[i];
        
        // spawn_x, spawn_zはtargetと同じ、spawn_yはtarget.y + 10
        glm::vec3 spawnPosition = glm::vec3(target.x, target.y -1, target.z);
        
        // 説明文を生成（番号付き）
        std::string description = baseDescription + std::to_string(i + 1);
        
        const float kSpeed = 20.0f;
        const float kDetectionRange = 2.5f;
        platformSystem.addPlatform(GameState::FlyingPlatform(
            target, size, color,
            spawnPosition, target, kSpeed, kDetectionRange
        ));
        
        printf("Created %s from spawn (%.1f, %.1f, %.1f) to target (%.1f, %.1f, %.1f) with speed %.1f, range %.1f\n",
               description.c_str(),
               spawnPosition.x, spawnPosition.y, spawnPosition.z,
               target.x, target.y, target.z,
               kSpeed, kDetectionRange);
    }
}

void createCyclingDisappearingPlatforms(PlatformSystem& platformSystem, const std::vector<CyclingDisappearingConfig>& configs) {
    for (const auto& config : configs) {
        platformSystem.addPlatform(GameState::CycleDisappearingPlatform(
            config.position, config.size, config.color,
            config.visibleTime, config.invisibleTime, config.initialTimer
        ));
        printf("Created %s at (%.1f, %.1f, %.1f) with cycle %.1fs visible, %.1fs invisible\n", 
               config.description.c_str(),
               config.position.x, config.position.y, config.position.z,
               config.visibleTime, config.invisibleTime);
    }
}

// 可変長のサイクリングディスアピアリングプラットフォーム生成用のヘルパー関数
void createCyclingDisappearingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                       std::initializer_list<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, float, float, float, glm::vec3, std::string>> platforms) {
    for (const auto& platform : platforms) {
        const auto& pos = std::get<0>(platform);
        const auto& size = std::get<1>(platform);
        glm::vec3 position = glm::vec3(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        float visibleTime = std::get<2>(platform);
        float invisibleTime = std::get<3>(platform);
        float initialTimer = std::get<4>(platform);
        glm::vec3 color = std::get<5>(platform);
        std::string description = std::get<6>(platform);
        
        platformSystem.addPlatform(GameState::CycleDisappearingPlatform(
            position, sizeVec, color,
            visibleTime, invisibleTime, initialTimer
        ));
        printf("Created %s at (%.1f, %.1f, %.1f) with cycle %.1fs visible, %.1fs invisible\n",
               description.c_str(),
               position.x, position.y, position.z,
               visibleTime, invisibleTime);
    }
}


// 可変長の連続サイクリングプラットフォーム生成用のヘルパー関数
void createConsecutiveCyclingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                    std::initializer_list<std::tuple<std::tuple<float, float, float>, int, float, std::tuple<float, float, float>, glm::vec3, float, float, float, float, std::tuple<float, float, float>, bool>> platforms) {
    for (const auto& platform : platforms) {
        const auto& startPos = std::get<0>(platform);
        int count = std::get<1>(platform);
        float spacing = std::get<2>(platform);
        const auto& size = std::get<3>(platform);
        glm::vec3 color = std::get<4>(platform);
        float visibleTime = std::get<5>(platform);
        float invisibleTime = std::get<6>(platform);
        float blinkTime = std::get<7>(platform);
        float delay = std::get<8>(platform);
        const auto& direction = std::get<9>(platform);
        bool reverseTimer = std::get<10>(platform);
        
        glm::vec3 startPosition = glm::vec3(std::get<0>(startPos), std::get<1>(startPos), std::get<2>(startPos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        glm::vec3 directionVec = glm::vec3(std::get<0>(direction), std::get<1>(direction), std::get<2>(direction));
        
        for (int i = 0; i < count; i++) {
            glm::vec3 position = startPosition + directionVec * (i * spacing);
            float initialTimer;
            if (reverseTimer) {
                initialTimer = (count - 1 - i) * delay;  // 逆順のタイマー
            } else {
                initialTimer = i * delay;  // 通常のタイマー
            }
            platformSystem.addPlatform(GameState::CycleDisappearingPlatform(
                position, sizeVec, color,
                visibleTime + invisibleTime, visibleTime, blinkTime, initialTimer
            ));
        }
        printf("Created %d consecutive cycling platforms from (%.1f, %.1f, %.1f) in direction (%.1f, %.1f, %.1f) with %.1f spacing (reverse timer: %s)\n",
               count, startPosition.x, startPosition.y, startPosition.z, directionVec.x, directionVec.y, directionVec.z, spacing, reverseTimer ? "true" : "false");
    }
}

void StageManager::initializeStages() {
    stages.clear();
    
    // ステージ0: ステージ選択フィールド
    stages.push_back({
        0, "ステージ選択フィールド",
        glm::vec3(8, 2.0f, 0),  // 中央に配置
        glm::vec3(0, 2.0f, 0),  // ゴール位置（使用しない）
        generateStageSelectionField,
        true,  // 最初からアンロック
        false,
        GameConstants::STAGE_0_TIME_LIMIT  // 制限時間なし
    });
    
    // ステージ1: 基本的なジャンプとプラットフォーム
    stages.push_back({
        1, "基本的なジャンプ",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 16.0f, 25.0f),
        generateStage1,
        true,  // 最初からアンロック
        false,
        GameConstants::STAGE_1_TIME_LIMIT
    });
    
    // ステージ2: 重力反転エリア
    stages.push_back({
        2, "重力反転エリア",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 20.0f, 30.0f),
        generateStage2,
        true,  // テスト用にアンロック
        false,
        GameConstants::STAGE_2_TIME_LIMIT
    });
    
    // ステージ3: スイッチと大砲
    stages.push_back({
        3, "スイッチと大砲",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 18.0f, 35.0f),
        generateStage3,
        true,
        false,
        GameConstants::STAGE_3_TIME_LIMIT
    });
    
    // ステージ4: 複雑な移動プラットフォーム
    stages.push_back({
        4, "移動プラットフォーム",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 22.0f, 40.0f),
        generateStage4,
        true,
        false,
        GameConstants::STAGE_4_TIME_LIMIT
    });
    
    // ステージ5: 最終ステージ
    stages.push_back({
        5, "最終ステージ",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 25.0f, 45.0f),
        generateStage5,
        true,
        false,
        GameConstants::STAGE_5_TIME_LIMIT
    });
    
    // ステージ6: チュートリアル専用ステージ
    stages.push_back({
        6, "チュートリアル",
        glm::vec3(0, 2.0f, 0),  // 中央に配置
        glm::vec3(0, 2.0f, 0),  // ゴール位置（使用しない）
        generateTutorialStage,
        true,  // 最初からアンロック
        false,
        999.0f  // 制限時間なし
    });
    
    printf("StageManager initialized with %zu stages\n", stages.size());
}

void StageManager::loadStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem) {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt == stages.end()) {
        printf("Stage %d not found\n", stageNumber);
        return;
    }
    
    if (!stageIt->isUnlocked) {
        printf("Stage %d is not unlocked yet\n", stageNumber);
        return;
    }
    
    // ゲーム状態をリセット
    gameState.platforms.clear();
    gameState.gravityZones.clear();
    gameState.switches.clear();
    gameState.cannons.clear();
    gameState.enemies.clear();
    gameState.items.clear();
    gameState.collectedItems = 0;
    gameState.gameWon = false;
    gameState.score = 0;
    gameState.gameTime = 0.0f;  // ステージ開始時にゲーム時間をリセット
    
    // 制限時間システムをリセット（ステージ固有の制限時間を適用）
    float baseTimeLimit = stageIt->timeLimit;        // ステージ固有の制限時間
    printf("Stage %d base time limit: %.1f, isFirstPersonMode: %s, isEasyMode: %s\n", 
           stageNumber, baseTimeLimit, gameState.isFirstPersonMode ? "true" : "false", 
           gameState.isEasyMode ? "true" : "false");
    
    // 制限時間の計算
    float finalTimeLimit = baseTimeLimit;
    if (gameState.isFirstPersonMode) {
        finalTimeLimit += GameConstants::FIRST_PERSON_TIME_BONUS;  // 1人称モードボーナス
        printf("1ST PERSON MODE: +%.1fs\n", GameConstants::FIRST_PERSON_TIME_BONUS);
    }
    if (gameState.isEasyMode) {
        finalTimeLimit += GameConstants::EASY_MODE_TIME_BONUS;  // お助けモードボーナス
        printf("EASY MODE: +%.1fs\n", GameConstants::EASY_MODE_TIME_BONUS);
    }
    
    gameState.timeLimit = finalTimeLimit;
    gameState.remainingTime = gameState.timeLimit;    // 残り時間を設定
    printf("Final time limit: %.1f seconds\n", gameState.timeLimit);
    gameState.earnedStars = 0;          // 星をリセット
    gameState.clearTime = 0.0f;         // クリア時間をリセット
    gameState.isTimeUp = false;         // 時間切れフラグをリセット
    gameState.isStageCompleted = false; // ステージ完了フラグをリセット
    gameState.showStageClearUI = false; // UIをリセット
    gameState.stageClearTimer = 0.0f;   // タイマーをリセット
    gameState.stageClearConfirmed = false; // 確認フラグをリセット
    gameState.isGoalReached = false;    // ゴール後の移動制限をリセット
    gameState.isGameOver = false;       // ゲームオーバーフラグをリセット
    gameState.isTimeUp = false;         // 時間切れフラグをリセット
    gameState.readyScreenShown = false; // Ready画面表示フラグをリセット
    
    // スキルの使用回数を最大値にリセット
    gameState.doubleJumpRemainingUses = gameState.doubleJumpMaxUses;
    gameState.heartFeelRemainingUses = gameState.heartFeelMaxUses;
    gameState.freeCameraRemainingUses = gameState.freeCameraMaxUses;
    gameState.burstJumpRemainingUses = gameState.burstJumpMaxUses;
    gameState.timeStopRemainingUses = gameState.timeStopMaxUses;
    
    // スキルのアクティブ状態をリセット
    gameState.isFreeCameraActive = false;
    gameState.freeCameraTimer = 0.0f;
    gameState.isBurstJumpActive = false;
    gameState.hasUsedBurstJump = false;
    gameState.isInBurstJumpAir = false;
    gameState.burstJumpDelayTimer = 0.0f;
    gameState.isTimeStopped = false;
    gameState.timeStopTimer = 0.0f;
    
    // チュートリアルステージの状態をリセット
    gameState.isTutorialStage = false;
    gameState.tutorialStep = 0;
    gameState.tutorialStepCompleted = false;
    gameState.showTutorialUI = false;
    gameState.tutorialInputEnabled = false;
    
    // チェックポイントをリセット
    gameState.lastCheckpoint = stageIt->playerStartPosition;
    gameState.lastCheckpointItemId = -1;
    
    // プラットフォームシステムをクリア
    platformSystem.clear();
    
    // プレイヤーとゴール位置を設定
    gameState.playerPosition = stageIt->playerStartPosition;
    gameState.goalPosition = stageIt->goalPosition;
    gameState.currentStage = stageNumber;
    
    // ステージ生成関数を実行
    stageIt->generateFunction(gameState, platformSystem);
    
    currentStage = stageNumber;
    printf("Loaded stage %d: %s (Time Limit: %.1f seconds)\n", stageNumber, stageIt->stageName.c_str(), stageIt->timeLimit);
}

void StageManager::unlockStage(int stageNumber) {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt != stages.end()) {
        stageIt->isUnlocked = true;
        printf("Stage %d unlocked\n", stageNumber);
    }
}

void StageManager::completeStage(int stageNumber) {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt != stages.end()) {
        stageIt->isCompleted = true;
        
        // 次のステージをアンロック
        if (stageNumber < stages.size()) {
            unlockStage(stageNumber + 1);
        }
        
        printf("Stage %d completed\n", stageNumber);
    }
}

void StageManager::resetStageProgress() {
    for (auto& stage : stages) {
        stage.isUnlocked = (stage.stageNumber == 1);
        stage.isCompleted = false;
    }
    currentStage = 1;
    printf("Stage progress reset\n");
}

bool StageManager::isStageUnlocked(int stageNumber) const {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    return stageIt != stages.end() && stageIt->isUnlocked;
}

bool StageManager::isStageCompleted(int stageNumber) const {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    return stageIt != stages.end() && stageIt->isCompleted;
}

const StageData* StageManager::getStageData(int stageNumber) const {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    return stageIt != stages.end() ? &(*stageIt) : nullptr;
}

bool StageManager::goToNextStage(GameState& gameState, PlatformSystem& platformSystem) {
    if (currentStage < stages.size()) {
        loadStage(currentStage + 1, gameState, platformSystem);
        return true;
    }
    return false;
}

bool StageManager::goToPreviousStage(GameState& gameState, PlatformSystem& platformSystem) {
    if (currentStage > 1) {
        loadStage(currentStage - 1, gameState, platformSystem);
        return true;
    }
    return false;
}

bool StageManager::goToStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem) {
    if (stageNumber >= 0 && stageNumber <= static_cast<int>(stages.size())) {
        loadStage(stageNumber, gameState, platformSystem);
        return true;
    }
    return false;
}

// ======================================================
// 各ステージの生成関数
// ======================================================

void StageManager::generateStage1(GameState& gameState, PlatformSystem& platformSystem) {
    createItems(gameState, platformSystem, {
        // {{x, y, z}, color, description}
        {{7, 5, -15}, Colors::RED, "アイテム1: スタート足場の右側"},
        {{-7, 7, -10}, Colors::GREEN, "アイテム2: スタート足場の左側"},
        {{5, 9, 3}, Colors::BLUE, "アイテム3: スタート足場の後ろ側"}
    });
    
    createStaticPlatforms(gameState, platformSystem, {
        // {{x, y, z}, {size_x, size_y, size_z}, color, description}
        {{0, 5, -25}, {4, 1, 4}, Colors::GREEN, "スタート足場"},
        {{0, 5, -15}, {3, 1, 3}, Colors::BLUE, "基本的なジャンプセクション"},
        {{0, 7, -10}, {3, 1, 3}, Colors::BLUE, "少し高いジャンプ"},
        {{0, 7, 0}, {4, 1, 4}, Colors::YELLOW, "ゴール足場"}
    });
}

void StageManager::generateStage2(GameState& gameState, PlatformSystem& platformSystem) {    
    createItems(gameState, platformSystem, {
        // {{x, y, z}, color, description}
        {{10, 5, -12}, Colors::RED, "アイテム1: スタート足場の右側"},
        {{-8, 3, -5}, Colors::GREEN, "アイテム2: スタート足場の左側"},
        {{0, 12, 15}, Colors::BLUE, "アイテム3: スタート足場の後ろ側"}
    });
    
    createStaticPlatforms(gameState, platformSystem, {
        // {{x, y, z}, {size_x, size_y, size_z}, color, description}
        {{0, 5, -25}, {4, 1, 4}, Colors::GREEN, "スタート足場"},
        {{8, 10, 15}, {4, 1, 4}, Colors::YELLOW, "ゴール足場"}
    });
    
    // 可変長パトロールプラットフォーム生成（パトロールポイント、説明を指定）
    createPatrolPlatforms(gameState, platformSystem, {
        //{{start_x, start_y, start_z}, {second_point_x, second_point_y, second_point_z}, {third_point_x, third_point_y, third_point_z}, {fourth_point_x, fourth_point_y, fourth_point_z}, {fifth_point_x, fifth_point_y, fifth_point_z}, description}
        {{{0, 5, -20}, {4, 5, -15}, {0, 5, -10}, {-4, 5, -15}, {0, 5, -20}}, "巡回足場1: 水平移動"},
        {{{0, 5, -5}, {4, 5, -5}, {0, 5, -5}, {-4, 5, -5}, {0, 5, -5}}, "巡回足場2: 水平移動"},
        {{{0, 5, 0}, {0, 5, 0}, {0, 10, 0}, {0, 10, 0}, {0, 5, 0}}, "巡回足場3: 垂直移動"},
        {{{0, 10, 5}, {4, 10, 10}, {0, 10, 5}, {-4, 10, 10}, {0, 10, 5}}, "巡回足場4: 複合移動"}
    });


    
    // 重力反転エリア内の足場（天井に配置）
    // platformSystem.addPlatform(GameState::StaticPlatform(
    //     glm::vec3(0, 12, -15), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.8f, 0.2f)
    // ));
    // platformSystem.addPlatform(GameState::StaticPlatform(
    //     glm::vec3(0, 15, -10), glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.8f, 0.2f, 0.8f)
    // ));
    // platformSystem.addPlatform(GameState::StaticPlatform(
    //     glm::vec3(0, 18, -5), glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.8f, 0.2f, 0.8f)
    // ));
    
    // 重力反転エリアを生成
    // gameState.gravityZones.clear();
    // GameState::GravityZone gravityZone;
    // gravityZone.position = glm::vec3(0, 8, -15);
    // gravityZone.size = glm::vec3(8, 2, 8);
    // gravityZone.gravityDirection = glm::vec3(0, 1, 0);
    // gravityZone.radius = 4.0f;
    // gravityZone.isActive = true;
    // gameState.gravityZones.push_back(gravityZone);
}

void StageManager::generateStage3(GameState& gameState, PlatformSystem& platformSystem) {
    createItems(gameState, platformSystem, {
        // {{x, y, z}, color, description}
        {{-8, 5, -8}, Colors::RED, "アイテム1: スタート足場の右側"},
        {{13, 11, 8}, Colors::GREEN, "アイテム2: スタート足場の左側"},
        {{18, 11, 32}, Colors::BLUE, "アイテム3: スタート足場の後ろ側"}
    });
    
    createStaticPlatforms(gameState, platformSystem, {
        // {{x, y, z}, {size_x, size_y, size_z}, color, description}
        {{0, 5, -25}, {4, 1, 4}, Colors::GREEN, "スタート足場"},
        {{5, 5, 4}, {4, 1, 4}, Colors::GREEN, "スタート足場"},
        {{0, 11, 15}, {4, 1, 4}, Colors::GREEN, "スタート足場"},
        {{2, 11, 53}, {4, 1, 4}, Colors::YELLOW, "ゴール足場"}
    });
    
    // 可変長サイクリングディスアピアリングプラットフォーム生成（個別配置）
    createCyclingDisappearingPlatforms(gameState, platformSystem, {
        // {{x, y, z}, {size_x, size_y, size_z}, visible_time, invisible_time, initial_timer, color, description}
        {{7, 7, 8}, {3, 1, 3}, 6.0f, 4.0f, 1.0f, Colors::ORANGE, "サイクル消える足場1"},
        {{3, 9, 8}, {3, 1, 3}, 6.0f, 4.0f, 1.0f, Colors::PURPLE, "サイクル消える足場2"},
        {{7, 11, 8}, {3, 1, 3}, 6.0f, 4.0f, 1.0f, Colors::PURPLE, "サイクル消える足場2"},
        {{7, 11, 15}, {3, 1, 3}, 6.0f, 4.0f, 1.0f, Colors::PURPLE, "サイクル消える足場2"},
        // {{5, 15, 10}, {3, 1, 3}, 1.0f, 2.0f, 0.0f, Colors::GREEN, "サイクル消える足場4"}
    });
    
    // 可変長連続サイクリングプラットフォーム生成（開始位置、プラットフォーム数、間隔、サイズ、色、表示時間、非表示時間、点滅時間、遅延時間、方向、逆順タイマーを指定）
    createConsecutiveCyclingPlatforms(gameState, platformSystem, {
        // {{start_x, start_y, start_z}, count, spacing, {size_x, size_y, size_z}, color, visible_time, invisible_time, blink_time, delay, {direction_x, direction_y, direction_z}, reverse_timer}
        {{0, 5, -20}, 5, 6.0f, {3, 1, 3}, Colors::CYAN, 12.0f, 6.0f, 0.5f, 1.0f, {0, 0, 1}, true},
        {{0, 11, 20}, 3, 6.0f, {3, 1, 3}, Colors::CYAN, 12.0f, 6.0f, 0.5f, 1.0f, {1, 0, 1}, true},
        {{8, 11, 40}, 2, 6.0f, {3, 1, 3}, Colors::CYAN, 12.0f, 6.0f, 0.5f, 1.0f, {-1, 0, 1}, true}
    });
    
    // スイッチで操作される足場（最初は非表示）
    // platformSystem.addPlatform(GameState::StaticPlatform(
    //     glm::vec3(0, 5, -15), glm::vec3(3, 1, 3), glm::vec3(1.0f, 0.2f, 0.2f)
    // ));
    
    // スイッチを生成
    // gameState.switches.clear();
    // GameState::Switch switch1;
    // switch1.position = glm::vec3(-1, 6.5, -25);
    // switch1.size = glm::vec3(1, 0.5, 1);
    // switch1.color = glm::vec3(1.0f, 0.2f, 0.2f);
    // switch1.isPressed = false;
    // switch1.isToggle = true;
    // switch1.targetPlatformIndices = {1};
    // switch1.targetStates = {true};
    // switch1.pressTimer = 0.0f;
    // switch1.cooldownTimer = 0.0f;
    // switch1.isMultiSwitch = false;
    // switch1.multiSwitchGroup = -1;
    // gameState.switches.push_back(switch1);
    
    // 大砲を生成
    // gameState.cannons.clear();
    // GameState::Cannon cannon1;
    // cannon1.position = glm::vec3(5, 5, -10);
    // cannon1.size = glm::vec3(2, 2, 2);
    // cannon1.color = glm::vec3(0.8f, 0.4f, 0.2f);
    // cannon1.targetPosition = glm::vec3(0, 15, 10);
    // cannon1.power = 15.0f;
    // cannon1.isActive = true;
    // cannon1.hasPlayerInside = false;
    // cannon1.cooldownTimer = 0.0f;
    // cannon1.cooldownTime = 2.0f;
    
    // 発射方向を計算
    // glm::vec3 direction = glm::normalize(cannon1.targetPosition - cannon1.position);
    // float distance = glm::length(cannon1.targetPosition - cannon1.position);
    // float gravity = 9.8f;
    // float timeToTarget = sqrt(2.0f * distance / gravity);
    // cannon1.launchDirection = direction * cannon1.power;
    // cannon1.launchDirection.y = (cannon1.targetPosition.y - cannon1.position.y) / timeToTarget + 0.5f * gravity * timeToTarget;
    
    // gameState.cannons.push_back(cannon1);
}

void StageManager::generateStage4(GameState& gameState, PlatformSystem& platformSystem) {
    // アイテム生成
    createItems(gameState, platformSystem, {
        // {{x, y, z}, color, description}
        {{-9, 5, -12}, Colors::RED, "アイテム1: スタート足場の右側"},
        {{3, 12, -12}, Colors::GREEN, "アイテム2: スタート足場の左側"},
        {{-3, 12, 3}, Colors::BLUE, "アイテム3: スタート足場の後ろ側"}
    });
    
    // 静的プラットフォーム生成
    createStaticPlatforms(gameState, platformSystem, {
        // {{x, y, z}, {size_x, size_y, size_z}, color, description}
        {{0, 5, -25}, {4, 1, 4}, Colors::GREEN, "スタート足場"},
        {{-9, 12, 15}, {4, 1, 4}, Colors::YELLOW, "ゴール足場"}
    });
    
    // 飛行足場のターゲット位置を定義
    std::vector<glm::vec3> stage1TargetPositions = {
        {0, 5, -21}, {0, 5, -18}, {3, 5, -18}, {3, 5, -15}, {3, 5, -12},
        {0, 5, -12}, {-3, 5, -12}, {-3, 5, -9}, {-3, 6, -6}, {-6, 7, -6},
        {-9, 8, -6}, {-9, 9, -9}, {-9, 10, -12}, {-6, 11, -12}, {-3, 12, -12},
        {-12, 11, -12}, {-15, 12, -12}, {-15, 12, -9}, {-15, 12, -6}, {-12, 12, -3},
        {-15, 12, 0}, {-12, 12, 3}, {-9, 12, 3}, {-15, 12, 6}, {-15, 12, 9}
    };
    
    // FlyingPlatform生成（近づくと飛んでくる足場）
    createFlyingPlatforms(gameState, platformSystem, stage1TargetPositions,
                         glm::vec3(3, 1, 3), Colors::ORANGE, "右から飛んでくる足場");
}

void StageManager::generateStage5(GameState& gameState, PlatformSystem& platformSystem) {
    std::vector<std::vector<glm::vec3>> patrolPaths={
        {{-10, 5, -21}, {10, 5, -21}, {10, 5, -2}, {-10, 5, -2}, {-10, 5, -21}},
        {{7, 5, -8}, {7, 5, -13}, {-7, 5, -13}, {-7, 5, -8}, {7, 5, -8}},
        {{4, 10, 16}, {0, 10, 16}, {0, 14, 16}, {0, 10, 16}, {4, 10, 16}},
        {{4, 14, 18}, {8, 14, 22}, {8, 14, 22}, {4, 14, 18}, {4, 14, 18}},
        {{0, 14, 76}, {6, 14, 80}, {0, 14, 84}, {-6, 14, 80}, {0, 14, 76}},
        {{-18, 14, 88}, {-12, 14, 92}, {-6, 14, 88}, {-12, 14, 84}, {-18, 14, 88}},
    };
    std::vector<glm::vec3> targetPositions = {
        {0, 3, 10},
        {0, 3, 13},
        {0, 9, 36},
        {0, 9, 39},
        {0, 9, 42},
        {0, 9, 45},
        {3, 10, 45},
        {6, 11, 45},
        {6, 12, 42},
        {6, 13, 39},
        {-12, 16, 107},
        {-12, 16, 110},
        {-12, 16, 113},
        {-12, 16, 116},
        {-12, 16, 119},
        {-12, 16, 122},
        {-12, 16, 125},
        {-12, 16, 128},
        {-12, 16, 131},
        {-12, 16, 134},
        {-12, 16, 137},
        {-12, 16, 140},
        {-12, 16, 143},
        {-12, 16, 146},
        {-12, 16, 149},
        {-9, 16, 149},
        {-9, 16, 151},
        {-9, 16, 154},
        {-9, 16, 157},
        {-12, 16, 157},
        {-12, 16, 160},
        {-12, 16, 163},
        {-12, 16, 166},
        {-12, 16, 169},
    };
    std::vector<CyclingDisappearingConfig> cycleConfigs;
    cycleConfigs.push_back({ {0, 5, 16},  {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {4, 7, 20}, {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {8, 9, 16}, {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {0, 14, 28}, {3, 1, 3}, Colors::ORANGE, 4.0f, 2.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-3, 3, 27}, {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });

    cycleConfigs.push_back({ {0, 14, 46}, {3, 1, 3}, Colors::ORANGE, 6.0f, 3.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {5, 14, 46}, {3, 1, 3}, Colors::ORANGE, 7.0f, 3.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-5, 14, 46}, {3, 1, 3}, Colors::ORANGE, 8.0f, 3.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {0, 14, 52}, {3, 1, 3}, Colors::ORANGE, 6.0f, 4.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {5, 14, 52}, {3, 1, 3}, Colors::ORANGE, 7.0f, 4.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-5, 14, 52}, {3, 1, 3}, Colors::ORANGE, 8.0f, 4.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {0, 14, 58}, {3, 1, 3}, Colors::ORANGE, 6.0f, 5.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {5, 14, 58}, {3, 1, 3}, Colors::ORANGE, 7.0f, 5.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-5, 14, 58}, {3, 1, 3}, Colors::ORANGE, 8.0f, 5.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {0, 14, 64}, {3, 1, 3}, Colors::ORANGE, 6.0f, 3.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {5, 14, 64}, {3, 1, 3}, Colors::ORANGE, 7.0f, 4.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-5, 14, 64}, {3, 1, 3}, Colors::ORANGE, 8.0f, 5.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {0, 14, 70}, {3, 1, 3}, Colors::ORANGE, 6.0f, 5.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {5, 14, 70}, {3, 1, 3}, Colors::ORANGE, 7.0f, 3.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-5, 14, 70}, {3, 1, 3}, Colors::ORANGE, 8.0f, 4.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-12, 14, 98}, {3, 1, 3}, Colors::ORANGE, 4.0f, 2.0f, 1.0f, "サイクル消える足場1" });
    // アイテム生成
    createItems(gameState, platformSystem, {
        // {{x, y, z}, color, description}
        {{0, 5, 3}, Colors::RED, "アイテム1: スタート足場の右側"},
        {{0, 14, 40}, Colors::GREEN, "アイテム2: スタート足場の左側"},
        {{-12, 16, 104}, Colors::BLUE, "アイテム3: スタート足場の後ろ側"}
    });
    createStaticPlatforms(gameState, platformSystem, {
        // {{x, y, z}, {size_x, size_y, size_z}, color, description}
        {{0, 5, -25}, {4, 1, 4}, Colors::GRAY, "スタート足場"},
        {{10, 5, -15}, {3, 5, 1}, Colors::GRAY, "スタート足場"},
        {{-12, 16, 170}, {4, 1, 4}, Colors::YELLOW, "ゴール足場"}
    });
    createPatrolPlatforms(gameState, platformSystem, patrolPaths, "ステージ選択エリア");
    createCyclingDisappearingPlatforms(platformSystem, cycleConfigs);
    createFlyingPlatforms(gameState, platformSystem, targetPositions, 
                         glm::vec3(3, 1, 3), Colors::ORANGE, "右から飛んでくる足場");
}

// ステージ選択フィールドの生成
void StageManager::generateStageSelectionField(GameState& gameState, PlatformSystem& platformSystem) {
    platformSystem.clear();
    std::vector<std::vector<glm::vec3>> patrolPaths={
        {{14, 0, 10}, {18, 0, 10}, {18, 0, 10}, {18, 0, 10}, {14, 0, 10}},
        {{22, 0, 14}, {22, 0, 10}, {22, 0, 10}, {22, 0, 10}, {22, 0, 14}},
        {{22, 0, 18}, {22, 4, 18}, {22, 4, 18}, {22, 4, 18}, {22, 0, 18}},
    };
    if (gameState.stageStars.count(2) && gameState.stageStars.at(2) > 0) {
        patrolPaths.push_back({{18, 4, 22}, {14, 4, 22}, {14, 4, 22}, {14, 4, 22}, {18, 4, 22}});
        patrolPaths.push_back({ {8, 0, 28}, {8, 0, 32}, {8, 0, 32}, {8, 0, 32}, {8, 0, 28} });
    }
    // ターゲット位置を冒頭で定義
    std::vector<glm::vec3> targetPositions = {
        {8, 0, 10},
        {8, 0, 13},
        {8, 0, 16},
        {8, 0, 19}
    };
    // ステージ4クリア時のみ最後の足場を出現
    if ((gameState.stageStars.count(4) && gameState.stageStars.at(4) > 0)) {
        targetPositions.push_back({8, 0, 39});
    }

    std::vector<CyclingDisappearingConfig> cycleConfigs;
    cycleConfigs.push_back({ {1, 0, 11},  {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-3, 0, 15}, {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-7, 1, 23}, {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-3, 2, 23}, {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-3, 3, 27}, {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
   if (gameState.stageStars.count(3) && gameState.stageStars.at(3) > 0) {
        cycleConfigs.push_back({ {1, 3, 23},  {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
        cycleConfigs.push_back({ {8, 0, 36},  {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    }
    
    // メインの選択フィールド（画像のような長方形）
    createStaticPlatforms(gameState, platformSystem, {
        // メインフィールド（1つ目）
        {{GameConstants::STAGE_AREAS[0].x, GameConstants::STAGE_AREAS[0].y - 1, GameConstants::STAGE_AREAS[0].z}, {10, 1, 10}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Right"},

        // ステージ1選択エリア（解放済みかどうかで色を変更）
        {{GameConstants::STAGE_AREAS[0].x, GameConstants::STAGE_AREAS[0].y, GameConstants::STAGE_AREAS[0].z}, {1, 1, 1}, gameState.unlockedStages[1] ? glm::vec3(0.2f, 1.0f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), "Stage 1 Selection Area"},

        // {{-26, 0, 0}, {2, 1, 2}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Center"},
        
        // メインフィールド（2つ目）
        {{GameConstants::STAGE_AREAS[1].x, GameConstants::STAGE_AREAS[1].y - 1, GameConstants::STAGE_AREAS[1].z}, {6, 1, 6}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Left"},
        
        // ステージ2選択エリア（解放済みかどうかで色を変更）
        {{GameConstants::STAGE_AREAS[1].x, GameConstants::STAGE_AREAS[1].y, GameConstants::STAGE_AREAS[1].z}, {1, 1, 1}, gameState.unlockedStages[2] ? glm::vec3(0.2f, 1.0f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), "Stage 2 Selection Area"},
        {{-7, 0, 19}, {3, 1, 3}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Left"},
        // メインフィールド（3つ目）
        {{GameConstants::STAGE_AREAS[2].x, GameConstants::STAGE_AREAS[2].y - 1, GameConstants::STAGE_AREAS[2].z}, {6, 1, 6}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Left"},

        // ステージ3選択エリア（解放済みかどうかで色を変更）
        {{GameConstants::STAGE_AREAS[2].x, GameConstants::STAGE_AREAS[2].y, GameConstants::STAGE_AREAS[2].z}, {1, 1, 1}, gameState.unlockedStages[3] ? glm::vec3(0.2f, 1.0f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), "Stage 3 Selection Area"},

        // メインフィールド（4つ目）
        {{GameConstants::STAGE_AREAS[3].x, GameConstants::STAGE_AREAS[3].y - 1, GameConstants::STAGE_AREAS[3].z}, {6, 1, 6}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Left"},
        
        // ステージ4選択エリア（解放済みかどうかで色を変更）
        {{GameConstants::STAGE_AREAS[3].x, GameConstants::STAGE_AREAS[3].y, GameConstants::STAGE_AREAS[3].z}, {1, 1, 1}, gameState.unlockedStages[4] ? glm::vec3(0.2f, 1.0f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), "Stage 4 Selection Area"},
        
        // メインフィールド（5つ目）
        {{GameConstants::STAGE_AREAS[4].x, GameConstants::STAGE_AREAS[4].y - 1, GameConstants::STAGE_AREAS[4].z}, {6, 1, 6}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Left"},

        // ステージ5選択エリア（解放済みかどうかで色を変更）
        {{GameConstants::STAGE_AREAS[4].x, GameConstants::STAGE_AREAS[4].y, GameConstants::STAGE_AREAS[4].z}, {1, 1, 1}, gameState.unlockedStages[5] ? glm::vec3(0.2f, 1.0f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), "Stage 5 Selection Area"},
    });

    // ターゲット位置を元に飛行足場を自動生成
    createFlyingPlatforms(gameState, platformSystem, targetPositions, 
                         glm::vec3(3, 1, 3), Colors::ORANGE, "右から飛んでくる足場");
    
    createPatrolPlatforms(gameState, platformSystem, patrolPaths, "ステージ選択エリア");
 
    // 可変長サイクリングディスアピアリングプラットフォーム生成（個別配置）    
    createCyclingDisappearingPlatforms(platformSystem, cycleConfigs);
    
    
}

// 星数管理メソッドの実装
int StageManager::getStageStars(int stageNumber) const {
    // この実装は後で修正が必要
    return 0;
}

int StageManager::getTotalStars() const {
    // この実装は後で修正が必要
    return 0;
}

void StageManager::updateStageStars(int stageNumber, int newStars) {
    // この実装は後で修正が必要
    printf("Stage %d stars updated to %d\n", stageNumber, newStars);
}

int StageManager::calculateStarDifference(int stageNumber, int newStars) const {
    // この実装は後で修正が必要
    return newStars;
}

// チュートリアルステージの生成関数
void StageManager::generateTutorialStage(GameState& gameState, PlatformSystem& platformSystem) {

    // チュートリアルステージの初期化
    gameState.isTutorialStage = true;
    gameState.tutorialStep = 0;
    gameState.tutorialStepCompleted = false;
    gameState.tutorialStartPosition = gameState.playerPosition;
    gameState.tutorialRequiredDistance = 5.0f;  // 移動距離を小さくする
    gameState.showTutorialUI = true;
    gameState.tutorialInputEnabled = true;
    gameState.tutorialMessage = "MOVING FORWARD: PRESS W";
    
    // チュートリアル用のアイテム管理を初期化
    gameState.earnedItems = 0;
    gameState.totalItems = 3;

    createItems(gameState, platformSystem, {
        {{2, 0, 9.5}, Colors::RED, "Tutorial Item1"},
        {{8.5, 0, 3}, Colors::GREEN, "Tutorial Item2"},
        {{-4.5, 0, 3}, Colors::BLUE, "Tutorial Item3"},
    });
    
    // チュートリアル用の簡単な足場を作成
    createStaticPlatforms(gameState, platformSystem, {
        {{2, 0, 3}, {10, 1, 10}, glm::vec3(0.3f, 0.3f, 0.3f), "Tutorial Platform"},
        {{2, 0, 16}, {4, 1, 4}, Colors::YELLOW, "Tutorial Platform"},
    });
    
}
