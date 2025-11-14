#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <GLFW/glfw3.h>
#include <map>
#include <functional>
#include <glm/glm.hpp>
#include "../game/game_state.h"
#include "../game/stage_manager.h"
#include "../game/platform_system.h"
#include "../core/utils/input_utils.h"
#include "../io/audio_manager.h"

namespace GameLoop {

/**
 * @brief ゲームの入力処理を担当するクラス
 * @details このクラスは以下の責任を持ちます：
 * - 入力処理の統合（handleInputProcessing）
 * - リプレイモードの入力制御
 * - チュートリアルモードの入力制御
 * - 通常のゲーム入力処理
 */
class InputHandler {
public:
    /**
     * @brief 入力処理を統合的に処理する
     * @details リプレイモード、チュートリアルモード、通常モードなど、
     * ゲーム状態に応じた入力処理を統合的に管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param keyStates キー状態マップ
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     * @param scaledDeltaTime スケールされたデルタタイム
     * @param audioManager オーディオマネージャー
     */
    static void handleInputProcessing(
        GLFWwindow* window, 
        GameState& gameState, 
        StageManager& stageManager, 
        PlatformSystem& platformSystem, 
        std::map<int, InputUtils::KeyState>& keyStates,
        std::function<void()> resetStageStartTime, 
        float scaledDeltaTime, 
        io::AudioManager& audioManager
    );
};

} // namespace GameLoop

