#pragma once

#include <glm/glm.hpp>

namespace GameConstants {
    // ステージ選択エリアの位置定義
    struct StageArea {
        float x, y, z;
        float height;
    };
    
    constexpr StageArea STAGE_AREAS[] = {
        {8.0f, 1.0f, 4.0f, 1.0f},    // ステージ1
        {23.0f, 4.0f, 23.0f, 1.0f},   // ステージ2
        {-3, 4, 32, 1.0f},  // ステージ3
        {8, 1, 23, 1.0f},  // ステージ4
        {8, 0, 48, 1.0f}   // ステージ5
    };
    
    // ステージ選択エリアの判定範囲
    constexpr float STAGE_SELECTION_RANGE = 0.5f;  // 選択範囲の半径
    
    // ステージ解放に必要な星数
    constexpr int STAGE_1_COST = 1;
    constexpr int STAGE_2_COST = 2;
    constexpr int STAGE_3_COST = 3;
    constexpr int STAGE_4_COST = 3;
    constexpr int STAGE_5_COST = 4;
    
    // 星獲得基準時間
    constexpr float STAR_3_TIME_LONG = 20.0f;
    constexpr float STAR_2_TIME_LONG = 10.0f;
    constexpr float STAR_3_TIME_SHORT = 10.0f;
    constexpr float STAR_2_TIME_SHORT = 5.0f;
    constexpr float LONG_TIME_THRESHOLD = 30.0f;
    
    // ステージ制限時間
    constexpr float STAGE_0_TIME_LIMIT = 999.0f;  // ステージ選択フィールド（制限なし）
    constexpr float STAGE_1_TIME_LIMIT = 20.0f;   // ステージ1
    constexpr float STAGE_2_TIME_LIMIT = 40.0f;   // ステージ2
    constexpr float STAGE_3_TIME_LIMIT = 70.0f;   // ステージ3
    constexpr float STAGE_4_TIME_LIMIT = 90.0f;   // ステージ4
    constexpr float STAGE_5_TIME_LIMIT = 120.0f;  // ステージ5
    
    // モード別時間ボーナス
    constexpr float FIRST_PERSON_TIME_BONUS = 20.0f;  // 1人称モードボーナス
    constexpr float EASY_MODE_TIME_BONUS = 20.0f;     // お助けモードボーナス
    
    // ステージ生成設定
    namespace StageConstants {
        // アイテム設定
        constexpr glm::vec3 ITEM_SIZE = glm::vec3(0.5f, 0.5f, 0.5f);
        constexpr float ITEM_BOB_HEIGHT = 1.0f;
        constexpr float ITEM_ROTATION_SPEED = 2.0f;
        
        // プラットフォーム設定
        constexpr glm::vec3 DEFAULT_PLATFORM_SIZE = glm::vec3(3.0f, 1.0f, 3.0f);
        constexpr glm::vec3 SMALL_PLATFORM_SIZE = glm::vec3(2.0f, 1.0f, 2.0f);
        constexpr glm::vec3 LARGE_PLATFORM_SIZE = glm::vec3(4.0f, 1.0f, 4.0f);
        constexpr float PLATFORM_SPEED = 2.0f;
        
        // サイクリングプラットフォーム設定
        constexpr float CYCLE_VISIBLE_TIME = 6.0f;
        constexpr float CYCLE_INVISIBLE_TIME = 4.0f;
        constexpr float CYCLE_INITIAL_TIMER = 1.0f;
        
        // 重力エリア設定
        constexpr float GRAVITY_ZONE_RADIUS = 3.0f;
        
        // スイッチ設定
        constexpr float SWITCH_COOLDOWN_TIME = 0.5f;
        
        // 大砲設定
        constexpr float CANNON_POWER_1 = 15.0f;
        constexpr float CANNON_POWER_2 = 12.0f;
        constexpr float CANNON_COOLDOWN_1 = 2.0f;
        constexpr float CANNON_COOLDOWN_2 = 3.0f;
        constexpr float CANNON_SIZE = 2.0f;
        
        // 敵設定
        constexpr float ENEMY_HEALTH = 1.0f;
        constexpr float WADDLE_DEE_SIZE = 0.5f;
        constexpr float ENEMY_DETECTION_RANGE = 6.0f;
        
        // ステージ座標設定
        constexpr glm::vec3 STAGE_0_SPAWN = glm::vec3(8.0f, 2.0f, 0.0f);
        constexpr glm::vec3 STAGE_1_SPAWN = glm::vec3(0.0f, 6.0f, -25.0f);
        constexpr glm::vec3 STAGE_1_GOAL = glm::vec3(0.0f, 16.0f, 25.0f);
        constexpr glm::vec3 STAGE_2_SPAWN = glm::vec3(0.0f, 6.0f, -25.0f);
        constexpr glm::vec3 STAGE_2_GOAL = glm::vec3(0.0f, 20.0f, 30.0f);
        constexpr glm::vec3 STAGE_3_SPAWN = glm::vec3(0.0f, 6.0f, -25.0f);
        constexpr glm::vec3 STAGE_3_GOAL = glm::vec3(0.0f, 18.0f, 35.0f);
        constexpr glm::vec3 STAGE_4_SPAWN = glm::vec3(0.0f, 6.0f, -25.0f);
        constexpr glm::vec3 STAGE_4_GOAL = glm::vec3(0.0f, 22.0f, 40.0f);
        constexpr glm::vec3 STAGE_5_SPAWN = glm::vec3(0.0f, 6.0f, -25.0f);
        constexpr glm::vec3 STAGE_5_GOAL = glm::vec3(0.0f, 25.0f, 45.0f);
    }
}
