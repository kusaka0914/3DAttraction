#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <GLFW/glfw3.h>
#include <memory>
#include <map>
#include <glm/glm.hpp>
#include "../game/game_state.h"
#include "../game/stage_manager.h"
#include "../game/platform_system.h"
#include "../gfx/opengl_renderer.h"
#include "../gfx/ui_renderer.h"
#include "../gfx/game_state_ui_renderer.h"
#include "../core/utils/input_utils.h"

namespace GameLoop {

/**
 * @brief ゲームの描画処理を担当するクラス
 * @details このクラスは以下の責任を持ちます：
 * - フレーム全体の描画（renderFrame）
 * - フレーム準備（prepareFrame）
 * - プラットフォームの描画（renderPlatforms）
 * - プレイヤーの描画（renderPlayer）
 */
class GameRenderer {
public:
    /**
     * @brief フレーム全体を描画する
     * @details タイトル画面、ゲーム画面、UI、エディタ、遷移アニメーションなどを統合的に描画します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param renderer OpenGLレンダラー
     * @param uiRenderer UIレンダラー
     * @param gameStateUIRenderer ゲーム状態UIレンダラー
     * @param keyStates キー状態マップ
     * @param deltaTime デルタタイム
     */
    static void renderFrame(
        GLFWwindow* window, 
        GameState& gameState, 
        StageManager& stageManager, 
        PlatformSystem& platformSystem,
        std::unique_ptr<gfx::OpenGLRenderer>& renderer,
        std::unique_ptr<gfx::UIRenderer>& uiRenderer,
        std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
        std::map<int, InputUtils::KeyState>& keyStates,
        float deltaTime
    );

    /**
     * @brief フレームの準備を行う
     * @details 背景の描画、カメラ設定、ウィンドウサイズの取得などを処理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param renderer OpenGLレンダラー
     * @param width 出力: ウィンドウ幅
     * @param height 出力: ウィンドウ高さ
     * @param deltaTime デルタタイム
     */
    static void prepareFrame(
        GLFWwindow* window, 
        GameState& gameState, 
        StageManager& stageManager,
        std::unique_ptr<gfx::OpenGLRenderer>& renderer, 
        int& width, 
        int& height, 
        float deltaTime
    );

    /**
     * @brief プラットフォームを描画する
     * @details 各種プラットフォームタイプに応じたテクスチャとエフェクトを適用して描画します。
     * 
     * @param platformSystem プラットフォームシステム
     * @param renderer OpenGLレンダラー
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     */
    static void renderPlatforms(
        PlatformSystem& platformSystem, 
        std::unique_ptr<gfx::OpenGLRenderer>& renderer,
        GameState& gameState,
        StageManager& stageManager
    );

    /**
     * @brief プレイヤーを描画する
     * @details プレイヤーの位置と状態に応じてテクスチャを適用して描画します。
     * 1人称視点時は描画をスキップします。
     * 
     * @param gameState ゲーム状態
     * @param renderer OpenGLレンダラー
     */
    static void renderPlayer(
        GameState& gameState, 
        std::unique_ptr<gfx::OpenGLRenderer>& renderer
    );
};

} // namespace GameLoop

