/**
 * @file input_handler.h
 * @brief ゲームの入力処理を担当するクラス
 * @details リプレイモード、チュートリアルモード、通常モードなど、ゲーム状態に応じた入力処理を統合的に管理します。
 */
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
    
    /**
     * @brief ステージ選択アクションを処理する
     * @details ステージ選択時の処理（ワープ、解放確認など）を実行します。
     * 
     * @param stageNumber ステージ番号
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     * @param window GLFWウィンドウ
     */
    static void processStageSelectionAction(
        int stageNumber,
        GameState& gameState,
        StageManager& stageManager,
        PlatformSystem& platformSystem,
        std::function<void()> resetStageStartTime,
        GLFWwindow* window
    );
};

} // namespace GameLoop

