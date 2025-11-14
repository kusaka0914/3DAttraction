#pragma once

#include <GLFW/glfw3.h>
#include <map>
#include <glm/glm.hpp>
#include "../game/game_state.h"
#include "../core/utils/input_utils.h"

/**
 * @brief チュートリアルマネージャー
 * @details チュートリアルステージの進行と入力制御を管理します。
 */
namespace TutorialManager {
    /**
     * @brief チュートリアルステージの進行を処理する
     * @details チュートリアルの各ステップの進行を管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param keyStates キー状態マップ
     */
    void processTutorialProgress(GLFWwindow* window, GameState& gameState, 
                                const std::map<int, InputUtils::KeyState>& keyStates);
    
    /**
     * @brief チュートリアル入力が有効か確認する
     * @details チュートリアルの進行状況に応じて、入力の有効性を判定します。
     * 
     * @param gameState ゲーム状態
     * @param window GLFWウィンドウ
     * @param keyStates キー状態マップ
     * @return 入力が有効な場合true
     */
    bool isTutorialInputEnabled(const GameState& gameState, GLFWwindow* window, 
                               const std::map<int, InputUtils::KeyState>& keyStates);
}
