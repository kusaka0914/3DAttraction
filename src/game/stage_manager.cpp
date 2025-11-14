#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "stage_manager.h"
#include "json_stage_loader.h"
#include "../core/constants/game_constants.h"
#include "../core/constants/debug_config.h"
#include "../core/utils/stage_utils.h"
#include "../core/utils/resource_path.h"
#include "../core/error_handler.h"
#include <iostream>
#include <algorithm>
#include <tuple>
#include <fstream>
#ifdef _WIN32
    #include <sys/stat.h>
    #include <io.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace Colors = GameConstants::Colors;

StageManager::StageManager() : currentStage(1), lastFileModificationTime(0) {
    initializeStages();
    
    stageFilePaths[0] = "../assets/stages/stage_selection.json";
    stageFilePaths[1] = "../assets/stages/stage1.json";
    stageFilePaths[2] = "../assets/stages/stage2.json";
    stageFilePaths[3] = "../assets/stages/stage3.json";
    stageFilePaths[4] = "../assets/stages/stage4.json";
    stageFilePaths[5] = "../assets/stages/stage5.json";
    stageFilePaths[6] = "../assets/stages/tutorial.json";
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
        ErrorHandler::logErrorFormat("Invalid stage number %d (valid range: 0-%zu)", stageNumber, stages.size() - 1);
        return false;
    }
    
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt == stages.end()) {
        ErrorHandler::logErrorFormat("Stage %d not found in stages vector", stageNumber);
        return false;
    }
    
    if (!stageIt->isUnlocked) {
        ErrorHandler::logErrorFormat("Stage %d is not unlocked yet", stageNumber);
        return false;
    }
    
    gameState.platforms.clear();
    gameState.gravityZones.clear();
    gameState.switches.clear();
    gameState.cannons.clear();
    gameState.items.items.clear();
    gameState.items.collectedItems = 0;
    gameState.progress.gameWon = false;
    
    gameState.progress.timeLimit = stageIt->timeLimit;
    gameState.progress.remainingTime = gameState.progress.timeLimit;    // 残り時間を設定
    printf("Time limit: %.1f seconds\n", gameState.progress.timeLimit);
    gameState.progress.earnedStars = 0;          // 星をリセット
    // リプレイモードの場合はclearTimeをリセットしない（リプレイ開始時に設定される）
    if (!gameState.replay.isReplayMode) {
        gameState.progress.clearTime = 0.0f;         // クリア時間をリセット
        DEBUG_PRINTF("DEBUG: clearTime reset to 0.0f for stage %d\n", stageNumber);
    }
    gameState.progress.isTimeUp = false;         // 時間切れフラグをリセット
    gameState.progress.isStageCompleted = false; // ステージ完了フラグをリセット
    gameState.ui.showStageClearUI = false; // UIをリセット
    gameState.ui.stageClearTimer = 0.0f;   // タイマーをリセット
    gameState.ui.stageClearConfirmed = false; // 確認フラグをリセット
    gameState.progress.isGoalReached = false;    // ゴール後の移動制限をリセット
    gameState.progress.isGameOver = false;       // ゲームオーバーフラグをリセット
    gameState.progress.isTimeUp = false;         // 時間切れフラグをリセット
    gameState.ui.readyScreenShown = false; // Ready画面表示フラグをリセット
    
    gameState.skills.doubleJumpRemainingUses = gameState.skills.doubleJumpMaxUses;
    gameState.skills.heartFeelRemainingUses = gameState.skills.heartFeelMaxUses;
    gameState.skills.freeCameraRemainingUses = gameState.skills.freeCameraMaxUses;
    gameState.skills.burstJumpRemainingUses = gameState.skills.burstJumpMaxUses;
    gameState.skills.timeStopRemainingUses = gameState.skills.timeStopMaxUses;

    gameState.camera.isFirstPersonMode = false;
    gameState.camera.isFirstPersonView = false;        
    
    gameState.skills.isFreeCameraActive = false;
    gameState.skills.freeCameraTimer = 0.0f;
    gameState.skills.isBurstJumpActive = false;
    gameState.skills.hasUsedBurstJump = false;
    gameState.skills.isInBurstJumpAir = false;
    gameState.skills.burstJumpDelayTimer = 0.0f;
    gameState.skills.isTimeStopped = false;
    gameState.skills.timeStopTimer = 0.0f;
    
    gameState.progress.isTutorialStage = false;
    gameState.progress.tutorialStep = 0;
    gameState.progress.tutorialStepCompleted = false;
    gameState.ui.showTutorialUI = false;
    
    if (stageNumber == 6) {
        gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
    }
    
    gameState.player.lastCheckpoint = stageIt->playerStartPosition;
    gameState.player.lastCheckpointItemId = -1;
    
    platformSystem.clear();
    
    gameState.player.position = stageIt->playerStartPosition;
    gameState.progress.goalPosition = stageIt->goalPosition;
    gameState.progress.currentStage = stageNumber;
    
    stageIt->generateFunction(gameState, platformSystem);
    
    if (stageNumber != 0 && stageNumber != 6 && gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::MAX_SPEED_STAR) {
        gameState.progress.timeScale = 3.0f;
        gameState.progress.timeScaleLevel = 2;  // 3倍に設定
    }
    
    updateCurrentStageFileInfo(stageNumber);
    
    currentStage = stageNumber;
    return true;
}

bool StageManager::unlockStage(int stageNumber, GameState* gameState) {
    if (stageNumber < 0 || stageNumber >= static_cast<int>(stages.size())) {
        ErrorHandler::logErrorFormat("Invalid stage number %d for unlock (valid range: 0-%zu)", stageNumber, stages.size() - 1);
        return false;
    }
    
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt == stages.end()) {
        ErrorHandler::logErrorFormat("Stage %d not found for unlock", stageNumber);
        return false;
    }
    
    if (stageIt->isUnlocked) {
        printf("WARNING: Stage %d is already unlocked\n", stageNumber);
        return true; // 既にアンロック済みでも成功とする
    }
    
    stageIt->isUnlocked = true;
    
    if (gameState != nullptr) {
        gameState->progress.unlockedStages[stageNumber] = true;
    }
    
    printf("Stage %d unlocked successfully\n", stageNumber);
    return true;
}

bool StageManager::unlockStageWithStars(int stageNumber, int requiredStars, GameState& gameState) {
    if (stageNumber < 0 || stageNumber >= static_cast<int>(stages.size())) {
        ErrorHandler::logErrorFormat("Invalid stage number %d for star unlock (valid range: 0-%zu)", stageNumber, stages.size() - 1);
        return false;
    }
    
    if (requiredStars < 0) {
        ErrorHandler::logErrorFormat("Invalid required stars %d (must be >= 0)", requiredStars);
        return false;
    }
    
    if (gameState.progress.totalStars < requiredStars) {
        ErrorHandler::logErrorFormat("Insufficient stars. Required: %d, Available: %d", requiredStars, gameState.progress.totalStars);
        return false;
    }
    
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt == stages.end()) {
        ErrorHandler::logErrorFormat("Stage %d not found for star unlock", stageNumber);
        return false;
    }
    
    if (stageIt->isUnlocked) {
        printf("WARNING: Stage %d is already unlocked\n", stageNumber);
        return true; // 既にアンロック済みでも成功とする
    }
    
    gameState.progress.totalStars -= requiredStars;
    stageIt->isUnlocked = true;
    gameState.progress.unlockedStages[stageNumber] = true;
    
    printf("Stage %d unlocked with %d stars! Remaining stars: %d\n", 
           stageNumber, requiredStars, gameState.progress.totalStars);
    return true;
}

void StageManager::completeStage(int stageNumber) {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt != stages.end()) {
        stageIt->isCompleted = true;
        
        if (stageNumber + 1 < static_cast<int>(stages.size())) {
            printf("Stage %d completed, attempting to unlock next stage %d\n", stageNumber, stageNumber + 1);
        }
        
        printf("Stage %d completed successfully\n", stageNumber);
    } else {
        ErrorHandler::logErrorFormat("Stage %d not found for completion", stageNumber);
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
    if (stageNumber < 0 || stageNumber >= static_cast<int>(stages.size())) {
        ErrorHandler::logErrorFormat("Invalid stage number %d for goToStage (valid range: 0-%zu)", stageNumber, stages.size() - 1);
        return false;
    }
    
    return loadStage(stageNumber, gameState, platformSystem);
}


void StageManager::generateStage1(GameState& gameState, PlatformSystem& platformSystem) {
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage1.json"), gameState, platformSystem)) {
        ErrorHandler::logError("Failed to load stage1 from JSON");
        return;
    }
    printf("Successfully loaded stage1 from JSON\n");
}

void StageManager::generateStage2(GameState& gameState, PlatformSystem& platformSystem) {
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage2.json"), gameState, platformSystem)) {
        ErrorHandler::logError("Failed to load stage2 from JSON");
        return;
    }
    printf("Successfully loaded stage2 from JSON\n");
}

void StageManager::generateStage3(GameState& gameState, PlatformSystem& platformSystem) {
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage3.json"), gameState, platformSystem)) {
        ErrorHandler::logError("Failed to load stage3 from JSON");
        return;
    }
    printf("Successfully loaded stage3 from JSON\n");
}

void StageManager::generateStage4(GameState& gameState, PlatformSystem& platformSystem) {
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage4.json"), gameState, platformSystem)) {
        ErrorHandler::logError("Failed to load stage4 from JSON");
        return;
    }
    printf("Successfully loaded stage4 from JSON\n");
}

void StageManager::generateStage5(GameState& gameState, PlatformSystem& platformSystem) {
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage5.json"), gameState, platformSystem)) {
        ErrorHandler::logError("Failed to load stage5 from JSON");
        return;
    }
    printf("Successfully loaded stage5 from JSON\n");
}

void StageManager::generateStageSelectionField(GameState& gameState, PlatformSystem& platformSystem) {
    platformSystem.clear();
    
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage_selection.json"), gameState, platformSystem)) {
        ErrorHandler::logError("Failed to load stage selection from JSON");
        return;
    }
    printf("Successfully loaded stage selection from JSON\n");
    
    /*
    std::vector<std::vector<glm::vec3>> patrolPaths={
        {{14, 0, 10}, {18, 0, 10}, {18, 0, 10}, {18, 0, 10}, {14, 0, 10}},
        {{22, 0, 14}, {22, 0, 10}, {22, 0, 10}, {22, 0, 10}, {22, 0, 14}},
        {{22, 0, 18}, {22, 4, 18}, {22, 4, 18}, {22, 4, 18}, {22, 0, 18}},
    };
    if (gameState.progress.stageStars.count(2) && gameState.progress.stageStars.at(2) > 0) {
        patrolPaths.push_back({{18, 4, 22}, {14, 4, 22}, {14, 4, 22}, {14, 4, 22}, {18, 4, 22}});
        patrolPaths.push_back({ {8, 0, 28}, {8, 0, 32}, {8, 0, 32}, {8, 0, 32}, {8, 0, 28} });
    }
    std::vector<glm::vec3> targetPositions = {
        {8, 0, 10},
        {8, 0, 13},
        {8, 0, 16},
        {8, 0, 19}
    };
    if ((gameState.progress.stageStars.count(4) && gameState.progress.stageStars.at(4) > 0)) {
        targetPositions.push_back({8, 0, 39});
    }

    std::vector<CyclingDisappearingConfig> cycleConfigs;
    cycleConfigs.push_back({ {1, 0, 11},  {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-3, 0, 15}, {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-7, 1, 23}, {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-3, 2, 23}, {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    cycleConfigs.push_back({ {-3, 3, 27}, {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
   if (gameState.progress.stageStars.count(3) && gameState.progress.stageStars.at(3) > 0) {
        cycleConfigs.push_back({ {1, 3, 23},  {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
        cycleConfigs.push_back({ {8, 0, 36},  {3, 1, 3}, Colors::ORANGE, 8.0f, 6.0f, 1.0f, "サイクル消える足場1" });
    }
    
    StageUtils::createStaticPlatforms(gameState, platformSystem, {
        {{GameConstants::STAGE_AREAS[0].x, GameConstants::STAGE_AREAS[0].y - 1, GameConstants::STAGE_AREAS[0].z}, {10, 1, 10}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Right"},

        {{GameConstants::STAGE_AREAS[0].x, GameConstants::STAGE_AREAS[0].y, GameConstants::STAGE_AREAS[0].z}, {1, 1, 1}, gameState.progress.unlockedStages[1] ? glm::vec3(0.2f, 1.0f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), "Stage 1 Selection Area"},

        
        {{GameConstants::STAGE_AREAS[1].x, GameConstants::STAGE_AREAS[1].y - 1, GameConstants::STAGE_AREAS[1].z}, {6, 1, 6}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Left"},
        
        {{GameConstants::STAGE_AREAS[1].x, GameConstants::STAGE_AREAS[1].y, GameConstants::STAGE_AREAS[1].z}, {1, 1, 1}, gameState.progress.unlockedStages[2] ? glm::vec3(0.2f, 1.0f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), "Stage 2 Selection Area"},
        {{-7, 0, 19}, {3, 1, 3}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Left"},
        {{GameConstants::STAGE_AREAS[2].x, GameConstants::STAGE_AREAS[2].y - 1, GameConstants::STAGE_AREAS[2].z}, {6, 1, 6}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Left"},

        {{GameConstants::STAGE_AREAS[2].x, GameConstants::STAGE_AREAS[2].y, GameConstants::STAGE_AREAS[2].z}, {1, 1, 1}, gameState.progress.unlockedStages[3] ? glm::vec3(0.2f, 1.0f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), "Stage 3 Selection Area"},

        {{GameConstants::STAGE_AREAS[3].x, GameConstants::STAGE_AREAS[3].y - 1, GameConstants::STAGE_AREAS[3].z}, {6, 1, 6}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Left"},
        
        {{GameConstants::STAGE_AREAS[3].x, GameConstants::STAGE_AREAS[3].y, GameConstants::STAGE_AREAS[3].z}, {1, 1, 1}, gameState.progress.unlockedStages[4] ? glm::vec3(0.2f, 1.0f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), "Stage 4 Selection Area"},
        
        {{GameConstants::STAGE_AREAS[4].x, GameConstants::STAGE_AREAS[4].y - 1, GameConstants::STAGE_AREAS[4].z}, {6, 1, 6}, glm::vec3(0.3f, 0.3f, 0.3f), "Main Field Left"},

        {{GameConstants::STAGE_AREAS[4].x, GameConstants::STAGE_AREAS[4].y, GameConstants::STAGE_AREAS[4].z}, {1, 1, 1}, gameState.progress.unlockedStages[5] ? glm::vec3(0.2f, 1.0f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), "Stage 5 Selection Area"},
    });

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
 
    StageUtils::createCyclingDisappearingPlatforms(gameState, platformSystem, cycleConfigs);
    
    
    */
}

int StageManager::getStageStars(int stageNumber) const {
    return 0;
}

int StageManager::getTotalStars() const {
    return 0;
}

void StageManager::updateStageStars(int stageNumber, int newStars) {
    printf("Stage %d stars updated to %d\n", stageNumber, newStars);
}

int StageManager::calculateStarDifference(int stageNumber, int newStars) const {
    return newStars;
}

void StageManager::generateTutorialStage(GameState& gameState, PlatformSystem& platformSystem) {

    gameState.progress.isTutorialStage = true;
    gameState.progress.tutorialStep = 0;
    gameState.progress.tutorialStepCompleted = false;
    gameState.progress.tutorialStartPosition = gameState.player.position;
    gameState.progress.tutorialRequiredDistance = 5.0f;  // 移動距離を小さくする
    gameState.ui.showTutorialUI = true;
    gameState.progress.tutorialInputEnabled = true;
    gameState.ui.tutorialMessage = "MOVING FORWARD: PRESS W";
    
    gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
    
    gameState.items.earnedItems = 0;
    gameState.items.totalItems = 3;

    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/tutorial.json"), gameState, platformSystem)) {
        ErrorHandler::logError("Failed to load tutorial from JSON");
        return;
    }
    printf("Successfully loaded tutorial from JSON\n");
    
    gameState.progress.tutorialStartPosition = gameState.player.position;
    printf("TUTORIAL: Start position set to (%.1f, %.1f, %.1f)\n", 
           gameState.progress.tutorialStartPosition.x, gameState.progress.tutorialStartPosition.y, gameState.progress.tutorialStartPosition.z);
    
}


std::time_t StageManager::getFileModificationTime(const std::string& filepath) {
#ifdef _WIN32
    struct _stat fileInfo;
    if (_stat(filepath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
    std::string altPath = filepath;
    if (altPath.find("../") == 0) {
        altPath = altPath.substr(3);
    } else if (altPath.find("assets/") == 0) {
        altPath = "../" + altPath;
    }
    if (_stat(altPath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
#else
    struct stat fileInfo;
    if (stat(filepath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
    std::string altPath = filepath;
    if (altPath.find("../") == 0) {
        altPath = altPath.substr(3);
    } else if (altPath.find("assets/") == 0) {
        altPath = "../" + altPath;
    }
    if (stat(altPath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
#endif
    return 0;
}

std::string StageManager::getStageFilePath(int stageNumber) {
    auto it = stageFilePaths.find(stageNumber);
    if (it != stageFilePaths.end()) {
        std::ifstream file(it->second);
        if (file.good()) {
            file.close();
            return it->second;
        }
        file.close();
    }
    
    std::vector<std::string> alternativePaths;
    switch (stageNumber) {
        case 0: alternativePaths = {"assets/stages/stage_selection.json", "../assets/stages/stage_selection.json"}; break;
        case 1: alternativePaths = {"assets/stages/stage1.json", "../assets/stages/stage1.json"}; break;
        case 2: alternativePaths = {"assets/stages/stage2.json", "../assets/stages/stage2.json"}; break;
        case 3: alternativePaths = {"assets/stages/stage3.json", "../assets/stages/stage3.json"}; break;
        case 4: alternativePaths = {"assets/stages/stage4.json", "../assets/stages/stage4.json"}; break;
        case 5: alternativePaths = {"assets/stages/stage5.json", "../assets/stages/stage5.json"}; break;
        case 6: alternativePaths = {"assets/stages/tutorial.json", "../assets/stages/tutorial.json"}; break;
        default: return "";
    }
    
    for (const auto& path : alternativePaths) {
        std::ifstream file(path);
        if (file.good()) {
            file.close();
            return path;
        }
        file.close();
    }
    
    return "";
}

void StageManager::updateCurrentStageFileInfo(int stageNumber) {
    currentStageFilePath = getStageFilePath(stageNumber);
    if (!currentStageFilePath.empty()) {
        lastFileModificationTime = getFileModificationTime(currentStageFilePath);
    } else {
        currentStageFilePath = "";
        lastFileModificationTime = 0;
    }
}

bool StageManager::checkAndReloadStage(GameState& gameState, PlatformSystem& platformSystem) {
    if (currentStageFilePath.empty()) {
        return false;
    }
    
    std::string actualPath = getStageFilePath(currentStage);
    if (actualPath != currentStageFilePath) {
        updateCurrentStageFileInfo(currentStage);
        return false;
    }
    
    std::time_t currentModTime = getFileModificationTime(currentStageFilePath);
    
    if (currentModTime > 0 && currentModTime != lastFileModificationTime && lastFileModificationTime > 0) {
        printf("Stage file changed! Reloading stage %d...\n", currentStage);
        lastFileModificationTime = currentModTime;
        
        return loadStage(currentStage, gameState, platformSystem);
    }
    
    if (lastFileModificationTime == 0 && currentModTime > 0) {
        lastFileModificationTime = currentModTime;
    }
    
    return false;
}
