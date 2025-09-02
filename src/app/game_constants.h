#pragma once

#include <GLFW/glfw3.h>
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
    constexpr int STAGE_2_COST = 1;
    constexpr int STAGE_3_COST = 1;
    constexpr int STAGE_4_COST = 4;
    constexpr int STAGE_5_COST = 3;
    
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
    
    // レンダリング設定
    namespace RenderConstants {
        // 背景色
        constexpr glm::vec3 DEFAULT_BACKGROUND_COLOR = glm::vec3(0.2f, 0.3f, 0.3f);
        
        // ステージ別背景色
        constexpr glm::vec3 STAGE_0_TOP_COLOR = glm::vec3(0.5f, 0.7f, 1.0f);     // 青空
        constexpr glm::vec3 STAGE_0_BOTTOM_COLOR = glm::vec3(0.2f, 0.5f, 0.1f);  // 深い緑
        constexpr glm::vec3 STAGE_1_TOP_COLOR = glm::vec3(0.5f, 0.7f, 1.0f);     // 青空
        constexpr glm::vec3 STAGE_1_BOTTOM_COLOR = glm::vec3(0.8f, 0.9f, 1.0f);  // 明るい青
        constexpr glm::vec3 STAGE_2_TOP_COLOR = glm::vec3(1.0f, 0.6f, 0.3f);     // オレンジ
        constexpr glm::vec3 STAGE_2_BOTTOM_COLOR = glm::vec3(1.0f, 0.8f, 0.5f);  // 明るいオレンジ
        constexpr glm::vec3 STAGE_3_TOP_COLOR = glm::vec3(0.1f, 0.1f, 0.3f);     // 深い青
        constexpr glm::vec3 STAGE_3_BOTTOM_COLOR = glm::vec3(0.3f, 0.2f, 0.5f);  // 紫
        constexpr glm::vec3 STAGE_4_TOP_COLOR = glm::vec3(0.2f, 0.2f, 0.3f);     // 暗い青
        constexpr glm::vec3 STAGE_4_BOTTOM_COLOR = glm::vec3(0.4f, 0.4f, 0.5f);  // グレー
        constexpr glm::vec3 STAGE_5_TOP_COLOR = glm::vec3(0.0f, 0.0f, 0.1f);     // 宇宙
        constexpr glm::vec3 STAGE_5_BOTTOM_COLOR = glm::vec3(0.2f, 0.1f, 0.3f);  // 星雲
        
        // ライティング設定
        constexpr glm::vec3 LIGHT_POSITION = glm::vec3(10.0f, 20.0f, 10.0f);
        constexpr glm::vec3 AMBIENT_LIGHT = glm::vec3(0.3f, 0.3f, 0.3f);
        constexpr glm::vec3 DIFFUSE_LIGHT = glm::vec3(0.7f, 0.7f, 0.7f);
        
        // UI設定
        constexpr float CHAR_WIDTH = 8.0f;
        constexpr float CHAR_HEIGHT = 12.0f;
        constexpr float SPACE_WIDTH = 6.0f;
        constexpr float CHAR_SPACING = 2.0f;
        
        // 3D描画設定
        constexpr float CUBE_HALF_SIZE = 0.5f;
        constexpr float LIGHTING_BRIGHTNESS_MULTIPLIER = 1.2f;
        constexpr float LIGHTING_DARKNESS_MULTIPLIER = 0.6f;
        constexpr float LIGHTING_MEDIUM_MULTIPLIER = 0.9f;
        constexpr float EDGE_ALPHA_MULTIPLIER = 0.3f;
        
        // フォント設定
        constexpr int FONT_WIDTH = 8;
        constexpr int FONT_HEIGHT = 12;
    }
    
    // 入力システム設定
    namespace InputConstants {
        // 移動設定
        constexpr float MOVE_SPEED = 6.0f;
        constexpr float MOUSE_SENSITIVITY = 0.1f;
        constexpr float MIN_CAMERA_DISTANCE = 1.0f;
        constexpr float MAX_CAMERA_DISTANCE = 50.0f;
        constexpr float MIN_CAMERA_PITCH = -89.0f;
        constexpr float MAX_CAMERA_PITCH = 89.0f;
        
        // ゲームパッド設定
        constexpr int MAX_GAMEPAD_BUTTONS = 15;
        constexpr int DEFAULT_GAMEPAD_ID = GLFW_JOYSTICK_1;
    }
    
    // 物理システム設定
    namespace PhysicsConstants {
        // 衝突判定設定
        constexpr float COLLISION_TOLERANCE = 0.1f;
        constexpr float PLATFORM_COLLISION_MARGIN = 0.1f;
        constexpr float GRAVITY_DIRECTION_THRESHOLD = 0.5f;
        
        // プレイヤー設定
        constexpr float PLAYER_COLLISION_MARGIN = 0.1f;
        constexpr float PLATFORM_SURFACE_TOLERANCE = 0.1f;
    }
    
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
    
    // 物理計算設定
    namespace PhysicsCalculationConstants {
        // 重力計算設定
        constexpr float GRAVITY_MULTIPLIER_STAGE_0 = 2.0f;
        constexpr float GRAVITY_MULTIPLIER_TIME_SCALE = 1.2f;
        constexpr float GRAVITY_MULTIPLIER_INVERTED = 0.7f;
        constexpr float GRAVITY_MULTIPLIER_BURST_JUMP = 0.5f;
        constexpr float GRAVITY_ACCELERATION = 9.8f;
        
        // 衝突判定設定
        constexpr float PLATFORM_HALF_SIZE_MULTIPLIER = 0.5f;
        constexpr float PLAYER_HALF_SIZE_MULTIPLIER = 0.5f;
        constexpr float PLATFORM_COLLISION_DISTANCE = 0.5f;
        constexpr float PLATFORM_COLLISION_DISTANCE_STRICT = 0.1f;
    }
}
