#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <GLFW/glfw3.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
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
#include "../game/save_manager.h"
#include "../game/platform_system.h"
#include "../physics/physics_system.h"
#include "../io/input_system.h"
#include "../io/audio_manager.h"
#include "../core/constants/game_constants.h"
#include "../core/error_handler.h"
#include "../gfx/camera_system.h"
#include "../core/utils/input_utils.h"
#include "../core/utils/ui_config_manager.h"
#include "game_loop.h"
#include "tutorial_manager.h"
#include "../core/constants/debug_config.h"

int main(int argc, char* argv[]) {
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // フルスクリーンなのでリサイズ不可
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    
    GLFWwindow* window = glfwCreateWindow(videoMode->width, videoMode->height, 
                                         GameConstants::WINDOW_TITLE, 
                                         nullptr, nullptr);
    if (!window) {
        ErrorHandler::handleGLFWError("window creation");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    #ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(window);
        if (hwnd) {
            LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
            style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
            SetWindowLongPtr(hwnd, GWL_STYLE, style);
            
            SetWindowPos(hwnd, HWND_TOP, 0, 0, videoMode->width, videoMode->height, 
                        SWP_FRAMECHANGED | SWP_NOZORDER);
            
            ShowWindow(hwnd, SW_SHOWMAXIMIZED);
            SetForegroundWindow(hwnd);
        }
    #else
        glfwSetWindowMonitor(window, primaryMonitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
    #endif
    
    glfwPollEvents();
    
    glfwShowWindow(window);
    glfwFocusWindow(window);
    
    glfwSetCharCallback(window, nullptr);
    
    auto renderer = std::make_unique<gfx::OpenGLRenderer>();
    if (!renderer->initialize(window)) {
        ErrorHandler::handleRendererError("OpenGL renderer initialization");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    auto uiRenderer = std::make_unique<gfx::UIRenderer>();
    auto gameStateUIRenderer = std::make_unique<gfx::GameStateUIRenderer>();
    
    GameState gameState;
    
    initializeGameState(gameState);
    
    SaveManager::loadGameData(gameState);
    
    int initialStage = 6;  // デフォルトはチュートリアルステージ
    bool debugEnding = false;  // デバッグ用エンドロール表示フラグ
    
    if (argc > 1) {
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
            int requestedStage = std::atoi(argv[1]);
            if (requestedStage >= 0 && requestedStage <= 5) {  // ステージ0-5を許可
                initialStage = requestedStage;
            } else {
                printf("Invalid stage number: %d. Using default stage %d\n", requestedStage, initialStage);
            }
        }
        
        if (argc > 2) {
            if (strcmp(argv[2], "-e") == 0 || strcmp(argv[2], "--ending") == 0) {
                debugEnding = true;
                
            }
        }
    }
    glfwSetWindowUserPointer(window, &gameState);
    
    glfwSetCursorPosCallback(window, InputSystem::mouse_callback);
    glfwSetScrollCallback(window, InputSystem::scroll_callback);
    
    PlatformSystem platformSystem;
    StageManager stageManager;
    
    io::AudioManager audioManager;
    if (!audioManager.initialize()) {
        std::cerr << "Failed to initialize audio system" << std::endl;
    }
    
    UIConfig::UIConfigManager::getInstance().loadConfig("assets/config/ui_config.json");
    
    if (debugEnding) {
        gameState.ui.isEndingSequence = true;
        gameState.ui.showStaffRoll = true;
        gameState.ui.staffRollTimer = 0.0f;
        gameState.ui.showTitleScreen = false;  // エンドロール表示時はタイトル画面をスキップ
    } else {
        gameState.ui.showTitleScreen = true;
    }

    InputSystem::initializeGamepad();

    std::map<int, InputUtils::KeyState> keyStates;
    for (int key : {GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, 
                    GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_ENTER, GLFW_KEY_R, GLFW_KEY_T,
                    GLFW_KEY_F, GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE}) {
        keyStates[key] = InputUtils::KeyState();
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    
    auto resetStageStartTime = [&startTime, &gameState]() {
        startTime = std::chrono::high_resolution_clock::now();
        gameState.progress.timeLimitApplied = false; // カウントダウン時の時間設定フラグをリセット
        DEBUG_PRINTF("DEBUG: timeLimitApplied reset to false\n");
    };
    
    GameLoop::run(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer, keyStates, resetStageStartTime, startTime, audioManager);
    
    renderer->cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
