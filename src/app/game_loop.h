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

namespace GameLoop {
    // ゲームループの実行
    void run(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
             PlatformSystem& platformSystem,
             std::unique_ptr<gfx::OpenGLRenderer>& renderer,
             std::unique_ptr<gfx::UIRenderer>& uiRenderer,
             std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
             std::map<int, InputUtils::KeyState>& keyStates,
             std::function<void()> resetStageStartTime,
             std::chrono::high_resolution_clock::time_point startTime);
    
    // Ready画面の処理
    void handleReadyScreen(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                          PlatformSystem& platformSystem,
                          std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                          std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                          std::map<int, InputUtils::KeyState>& keyStates,
                          std::function<void()> resetStageStartTime);
    
    // カウントダウンの処理
    void handleCountdown(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                        PlatformSystem& platformSystem,
                        std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                        std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                        std::map<int, InputUtils::KeyState>& keyStates,
                        std::function<void()> resetStageStartTime,
                        float deltaTime);
    
    // エンディングシーケンスの処理
    void handleEndingSequence(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                             PlatformSystem& platformSystem,
                             std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                             std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                             std::map<int, InputUtils::KeyState>& keyStates,
                             float deltaTime);
    
    // ゲーム状態の更新
    void updateGameState(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                        PlatformSystem& platformSystem, float deltaTime, float scaledDeltaTime,
                        std::map<int, InputUtils::KeyState>& keyStates,
                        std::function<void()> resetStageStartTime);
    
    // 物理演算と衝突判定
    void updatePhysicsAndCollisions(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                                   PlatformSystem& platformSystem, float deltaTime, float scaledDeltaTime);
    
    // アイテムの更新
    void updateItems(GameState& gameState, float scaledDeltaTime);
    
    // ステージ選択エリアの処理
    void handleStageSelectionArea(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                                 PlatformSystem& platformSystem, std::function<void()> resetStageStartTime);
    
    // 描画処理
    void renderFrame(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                    PlatformSystem& platformSystem,
                    std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                    std::unique_ptr<gfx::UIRenderer>& uiRenderer,
                    std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer);
    
    // 入力処理の統合
    void handleInputProcessing(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                              PlatformSystem& platformSystem, 
                              std::map<int, InputUtils::KeyState>& keyStates,
                              std::function<void()> resetStageStartTime, float scaledDeltaTime);
}
