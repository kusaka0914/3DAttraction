#pragma once

#include <GLFW/glfw3.h>
#include "../game/game_state.h"
#include "../game/platform_system.h"

// 入力システム
class InputSystem {
public:
    // マウスコールバック
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double, double yoffset);
    
    // 入力処理
    static void processInput(GLFWwindow* window, GameState& gameState, float deltaTime);
    
    // ジャンプと浮遊の処理
    static void processJumpAndFloat(GLFWwindow* window, GameState& gameState, float deltaTime, const glm::vec3& gravityDirection, PlatformSystem& platformSystem);
};

