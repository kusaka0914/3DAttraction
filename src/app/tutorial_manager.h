#pragma once

#include <GLFW/glfw3.h>
#include <map>
#include <glm/glm.hpp>
#include "../game/game_state.h"
#include "../core/utils/input_utils.h"

namespace TutorialManager {
    // チュートリアルステージの進行処理
    void processTutorialProgress(GLFWwindow* window, GameState& gameState, 
                                const std::map<int, InputUtils::KeyState>& keyStates);
    
    // チュートリアル入力制御
    bool isTutorialInputEnabled(const GameState& gameState, GLFWwindow* window, 
                               const std::map<int, InputUtils::KeyState>& keyStates);
}
