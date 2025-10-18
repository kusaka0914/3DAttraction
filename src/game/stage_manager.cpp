#include "stage_manager.h"
#include "json_stage_loader.h"
#include "../core/constants/game_constants.h"
#include "../core/constants/debug_config.h"
#include "../core/utils/stage_utils.h"
#include <iostream>
#include <algorithm>
#include <tuple>

namespace Colors = GameConstants::Colors;

StageManager::StageManager() : currentStage(1) {
    initializeStages();
}

void StageManager::initializeStages() {
    stages.clear();
    
    stages.push_back({
        0, "ステージ選択フィールド",
        glm::vec3(8, 2.0f, 0),  
        glm::vec3(0, 2.0f, 0),  
        generateStageSelectionField,
        true,
        false,
        GameConstants::STAGE_0_TIME_LIMIT
    });
    
    stages.push_back({
        1, "基本的なジャンプ",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 16.0f, 25.0f),
        generateStage1,
        true,
        false,
        GameConstants::STAGE_1_TIME_LIMIT
    });
    
    stages.push_back({
        2, "重力反転エリア",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 20.0f, 30.0f),
        generateStage2,
        true,
        false,
        GameConstants::STAGE_2_TIME_LIMIT
    });
    
    stages.push_back({
        3, "スイッチと大砲",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 18.0f, 35.0f),
        generateStage3,
        true,
        false,
        GameConstants::STAGE_3_TIME_LIMIT
    });
    
    stages.push_back({
        4, "移動プラットフォーム",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 22.0f, 40.0f),
        generateStage4,
        true,
        false,
        GameConstants::STAGE_4_TIME_LIMIT
    });
    
    stages.push_back({
        5, "最終ステージ",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 25.0f, 45.0f),
        generateStage5,
        true,
        false,
        GameConstants::STAGE_5_TIME_LIMIT
    });

    stages.push_back({
        6, "チュートリアル",
        glm::vec3(0, 2.0f, 0),  
        glm::vec3(0, 2.0f, 0),  
        generateTutorialStage,
        true,
        false,
        999.0f
    });
    
    printf("StageManager initialized with %zu stages\n", stages.size());
}

bool StageManager::loadStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem) {
    if (stageNumber < 0 || stageNumber >= static_cast<int>(stages.size())) {
        printf("ERROR: Invalid stage number %d (valid range: 0-%zu)\n", stageNumber, stages.size() - 1);
        return false;
    }
    
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt == stages.end()) {
        printf("ERROR: Stage %d not found in stages vector\n", stageNumber);
        return false;
    }
    
    if (!stageIt->isUnlocked) {
        printf("ERROR: Stage %d is not unlocked yet\n", stageNumber);
        return false;
    }
    
    // ゲーム状態をリセット
    gameState.platforms.clear();
    gameState.gravityZones.clear();
    gameState.switches.clear();
    gameState.cannons.clear();
    gameState.items.clear();
    gameState.collectedItems = 0;
    gameState.gameWon = false;
    
    gameState.timeLimit = stageIt->timeLimit;
    gameState.remainingTime = gameState.timeLimit;    // 残り時間を設定
    printf("Time limit: %.1f seconds\n", gameState.timeLimit);
    gameState.earnedStars = 0;          // 星をリセット
    gameState.clearTime = 0.0f;         // クリア時間をリセット
    DEBUG_PRINTF("DEBUG: clearTime reset to 0.0f for stage %d\n", stageNumber);
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

    gameState.isFirstPersonMode = false;
    gameState.isFirstPersonView = false;        
    
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
    return true;
}

bool StageManager::unlockStage(int stageNumber, GameState* gameState) {
    // ステージ番号の範囲チェック
    if (stageNumber < 0 || stageNumber >= static_cast<int>(stages.size())) {
        printf("ERROR: Invalid stage number %d for unlock (valid range: 0-%zu)\n", stageNumber, stages.size() - 1);
        return false;
    }
    
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt == stages.end()) {
        printf("ERROR: Stage %d not found for unlock\n", stageNumber);
        return false;
    }
    
    if (stageIt->isUnlocked) {
        printf("WARNING: Stage %d is already unlocked\n", stageNumber);
        return true; // 既にアンロック済みでも成功とする
    }
    
    // ステージをアンロック
    stageIt->isUnlocked = true;
    
    // GameStateも同期（nullptrでない場合のみ）
    if (gameState != nullptr) {
        gameState->unlockedStages[stageNumber] = true;
    }
    
    printf("Stage %d unlocked successfully\n", stageNumber);
    return true;
}

bool StageManager::unlockStageWithStars(int stageNumber, int requiredStars, GameState& gameState) {
    // ステージ番号の範囲チェック
    if (stageNumber < 0 || stageNumber >= static_cast<int>(stages.size())) {
        printf("ERROR: Invalid stage number %d for star unlock (valid range: 0-%zu)\n", stageNumber, stages.size() - 1);
        return false;
    }
    
    // 必要スター数のチェック
    if (requiredStars < 0) {
        printf("ERROR: Invalid required stars %d (must be >= 0)\n", requiredStars);
        return false;
    }
    
    // 現在のスター数チェック
    if (gameState.totalStars < requiredStars) {
        printf("ERROR: Insufficient stars. Required: %d, Available: %d\n", requiredStars, gameState.totalStars);
        return false;
    }
    
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt == stages.end()) {
        printf("ERROR: Stage %d not found for star unlock\n", stageNumber);
        return false;
    }
    
    if (stageIt->isUnlocked) {
        printf("WARNING: Stage %d is already unlocked\n", stageNumber);
        return true; // 既にアンロック済みでも成功とする
    }
    
    // アトミックに実行：星を消費してステージを解放
    gameState.totalStars -= requiredStars;
    stageIt->isUnlocked = true;
    gameState.unlockedStages[stageNumber] = true;
    
    printf("Stage %d unlocked with %d stars! Remaining stars: %d\n", 
           stageNumber, requiredStars, gameState.totalStars);
    return true;
}

void StageManager::completeStage(int stageNumber) {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt != stages.end()) {
        stageIt->isCompleted = true;
        
        // 次のステージをアンロック（GameStateなしで呼び出し）
        if (stageNumber + 1 < static_cast<int>(stages.size())) {
            // 一時的なGameStateを作成してunlockStageを呼び出し
            // 実際の実装では、GameStateの参照を渡すべき
            printf("Stage %d completed, attempting to unlock next stage %d\n", stageNumber, stageNumber + 1);
        }
        
        printf("Stage %d completed successfully\n", stageNumber);
    } else {
        printf("ERROR: Stage %d not found for completion\n", stageNumber);
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
    // ステージ番号の範囲チェック
    if (stageNumber < 0 || stageNumber >= static_cast<int>(stages.size())) {
        printf("ERROR: Invalid stage number %d for goToStage (valid range: 0-%zu)\n", stageNumber, stages.size() - 1);
        return false;
    }
    
    // loadStageを呼び出し（内部でアンロック状態もチェックされる）
    return loadStage(stageNumber, gameState, platformSystem);
}

// ======================================================
// 各ステージの生成関数
// ======================================================

void StageManager::generateStage1(GameState& gameState, PlatformSystem& platformSystem) {
    // JSONファイルから読み込み
    if (!JsonStageLoader::loadStageFromJSON("../assets/stages/stage1.json", gameState, platformSystem)) {
        printf("ERROR: Failed to load stage1 from JSON\n");
        return;
    }
    printf("Successfully loaded stage1 from JSON\n");
}

void StageManager::generateStage2(GameState& gameState, PlatformSystem& platformSystem) {
    // JSONファイルから読み込み
    if (!JsonStageLoader::loadStageFromJSON("../assets/stages/stage2.json", gameState, platformSystem)) {
        printf("ERROR: Failed to load stage2 from JSON\n");
        return;
    }
    printf("Successfully loaded stage2 from JSON\n");
}

void StageManager::generateStage3(GameState& gameState, PlatformSystem& platformSystem) {
    // JSONファイルから読み込み
    if (!JsonStageLoader::loadStageFromJSON("../assets/stages/stage3.json", gameState, platformSystem)) {
        printf("ERROR: Failed to load stage3 from JSON\n");
        return;
    }
    printf("Successfully loaded stage3 from JSON\n");
}

void StageManager::generateStage4(GameState& gameState, PlatformSystem& platformSystem) {
    // JSONファイルから読み込み
    if (!JsonStageLoader::loadStageFromJSON("../assets/stages/stage4.json", gameState, platformSystem)) {
        printf("ERROR: Failed to load stage4 from JSON\n");
        return;
    }
    printf("Successfully loaded stage4 from JSON\n");
}

void StageManager::generateStage5(GameState& gameState, PlatformSystem& platformSystem) {
    // JSONファイルから読み込み
    if (!JsonStageLoader::loadStageFromJSON("../assets/stages/stage5.json", gameState, platformSystem)) {
        printf("ERROR: Failed to load stage5 from JSON\n");
        return;
    }
    printf("Successfully loaded stage5 from JSON\n");
}

// ステージ選択フィールドの生成
void StageManager::generateStageSelectionField(GameState& gameState, PlatformSystem& platformSystem) {
    platformSystem.clear();
    
    // JSONファイルから読み込み
    if (!JsonStageLoader::loadStageFromJSON("../assets/stages/stage_selection.json", gameState, platformSystem)) {
        printf("ERROR: Failed to load stage selection from JSON\n");
        return;
    }
    printf("Successfully loaded stage selection from JSON\n");
    
    // 以下は元のコード（条件付きで追加の足場を生成する場合に使用）
    /*
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
    StageUtils::createStaticPlatforms(gameState, platformSystem, {
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
    std::vector<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, float, float, std::string>> flyingPlatforms3;
    for (const auto& target : targetPositions) {
        flyingPlatforms3.push_back({
            {target.x, target.y, target.z},  // position
            {3, 1, 3},                       // size
            {target.x, target.y - 1, target.z}, // spawn position
            {target.x, target.y, target.z},  // target position
            20.0f,                           // speed
            2.5f,                            // range
            "右から飛んでくる足場"
        });
    }
    StageUtils::createFlyingPlatforms(gameState, platformSystem, flyingPlatforms3);
    
    StageUtils::createPatrolPlatforms(gameState, platformSystem, patrolPaths, "ステージ選択エリア");
 
    // 可変長サイクリングディスアピアリングプラットフォーム生成（個別配置）    
    StageUtils::createCyclingDisappearingPlatforms(gameState, platformSystem, cycleConfigs);
    
    
    */
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

    // JSONファイルから読み込み
    if (!JsonStageLoader::loadStageFromJSON("../assets/stages/tutorial.json", gameState, platformSystem)) {
        printf("ERROR: Failed to load tutorial from JSON\n");
        return;
    }
    printf("Successfully loaded tutorial from JSON\n");
    
    // JSON読み込み後にチュートリアル開始位置を再設定（復活用）
    gameState.tutorialStartPosition = gameState.playerPosition;
    printf("TUTORIAL: Start position set to (%.1f, %.1f, %.1f)\n", 
           gameState.tutorialStartPosition.x, gameState.tutorialStartPosition.y, gameState.tutorialStartPosition.z);
    
}
