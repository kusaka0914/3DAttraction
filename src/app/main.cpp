#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <map>
#include <glm/glm.hpp>
#include "../gfx/opengl_renderer.h"
#include "../game/game_state.h"
#include "../game/stage_manager.h"
#include "../game/platform_system.h"
#include "../physics/physics_system.h"
#include "../io/input_system.h"
#include "../core/constants/game_constants.h"
#include "../core/error_handler.h"
#include "../gfx/camera_system.h"
#include "../core/utils/input_utils.h"
#include "game_loop.h"
#include "tutorial_manager.h"
#include "../core/constants/debug_config.h"

// ======================================================
//                        main
// ======================================================
int main(int argc, char* argv[]) {
    
    // GLFW初期化
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // OpenGL 2.1設定
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    
    // ウィンドウ作成
    GLFWwindow* window = glfwCreateWindow(GameConstants::WINDOW_WIDTH, GameConstants::WINDOW_HEIGHT, 
                                         GameConstants::WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        ErrorHandler::handleGLFWError("window creation");
        glfwTerminate();
        return -1;
    }
    
    // OpenGLレンダラー初期化
    auto renderer = std::make_unique<gfx::OpenGLRenderer>();
    if (!renderer->initialize(window)) {
        ErrorHandler::handleRendererError("OpenGL renderer initialization");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // UI描画クラスのインスタンス
    auto uiRenderer = std::make_unique<gfx::UIRenderer>();
    auto gameStateUIRenderer = std::make_unique<gfx::GameStateUIRenderer>();
    
    // ゲーム状態
    GameState gameState;
    
    // ゲーム状態の初期化
    initializeGameState(gameState);
    
    // コマンドライン引数で初期ステージを指定
    int initialStage = 6;  // デフォルトはチュートリアルステージ
    bool debugEnding = false;  // デバッグ用エンドロール表示フラグ
    
    if (argc > 1) {
        // エンドロール表示フラグのチェック
        if (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--ending") == 0) {
            debugEnding = true;
        } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            printf("Usage: %s [stage_number] [options]\n", argv[0]);
            printf("  stage_number: 0-5 (default: 6 for tutorial)\n");
            printf("  options:\n");
            printf("    -e, --ending: Show ending sequence (debug mode)\n");
            printf("    -h, --help: Show this help message\n");
            printf("Examples:\n");
            printf("  %s          # Start tutorial stage\n", argv[0]);
            printf("  %s 5        # Start stage 5\n", argv[0]);
            printf("  %s -e       # Show ending sequence\n", argv[0]);
            printf("  %s 5 -e     # Start stage 5 and show ending\n", argv[0]);
            return 0;
        } else {
            // ステージ番号の処理
            int requestedStage = std::atoi(argv[1]);
            if (requestedStage >= 0 && requestedStage <= 5) {  // ステージ0-5を許可
                initialStage = requestedStage;
            } else {
                printf("Invalid stage number: %d. Using default stage %d\n", requestedStage, initialStage);
            }
        }
        
        // 2番目の引数でエンドロール表示フラグをチェック
        if (argc > 2) {
            if (strcmp(argv[2], "-e") == 0 || strcmp(argv[2], "--ending") == 0) {
                debugEnding = true;
                
            }
        }
    }
    
    glfwSetWindowUserPointer(window, &gameState);
    
    // マウスコールバックを設定（1人称視点用）
    glfwSetCursorPosCallback(window, InputSystem::mouse_callback);
    glfwSetScrollCallback(window, InputSystem::scroll_callback);
    
    // システム初期化
    PlatformSystem platformSystem;
    StageManager stageManager;
    
    // デバッグ用エンドロール表示フラグが設定されている場合
    if (debugEnding) {
        gameState.isEndingSequence = true;
        gameState.showStaffRoll = true;
        gameState.staffRollTimer = 0.0f;
    } else {
        stageManager.loadStage(initialStage, gameState, platformSystem);
    }

    // ゲームパッド初期化
    InputSystem::initializeGamepad();

    // キー状態管理
    std::map<int, InputUtils::KeyState> keyStates;
    for (int key : {GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, 
                    GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_ENTER, GLFW_KEY_R, GLFW_KEY_T,
                    GLFW_KEY_F, GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE}) {
        keyStates[key] = InputUtils::KeyState();
    }

    // ゲーム開始準備完了
    bool gameRunning = true;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // ステージ開始時間を管理する関数
    auto resetStageStartTime = [&startTime]() {
        startTime = std::chrono::high_resolution_clock::now();
        DEBUG_PRINTF("DEBUG: resetStageStartTime called, startTime reset to current time\n");
        DEBUG_PRINTF("DEBUG: Next gameTime calculation should start from 0.00\n");
    };
    
    // ゲームループ実行
    GameLoop::run(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer, keyStates, resetStageStartTime, startTime);
    
    // クリーンアップ
    renderer->cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
