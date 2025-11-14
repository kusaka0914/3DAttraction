/**
 * @file input_constants.h
 * @brief 入力定数
 * @details キーボード入力に関する定数を定義します。
 */
#pragma once

#include <GLFW/glfw3.h>

namespace GameConstants {
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
}
