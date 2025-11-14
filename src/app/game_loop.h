/**
 * @file game_loop.h
 * @brief ゲームループ
 * @details メインゲームループと各種画面処理（タイトル、Ready、ステージクリアなど）を管理します。
 */
#pragma once

#include <GLFW/glfw3.h>
#include <memory>
#include <map>
#include <chrono>
#include <functional>
#include <glm/glm.hpp>
#include "../game/game_state.h"
#include "../game/stage_manager.h"
#include "../game/platform_system.h"
#include "../gfx/opengl_renderer.h"
#include "../gfx/ui_renderer.h"
#include "../gfx/game_state_ui_renderer.h"
#include "../gfx/camera_system.h"
#include "../core/utils/input_utils.h"
#include "../io/audio_manager.h"

namespace GameLoop {
    /**
     * @brief ゲームループの実行
     * @details メインゲームループを実行し、更新処理と描画処理を統合的に管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param renderer OpenGLレンダラー
     * @param uiRenderer UIレンダラー
     * @param gameStateUIRenderer ゲーム状態UIレンダラー
     * @param keyStates キー状態マップ
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     * @param startTime 開始時刻
     * @param audioManager オーディオマネージャー
     */
    void run(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
             PlatformSystem& platformSystem,
             std::unique_ptr<gfx::OpenGLRenderer>& renderer,
             std::unique_ptr<gfx::UIRenderer>& uiRenderer,
             std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
             std::map<int, InputUtils::KeyState>& keyStates,
             std::function<void()> resetStageStartTime,
             std::chrono::high_resolution_clock::time_point& startTime,
             io::AudioManager& audioManager);
    
    /**
     * @brief Ready画面の処理
     * @details Ready画面の表示と入力処理を管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param renderer OpenGLレンダラー
     * @param gameStateUIRenderer ゲーム状態UIレンダラー
     * @param keyStates キー状態マップ
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     * @param audioManager オーディオマネージャー
     * @param deltaTime デルタタイム
     */
    void handleReadyScreen(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                          PlatformSystem& platformSystem,
                          std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                          std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                          std::map<int, InputUtils::KeyState>& keyStates,
                          std::function<void()> resetStageStartTime,
                          io::AudioManager& audioManager,
                          float deltaTime);
    
    /**
     * @brief カウントダウンの処理
     * @details カウントダウン表示と入力処理を管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param renderer OpenGLレンダラー
     * @param gameStateUIRenderer ゲーム状態UIレンダラー
     * @param keyStates キー状態マップ
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     * @param deltaTime デルタタイム
     */
    void handleCountdown(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                        PlatformSystem& platformSystem,
                        std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                        std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                        std::map<int, InputUtils::KeyState>& keyStates,
                        std::function<void()> resetStageStartTime,
                        float deltaTime);
    
    /**
     * @brief エンディングシーケンスの処理
     * @details エンディングシーケンスの表示と入力処理を管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param renderer OpenGLレンダラー
     * @param gameStateUIRenderer ゲーム状態UIレンダラー
     * @param keyStates キー状態マップ
     * @param deltaTime デルタタイム
     */
    void handleEndingSequence(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                             PlatformSystem& platformSystem,
                             std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                             std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                             std::map<int, InputUtils::KeyState>& keyStates,
                             float deltaTime);
    
    /**
     * @brief ゲーム状態の更新
     * @details ゲーム状態の更新処理を実行します。
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
    void updateGameState(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                        PlatformSystem& platformSystem, float deltaTime, float scaledDeltaTime,
                        std::map<int, InputUtils::KeyState>& keyStates,
                        std::function<void()> resetStageStartTime, io::AudioManager& audioManager);
    
    /**
     * @brief 物理演算と衝突判定
     * @details 物理演算と衝突判定の更新処理を実行します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param deltaTime デルタタイム
     * @param scaledDeltaTime スケールされたデルタタイム
     * @param audioManager オーディオマネージャー
     */
    void updatePhysicsAndCollisions(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                                   PlatformSystem& platformSystem, float deltaTime, float scaledDeltaTime, io::AudioManager& audioManager);
    
    /**
     * @brief アイテムの更新
     * @details アイテムの更新処理を実行します。
     * 
     * @param gameState ゲーム状態
     * @param scaledDeltaTime スケールされたデルタタイム
     * @param audioManager オーディオマネージャー
     */
    void updateItems(GameState& gameState, float scaledDeltaTime, io::AudioManager& audioManager);
    
    /**
     * @brief ステージ選択エリアの処理
     * @details ステージ選択エリアでの入力処理を管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     */
    void handleStageSelectionArea(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                                 PlatformSystem& platformSystem, std::function<void()> resetStageStartTime);
    
    /**
     * @brief 描画処理
     * @details フレーム全体の描画処理を実行します。
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
    void renderFrame(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                    PlatformSystem& platformSystem,
                    std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                    std::unique_ptr<gfx::UIRenderer>& uiRenderer,
                    std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                    std::map<int, InputUtils::KeyState>& keyStates,
                    float deltaTime = 0.016f);
    
    /**
     * @brief 入力処理の統合
     * @details 入力処理を統合的に管理します。
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
    void handleInputProcessing(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                              PlatformSystem& platformSystem, 
                              std::map<int, InputUtils::KeyState>& keyStates,
                              std::function<void()> resetStageStartTime, float scaledDeltaTime, io::AudioManager& audioManager);
    
    /**
     * @brief フレームの準備を行う
     * @details フレーム描画の準備処理を実行します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param renderer OpenGLレンダラー
     * @param width 出力: ウィンドウ幅
     * @param height 出力: ウィンドウ高さ
     * @param deltaTime デルタタイム
     */
    void prepareFrame(GLFWwindow* window, GameState& gameState, StageManager& stageManager,
                     std::unique_ptr<gfx::OpenGLRenderer>& renderer, int& width, int& height, float deltaTime);
    
    /**
     * @brief プラットフォームを描画する
     * @details プラットフォームの描画処理を実行します。
     * 
     * @param platformSystem プラットフォームシステム
     * @param renderer OpenGLレンダラー
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     */
    void renderPlatforms(PlatformSystem& platformSystem, 
                        std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                        GameState& gameState,
                        StageManager& stageManager);
    
    /**
     * @brief プレイヤーを描画する
     * @details プレイヤーの描画処理を実行します。
     * 
     * @param gameState ゲーム状態
     * @param renderer OpenGLレンダラー
     */
    void renderPlayer(GameState& gameState, 
                     std::unique_ptr<gfx::OpenGLRenderer>& renderer);
}
