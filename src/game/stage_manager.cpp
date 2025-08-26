#include "stage_manager.h"
#include "stage_generator.h"
#include <iostream>
#include <algorithm>

StageManager::StageManager() : currentStage(1) {
    initializeStages();
}

void StageManager::initializeStages() {
    stages.clear();
    
    // ステージ1: 基本的なジャンプとプラットフォーム
    stages.push_back({
        1, "基本的なジャンプ",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 16.0f, 25.0f),
        generateStage1,
        true,  // 最初からアンロック
        false
    });
    
    // ステージ2: 重力反転エリア
    stages.push_back({
        2, "重力反転エリア",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 20.0f, 30.0f),
        generateStage2,
        false,  // ステージ1クリア後にアンロック
        false
    });
    
    // ステージ3: スイッチと大砲
    stages.push_back({
        3, "スイッチと大砲",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 18.0f, 35.0f),
        generateStage3,
        false,
        false
    });
    
    // ステージ4: 複雑な移動プラットフォーム
    stages.push_back({
        4, "移動プラットフォーム",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 22.0f, 40.0f),
        generateStage4,
        false,
        false
    });
    
    // ステージ5: 最終ステージ
    stages.push_back({
        5, "最終ステージ",
        glm::vec3(0, 6.0f, -25.0f),
        glm::vec3(0, 25.0f, 45.0f),
        generateStage5,
        false,
        false
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
    gameState.gameWon = false;
    gameState.score = 0;
    gameState.gameTime = 0.0f;
    
    // プラットフォームシステムをクリア
    platformSystem.clear();
    
    // プレイヤーとゴール位置を設定
    gameState.playerPosition = stageIt->playerStartPosition;
    gameState.goalPosition = stageIt->goalPosition;
    gameState.currentStage = stageNumber;
    
    // ステージ生成関数を実行
    stageIt->generateFunction(gameState, platformSystem);
    
    currentStage = stageNumber;
    printf("Loaded stage %d: %s\n", stageNumber, stageIt->stageName.c_str());
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
    if (stageNumber >= 1 && stageNumber <= static_cast<int>(stages.size())) {
        loadStage(stageNumber, gameState, platformSystem);
        return true;
    }
    return false;
}

// ======================================================
// 各ステージの生成関数
// ======================================================

void StageManager::generateStage1(GameState& gameState, PlatformSystem& platformSystem) {
    // ステージ1: 基本的なジャンプとプラットフォーム
    printf("Generating Stage 1: Basic Jumping\n");
    
    // スタート足場（緑）
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 5, -25), glm::vec3(4, 1, 4), glm::vec3(0.2f, 0.8f, 0.2f)
    ));
    
    // 基本的なジャンプセクション
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 5, -15), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.6f, 1.0f)
    ));
    
    // 少し高いジャンプ
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 7, -10), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.6f, 1.0f)
    ));
    
    // ゴール足場（黄色）
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 7, 0), glm::vec3(4, 1, 4), glm::vec3(1.0f, 1.0f, 0.0f)
    ));
    
    printf("Stage 1 generated: %zu platforms\n", platformSystem.getPlatforms().size());
}

void StageManager::generateStage2(GameState& gameState, PlatformSystem& platformSystem) {
    // ステージ2: 重力反転エリア
    printf("Generating Stage 2: Gravity Reversal\n");
    
    // スタート足場
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 5, -25), glm::vec3(4, 1, 4), glm::vec3(0.2f, 0.8f, 0.2f)
    ));
    
    // 循環足場
    std::vector<glm::vec3> patrolPoints = {
        glm::vec3(8, 5, -5), glm::vec3(12, 7, 0), glm::vec3(8, 9, 5), glm::vec3(4, 7, 0), glm::vec3(8, 5, -5)
    };
    platformSystem.addPlatform(GameState::PatrollingPlatform(
        glm::vec3(8, 5, -5), glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.2f, 0.8f, 0.8f),
        patrolPoints, 1.0f
    ));
    
    // 重力反転エリア内の足場（天井に配置）
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 12, -15), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.8f, 0.2f)
    ));
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 15, -10), glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.8f, 0.2f, 0.8f)
    ));
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 18, -5), glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.8f, 0.2f, 0.8f)
    ));
    
    // 重力反転エリアを生成
    gameState.gravityZones.clear();
    GameState::GravityZone gravityZone;
    gravityZone.position = glm::vec3(0, 8, -15);
    gravityZone.size = glm::vec3(8, 2, 8);
    gravityZone.gravityDirection = glm::vec3(0, 1, 0);
    gravityZone.radius = 4.0f;
    gravityZone.isActive = true;
    gameState.gravityZones.push_back(gravityZone);
    
    // ゴール足場
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 20, 30), glm::vec3(4, 1, 4), glm::vec3(1.0f, 1.0f, 0.0f)
    ));
    
    printf("Stage 2 generated: %zu platforms, %zu gravity zones\n", 
           platformSystem.getPlatforms().size(), gameState.gravityZones.size());
}

void StageManager::generateStage3(GameState& gameState, PlatformSystem& platformSystem) {
    // ステージ3: スイッチと大砲
    printf("Generating Stage 3: Switches and Cannons\n");
    
    // スタート足場
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 5, -25), glm::vec3(4, 1, 4), glm::vec3(0.2f, 0.8f, 0.2f)
    ));
    
    // スイッチで操作される足場（最初は非表示）
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 5, -15), glm::vec3(3, 1, 3), glm::vec3(1.0f, 0.2f, 0.2f)
    ));
    
    // スイッチを生成
    gameState.switches.clear();
    GameState::Switch switch1;
    switch1.position = glm::vec3(-1, 6.5, -25);
    switch1.size = glm::vec3(1, 0.5, 1);
    switch1.color = glm::vec3(1.0f, 0.2f, 0.2f);
    switch1.isPressed = false;
    switch1.isToggle = true;
    switch1.targetPlatformIndices = {1};
    switch1.targetStates = {true};
    switch1.pressTimer = 0.0f;
    switch1.cooldownTimer = 0.0f;
    switch1.isMultiSwitch = false;
    switch1.multiSwitchGroup = -1;
    gameState.switches.push_back(switch1);
    
    // 大砲を生成
    gameState.cannons.clear();
    GameState::Cannon cannon1;
    cannon1.position = glm::vec3(5, 5, -10);
    cannon1.size = glm::vec3(2, 2, 2);
    cannon1.color = glm::vec3(0.8f, 0.4f, 0.2f);
    cannon1.targetPosition = glm::vec3(0, 15, 10);
    cannon1.power = 15.0f;
    cannon1.isActive = true;
    cannon1.hasPlayerInside = false;
    cannon1.cooldownTimer = 0.0f;
    cannon1.cooldownTime = 2.0f;
    
    // 発射方向を計算
    glm::vec3 direction = glm::normalize(cannon1.targetPosition - cannon1.position);
    float distance = glm::length(cannon1.targetPosition - cannon1.position);
    float gravity = 9.8f;
    float timeToTarget = sqrt(2.0f * distance / gravity);
    cannon1.launchDirection = direction * cannon1.power;
    cannon1.launchDirection.y = (cannon1.targetPosition.y - cannon1.position.y) / timeToTarget + 0.5f * gravity * timeToTarget;
    
    gameState.cannons.push_back(cannon1);
    
    // ゴール足場
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 18, 35), glm::vec3(4, 1, 4), glm::vec3(1.0f, 1.0f, 0.0f)
    ));
    
    printf("Stage 3 generated: %zu platforms, %zu switches, %zu cannons\n", 
           platformSystem.getPlatforms().size(), gameState.switches.size(), gameState.cannons.size());
}

void StageManager::generateStage4(GameState& gameState, PlatformSystem& platformSystem) {
    // ステージ4: 複雑な移動プラットフォーム
    printf("Generating Stage 4: Moving Platforms\n");
    
    // スタート足場
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 5, -25), glm::vec3(4, 1, 4), glm::vec3(0.2f, 0.8f, 0.2f)
    ));
    
    // 移動足場
    platformSystem.addPlatform(GameState::MovingPlatform(
        glm::vec3(-5, 5, -15), glm::vec3(3, 1, 3), glm::vec3(0.8f, 0.4f, 0.8f),
        glm::vec3(5, 8, -15), 2.0f
    ));
    
    // 回転足場
    platformSystem.addPlatform(GameState::RotatingPlatform(
        glm::vec3(0, 5, -10), glm::vec3(3, 1, 3), glm::vec3(0.8f, 0.2f, 0.8f),
        glm::vec3(1, 0, 0), 90.0f
    ));
    
    // 巡回足場
    std::vector<glm::vec3> patrolPoints = {
        glm::vec3(8, 5, -5), glm::vec3(12, 7, 0), glm::vec3(8, 9, 5), glm::vec3(4, 7, 0), glm::vec3(8, 5, -5)
    };
    platformSystem.addPlatform(GameState::PatrollingPlatform(
        glm::vec3(8, 5, -5), glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.2f, 0.8f, 0.8f),
        patrolPoints, 1.0f
    ));
    
    // 消える足場
    platformSystem.addPlatform(GameState::DisappearingPlatform(
        glm::vec3(0, 11, 10), glm::vec3(2, 1, 2), glm::vec3(1.0f, 0.2f, 0.2f)
    ));
    
    // ジャンプパッド
    platformSystem.addPlatform(GameState::JumpPad(
        glm::vec3(0, 13, 15), glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.0f, 0.8f, 1.0f)
    ));
    
    // ゴール足場
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 22, 40), glm::vec3(4, 1, 4), glm::vec3(1.0f, 1.0f, 0.0f)
    ));
    
    printf("Stage 4 generated: %zu platforms\n", platformSystem.getPlatforms().size());
}

void StageManager::generateStage5(GameState& gameState, PlatformSystem& platformSystem) {
    // ステージ5: 最終ステージ（全ての要素を組み合わせ）
    printf("Generating Stage 5: Final Stage\n");
    
    // スタート足場
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 5, -25), glm::vec3(4, 1, 4), glm::vec3(0.2f, 0.8f, 0.2f)
    ));
    
    // 重力反転エリア
    gameState.gravityZones.clear();
    GameState::GravityZone gravityZone;
    gravityZone.position = glm::vec3(0, 8, -20);
    gravityZone.size = glm::vec3(6, 2, 6);
    gravityZone.gravityDirection = glm::vec3(0, 1, 0);
    gravityZone.radius = 3.0f;
    gravityZone.isActive = true;
    gameState.gravityZones.push_back(gravityZone);
    
    // 重力反転エリア内の足場
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 12, -20), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.8f, 0.2f)
    ));
    
    // 複数スイッチ
    gameState.switches.clear();
    GameState::Switch multiSwitch1;
    multiSwitch1.position = glm::vec3(-1, 6.5, -15);
    multiSwitch1.size = glm::vec3(1, 0.5, 1);
    multiSwitch1.color = glm::vec3(0.2f, 1.0f, 0.2f);
    multiSwitch1.isPressed = false;
    multiSwitch1.isToggle = false;
    multiSwitch1.targetPlatformIndices = {};
    multiSwitch1.targetStates = {};
    multiSwitch1.pressTimer = 0.0f;
    multiSwitch1.cooldownTimer = 0.0f;
    multiSwitch1.isMultiSwitch = true;
    multiSwitch1.multiSwitchGroup = 0;
    gameState.switches.push_back(multiSwitch1);
    
    GameState::Switch multiSwitch2;
    multiSwitch2.position = glm::vec3(-1, 8.5, -10);
    multiSwitch2.size = glm::vec3(1, 0.5, 1);
    multiSwitch2.color = glm::vec3(1.0f, 1.0f, 0.2f);
    multiSwitch2.isPressed = false;
    multiSwitch2.isToggle = false;
    multiSwitch2.targetPlatformIndices = {};
    multiSwitch2.targetStates = {};
    multiSwitch2.pressTimer = 0.0f;
    multiSwitch2.cooldownTimer = 0.0f;
    multiSwitch2.isMultiSwitch = true;
    multiSwitch2.multiSwitchGroup = 0;
    gameState.switches.push_back(multiSwitch2);
    
    // 大砲
    gameState.cannons.clear();
    GameState::Cannon cannon1;
    cannon1.position = glm::vec3(5, 5, -15);
    cannon1.size = glm::vec3(2, 2, 2);
    cannon1.color = glm::vec3(0.8f, 0.4f, 0.2f);
    cannon1.targetPosition = glm::vec3(20, 8, -5);
    cannon1.power = 15.0f;
    cannon1.isActive = true;
    cannon1.hasPlayerInside = false;
    cannon1.cooldownTimer = 0.0f;
    cannon1.cooldownTime = 2.0f;
    
    glm::vec3 direction = glm::normalize(cannon1.targetPosition - cannon1.position);
    float distance = glm::length(cannon1.targetPosition - cannon1.position);
    float gravity = 9.8f;
    float timeToTarget = sqrt(2.0f * distance / gravity);
    cannon1.launchDirection = direction * cannon1.power;
    cannon1.launchDirection.y = (cannon1.targetPosition.y - cannon1.position.y) / timeToTarget + 0.5f * gravity * timeToTarget;
    
    gameState.cannons.push_back(cannon1);
    
    // 複雑な移動要素
    platformSystem.addPlatform(GameState::MovingPlatform(
        glm::vec3(-5, 5, -10), glm::vec3(3, 1, 3), glm::vec3(0.8f, 0.4f, 0.8f),
        glm::vec3(5, 8, -10), 2.0f
    ));
    
    platformSystem.addPlatform(GameState::RotatingPlatform(
        glm::vec3(0, 5, -5), glm::vec3(3, 1, 3), glm::vec3(0.8f, 0.2f, 0.8f),
        glm::vec3(1, 0, 0), 90.0f
    ));
    
    // 順次出現・消失する足場
    for (int i = 0; i < 5; i++) {
        platformSystem.addPlatform(GameState::CycleDisappearingPlatform(
            glm::vec3(0, 5, 5 + i * 4), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.8f, 0.8f),
            8.0f, 4.0f, 0.5f, i * 1.0f
        ));
    }
    
    // ゴール足場
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 25, 45), glm::vec3(4, 1, 4), glm::vec3(1.0f, 1.0f, 0.0f)
    ));
    
    printf("Stage 5 generated: %zu platforms, %zu gravity zones, %zu switches, %zu cannons\n", 
           platformSystem.getPlatforms().size(), gameState.gravityZones.size(), 
           gameState.switches.size(), gameState.cannons.size());
}
