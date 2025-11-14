/**
 * @file game_updater.h
 * @brief ゲーム状態の更新を担当するクラス
 * @details ゲーム状態の更新、物理演算と衝突判定、アイテムの更新を統合的に管理します。
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
 * @brief ゲーム状態の更新を担当するクラス
 * @details このクラスは以下の責任を持ちます：
 * - ゲーム状態の更新（updateGameState）
 * - 物理演算と衝突判定（updatePhysicsAndCollisions）
 * - アイテムの更新（updateItems）
 */
class GameUpdater {
public:
    /**
     * @brief ゲーム状態を更新する
     * @details エディタモード、タイトル画面、ステージ遷移アニメーション、
     * タイトル画面アニメーション、BGM管理、入力処理などを統合的に管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param deltaTime デルタタイム
     * @param scaledDeltaTime スケールされたデルタタイム
     * @param keyStates キー状態マップ
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     * @param audioManager オーディオマネージャー
     */
    static void updateGameState(
        GLFWwindow* window, 
        GameState& gameState, 
        StageManager& stageManager, 
        PlatformSystem& platformSystem, 
        float deltaTime, 
        float scaledDeltaTime,
        std::map<int, InputUtils::KeyState>& keyStates,
        std::function<void()> resetStageStartTime, 
        io::AudioManager& audioManager
    );

    /**
     * @brief 物理演算と衝突判定を更新する
     * @details 重力、速度、位置、プラットフォーム衝突、落下判定などを処理します。
     * ゲームオーバー時は物理演算をスキップします。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param deltaTime デルタタイム
     * @param scaledDeltaTime スケールされたデルタタイム
     * @param audioManager オーディオマネージャー
     */
    static void updatePhysicsAndCollisions(
        GLFWwindow* window, 
        GameState& gameState, 
        StageManager& stageManager, 
        PlatformSystem& platformSystem, 
        float deltaTime, 
        float scaledDeltaTime, 
        io::AudioManager& audioManager
    );

    /**
     * @brief アイテムの更新処理
     * @details アイテムの回転、上下動、収集判定などを処理します。
     * 
     * @param gameState ゲーム状態
     * @param scaledDeltaTime スケールされたデルタタイム
     * @param audioManager オーディオマネージャー
     */
    static void updateItems(
        GameState& gameState, 
        float scaledDeltaTime, 
        io::AudioManager& audioManager
    );
};

} // namespace GameLoop

