#pragma once

#include <glm/glm.hpp>

namespace GameConstants {
    // ウィンドウ設定
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr const char* WINDOW_TITLE = "Vulkan3D - Portal Physics Puzzle";
    
    // フレームレート設定
    constexpr float MAX_DELTA_TIME = 0.1f;  // 最大100ms（10FPS相当）
    constexpr int TARGET_FPS = 60;
    constexpr int FRAME_DELAY_MS = 16;  // ~60 FPS
    
    // カメラ設定
    constexpr float CAMERA_FOV = 45.0f;
    constexpr float CAMERA_NEAR = 0.1f;
    constexpr float CAMERA_FAR = 100.0f;
    
    // ステージ選択フィールド用カメラ
    constexpr glm::vec3 STAGE_SELECT_CAMERA_OFFSET = glm::vec3(0, 15, -15);
    constexpr glm::vec3 NORMAL_STAGE_CAMERA_OFFSET = glm::vec3(0, 2, -8);
    
    // プレイヤー設定
    constexpr glm::vec3 PLAYER_SIZE = glm::vec3(1.0f, 1.0f, 1.0f);
    constexpr float PLAYER_SCALE = 0.5f;
    
    // 物理設定
    constexpr float BASE_GRAVITY = 12.0f;
    constexpr float AIR_RESISTANCE_NORMAL = 0.98f;
    constexpr float AIR_RESISTANCE_FAST = 0.99f;
    
    // ステージ選択エリアの位置定義
    struct StageArea {
        float x, y, z;  // エリアの中心位置
        float height;   // エリアの高さ
    };
    
    constexpr StageArea STAGE_AREAS[] = {
        {6.0f, 0.0f, 0.0f, 1.0f},    // ステージ1
        {-8.0f, 0.0f, 0.0f, 1.0f},   // ステージ2
        {-22.0f, 0.0f, 0.0f, 1.0f},  // ステージ3
        {-34.0f, 0.0f, 0.0f, 1.0f},  // ステージ4
        {-46.0f, 0.0f, 0.0f, 1.0f}   // ステージ5
    };
    
    // ステージ選択エリアの判定範囲
    constexpr float STAGE_SELECTION_RANGE = 0.5f;  // 選択範囲の半径
    constexpr float STAGE_SELECTION_HEIGHT_MIN = 0.5f;  // 選択範囲の高さ下限
    constexpr float STAGE_SELECTION_HEIGHT_MAX = 1.5f;  // 選択範囲の高さ上限
    
    // ステージ解禁条件
    constexpr int STAGE_2_REQUIRED_STARS = 1;
    constexpr int STAGE_3_REQUIRED_STARS = 3;
    constexpr int STAGE_4_REQUIRED_STARS = 5;
    constexpr int STAGE_5_REQUIRED_STARS = 7;
    
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
        
        // 重力エリア
        constexpr glm::vec3 GRAVITY_ZONE_COLOR = glm::vec3(0.2f, 0.6f, 1.0f);
        constexpr float GRAVITY_ZONE_ALPHA = 0.3f;
    }
}
