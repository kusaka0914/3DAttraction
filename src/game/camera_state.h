#pragma once

/**
 * @brief カメラの状態を管理する構造体
 */
struct CameraState {
    float yaw = 90.0f;  // ヨー角（単位: 度）
    float pitch = 0.0f;  // ピッチ角（単位: 度）
    float distance = 30.0f;  // カメラ距離（単位: ワールド座標）
    bool firstMouse = true;
    float lastMouseX = 640.0f;  // 前回のマウスX座標（ピクセル）
    float lastMouseY = 360.0f;  // 前回のマウスY座標（ピクセル）
    
    bool isFirstPersonView = false;
    bool isFirstPersonMode = false;
};

