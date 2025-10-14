#pragma once

#include <GLFW/glfw3.h>

namespace GameConstants {
    // ウィンドウ設定
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr const char* WINDOW_TITLE = "Vulkan3D - Portal Physics Puzzle";
    
    // フレームレート設定
    constexpr float MAX_DELTA_TIME = 0.1f;  // 最大100ms（10FPS相当）
    constexpr int TARGET_FPS = 60;
    constexpr int FRAME_DELAY_MS = 16;  // ~60 FPS
}
