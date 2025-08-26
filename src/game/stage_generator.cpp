#include "stage_generator.h"
#include <iostream>
#include <cmath>

// ヘルパー関数：プラットフォームを追加
template<typename T, typename... Args>
void addPlatform(GameState& gameState, const glm::vec3& pos, const glm::vec3& size, 
                const glm::vec3& color, Args&&... args) {
    T platform(pos, size, color, std::forward<Args>(args)...);
    gameState.platforms.push_back(platform.toGameStatePlatform());
}

void StageGenerator::generateTerrain(GameState& gameState) {
    // 古い設計では何もしない（新しい設計を使用）
    // 重力反転エリアを生成
    generateGravityZones(gameState);
    // スイッチを生成
    generateSwitches(gameState);
    // 大砲を生成
    generateCannons(gameState);
}

void StageGenerator::generateTerrain(GameState& gameState, PlatformSystem& platformSystem) {
    // 新しい設計での全ステージ要素の生成
    generatePlatforms(gameState, platformSystem);
    // 重力反転エリアを生成
    generateGravityZones(gameState);
    // スイッチを生成
    generateSwitches(gameState);
    // 大砲を生成
    generateCannons(gameState);
}



void StageGenerator::generatePlatforms(GameState& gameState, PlatformSystem& platformSystem) {
    // 新しい設計でのプラットフォーム生成
    
    // スタート足場（緑）
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 5, -25), glm::vec3(4, 1, 4), glm::vec3(0.2f, 0.8f, 0.2f)
    ));
    
    // 回転する足場（スタート地点の左）
    platformSystem.addPlatform(GameState::RotatingPlatform(
        glm::vec3(-8, 5, -25), glm::vec3(3, 1, 3), glm::vec3(0.8f, 0.2f, 0.8f),
        glm::vec3(1, 0, 0), 90.0f
    ));
    
    // 第1セクション：基本的なジャンプ
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 5, -20), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.6f, 1.0f)
    ));
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 5, -15), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.6f, 1.0f)
    ));
    
    // 第2セクション：少し高いジャンプ
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 7, -10), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.6f, 1.0f)
    ));
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 7, -5), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.6f, 1.0f)
    ));
    
    // 第4セクション：消える足場
    platformSystem.addPlatform(GameState::DisappearingPlatform(
        glm::vec3(0, 11, 10), glm::vec3(2, 1, 2), glm::vec3(1.0f, 0.2f, 0.2f)
    ));
    platformSystem.addPlatform(GameState::DisappearingPlatform(
        glm::vec3(0, 11, 15), glm::vec3(2, 1, 2), glm::vec3(1.0f, 0.2f, 0.2f)
    ));
    
    // 第5セクション：ジャンプパッド
    platformSystem.addPlatform(GameState::JumpPad(
        glm::vec3(0, 13, 20), glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.0f, 0.8f, 1.0f)
    ));
    
    // ゴール足場（黄色）
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 15, 25), glm::vec3(4, 1, 4), glm::vec3(1.0f, 1.0f, 0.0f)
    ));
    
    // 巡回する足場（スタート地点の右）
    std::vector<glm::vec3> patrolPoints = {
        glm::vec3(8, 5, -25), glm::vec3(12, 7, -20), glm::vec3(8, 9, -15), glm::vec3(4, 7, -20), glm::vec3(8, 5, -25)
    };
    platformSystem.addPlatform(GameState::PatrollingPlatform(
        glm::vec3(8, 5, -25), glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.2f, 0.8f, 0.8f),
        patrolPoints, 2.0f
    ));
    
    // 周期的に消える足場（スタート地点の後ろ）
    platformSystem.addPlatform(GameState::CycleDisappearingPlatform(
        glm::vec3(0, 5, -30), glm::vec3(3, 1, 3), glm::vec3(1.0f, 0.5f, 0.0f),
        4.0f, 2.0f, 1.0f
    ));
    
    // 重力反転エリア内の足場（天井に配置）
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 12, -20), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.8f, 0.2f)
    ));
    platformSystem.addPlatform(GameState::StaticPlatform(
        glm::vec3(0, 15, -18), glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.8f, 0.2f, 0.8f)
    ));
    
    // プレイヤー接近で飛んでくる足場
    platformSystem.addPlatform(GameState::FlyingPlatform(
        glm::vec3(0, 15, -40), glm::vec3(3, 0.5, 3), glm::vec3(0.8f, 0.2f, 0.8f),
        glm::vec3(20, 12, -25), glm::vec3(3.5, 5, -25), 80.0f, 20.0f
    ));
    
    // 順次出現・消失する足場（一直線上に配置）
    for (int i = 0; i < 8; i++) {
        platformSystem.addPlatform(GameState::CycleDisappearingPlatform(
            glm::vec3(0, 5, 5 + i * 4), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.8f, 0.8f),
            8.0f, 4.0f, 0.5f, i * 1.0f
        ));
    }
    
    // テレポート足場
    platformSystem.addPlatform(GameState::TeleportPlatform(
        glm::vec3(10, 5, -15), glm::vec3(3, 1, 3), glm::vec3(1.0f, 0.5f, 0.0f),
        glm::vec3(0, 6, -25)
    ));
    platformSystem.addPlatform(GameState::TeleportPlatform(
        glm::vec3(-10, 8, 10), glm::vec3(3, 1, 3), glm::vec3(0.0f, 0.5f, 1.0f),
        glm::vec3(0, 6, -25)
    ));
    platformSystem.addPlatform(GameState::TeleportPlatform(
        glm::vec3(5, 6.5, -25), glm::vec3(1.5, 0.5, 1.5), glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(15, 5, -10)
    ));
    
    // 移動足場
    platformSystem.addPlatform(GameState::MovingPlatform(
        glm::vec3(-5, 5, -10), glm::vec3(3, 1, 3), glm::vec3(0.8f, 0.4f, 0.8f),
        glm::vec3(5, 8, -10), 2.0f
    ));
    platformSystem.addPlatform(GameState::MovingPlatform(
        glm::vec3(15, 5, -5), glm::vec3(3, 1, 3), glm::vec3(0.4f, 0.8f, 0.4f),
        glm::vec3(25, 5, -5), 2.0f
    ));
    
    printf("New design platforms generated: %zu platforms\n", platformSystem.getPlatforms().size());
}

void StageGenerator::generateGravityZones(GameState& gameState) {
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

void StageGenerator::generateSwitches(GameState& gameState) {
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
    
    printf("Switches generated: %zu switches\n", gameState.switches.size());
}

void StageGenerator::generateCannons(GameState& gameState) {
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

void StageGenerator::initializeEnemies(GameState& gameState) {
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
    
    printf("Enemies initialized: %zu enemies\n", gameState.enemies.size());
}

