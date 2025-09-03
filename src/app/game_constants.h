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
        
        // 背景レイアウト設定
        namespace BackgroundLayout {
            // 画面サイズ
            constexpr float SCREEN_WIDTH = 1280.0f;
            constexpr float SCREEN_HEIGHT = 720.0f;
            constexpr float HALF_SCREEN_HEIGHT = 360.0f;
            
            // 雲の設定
            constexpr int CLOUD_COUNT_STAGE_0 = 4;
            constexpr int CLOUD_COUNT_STAGE_1 = 5;
            constexpr int CLOUD_COUNT_STAGE_4 = 3;
            constexpr float CLOUD_BASE_SPACING = 350.0f;
            constexpr float CLOUD_SPACING_OFFSET = 30.0f;
            constexpr float CLOUD_BASE_Y = 80.0f;
            constexpr float CLOUD_Y_OFFSET = 25.0f;
            constexpr float CLOUD_BASE_SIZE = 70.0f;
            constexpr float CLOUD_SIZE_OFFSET = 15.0f;
            constexpr float CLOUD_SEGMENTS = 12.0f;
            constexpr float CLOUD_ASPECT_RATIO = 0.6f;
            
            // 草の設定
            constexpr int GRASS_COUNT = 100;
            constexpr float GRASS_SPACING = 12.0f;
            constexpr float GRASS_SPACING_OFFSET = 7.0f;
            constexpr float GRASS_BASE_Y_OFFSET = 8.0f;
            constexpr float GRASS_Y_OFFSET = 13.0f;
            constexpr float GRASS_HEIGHT_BASE = 15.0f;
            constexpr float GRASS_HEIGHT_VARIATION = 3.0f;
            constexpr float GRASS_WIDTH_BASE = 2.0f;
            constexpr int GRASS_WIDTH_VARIATION = 3;
            
            // 花の設定
            constexpr int FLOWER_COUNT = 20;
            constexpr float FLOWER_SPACING = 60.0f;
            constexpr float FLOWER_SPACING_OFFSET = 23.0f;
            constexpr float FLOWER_BASE_Y_OFFSET = 15.0f;
            constexpr float FLOWER_Y_OFFSET = 37.0f;
            constexpr float FLOWER_SIZE_BASE = 8.0f;
            constexpr float FLOWER_SIZE_VARIATION = 2.0f;
            constexpr float FLOWER_SEGMENTS = 8.0f;
            
            // 木の設定
            constexpr int TREE_COUNT = 8;
            constexpr float TREE_SPACING = 150.0f;
            constexpr float TREE_START_X = 100.0f;
            constexpr float TREE_BASE_Y = 80.0f;
            constexpr float TREE_TRUNK_WIDTH = 8.0f;
            constexpr float TREE_TRUNK_HEIGHT = 60.0f;
            constexpr float TREE_LEAVES_RADIUS = 40.0f;
            constexpr float TREE_LEAVES_SEGMENTS = 12.0f;
            
            // 丘の設定
            constexpr int HILL_COUNT = 3;
            constexpr float HILL_SPACING = 400.0f;
            constexpr float HILL_START_X = 200.0f;
            constexpr float HILL_BASE_Y = 50.0f;
            constexpr float HILL_WIDTH = 200.0f;
            constexpr float HILL_HEIGHT = 80.0f;
            constexpr float HILL_SEGMENTS = 16.0f;
            
            // 星の設定
            constexpr int STAR_COUNT = 50;
            constexpr float STAR_SPACING_X = 25.0f;
            constexpr float STAR_SPACING_X_OFFSET = 17.0f;
            constexpr float STAR_SPACING_Y = 15.0f;
            constexpr float STAR_SPACING_Y_OFFSET = 23.0f;
            constexpr float STAR_MAX_Y = 300.0f;
            constexpr float STAR_SIZE_BASE = 2.0f;
            constexpr int STAR_SIZE_VARIATION = 3;
            
            // 夕日の設定
            constexpr float SUNSET_X = 1000.0f;
            constexpr float SUNSET_Y = 150.0f;
            constexpr float SUNSET_RADIUS = 80.0f;
            constexpr float SUNSET_SEGMENTS = 20.0f;
            
            // 雷雲の設定
            constexpr int THUNDER_CLOUD_COUNT = 3;
            constexpr float THUNDER_CLOUD_SPACING = 400.0f;
            constexpr float THUNDER_CLOUD_START_X = 200.0f;
            constexpr float THUNDER_CLOUD_BASE_Y = 80.0f;
            constexpr float THUNDER_CLOUD_Y_OFFSET = 30.0f;
            constexpr float THUNDER_CLOUD_BASE_SIZE = 120.0f;
            constexpr float THUNDER_CLOUD_SIZE_OFFSET = 20.0f;
            constexpr float THUNDER_CLOUD_ASPECT_RATIO = 0.8f;
            constexpr float THUNDER_CLOUD_SEGMENTS = 12.0f;
            
            // 星雲の設定
            constexpr int NEBULA_COUNT = 3;
            constexpr float NEBULA_SPACING = 300.0f;
            constexpr float NEBULA_START_X = 300.0f;
            constexpr float NEBULA_BASE_Y = 200.0f;
            constexpr float NEBULA_Y_OFFSET = 100.0f;
            constexpr float NEBULA_BASE_SIZE = 150.0f;
            constexpr float NEBULA_SIZE_OFFSET = 50.0f;
            constexpr float NEBULA_ASPECT_RATIO = 0.7f;
            constexpr float NEBULA_SEGMENTS = 16.0f;

            // 星雲の色
            constexpr glm::vec3 NEBULA_PURPLE = glm::vec3(0.8f, 0.2f, 0.8f);  // 紫
            constexpr glm::vec3 NEBULA_BLUE = glm::vec3(0.2f, 0.4f, 0.8f);    // 青
            constexpr glm::vec3 NEBULA_PINK = glm::vec3(0.8f, 0.3f, 0.6f);    // ピンク
        }
        
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
        
        // UIレイアウト設定
        namespace UILayout {
            // 画面サイズ
            constexpr float SCREEN_WIDTH = 1280.0f;
            constexpr float SCREEN_HEIGHT = 720.0f;
            
            // 右上UIエリア
            constexpr float RIGHT_UI_X = 980.0f;
            constexpr float TOP_UI_Y = 30.0f;
            constexpr float TIME_UI_X = 1170.0f;
            constexpr float TIME_UI_Y = 30.0f;
            constexpr float GOAL_UI_X = 962.0f;
            constexpr float GOAL_UI_Y = 65.0f;
            constexpr float GOAL_TIME_1_X = 1040.0f;
            constexpr float GOAL_TIME_2_X = 1110.0f;
            constexpr float GOAL_TIME_Y = 65.0f;
            constexpr float STAR_UI_Y = 40.0f;
            constexpr float STAR_SPACING = 70.0f;
            
            // 左上UIエリア
            constexpr float LEFT_UI_X = 200.0f;
            constexpr float LEFT_UI_Y = 45.0f;
            constexpr float LIFE_SPACING = 40.0f;
            
            // チュートリアルUI
            constexpr float TUTORIAL_BOX_X = 100.0f;
            constexpr float TUTORIAL_BOX_Y = 100.0f;
            constexpr float TUTORIAL_BOX_WIDTH = 600.0f;
            constexpr float TUTORIAL_BOX_HEIGHT = 500.0f;
            constexpr float TUTORIAL_TITLE_X_OFFSET = -130.0f;
            constexpr float TUTORIAL_TITLE_Y = 40.0f;
            constexpr float TUTORIAL_SECTION_X = 60.0f;
            constexpr float TUTORIAL_SECTION_Y = 120.0f;
            constexpr float TUTORIAL_ITEM_X = 100.0f;
            constexpr float TUTORIAL_ITEM_SPACING = 40.0f;
            constexpr float TUTORIAL_ENTER_X_OFFSET = 200.0f;
            constexpr float TUTORIAL_ENTER_Y = 500.0f;
        }
    }

    // UI色
    constexpr glm::vec3 UI_TEXT_COLOR = glm::vec3(1.0f, 1.0f, 1.0f);         // 白
    constexpr glm::vec3 UI_WARNING_COLOR = glm::vec3(1.0f, 0.0f, 0.0f);       // 赤（警告）
    constexpr glm::vec3 UI_SUCCESS_COLOR = glm::vec3(0.2f, 0.8f, 0.2f);       // 緑（成功）
    constexpr glm::vec3 UI_DISABLED_COLOR = glm::vec3(0.5f, 0.5f, 0.5f);      // 灰色（無効）
    constexpr glm::vec3 UI_ACCENT_COLOR = glm::vec3(0.2f, 0.4f, 1.0f);  
    constexpr glm::vec3 UI_ACTIVE_COLOR = glm::vec3(0.5f, 0.5f, 1.0f);      // 青（アクセント）
    
    // 装飾オブジェクト色
    constexpr glm::vec3 FLOWER_RED = glm::vec3(1.0f, 0.3f, 0.3f);             // 花の赤
    constexpr glm::vec3 FLOWER_YELLOW = glm::vec3(1.0f, 0.8f, 0.2f);          // 花の黄
    constexpr glm::vec3 FLOWER_PINK = glm::vec3(0.8f, 0.2f, 0.8f);            // 花のピンク
    constexpr glm::vec3 FLOWER_WHITE = glm::vec3(1.0f, 1.0f, 1.0f);           // 花の白
    constexpr glm::vec3 TREE_TRUNK = glm::vec3(0.4f, 0.2f, 0.1f);             // 木の幹
    constexpr glm::vec3 TREE_LEAVES = glm::vec3(0.2f, 0.5f, 0.1f);            // 木の葉
    constexpr glm::vec3 HILL_COLOR = glm::vec3(0.3f, 0.6f, 0.2f);             // 丘の色
    constexpr glm::vec3 CLOUD_COLOR = glm::vec3(1.0f, 1.0f, 1.0f);            // 雲の色
    constexpr glm::vec3 SUNSET_COLOR = glm::vec3(1.0f, 0.8f, 0.3f);           // 夕日の色
    
    // ライフ・スター色
    constexpr glm::vec3 LIFE_ACTIVE = glm::vec3(1.0f, 0.3f, 0.3f);            // ライフ（有効）
    constexpr glm::vec3 LIFE_INACTIVE = glm::vec3(0.3f, 0.3f, 0.3f);          // ライフ（無効）
    constexpr glm::vec3 STAR_ACTIVE = glm::vec3(1.0f, 1.0f, 0.0f);            // スター（有効）
    constexpr glm::vec3 STAR_INACTIVE = glm::vec3(0.5f, 0.5f, 0.5f);          // スター（無効）
    
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

    // 星の設定
    constexpr int STAR_COUNT = 50;
    constexpr float STAR_SPACING_X = 25.0f;
    constexpr float STAR_SPACING_X_OFFSET = 17.0f;
    constexpr float STAR_SPACING_Y = 15.0f;
    constexpr float STAR_SPACING_Y_OFFSET = 23.0f;
    constexpr float STAR_MAX_Y = 300.0f;
    constexpr float STAR_SIZE_BASE = 2.0f;
    constexpr int STAR_SIZE_VARIATION = 3;
}
