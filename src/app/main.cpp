#ifndef NOMINMAX
#define NOMINMAX
#endif

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#ifdef _WIN32
#include <windows.h>
#endif
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
#include "../io/audio_manager.h"
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
    
    // GLFW初期化を行う
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // OpenGL 2.1に設定する
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // フルスクリーンなのでリサイズ不可
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    
    // プライマリモニターの解像度を取得
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    
    // ウィンドウの作成（一旦ウィンドウモードで作成）
    GLFWwindow* window = glfwCreateWindow(videoMode->width, videoMode->height, 
                                         GameConstants::WINDOW_TITLE, 
                                         nullptr, nullptr);
    if (!window) {
        ErrorHandler::handleGLFWError("window creation");
        glfwTerminate();
        return -1;
    }
    
    // OpenGLコンテキストを作成（glfwSetWindowMonitorの前に必要）
    glfwMakeContextCurrent(window);
    
    // Windows APIを使ってフルスクリーンに切り替え
    #ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(window);
        if (hwnd) {
            // ウィンドウスタイルを変更（タイトルバーと境界線を削除）
            LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
            style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
            SetWindowLongPtr(hwnd, GWL_STYLE, style);
            
            // ウィンドウをフルスクリーンに設定
            SetWindowPos(hwnd, HWND_TOP, 0, 0, videoMode->width, videoMode->height, 
                        SWP_FRAMECHANGED | SWP_NOZORDER);
            
            // ウィンドウを前面に表示
            ShowWindow(hwnd, SW_SHOWMAXIMIZED);
            SetForegroundWindow(hwnd);
        }
    #else
        // macOS/LinuxではglfwSetWindowMonitorを使用
        glfwSetWindowMonitor(window, primaryMonitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
    #endif
    
    // イベントを処理してウィンドウを確実に表示
    glfwPollEvents();
    
    // ウィンドウを前面に表示
    glfwShowWindow(window);
    glfwFocusWindow(window);
    
    // 文字入力を無効化（ゲーム中にテキストが表示されないようにする）
    glfwSetCharCallback(window, nullptr);
    
    // OpenGLレンダラー初期化
    auto renderer = std::make_unique<gfx::OpenGLRenderer>();
    if (!renderer->initialize(window)) {
        ErrorHandler::handleRendererError("OpenGL renderer initialization");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // UI描画クラス系のインスタンス作成
    auto uiRenderer = std::make_unique<gfx::UIRenderer>();
    auto gameStateUIRenderer = std::make_unique<gfx::GameStateUIRenderer>();
    
    // ゲーム状態の管理
    GameState gameState;
    
    // ゲーム状態の初期化
    initializeGameState(gameState);
    
    // コマンドライン引数で初期ステージを指定することができる
    int initialStage = 6;  // デフォルトはチュートリアルステージ
    bool debugEnding = false;  // デバッグ用エンドロール表示フラグ
    
    if (argc > 1) {
        // エンドロール表示フラグのチェック
        if (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--ending") == 0) {
            debugEnding = true;
            //コマンドに関するヘルプ
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
        
        // 2番目の引数でもエンドロール表示フラグをチェック
        if (argc > 2) {
            if (strcmp(argv[2], "-e") == 0 || strcmp(argv[2], "--ending") == 0) {
                debugEnding = true;
                
            }
        }
    }
    glfwSetWindowUserPointer(window, &gameState);
    
    // マウスコールバックとスクロールコールバックを設定（1人称視点用）
    glfwSetCursorPosCallback(window, InputSystem::mouse_callback);
    glfwSetScrollCallback(window, InputSystem::scroll_callback);
    
    // システム初期化
    PlatformSystem platformSystem;
    StageManager stageManager;
    
    // 音声システム初期化
    io::AudioManager audioManager;
    if (!audioManager.initialize()) {
        std::cerr << "Failed to initialize audio system" << std::endl;
        // 音声システムの初期化に失敗してもゲームは続行
    }
    
    // デバッグ用エンドロール表示フラグがtrueの時は開始時にエンドロールが流れる
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

    //開始時間を現在に設定
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // ステージ開始時間を管理するラムダ関数
    auto resetStageStartTime = [&startTime, &gameState]() {
        startTime = std::chrono::high_resolution_clock::now();
        gameState.timeLimitApplied = false; // カウントダウン時の時間設定フラグをリセット
        DEBUG_PRINTF("DEBUG: timeLimitApplied reset to false\n");
    };
    
    // ゲームループ開始
    GameLoop::run(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer, keyStates, resetStageStartTime, startTime, audioManager);
    
    // クリーンアップ
    renderer->cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
