/**
 * @file color_constants.h
 * @brief 色定数
 * @details ゲームで使用する色の定数を定義します。
 */
#pragma once

#include <glm/glm.hpp>

namespace GameConstants {
    // 色の定数定義
    namespace Colors {
        constexpr glm::vec3 RED = glm::vec3(1.0f, 0.0f, 0.0f);
        constexpr glm::vec3 GREEN = glm::vec3(0.0f, 1.0f, 0.0f);
        constexpr glm::vec3 BLUE = glm::vec3(0.0f, 0.0f, 1.0f);
        constexpr glm::vec3 YELLOW = glm::vec3(1.0f, 1.0f, 0.0f);
        constexpr glm::vec3 CYAN = glm::vec3(0.0f, 1.0f, 1.0f);
        constexpr glm::vec3 MAGENTA = glm::vec3(1.0f, 0.0f, 1.0f);
        constexpr glm::vec3 ORANGE = glm::vec3(1.0f, 0.5f, 0.0f);
        constexpr glm::vec3 PURPLE = glm::vec3(0.5f, 0.0f, 1.0f);
        constexpr glm::vec3 PINK = glm::vec3(1.0f, 0.0f, 0.5f);
        constexpr glm::vec3 LIME = glm::vec3(0.5f, 1.0f, 0.0f);
        constexpr glm::vec3 BROWN = glm::vec3(0.6f, 0.4f, 0.2f);
        constexpr glm::vec3 GRAY = glm::vec3(0.5f, 0.5f, 0.5f);
        constexpr glm::vec3 WHITE = glm::vec3(1.0f, 1.0f, 1.0f);
        constexpr glm::vec3 BLACK = glm::vec3(0.0f, 0.0f, 0.0f);
        
        // プレイヤー関連
        constexpr glm::vec3 PLAYER_COLOR = glm::vec3(1.0f, 0.8f, 0.9f);  // ピンク色
        constexpr glm::vec3 GOAL_COLOR = glm::vec3(1.0f, 1.0f, 0.0f);    // 黄色（ゴール）
        
        // UI関連
        constexpr glm::vec3 UI_TEXT_COLOR = glm::vec3(1.0f, 1.0f, 1.0f);
        constexpr glm::vec3 UI_WARNING_COLOR = glm::vec3(1.0f, 0.5f, 0.0f);
        constexpr glm::vec3 UI_ERROR_COLOR = glm::vec3(1.0f, 0.0f, 0.0f);
        constexpr glm::vec3 UI_SUCCESS_COLOR = glm::vec3(0.0f, 1.0f, 0.0f);
        constexpr glm::vec3 UI_DISABLED_COLOR = glm::vec3(0.5f, 0.5f, 0.5f);
        constexpr glm::vec3 UI_ACCENT_COLOR = glm::vec3(0.2f, 0.4f, 1.0f);
        constexpr glm::vec3 UI_ACTIVE_COLOR = glm::vec3(0.5f, 0.5f, 1.0f);
        
        // 重力エリア
        constexpr glm::vec3 GRAVITY_ZONE_COLOR = glm::vec3(0.2f, 0.6f, 1.0f);
        constexpr float GRAVITY_ZONE_ALPHA = 0.3f;
        
        // 装飾オブジェクト色
        constexpr glm::vec3 FLOWER_RED = glm::vec3(1.0f, 0.3f, 0.3f);
        constexpr glm::vec3 FLOWER_YELLOW = glm::vec3(1.0f, 0.8f, 0.2f);
        constexpr glm::vec3 FLOWER_PINK = glm::vec3(0.8f, 0.2f, 0.8f);
        constexpr glm::vec3 FLOWER_WHITE = glm::vec3(1.0f, 1.0f, 1.0f);
        constexpr glm::vec3 TREE_TRUNK = glm::vec3(0.4f, 0.2f, 0.1f);
        constexpr glm::vec3 TREE_LEAVES = glm::vec3(0.2f, 0.5f, 0.1f);
        constexpr glm::vec3 HILL_COLOR = glm::vec3(0.3f, 0.6f, 0.2f);
        constexpr glm::vec3 CLOUD_COLOR = glm::vec3(1.0f, 1.0f, 1.0f);
        constexpr glm::vec3 SUNSET_COLOR = glm::vec3(1.0f, 0.8f, 0.3f);
        
        // ライフ・スター色
        constexpr glm::vec3 LIFE_ACTIVE = glm::vec3(1.0f, 0.3f, 0.3f);
        constexpr glm::vec3 LIFE_INACTIVE = glm::vec3(0.3f, 0.3f, 0.3f);
        constexpr glm::vec3 STAR_ACTIVE = glm::vec3(1.0f, 1.0f, 0.0f);
        constexpr glm::vec3 STAR_INACTIVE = glm::vec3(0.5f, 0.5f, 0.5f);
        
        // UIレイアウト定数
        namespace UILayout {
            // ハートUIの位置
            constexpr float HEART_START_X = 200.0f;
            constexpr float HEART_Y = 45.0f;
            constexpr float HEART_SPACING = 40.0f;
            constexpr float HEART_SCALE = 1.0f;
            
            // 時間UIの位置
            constexpr float TIME_UI_X = 1170.0f;
            constexpr float TIME_UI_Y = 30.0f;
            constexpr float TIME_UI_SCALE = 3.0f;
            
            // ゴールUIの位置
            constexpr float GOAL_UI_X = 962.0f;
            constexpr float GOAL_UI_Y = 65.0f;
            constexpr float GOAL_TIME_5S_X = 1040.0f;
            constexpr float GOAL_TIME_10S_X = 1110.0f;
            constexpr float GOAL_TIME_20S_X = 1110.0f;
            constexpr float GOAL_UI_SCALE = 1.0f;
            
            // 星UIの位置
            constexpr float STARS_START_X = 980.0f;
            constexpr float STARS_Y = 40.0f;
            constexpr float STARS_SPACING = 70.0f;
            constexpr float STARS_SCALE = 1.5f;
            
            // スキルUIの位置
            constexpr float SKILL_START_Y = 650.0f;
            constexpr float SKILL_COUNT_Y = 600.0f;
            constexpr float SKILL_INSTRUCTION_Y = 680.0f;
            constexpr float SKILL_SCALE = 1.0f;
            constexpr float SKILL_COUNT_SCALE = 3.0f;
            constexpr float SKILL_INSTRUCTION_SCALE = 1.2f;
            constexpr float SKILL_ACTIVE_SCALE = 2.0f;
            
            // スキル間隔
            constexpr float SKILL_SPACING = 200.0f;
            
            // 説明テキストの位置
            constexpr float EXPLANATION_CENTER_X = 640.0f;
            constexpr float EXPLANATION_START_Y = 100.0f;
            constexpr float EXPLANATION_LINE_SPACING = 40.0f;
            constexpr float EXPLANATION_SCALE = 1.2f;

            // スキルUIのX座標
            constexpr float TIME_STOP_SKILL_X = 30.0f;
            constexpr float DOUBLE_JUMP_SKILL_X = 230.0f;
            constexpr float HEART_FEEL_SKILL_X = 460.0f;
            constexpr float FREE_CAMERA_SKILL_X = 850.0f;
            constexpr float BURST_JUMP_SKILL_X = 650.0f;
            
            // スキルUIのオフセット値
            constexpr float TIME_STOP_COUNT_OFFSET = 50.0f;
            constexpr float TIME_STOP_INSTRUCTION_OFFSET = 20.0f;
            constexpr float DOUBLE_JUMP_COUNT_OFFSET = 70.0f;
            constexpr float DOUBLE_JUMP_INSTRUCTION_OFFSET = -25.0f;
            constexpr float HEART_FEEL_COUNT_OFFSET = 60.0f;
            constexpr float HEART_FEEL_INSTRUCTION_OFFSET = 30.0f;
            constexpr float FREE_CAMERA_COUNT_OFFSET = 60.0f;
            constexpr float FREE_CAMERA_INSTRUCTION_OFFSET = 40.0f;
            constexpr float BURST_JUMP_COUNT_OFFSET = 50.0f;
            constexpr float BURST_JUMP_INSTRUCTION_OFFSET = 25.0f;
            
            // バーストジャンプのアクティブ表示位置
            constexpr float BURST_JUMP_ACTIVE_X = 550.0f;
            constexpr float BURST_JUMP_ACTIVE_Y = 490.0f;
        }
    }
}
