#pragma once

#include <glm/glm.hpp>

namespace GameConstants {
    // カメラ設定
    constexpr float CAMERA_FOV = 45.0f;
    constexpr float CAMERA_NEAR = 0.1f;
    constexpr float CAMERA_FAR = 100.0f;
    
    // ステージ選択フィールド用カメラ
    constexpr glm::vec3 STAGE_SELECT_CAMERA_OFFSET = glm::vec3(0, 15, -15);
    constexpr glm::vec3 NORMAL_STAGE_CAMERA_OFFSET = glm::vec3(0, 2, -8);
    
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
    
    // 星の設定
    constexpr int STAR_COUNT = 50;
    constexpr float STAR_SPACING_X = 25.0f;
    constexpr float STAR_SPACING_X_OFFSET = 17.0f;
    constexpr float STAR_SPACING_Y = 15.0f;
    constexpr float STAR_SPACING_Y_OFFSET = 23.0f;
    constexpr float STAR_MAX_Y = 300.0f;
    constexpr float STAR_SIZE_BASE = 2.0f;
    constexpr int STAR_SIZE_VARIATION = 3;
    
    // 3D描画の定数
    constexpr float STAR_ANGLE_STEP = 72.0f;        // 星の角度ステップ（360度/5）
    constexpr float STAR_INNER_ANGLE_OFFSET = 36.0f; // 星の内側角度オフセット
    constexpr float STAR_OUTER_RADIUS = 0.3f;       // 星の外側半径
    constexpr float STAR_INNER_RADIUS = 0.12f;      // 星の内側半径
    constexpr float NUMBER_CHAR_SPACING = 0.6f;     // 数字の文字間隔
    constexpr float NUMBER_WIDTH = 0.4f;            // 数字の幅
    constexpr float NUMBER_HEIGHT = 0.6f;           // 数字の高さ
    constexpr float NUMBER_THICKNESS = 0.1f;        // 数字の太さ
    constexpr float X_MARK_SIZE = 0.3f;             // ×記号のサイズ
    constexpr float X_MARK_THICKNESS = 0.1f;        // ×記号の太さ
}
