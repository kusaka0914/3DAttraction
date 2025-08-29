#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../gfx/opengl_renderer.h"
#include "../game/game_state.h"
#include "../game/stage_generator.h"
#include "../game/stage_manager.h"
#include "../game/platform_system.h"
#include "../game/cannon_system.h"
#include "../game/switch_system.h"
#include "../game/gravity_system.h"
#include "../physics/physics_system.h"
#include "../io/input_system.h"

// ======================================================
//                        ヘルパー関数
// ======================================================

// 重力方向に応じたプレイヤー位置調整
void adjustPlayerPositionForGravity(GameState& gameState, const glm::vec3& platformPosition, 
                                   const glm::vec3& platformSize, const glm::vec3& playerSize, 
                                   const glm::vec3& gravityDirection) {
    if (gravityDirection.y > 0.5f) {
        // 重力反転時：足場の下面に配置
        gameState.playerPosition.y = platformPosition.y - platformSize.y * 0.5f - playerSize.y * 0.5f;
    } else {
        // 通常重力：足場の上面に配置
        gameState.playerPosition.y = platformPosition.y + platformSize.y * 0.5f + playerSize.y * 0.5f;
    }
    gameState.playerVelocity.y = 0.0f;
}

// 速度倍率に応じた重力強度を計算
float calculateGravityStrength(float baseGravity, float deltaTime, float timeScale, const glm::vec3& gravityDirection, GameState& gameState) {
    // 基本重力強度
    float gravityStrength = baseGravity * deltaTime;
    
    // 速度倍率に応じて重力を増強（地面につくスピードを上げる）
    if (timeScale > 1.0f) {
        // 速度倍率の2乗に比例して重力を増強（より明確な効果）
        // 2倍速で4倍の重力、3倍速で9倍の重力
        gravityStrength *= timeScale *1.2f;
    }
    if(gameState.currentStage==0){
        gravityStrength *= 2.0f;
    }
    
    // 重力反転時は70%の強度
    if (gravityDirection.y > 0.5f) {
        gravityStrength *= 0.7f;
    }
    
    return gravityStrength;
}

// キー入力の状態管理
struct KeyState {
    bool isPressed = false;
    bool wasPressed = false;
    
    void update(bool currentlyPressed) {
        wasPressed = isPressed;
        isPressed = currentlyPressed;
    }
    
    bool justPressed() const { return isPressed && !wasPressed; }
    bool justReleased() const { return !isPressed && wasPressed; }
};

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
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Vulkan3D - Portal Physics Puzzle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // OpenGLレンダラー初期化
    auto renderer = std::make_unique<gfx::OpenGLRenderer>();
    if (!renderer->initialize(window)) {
        std::cerr << "Failed to initialize OpenGL renderer" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // ゲーム状態
    GameState gameState;
    
    // コマンドライン引数で初期ステージを指定
    if (argc > 1) {
        int requestedStage = std::atoi(argv[1]);
        if (requestedStage >= 0 && requestedStage <= 5) {  // ステージ0を許可
            gameState.currentStage = requestedStage;
            printf("Initial stage set to %d via command line argument\n", requestedStage);
        }
    }
    
    glfwSetWindowUserPointer(window, &gameState);
    
    // マウスコールバックを設定（1人称視点用）
    glfwSetCursorPosCallback(window, InputSystem::mouse_callback);
    glfwSetScrollCallback(window, InputSystem::scroll_callback);
    
    // システム初期化
    PlatformSystem platformSystem;
    StageManager stageManager;
    
    // 初期ステージ設定
    int initialStage = 1;
    gameState.showTutorial = true;
    
    printf("Starting from stage %d\n", initialStage);
    stageManager.loadStage(initialStage, gameState, platformSystem);

    // ゲームパッド初期化
    InputSystem::initializeGamepad();

    // キー状態管理
    std::map<int, KeyState> keyStates;
    for (int key : {GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, 
                    GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_ENTER, GLFW_KEY_R, GLFW_KEY_T,
                    GLFW_KEY_F}) {  // Fキーでカメラ切り替え
        keyStates[key] = KeyState();
    }

    // ゲーム開始準備完了
    bool gameRunning = true;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastFrameTime = startTime;
    
    // ステージ開始時間を管理する関数
    auto resetStageStartTime = [&startTime]() {
        startTime = std::chrono::high_resolution_clock::now();
        printf("Stage start time reset\n");
    };
    
    // --------------------------
    //         ゲームループ
    // --------------------------
    while (!glfwWindowShouldClose(window) && gameRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
        
        // deltaTimeの異常値を制限（フレームレート低下時の問題を防ぐ）
        deltaTime = std::min(deltaTime, 0.1f); // 最大100ms（10FPS相当）
        
        lastFrameTime = currentTime;
        
        gameState.gameTime = std::chrono::duration<float>(currentTime - startTime).count();
        
        // 速度倍率を適用したdeltaTimeを計算（全ステージで有効）
        float scaledDeltaTime = deltaTime * gameState.timeScale;
        
        // チュートリアル表示中の処理
        if (gameState.showTutorial) {
            // チュートリアル表示中はゲームを一時停止
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                gameState.showTutorial = false;
                printf("Tutorial dismissed.\n");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                // チュートリアル終了後にReady画面を表示
                if (!gameState.readyScreenShown) {
                    gameState.showReadyScreen = true;
                    gameState.readyScreenShown = false;
                    gameState.readyScreenSpeedLevel = 0;
                }
            }
            // チュートリアル表示中は他の処理をスキップ
            renderer->beginFrameWithBackground(stageManager.getCurrentStage());
            
            // カメラ設定
            glm::vec3 cameraPos, cameraTarget;
            if (stageManager.getCurrentStage() == 0) {
                cameraPos = gameState.playerPosition + glm::vec3(0, 15, -15);
                cameraTarget = gameState.playerPosition;
            } else {
                cameraPos = gameState.playerPosition + glm::vec3(0, 2, -8);
                cameraTarget = gameState.playerPosition;
            }
            renderer->setCamera(cameraPos, cameraTarget);

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            renderer->setProjection(45.0f, (float)width / (float)height, 0.1f, 100.0f);
            
            // 通常のゲーム要素を描画（背景として）
            auto positions = platformSystem.getPositions();
            auto sizes = platformSystem.getSizes();
            auto colors = platformSystem.getColors();
            auto visibility = platformSystem.getVisibility();
            auto isRotating = platformSystem.getIsRotating();
            auto rotationAngles = platformSystem.getRotationAngles();
            auto rotationAxes = platformSystem.getRotationAxes();
            auto blinkAlphas = platformSystem.getBlinkAlphas();
            
            for (size_t i = 0; i < positions.size(); i++) {
                if (!visibility[i] || sizes[i].x <= 0 || sizes[i].y <= 0 || sizes[i].z <= 0) continue;
                
                if (isRotating[i]) {
                    renderer->renderRotatedBox(positions[i], colors[i], sizes[i], rotationAxes[i], rotationAngles[i]);
                } else {
                    renderer->renderRealisticBox(positions[i], colors[i], sizes[i], blinkAlphas[i]);
                }
            }
            
            // プレイヤーの描画
            renderer->renderCube(gameState.playerPosition, gameState.playerColor, 0.5f);
            
            // チュートリアルUIを描画
            renderer->renderTutorialUI(width, height);
            
            renderer->endFrame();
            glfwPollEvents();
            continue; // チュートリアル表示中は他の処理をスキップ
        }
        
        // Ready画面表示中の処理
        if (gameState.showReadyScreen) {
            // Ready画面表示中はゲームを一時停止
            renderer->beginFrameWithBackground(stageManager.getCurrentStage());
            
            // カメラ設定
            glm::vec3 cameraPos, cameraTarget;
            if (stageManager.getCurrentStage() == 0) {
                cameraPos = gameState.playerPosition + glm::vec3(0, 15, -15);
                cameraTarget = gameState.playerPosition;
            } else {
                cameraPos = gameState.playerPosition + glm::vec3(0, 2, -8);
                cameraTarget = gameState.playerPosition;
            }
            renderer->setCamera(cameraPos, cameraTarget);

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            renderer->setProjection(45.0f, (float)width / (float)height, 0.1f, 100.0f);
            
            // 通常のゲーム要素を描画（背景として）
            auto positions = platformSystem.getPositions();
            auto sizes = platformSystem.getSizes();
            auto colors = platformSystem.getColors();
            auto visibility = platformSystem.getVisibility();
            auto isRotating = platformSystem.getIsRotating();
            auto rotationAngles = platformSystem.getRotationAngles();
            auto rotationAxes = platformSystem.getRotationAxes();
            auto blinkAlphas = platformSystem.getBlinkAlphas();
            
            for (size_t i = 0; i < positions.size(); i++) {
                if (!visibility[i] || sizes[i].x <= 0 || sizes[i].y <= 0 || sizes[i].z <= 0) continue;
                
                if (isRotating[i]) {
                    renderer->renderRotatedBox(positions[i], colors[i], sizes[i], rotationAxes[i], rotationAngles[i]);
                } else {
                    renderer->renderRealisticBox(positions[i], colors[i], sizes[i], blinkAlphas[i]);
                }
            }
            
            // プレイヤーの描画
            renderer->renderCube(gameState.playerPosition, gameState.playerColor, 0.5f);
            
            // Ready画面UIを描画
            renderer->renderReadyScreen(width, height, gameState.readyScreenSpeedLevel, gameState.isFirstPersonMode);
            
            renderer->endFrame();
            
            // キー状態更新（Ready画面中でも必要）
            for (auto& [key, state] : keyStates) {
                state.update(glfwGetKey(window, key) == GLFW_PRESS);
            }
            
            // Ready画面でのキー入力処理
            if (keyStates[GLFW_KEY_T].justPressed()) {
                gameState.readyScreenSpeedLevel = (gameState.readyScreenSpeedLevel + 1) % 3;
                printf("Ready screen speed level: %d\n", gameState.readyScreenSpeedLevel);
            }
            
            if (keyStates[GLFW_KEY_F].justPressed()) {
                gameState.isFirstPersonMode = !gameState.isFirstPersonMode;
                printf("Ready screen mode: %s\n", gameState.isFirstPersonMode ? "1ST PERSON" : "3RD PERSON");
            }
            
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.showReadyScreen = false;
                gameState.isCountdownActive = true;
                gameState.countdownTimer = 3.0f;
                
                // 選択されたモードを適用
                gameState.isFirstPersonView = gameState.isFirstPersonMode;
                
                // 1人称モード選択時はカメラ角度を初期化し、マウスカーソルを非表示
                if (gameState.isFirstPersonMode) {
                    gameState.cameraYaw = 90.0f;    // 後ろを向く
                    gameState.cameraPitch = -10.0f;   // 水平方向
                    gameState.firstMouse = true;  // マウス初期化フラグをリセット
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                } else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                
                // 選択された速度を設定
                switch (gameState.readyScreenSpeedLevel) {
                    case 0:
                        gameState.timeScale = 1.0f;
                        gameState.timeScaleLevel = 0;
                        break;
                    case 1:
                        gameState.timeScale = 2.0f;
                        gameState.timeScaleLevel = 1;
                        break;
                    case 2:
                        gameState.timeScale = 3.0f;
                        gameState.timeScaleLevel = 2;
                        break;
                }
                

                
                printf("Starting countdown with speed: %.1fx, mode: %s, isFirstPersonMode: %s\n", 
                       gameState.timeScale, 
                       gameState.isFirstPersonMode ? "1ST PERSON" : "3RD PERSON",
                       gameState.isFirstPersonMode ? "true" : "false");
            }
            
            glfwPollEvents();
            continue; // Ready画面表示中は他の処理をスキップ
        }
        
        // カウントダウン中の処理
        if (gameState.isCountdownActive) {
            // カウントダウン中はゲームを一時停止
            renderer->beginFrameWithBackground(stageManager.getCurrentStage());
            
            // カメラ設定
            glm::vec3 cameraPos, cameraTarget;
            if (stageManager.getCurrentStage() == 0) {
                cameraPos = gameState.playerPosition + glm::vec3(0, 15, -15);
                cameraTarget = gameState.playerPosition;
            } else {
                cameraPos = gameState.playerPosition + glm::vec3(0, 2, -8);
                cameraTarget = gameState.playerPosition;
            }
            renderer->setCamera(cameraPos, cameraTarget);

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            renderer->setProjection(45.0f, (float)width / (float)height, 0.1f, 100.0f);
            
            // 通常のゲーム要素を描画（背景として）
            auto positions = platformSystem.getPositions();
            auto sizes = platformSystem.getSizes();
            auto colors = platformSystem.getColors();
            auto visibility = platformSystem.getVisibility();
            auto isRotating = platformSystem.getIsRotating();
            auto rotationAngles = platformSystem.getRotationAngles();
            auto rotationAxes = platformSystem.getRotationAxes();
            auto blinkAlphas = platformSystem.getBlinkAlphas();
            
            for (size_t i = 0; i < positions.size(); i++) {
                if (!visibility[i] || sizes[i].x <= 0 || sizes[i].y <= 0 || sizes[i].z <= 0) continue;
                
                if (isRotating[i]) {
                    renderer->renderRotatedBox(positions[i], colors[i], sizes[i], rotationAxes[i], rotationAngles[i]);
                } else {
                    renderer->renderRealisticBox(positions[i], colors[i], sizes[i], blinkAlphas[i]);
                }
            }
            
            // プレイヤーの描画
            renderer->renderCube(gameState.playerPosition, gameState.playerColor, 0.5f);
            
            // カウントダウンUIを描画
            int count = (int)gameState.countdownTimer + 1;
            if (count > 0) {
                renderer->renderCountdown(width, height, count);
            }
            
            renderer->endFrame();
            
            // キー状態更新（カウントダウン中でも必要）
            for (auto& [key, state] : keyStates) {
                state.update(glfwGetKey(window, key) == GLFW_PRESS);
            }
            
            // カウントダウン処理
            gameState.countdownTimer -= deltaTime;
            
            if (gameState.countdownTimer <= 0.0f) {
                gameState.isCountdownActive = false;
                resetStageStartTime();  // カウントダウン終了時にゲーム開始時間をリセット
                // カウントダウン終了時にステージをロード（制限時間設定のため）
                stageManager.loadStage(stageManager.getCurrentStage(), gameState, platformSystem);
                printf("Countdown finished, starting gameplay! Mode: %s\n", 
                       gameState.isFirstPersonMode ? "1ST PERSON" : "3RD PERSON");
            }
            
            glfwPollEvents();
            continue; // カウントダウン中は他の処理をスキップ
        }
        
        // 時間停止スキルの更新
        if (gameState.isTimeStopped) {
            gameState.timeStopTimer -= deltaTime;
            if (gameState.timeStopTimer <= 0.0f) {
                gameState.isTimeStopped = false;
                gameState.timeStopTimer = 0.0f;
                printf("時間停止終了！\n");
            }
        }
        
        // 制限時間システムの更新（時間停止中は更新しない）
        if (!gameState.isStageCompleted && !gameState.isTimeUp && !gameState.isTimeStopped) {
            gameState.remainingTime -= deltaTime;
            
            // 時間切れ判定
            if (gameState.remainingTime <= 0.0f) {
                gameState.remainingTime = 0.0f;
                gameState.isTimeUp = true;
                gameState.isGameOver = true;  // ゲームオーバーフラグを設定
                printf("Time's up! Stage failed.\n");
            }
        }
        
        // システム更新（速度倍率を適用）
        platformSystem.update(scaledDeltaTime, gameState.playerPosition);
        GravitySystem::updateGravityZones(gameState, scaledDeltaTime);
        SwitchSystem::updateSwitches(gameState, scaledDeltaTime);
        CannonSystem::updateCannons(gameState, scaledDeltaTime);
        
        // 入力処理
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            gameRunning = false;
        }
        
        // キー状態更新
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
        
        // ステージ切り替え処理
        if (keyStates[GLFW_KEY_0].justPressed()) {
            resetStageStartTime();  // ステージ開始時間をリセット
            stageManager.goToStage(0, gameState, platformSystem);
            // 速度倍率をリセット
            gameState.timeScale = 1.0f;
            gameState.timeScaleLevel = 0;
            // 残機をリセット
            gameState.lives = 6;
            // Ready画面を非表示（ステージ選択フィールドでは表示しない）
            gameState.showReadyScreen = false;
            gameState.readyScreenShown = false;
        }
        if (keyStates[GLFW_KEY_1].justPressed()) {
            resetStageStartTime();  // ステージ開始時間をリセット
            stageManager.goToStage(1, gameState, platformSystem);
            // 速度倍率をリセット
            gameState.timeScale = 1.0f;
            gameState.timeScaleLevel = 0;
            // 残機をリセット
            gameState.lives = 6;
            
            // Ready画面のフラグをリセットして表示
            gameState.readyScreenShown = false;
            gameState.showReadyScreen = true;
            gameState.readyScreenSpeedLevel = 0;
        }
        if (keyStates[GLFW_KEY_2].justPressed()) {
            resetStageStartTime();  // ステージ開始時間をリセット
            stageManager.goToStage(2, gameState, platformSystem);
            // 速度倍率をリセット
            gameState.timeScale = 1.0f;
            gameState.timeScaleLevel = 0;
            // 残機をリセット
            gameState.lives = 6;
            
            // Ready画面のフラグをリセットして表示
            gameState.readyScreenShown = false;
            gameState.showReadyScreen = true;
            gameState.readyScreenSpeedLevel = 0;
        }
        if (keyStates[GLFW_KEY_3].justPressed()) {
            resetStageStartTime();  // ステージ開始時間をリセット
            stageManager.goToStage(3, gameState, platformSystem);
            // 速度倍率をリセット
            gameState.timeScale = 1.0f;
            gameState.timeScaleLevel = 0;
            // 残機をリセット
            gameState.lives = 6;
            
            // Ready画面のフラグをリセットして表示
            gameState.readyScreenShown = false;
            gameState.showReadyScreen = true;
            gameState.readyScreenSpeedLevel = 0;
        }
        if (keyStates[GLFW_KEY_4].justPressed()) {
            resetStageStartTime();  // ステージ開始時間をリセット
            stageManager.goToStage(4, gameState, platformSystem);
            // 速度倍率をリセット
            gameState.timeScale = 1.0f;
            gameState.timeScaleLevel = 0;
            // 残機をリセット
            gameState.lives = 6;
            
            // Ready画面のフラグをリセットして表示
            gameState.readyScreenShown = false;
            gameState.showReadyScreen = true;
            gameState.readyScreenSpeedLevel = 0;
        }
        if (keyStates[GLFW_KEY_5].justPressed()) {
            resetStageStartTime();  // ステージ開始時間をリセット
            stageManager.goToStage(5, gameState, platformSystem);
            // 速度倍率をリセット
            gameState.timeScale = 1.0f;
            gameState.timeScaleLevel = 0;
            // 残機をリセット
            gameState.lives = 6;
            
            // Ready画面のフラグをリセットして表示
            gameState.readyScreenShown = false;
            gameState.showReadyScreen = true;
            gameState.readyScreenSpeedLevel = 0;
        }
        
        
        // 速度制御処理（Tキー）- 全ステージで有効
        if (keyStates[GLFW_KEY_T].justPressed()) {
            if(gameState.currentStage==0){
                gameState.timeScaleLevel = 0;
            }else{
                gameState.timeScaleLevel = (gameState.timeScaleLevel + 1) % 3;
            }
            switch (gameState.timeScaleLevel) {
                case 0:
                    gameState.timeScale = 1.0f;
                    printf("Speed: Normal (1x), Gravity: Normal (1x)\n");
                    break;
                case 1:
                    gameState.timeScale = 2.0f;
                    printf("Speed: Fast (2x), Gravity: Strong (4x)\n");
                    break;
                case 2:
                    gameState.timeScale = 3.0f;
                    printf("Speed: Very Fast (3x), Gravity: Very Strong (9x)\n");
                    break;
            }
        }
        
        // お助けモード切り替え処理（Eキー）- ステージ0でのみ有効
        if (keyStates[GLFW_KEY_E].justPressed() && stageManager.getCurrentStage() == 0) {
            gameState.isEasyMode = !gameState.isEasyMode;
            printf("Easy mode: %s\n", gameState.isEasyMode ? "ON" : "OFF");
        }
        
        // 時間停止スキル取得切り替え処理（Rキー）- ステージ0でのみ有効（テスト用）
        if (keyStates[GLFW_KEY_R].justPressed() && stageManager.getCurrentStage() == 0) {
            gameState.hasTimeStopSkill = !gameState.hasTimeStopSkill;
            if (gameState.hasTimeStopSkill) {
                gameState.timeStopRemainingUses = gameState.timeStopMaxUses; // 使用回数をリセット
            }
            printf("Time Stop Skill: %s\n", gameState.hasTimeStopSkill ? "ACQUIRED" : "NOT ACQUIRED");
        }
        
        // 二段ジャンプスキル取得切り替え処理（Tキー）- ステージ0でのみ有効（テスト用）
        if (keyStates[GLFW_KEY_T].justPressed() && stageManager.getCurrentStage() == 0) {
            gameState.hasDoubleJumpSkill = !gameState.hasDoubleJumpSkill;
            if (gameState.hasDoubleJumpSkill) {
                gameState.doubleJumpRemainingUses = gameState.doubleJumpMaxUses; // 使用回数をリセット
            }
            printf("Double Jump Skill: %s\n", gameState.hasDoubleJumpSkill ? "ACQUIRED" : "NOT ACQUIRED");
        }
        
        // ハートフエールスキル取得切り替え処理（Yキー）- ステージ0でのみ有効（テスト用）
        if (keyStates[GLFW_KEY_Y].justPressed() && stageManager.getCurrentStage() == 0) {
            gameState.hasHeartFeelSkill = !gameState.hasHeartFeelSkill;
            if (gameState.hasHeartFeelSkill) {
                gameState.heartFeelRemainingUses = gameState.heartFeelMaxUses; // 使用回数をリセット
            }
            printf("Heart Feel Skill: %s\n", gameState.hasHeartFeelSkill ? "ACQUIRED" : "NOT ACQUIRED");
        }
        
        // 時間停止スキル処理（Qキー）
        if (keyStates[GLFW_KEY_Q].justPressed() && gameState.hasTimeStopSkill && !gameState.isTimeStopped && gameState.timeStopRemainingUses > 0) {
            gameState.isTimeStopped = true;
            gameState.timeStopTimer = gameState.timeStopDuration;
            gameState.timeStopRemainingUses--;
            printf("時間よ止まれ！残り使用回数: %d/%d\n", gameState.timeStopRemainingUses, gameState.timeStopMaxUses);
        }
        
        // ハートフエールスキル処理（Hキー）
        if (keyStates[GLFW_KEY_H].justPressed() && gameState.hasHeartFeelSkill && gameState.heartFeelRemainingUses > 0 && gameState.lives < 6) {
            gameState.lives++;
            gameState.heartFeelRemainingUses--;
            printf("ハートフエール！残機が1増加しました。残り使用回数: %d/%d, 残機: %d\n", 
                   gameState.heartFeelRemainingUses, gameState.heartFeelMaxUses, gameState.lives);
        }
        
        // カメラ切り替え処理（Fキー）- ステージ0でのみ有効
        if (keyStates[GLFW_KEY_F].justPressed() && stageManager.getCurrentStage() == 0) {
            gameState.isFirstPersonView = !gameState.isFirstPersonView;
            if (gameState.isFirstPersonView) {
                printf("Switched to First Person View\n");
                // 1人称視点時はマウスカーソルを非表示
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                // 1人称視点用のカメラ角度にリセット
                gameState.cameraYaw = 180.0f;    // 後ろを向く
                gameState.cameraPitch = -10.0f;   // 水平方向
                gameState.firstMouse = true;  // マウス初期化フラグをリセット
            } else {
                printf("Switched to Third Person View\n");
                // 3人称視点時はマウスカーソルを表示
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
        
        // ステージクリアUIでのキー入力処理
        if (gameState.showStageClearUI) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                // クリアしたステージ番号を保存
                int clearedStage = stageManager.getCurrentStage();
                
                // ステージ選択フィールドに戻る
                stageManager.completeStage(clearedStage);
                stageManager.goToStage(0, gameState, platformSystem);
                gameState.timeScale = 1.0f;
                gameState.timeScaleLevel = 0;
                
                // クリアしたステージに対応する位置にプレイヤーを配置
                glm::vec3 returnPosition;
                
                switch (clearedStage) {
                    case 1: // ステージ1クリア後
                        returnPosition = glm::vec3(7, 2.0f, 0); // 赤色エリア +1
                        break;
                    case 2: // ステージ2クリア後
                        returnPosition = glm::vec3(-7, 2.0f, 0); // 緑色エリア +1
                        break;
                    case 3: // ステージ3クリア後
                        returnPosition = glm::vec3(-3, 2.0f, 0); // 青色エリア +1
                        break;
                    case 4: // ステージ4クリア後
                        returnPosition = glm::vec3(5, 2.0f, 0); // 黄色エリア +1
                        break;
                    case 5: // ステージ5クリア後
                        returnPosition = glm::vec3(1, 2.0f, -5); // マゼンタエリア +1
                        break;
                    default:
                        returnPosition = glm::vec3(0, 2.0f, 0); // デフォルト位置
                        break;
                }
                
                gameState.playerPosition = returnPosition;
                gameState.playerVelocity = glm::vec3(0, 0, 0);
                
                printf("Returning to stage selection field at position (%.1f, %.1f, %.1f) after clearing stage %d\n", 
                       returnPosition.x, returnPosition.y, returnPosition.z, clearedStage);
                
                gameState.showStageClearUI = false;
                gameState.gameWon = false;
                gameState.isGoalReached = false;  // ゴール後の移動制限をリセット
            }
            
            if (keyStates[GLFW_KEY_R].justPressed()) {
                resetStageStartTime();  // ステージ開始時間をリセット
                stageManager.loadStage(stageManager.getCurrentStage(), gameState, platformSystem);
                gameState.showStageClearUI = false;
                gameState.gameWon = false;
                gameState.isGoalReached = false;  // ゴール後の移動制限をリセット
                // 速度倍率をリセット
                gameState.timeScale = 1.0f;
                gameState.timeScaleLevel = 0;
                // 残機をリセット
                gameState.lives = 6;
                gameState.showReadyScreen = true;
                gameState.readyScreenShown = false;
                gameState.readyScreenSpeedLevel = 0;
            }
        }
        

        
        // ゲームオーバーUIでのキー入力処理
        if (gameState.isGameOver) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                // ステージ選択フィールドに戻る
                stageManager.goToStage(0, gameState, platformSystem);
                gameState.timeScale = 1.0f;
                gameState.timeScaleLevel = 0;
                gameState.isGameOver = false;  // ゲームオーバーフラグをリセット
                gameState.isTimeUp = false;    // 時間切れフラグをリセット
                gameState.remainingTime = gameState.timeLimit;  // 残り時間をリセット
                
                // ステージ選択フィールドの中央にプレイヤーを配置
                gameState.playerPosition = glm::vec3(0, 2.0f, 0);
                gameState.playerVelocity = glm::vec3(0, 0, 0);
                
                printf("Returning to stage selection field after game over.\n");
            }
            
            if (keyStates[GLFW_KEY_R].justPressed()) {
                // 現在のステージをリトライ
                resetStageStartTime();  // ステージ開始時間をリセット
                stageManager.loadStage(stageManager.getCurrentStage(), gameState, platformSystem);
                gameState.isGameOver = false;  // ゲームオーバーフラグをリセット
                gameState.isTimeUp = false;    // 時間切れフラグをリセット
                gameState.remainingTime = gameState.timeLimit;  // 残り時間をリセット
                // 速度倍率をリセット
                gameState.timeScale = 1.0f;
                gameState.timeScaleLevel = 0;
                // 残機をリセット
                gameState.lives = 6;
                // プレイヤーの速度をリセット
                gameState.playerVelocity = glm::vec3(0, 0, 0);
                // チェックポイントをリセット
                gameState.lastCheckpoint = glm::vec3(0, 30.0f, 0);
                gameState.lastCheckpointItemId = -1;
                gameState.showReadyScreen = true;
                gameState.readyScreenShown = false;
                gameState.readyScreenSpeedLevel = 0;
                
                
                printf("Retrying current stage after game over.\n");
            }
        }
        
        // アイテム更新
        for (auto& item : gameState.items) {
            if (!item.isCollected) {
                // アイテムの回転
                item.rotationAngle += scaledDeltaTime * 90.0f; // 1秒で90度回転
                if (item.rotationAngle >= 360.0f) {
                    item.rotationAngle -= 360.0f;
                }
                
                // アイテムの上下の揺れ
                item.bobTimer += scaledDeltaTime;
                item.bobHeight = sin(item.bobTimer * 2.0f) * 0.2f;
                
                // プレイヤーとの距離チェック（アイテム収集）
                float distance = glm::length(gameState.playerPosition - item.position);
                if (distance < 1.5f) { // 収集範囲
                    item.isCollected = true;
                    gameState.collectedItems++;
                    
                    // チェックポイントを更新
                    gameState.lastCheckpoint = item.position;
                    gameState.lastCheckpointItemId = item.itemId;
                    
                    printf("Item %d collected! (%d/%d) - Checkpoint set at (%.1f, %.1f, %.1f)\n", 
                           item.itemId, gameState.collectedItems, gameState.requiredItems,
                           gameState.lastCheckpoint.x, gameState.lastCheckpoint.y, gameState.lastCheckpoint.z);
                }
            }
        }
        
        // 重力反転エリアのチェック
        glm::vec3 gravityDirection = glm::vec3(0, -1, 0); // デフォルトは下向き
        bool inGravityZone = PhysicsSystem::isPlayerInGravityZone(gameState, gameState.playerPosition, gravityDirection);
        
        // 物理演算
        float gravityStrength = calculateGravityStrength(12.0f, deltaTime, gameState.timeScale, gravityDirection, gameState);
        glm::vec3 gravityForce = gravityDirection * gravityStrength;

        // デバッグ: 重力値を出力（速度倍率が1倍より大きい時）
        if (gameState.timeScale > 1.0f) {
            static int debugCounter = 0;
            debugCounter++;
            if (debugCounter % 60 == 0) { // 1秒に1回程度
                printf("Debug - TimeScale: %.1f, GravityStrength: %.3f, Velocity.y: %.3f\n", 
                       gameState.timeScale, gravityStrength, gameState.playerVelocity.y);
            }
        }

        gameState.playerVelocity += gravityForce;
        
        // 速度倍率に応じて空気抵抗も調整（高速時はより少ない抵抗）
        float airResistance = (gameState.timeScale > 1.0f) ? 0.99f : 0.98f;
        gameState.playerVelocity *= airResistance;
        
        // 入力処理
        InputSystem::processInput(window, gameState, scaledDeltaTime);
        InputSystem::processJumpAndFloat(window, gameState, scaledDeltaTime, gravityDirection, platformSystem);

        // 垂直位置更新
        gameState.playerPosition.y += gameState.playerVelocity.y * scaledDeltaTime;

        // 衝突判定
        glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
        
        SwitchSystem::checkSwitchCollision(gameState, gameState.playerPosition, playerSize);
        CannonSystem::checkCannonCollision(gameState, gameState.playerPosition, playerSize);
        
        // プラットフォーム衝突判定（インデックス付き）
        auto collisionResult = platformSystem.checkCollisionWithIndex(gameState.playerPosition, playerSize);
        GameState::PlatformVariant* currentPlatform = collisionResult.first;
        int currentPlatformIndex = collisionResult.second;
        
        // 足場衝突処理
        if (currentPlatform != nullptr) {
            // お助けモード用：足場の中心位置とインデックスを記録
            if (gameState.isEasyMode) {
                std::visit([&](const auto& platform) {
                    gameState.lastPlatformPosition = platform.position;
                    gameState.lastPlatformIndex = currentPlatformIndex;
                    gameState.isTrackingPlatform = true;
                    
                    // 足場の種類を判定
                    std::string platformType = "Unknown";
                    if constexpr (std::is_same_v<decltype(platform), const GameState::StaticPlatform&>) {
                        platformType = "Static";
                    } else if constexpr (std::is_same_v<decltype(platform), const GameState::MovingPlatform&>) {
                        platformType = "Moving";
                    } else if constexpr (std::is_same_v<decltype(platform), const GameState::RotatingPlatform&>) {
                        platformType = "Rotating";
                    } else if constexpr (std::is_same_v<decltype(platform), const GameState::PatrollingPlatform&>) {
                        platformType = "Patrolling";
                    } else if constexpr (std::is_same_v<decltype(platform), const GameState::TeleportPlatform&>) {
                        platformType = "Teleport";
                    } else if constexpr (std::is_same_v<decltype(platform), const GameState::JumpPad&>) {
                        platformType = "JumpPad";
                    } else if constexpr (std::is_same_v<decltype(platform), const GameState::CycleDisappearingPlatform&>) {
                        platformType = "CycleDisappearing";
                    }
                    printf("Easy mode: Updated last %s platform (index %d) center to (%.1f, %.1f, %.1f)\n", 
                           platformType.c_str(), currentPlatformIndex, gameState.lastPlatformPosition.x, gameState.lastPlatformPosition.y, gameState.lastPlatformPosition.z);
                }, *currentPlatform);
            }
            
            std::visit(overloaded{
                [&](const GameState::StaticPlatform& platform) {
                    if (!PhysicsSystem::checkPlatformCollisionHorizontal(gameState, gameState.playerPosition, playerSize)) {
                        adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    }
                    
                    // ステージ選択フィールドでのステージ選択処理
                    if (stageManager.getCurrentStage() == 0) {
                        // デバッグ: ステージ3選択エリアにいるプラットフォームをログ出力
                        if (platform.position.x > -22.5f && platform.position.x < -21.5f && 
                            platform.position.y > 0.5f && platform.position.y < 1.5f &&
                            platform.position.z > -0.5f && platform.position.z < 0.5f) {
                            printf("DEBUG: Platform in Stage 3 selection area - Type: StaticPlatform, Position: (%.1f, %.1f, %.1f)\n", 
                                   platform.position.x, platform.position.y, platform.position.z);
                        }
                        // ステージ1選択エリア（位置で判定）
                        if (platform.position.x > 5.5f && platform.position.x < 6.5f && 
                            platform.position.z > -0.5f && platform.position.z < 0.5f) {
                            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                                int existingStars = gameState.stageStars[1];
                                printf("Selected Stage 1 (Current stars: %d)\n", existingStars);
    
                                resetStageStartTime();  // ステージ開始時間をリセット
                                gameState.lives = 6;
                                stageManager.goToStage(1, gameState, platformSystem);

                                gameState.showReadyScreen = true;
                                gameState.readyScreenShown = false;
                                gameState.readyScreenSpeedLevel = 0;
                            }
                        }
                        // ステージ2選択エリア（位置で判定）
                        else if (platform.position.x > -8.5f && platform.position.x < -7.5f && 
                                 platform.position.z > -0.5f && platform.position.z < 0.5f) {
                            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                                int existingStars = gameState.stageStars[2];
                                printf("Selected Stage 2 (Current stars: %d)\n", existingStars);
                                resetStageStartTime();  // ステージ開始時間をリセット
                                gameState.lives = 6;
                                if(gameState.totalStars>=1){
                                    stageManager.goToStage(2, gameState, platformSystem);
                                    gameState.readyScreenShown = false;
                                    gameState.showReadyScreen = true;
                                    gameState.readyScreenSpeedLevel = 0;
                                }
                            }
                        }
                        // ステージ3選択エリア（位置で判定）
                        else if (platform.position.x > -22.5f && platform.position.x < -21.5f && 
                                 platform.position.y > 0.5f && platform.position.y < 1.5f &&
                                 platform.position.z > -0.5f && platform.position.z < 0.5f) {
                            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                                int existingStars = gameState.stageStars[3];
                                printf("Selected Stage 3 (Current stars: %d)\n", existingStars);
                                resetStageStartTime();  // ステージ開始時間をリセット
                                gameState.lives = 6;
                                if(gameState.totalStars>=3){
                                    stageManager.goToStage(3, gameState, platformSystem);
                                    gameState.readyScreenShown = false;
                                    gameState.showReadyScreen = true;
                                    gameState.readyScreenSpeedLevel = 0;
                                }
                            }
                        }
                        // ステージ4選択エリア（位置で判定）
                        else if (platform.position.x > -34.5f && platform.position.x < -33.5f && 
                                 platform.position.y > 0.5f && platform.position.y < 1.5f &&
                                 platform.position.z > -0.5f && platform.position.z < 0.5f) {
                            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                                int existingStars = gameState.stageStars[4];
                                printf("Selected Stage 4 (Current stars: %d)\n", existingStars);
                                resetStageStartTime();  // ステージ開始時間をリセット
                                gameState.lives = 6;
                                if(gameState.totalStars>=5){
                                    stageManager.goToStage(4, gameState, platformSystem);
                                    gameState.readyScreenShown = false;
                                    gameState.showReadyScreen = true;
                                    gameState.readyScreenSpeedLevel = 0;
                                }
                            }
                        }
                        // ステージ5選択エリア（位置で判定）
                        else if (platform.position.x > -46.5f && platform.position.x < -45.5f && 
                                 platform.position.z > -0.5f && platform.position.z < 0.5f) {
                            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                                int existingStars = gameState.stageStars[5];
                                printf("Selected Stage 5 (Current stars: %d)\n", existingStars);
                                resetStageStartTime();  // ステージ開始時間をリセット
                                gameState.lives = 6;
                                if(gameState.totalStars>=7){
                                    stageManager.goToStage(5, gameState, platformSystem);
                                }
                            }
                        }
                    }
                    
                    // ゴール判定（黄色い足場の場合）
                    if (platform.color.r > 0.9f && platform.color.g > 0.9f && platform.color.b < 0.1f) {
                        if (!gameState.gameWon && gameState.collectedItems >= gameState.requiredItems) {
                            gameState.gameWon = true;
                            gameState.isStageCompleted = true;
                            gameState.isGoalReached = true;  // ゴール後の移動制限を有効化
                            printf("=== GOAL REACHED DEBUG ===\n");
                            gameState.clearTime = gameState.gameTime;
                            
                            // 星の計算（固定基準）
                            float remainingTime = gameState.timeLimit - gameState.clearTime;
                            float limitTime = gameState.timeLimit;
                            
                            if(limitTime>=30){
                                if (remainingTime >= 20.0f) {
                                    gameState.earnedStars = 3;
                                } else if (remainingTime >= 10.0f) {
                                    gameState.earnedStars = 2;
                                } else {
                                    gameState.earnedStars = 1;
                                }
                            }else{
                                if (remainingTime >= 10.0f) {
                                    gameState.earnedStars = 3;
                                } else if (remainingTime >= 5.0f) {
                                    gameState.earnedStars = 2;
                                } else {
                                    gameState.earnedStars = 1;
                                }
                            }
                            
                            // 星数管理システムの更新
                            int currentStage = stageManager.getCurrentStage();
                            int oldStars = gameState.stageStars[currentStage];
                            int starDifference = gameState.earnedStars - oldStars;
                            

                            
                            if (starDifference > 0) {
                                gameState.stageStars[currentStage] = gameState.earnedStars;
                                gameState.totalStars += starDifference;
                                printf("Stage %d stars updated: %d -> %d (+%d), Total: %d\n", 
                                       currentStage, oldStars, gameState.earnedStars, starDifference, gameState.totalStars);
                            }
                        
                            
                            

                            
                            gameState.showStageClearUI = true;
                            printf("Stage %d completed! All items collected (%d/%d) in %.1f seconds - %d stars earned!\n", 
                                   currentStage, gameState.collectedItems, gameState.requiredItems,
                                   gameState.clearTime, gameState.earnedStars);
                        } else if (!gameState.gameWon && gameState.collectedItems < gameState.requiredItems) {
                            printf("Need to collect all items first! (%d/%d)\n", gameState.collectedItems, gameState.requiredItems);
                        }
                    }
                },
                [&](const GameState::MovingPlatform& platform) {
                    adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    const_cast<GameState::MovingPlatform&>(platform).hasPlayerOnBoard = true;
                },
                [&](const GameState::RotatingPlatform& platform) {
                    adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                },
                [&](const GameState::PatrollingPlatform& platform) {
                    adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                },
                [&](const GameState::TeleportPlatform& platform) {
                    adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    
                    if (!platform.hasTeleported && platform.cooldownTimer <= 0.0f) {
                        gameState.playerPosition = platform.teleportDestination;
                        const_cast<GameState::TeleportPlatform&>(platform).hasTeleported = true;
                        const_cast<GameState::TeleportPlatform&>(platform).cooldownTimer = 2.0f;
                    }
                },
                [&](const GameState::JumpPad& platform) {
                    adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    gameState.playerVelocity.y = platform.jumpPower;
                },
                [&](const GameState::CycleDisappearingPlatform& platform) {
                    if (platform.isCurrentlyVisible) {
                        adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    }
                },
                [&](const GameState::DisappearingPlatform& platform) {
                    adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                },
                [&](const GameState::FlyingPlatform& platform) {
                    adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                }
            }, *currentPlatform);
        }
        
        // プレイヤー-足場同期処理
        if (currentPlatform != nullptr) {
            std::visit(overloaded{
                [&](const GameState::StaticPlatform& platform) {
                    // 静的足場は何もしない
                },
                [&](GameState::MovingPlatform& platform) {
                    if (platform.hasPlayerOnBoard) {
                        adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                        
                        // プレイヤーのX座標とZ座標を足場の移動に合わせて更新
                        glm::vec3 platformMovement = platform.position - platform.previousPosition;
                        gameState.playerPosition.x += platformMovement.x;
                        gameState.playerPosition.z += platformMovement.z;
                    }
                },
                [&](GameState::RotatingPlatform& platform) {
                    // 回転足場の処理
                    glm::vec3 halfSize = platform.size * 0.5f;
                    glm::vec3 platformMin = platform.position - halfSize;
                    glm::vec3 platformMax = platform.position + halfSize;
                    
                    bool onPlatform = (gameState.playerPosition.x >= platformMin.x && gameState.playerPosition.x <= platformMax.x &&
                                      gameState.playerPosition.z >= platformMin.z && gameState.playerPosition.z <= platformMax.z);
                    
                    if (onPlatform) {
                        // プレイヤーの位置を足場のローカル座標系に変換
                        glm::vec3 localPlayerPos = gameState.playerPosition - platform.position;
                        
                        // 回転軸に応じて回転を適用
                        if (glm::length(platform.rotationAxis - glm::vec3(0, 1, 0)) < 0.1f) {
                            // Y軸回転の場合、XZ平面での回転
                            float angle = glm::radians(platform.rotationSpeed * gameState.gameTime);
                            float cosAngle = cos(angle);
                            float sinAngle = sin(angle);
                            
                            float newX = localPlayerPos.x * cosAngle - localPlayerPos.z * sinAngle;
                            float newZ = localPlayerPos.x * sinAngle + localPlayerPos.z * cosAngle;
                            
                            gameState.playerPosition = platform.position + glm::vec3(newX, localPlayerPos.y, newZ);
                        } else if (glm::length(platform.rotationAxis - glm::vec3(1, 0, 0)) < 0.1f) {
                            // X軸回転（縦回転）の場合、YZ平面での回転
                            float angle = glm::radians(platform.rotationSpeed * gameState.gameTime);
                            float cosAngle = cos(angle);
                            float sinAngle = sin(angle);
                            
                            float newY = localPlayerPos.y * cosAngle - localPlayerPos.z * sinAngle;
                            float newZ = localPlayerPos.y * sinAngle + localPlayerPos.z * cosAngle;
                            
                            gameState.playerPosition = platform.position + glm::vec3(localPlayerPos.x, newY, newZ);
                        }
                        
                        adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    }
                },
                [&](GameState::PatrollingPlatform& platform) {
                    // 巡回足場の場合も同様の処理
                    glm::vec3 halfSize = platform.size * 0.5f;
                    glm::vec3 platformMin = platform.position - halfSize;
                    glm::vec3 platformMax = platform.position + halfSize;
                    
                    glm::vec3 extendedHalfSize = halfSize * 1.5f;
                    glm::vec3 extendedMin = platform.position - extendedHalfSize;
                    glm::vec3 extendedMax = platform.position + extendedHalfSize;
                    
                    bool onPlatform = (gameState.playerPosition.x >= platformMin.x && gameState.playerPosition.x <= platformMax.x &&
                                      gameState.playerPosition.z >= platformMin.z && gameState.playerPosition.z <= platformMax.z);
                    
                    bool inExtendedRange = (gameState.playerPosition.x >= extendedMin.x && gameState.playerPosition.x <= extendedMax.x &&
                                           gameState.playerPosition.z >= extendedMin.z && gameState.playerPosition.z <= extendedMax.z);
                    
                    if (onPlatform || inExtendedRange) {
                        adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                        
                        // プレイヤーのX座標とZ座標を足場の移動に合わせて更新
                        glm::vec3 platformMovement = platform.position - platform.previousPosition;
                        gameState.playerPosition.x += platformMovement.x;
                        gameState.playerPosition.z += platformMovement.z;
                        
                        // プレイヤーが足場の中心に近づくように調整
                        if (!onPlatform && inExtendedRange) {
                            glm::vec3 directionToCenter = platform.position - gameState.playerPosition;
                            directionToCenter.y = 0; // Y軸は無視
                            float distanceToCenter = glm::length(directionToCenter);
                            if (distanceToCenter > 0.1f) {
                                glm::vec3 normalizedDirection = glm::normalize(directionToCenter);
                                gameState.playerPosition += normalizedDirection * 0.5f * deltaTime;
                            }
                        }
                    }
                },
                [&](const auto& platform) {
                    // その他の足場タイプは何もしない
                }
            }, *currentPlatform);
        }
        
        // プレイヤーが足場から離れた時の処理
        if (currentPlatform == nullptr) {
            platformSystem.resetMovingPlatformFlags();
        }
        
        // 下限チェック（奈落に落ちた場合）
        if (gameState.playerPosition.y < 0) {
            if (gameState.isEasyMode) {
                // お助けモード：足場の追跡を試行
                if (gameState.isTrackingPlatform && gameState.lastPlatformIndex >= 0) {
                    const auto& platforms = platformSystem.getPlatforms();
                    if (gameState.lastPlatformIndex < platforms.size()) {
                        // 追跡中の足場の現在位置を取得
                        std::visit([&](const auto& platform) {
                            glm::vec3 currentPlatformPos = platform.position;
                            gameState.playerPosition = currentPlatformPos + glm::vec3(0, 2.0f, 0);
                            printf("Easy mode: Respawned at tracked platform (index %d) current position (%.1f, %.1f, %.1f)\n", 
                                   gameState.lastPlatformIndex, currentPlatformPos.x, currentPlatformPos.y, currentPlatformPos.z);
                        }, platforms[gameState.lastPlatformIndex]);
                    } else {
                        // インデックスが無効な場合は記録された位置にリスポーン
                        gameState.playerPosition = gameState.lastPlatformPosition + glm::vec3(0, 2.0f, 0);
                        printf("Easy mode: Respawned at last platform center (%.1f, %.1f, %.1f) - invalid index\n", 
                               gameState.lastPlatformPosition.x, gameState.lastPlatformPosition.y, gameState.lastPlatformPosition.z);
                    }
                } else {
                    // 追跡情報がない場合は記録された位置にリスポーン
                    gameState.playerPosition = gameState.lastPlatformPosition + glm::vec3(0, 2.0f, 0);
                    printf("Easy mode: Respawned at last platform center (%.1f, %.1f, %.1f) - no tracking\n", 
                           gameState.lastPlatformPosition.x, gameState.lastPlatformPosition.y, gameState.lastPlatformPosition.z);
                }
                gameState.playerVelocity = glm::vec3(0, 0, 0);
            } else {
                // 通常モード：残機を減らす
                gameState.lives--; // 残機を1つ減らす
                printf("Fell off! Lives remaining: %d\n", gameState.lives);
                
                if (gameState.lives <= 0) {
                    // ゲームオーバー
                    gameState.isGameOver = true;
                    printf("Game Over! No lives remaining.\n");
                } else {
                    // 残機がある場合はチェックポイントにリセット（アイテムは保持）
                    if (gameState.lastCheckpointItemId != -1) {
                        gameState.playerPosition = gameState.lastCheckpoint;
                        printf("Respawned at checkpoint (Item %d) at (%.1f, %.1f, %.1f). Items preserved.\n", 
                               gameState.lastCheckpointItemId,
                               gameState.lastCheckpoint.x, gameState.lastCheckpoint.y, gameState.lastCheckpoint.z);
                    } else {
                        gameState.playerPosition = glm::vec3(0, 6.0f, -25.0f); // スタート地点にリセット
                        printf("Respawned at start position (no checkpoint). Items preserved.\n");
                    }
                    gameState.playerVelocity = glm::vec3(0, 0, 0);
                    
                    // アイテム収集状況は保持（リセットしない）
                }
            }
        }

        // --------------------------
        //            描画
        // --------------------------
        renderer->beginFrameWithBackground(stageManager.getCurrentStage());
        
        // カメラ設定
        glm::vec3 cameraPos, cameraTarget;
        if (gameState.isFirstPersonView) {
            // 1人称視点：プレイヤーの目の位置
            cameraPos = gameState.playerPosition + glm::vec3(0, 1.0f, 0); // 目の高さ
            
            // マウス入力でカメラの向きを制御
            float yaw = glm::radians(gameState.cameraYaw);
            float pitch = glm::radians(gameState.cameraPitch);
            
            // カメラの向きを計算
            glm::vec3 direction;
            direction.x = cos(yaw) * cos(pitch);
            direction.y = sin(pitch);
            direction.z = sin(yaw) * cos(pitch);
            
            cameraTarget = cameraPos + direction;
        } else {
            // 3人称視点（現在の実装）
            if (stageManager.getCurrentStage() == 0) {
                // ステージ選択フィールドでは上からのアングル
                cameraPos = gameState.playerPosition + glm::vec3(0, 10, -15);
                cameraTarget = gameState.playerPosition;
            } else {
                // 通常のステージでは従来のアングル
                cameraPos = gameState.playerPosition + glm::vec3(0, 2, -8);
                cameraTarget = gameState.playerPosition;
            }
        }
        renderer->setCamera(cameraPos, cameraTarget);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        renderer->setProjection(45.0f, (float)width / (float)height, 0.1f, 100.0f);
        
        // 足場の描画
        auto positions = platformSystem.getPositions();
        auto sizes = platformSystem.getSizes();
        auto colors = platformSystem.getColors();
        auto visibility = platformSystem.getVisibility();
        auto isRotating = platformSystem.getIsRotating();
        auto rotationAngles = platformSystem.getRotationAngles();
        auto rotationAxes = platformSystem.getRotationAxes();
        auto blinkAlphas = platformSystem.getBlinkAlphas();
        
        for (size_t i = 0; i < positions.size(); i++) {
            if (!visibility[i] || sizes[i].x <= 0 || sizes[i].y <= 0 || sizes[i].z <= 0) continue;
            
            if (isRotating[i]) {
                renderer->renderRotatedBox(positions[i], colors[i], sizes[i], rotationAxes[i], rotationAngles[i]);
            } else {
                renderer->renderRealisticBox(positions[i], colors[i], sizes[i], blinkAlphas[i]);
            }
        }
        
        // 重力反転エリアの描画
        for (const auto& zone : gameState.gravityZones) {
            if (zone.isActive) {
                glm::vec3 zoneColor = glm::vec3(0.2f, 0.6f, 1.0f);
                renderer->renderBoxWithAlpha(zone.position, zoneColor, zone.size, 0.3f);
            }
        }
        
        // スイッチの描画
        for (const auto& switch_obj : gameState.switches) {
            glm::vec3 switchColor = switch_obj.color;
            if (switch_obj.isPressed) {
                switchColor *= 0.7f;
            }
            renderer->renderBox(switch_obj.position, switchColor, switch_obj.size);
        }
        
        // 大砲の描画
        for (const auto& cannon : gameState.cannons) {
            if (cannon.isActive) {
                glm::vec3 color = cannon.color;
                if (cannon.cooldownTimer > 0.0f) {
                    color *= 0.5f;
                }
                renderer->renderBox(cannon.position, color, cannon.size);
            }
        }
        
        // アイテムの描画
        for (const auto& item : gameState.items) {
            if (!item.isCollected) {
                glm::vec3 itemPos = item.position + glm::vec3(0, item.bobHeight, 0);
                renderer->renderRotatedBox(itemPos, item.color, item.size, glm::vec3(0, 1, 0), item.rotationAngle);
            }
        }
        
        // プレイヤーの描画（1人称視点時は描画しない）
        if (!gameState.isFirstPersonView) {
            renderer->renderCube(gameState.playerPosition, gameState.playerColor, 0.5f);
        }

        // UI（時間表示はrenderTimeUI関数で行うため削除）
        
        // ステージ情報
        const StageData* currentStageData = stageManager.getStageData(stageManager.getCurrentStage());
        if (currentStageData && stageManager.getCurrentStage()!=0) {
            // STAGEテキストを表示
            renderer->renderText("STAGE " + std::to_string(stageManager.getCurrentStage()), 
                               glm::vec2(30, 30), glm::vec3(1, 1, 0), 2.0f);
        }
        
        // // トータル星数表示
        // renderer->renderText("Total Stars: " + std::to_string(gameState.totalStars), 
        //                    glm::vec2(10, 230), glm::vec3(1.0f, 1.0f, 0.0f));
        
        
        
        // // アイテム収集状況の表示
        // renderer->renderText("Items: " + std::to_string(gameState.collectedItems) + "/" + std::to_string(gameState.requiredItems), 
        //                    glm::vec2(10, 70), glm::vec3(1, 1, 0));
        
        // // チェックポイント情報の表示
        // if (gameState.lastCheckpointItemId != -1) {
        //     renderer->renderText("Checkpoint: Item " + std::to_string(gameState.lastCheckpointItemId), 
        //                        glm::vec2(10, 90), glm::vec3(0, 1, 1));
        // } else {
        //     renderer->renderText("Checkpoint: None", glm::vec2(10, 90), glm::vec3(0.5f, 0.5f, 0.5f));
        // }
        
    
        // 速度倍率の表示（全ステージで表示）
        if(stageManager.getCurrentStage()!=0){
        std::string speedText = std::to_string((int)gameState.timeScale) + "x";
        glm::vec3 speedColor = (gameState.timeScale > 1.0f) ? glm::vec3(1.0f, 0.8f, 0.2f) : glm::vec3(1.0f, 1.0f, 1.0f);
        renderer->renderText(speedText, glm::vec2(880, 25), speedColor, 2.0f);

        std::string speedText2 =  "PRESS T";
        glm::vec3 speedColor2 = (gameState.timeScale > 1.0f) ? glm::vec3(1.0f, 0.8f, 0.2f) : glm::vec3(1.0f, 1.0f, 1.0f);
        renderer->renderText(speedText2, glm::vec2(870, 65), speedColor2, 1.0f);
        }
        // // 重力倍率の表示（速度倍率が1倍より大きい時のみ）
        // if (gameState.timeScale > 1.0f) {
        //     float gravityMultiplier = gameState.timeScale * gameState.timeScale;
        //     std::string gravityText = "Gravity: " + std::to_string((int)gravityMultiplier) + "x";
        //     renderer->renderText(gravityText, glm::vec2(10, 190), glm::vec3(1.0f, 0.6f, 0.2f));
        // }
        
        // 制限時間UIの表示（ステージ選択フィールドでは非表示）
        if (stageManager.getCurrentStage() != 0) {
            int currentStageStars = gameState.stageStars[stageManager.getCurrentStage()];
            renderer->renderTimeUI(gameState.remainingTime, gameState.timeLimit, gameState.earnedStars, currentStageStars, gameState.lives);
            
            // ハートフエールスキルUIを描画
            renderer->renderHeartFeelUI(gameState.hasHeartFeelSkill, gameState.heartFeelRemainingUses, 
                                       gameState.heartFeelMaxUses, gameState.lives);
            
            // 二段ジャンプスキルUIを描画
            renderer->renderDoubleJumpUI(gameState.hasDoubleJumpSkill, gameState.isEasyMode, 
                                        gameState.doubleJumpRemainingUses, gameState.doubleJumpMaxUses);
            
            // 時間停止スキルUIを描画
            renderer->renderTimeStopUI(gameState.hasTimeStopSkill, gameState.isTimeStopped, gameState.timeStopTimer, 
                                      gameState.timeStopRemainingUses, gameState.timeStopMaxUses);
        }
        

        

        
        // ステージクリアUI
        if (gameState.showStageClearUI) {
            // 背景とボックスを描画
            renderer->renderStageClearBackground(width, height, gameState.clearTime, gameState.earnedStars);
        }
        
        // ゲームオーバーUI
        if (gameState.isGameOver) {
            // 背景とUIを描画
            renderer->renderGameOverBackground(width, height);
        }
        
        // ステージ選択フィールド用のUI表示
        if (stageManager.getCurrentStage() == 0) {
            // 2DモードでUIを描画
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, width, height, 0, -1, 1);
            
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            
            // 深度テストを無効化（UI表示のため）
            glDisable(GL_DEPTH_TEST);
            
            renderer->renderText("WORLD 1", glm::vec2(width/2 - 50, 30), glm::vec3(1, 1, 0), 1.5f);
            
            // 左上に星アイコンとトータル星数を表示
            renderer->renderStar(glm::vec2(70, 70), glm::vec3(1.0f, 1.0f, 0.0f), 3.0f);
            renderer->renderText("x " + std::to_string(gameState.totalStars), glm::vec2(72, 27), glm::vec3(1.0f, 1.0f, 0.0f), 1.5f);
            
            // 2Dモードを終了
            glEnable(GL_DEPTH_TEST);
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
        }
        
        // ステージ0での3D星の描画（ステージ選択エリアの上に固定）
        if (stageManager.getCurrentStage() == 0) {
            // ステージ選択エリアの位置と高さを定義
            struct StageArea {
                float x, y, z;  // エリアの中心位置
                float height;   // エリアの高さ
            };
            
            StageArea stageAreas[] = {
                {6.0f, 0.0f, 0.0f, 1.0f},    // ステージ1
                {-8.0f, 0.0f, 0.0f, 1.0f},   // ステージ2
                {-22.0f, 0.0f, 0.0f, 1.0f},  // ステージ3
                {-34.0f, 0.0f, 0.0f, 1.0f},  // ステージ4
                {-46.0f, 0.0f, 0.0f, 1.0f}   // ステージ5
            };
            
            // 各ステージの星を描画
            for (int stage = 0; stage < 5; stage++) {
                StageArea& area = stageAreas[stage];
                int stageNumber = stage + 1;
                
                // ステージ2~5の解禁条件チェック
                bool isUnlocked = true;
                int requiredStars = 0;
                
                if (stageNumber == 2 && gameState.totalStars < 1) {
                    isUnlocked = false;
                    requiredStars = 1;
                } else if (stageNumber == 3 && gameState.totalStars < 3) {
                    isUnlocked = false;
                    requiredStars = 3;
                } else if (stageNumber == 4 && gameState.totalStars < 5) {
                    isUnlocked = false;
                    requiredStars = 5;
                } else if (stageNumber == 5 && gameState.totalStars < 7) {
                    isUnlocked = false;
                    requiredStars = 7;
                }
                
                if (isUnlocked) {
                    // 解禁済み：通常の星を表示
                    for (int i = 0; i < 3; i++) {
                        glm::vec3 starColor = (i < gameState.stageStars[stageNumber]) ? 
                            glm::vec3(1.0f, 1.0f, 0.0f) :  // 黄色（獲得済み）
                            glm::vec3(0.5f, 0.5f, 0.5f);   // 灰色（未獲得）
                        
                        // 星の位置をエリアの中心から計算
                        glm::vec3 starPos = glm::vec3(
                            area.x - 0.8f + i * 0.8f,  // 左から右に並べる
                            area.y + area.height + 1.5f,  // エリアの上に配置
                            area.z
                        );
                        
                        renderer->renderStar3D(starPos, starColor, 1.0f);
                    }
                } else {
                    // 未解禁：数値×星鍵穴の順番で横並びに表示
                    // 数値を左側に表示
                    glm::vec3 numberPos = glm::vec3(
                        area.x - 0.6f,  // エリアの中心より左
                        area.y + area.height + 1.5f,  // エリアの上に配置
                        area.z
                    );
                    renderer->renderNumber3D(numberPos, requiredStars, glm::vec3(1.0f, 1.0f, 0.0f), 1.0f);
                    
                    // ×記号を中央に表示
                    glm::vec3 xMarkPos = glm::vec3(
                        area.x,  // エリアの中心
                        area.y + area.height + 1.5f,  // エリアの上に配置
                        area.z
                    );
                    renderer->renderXMark3D(xMarkPos, glm::vec3(1.0f, 1.0f, 0.0f), 0.5f);
                    
                    // 星1個を右側に表示
                    glm::vec3 starPos = glm::vec3(
                        area.x + 0.6f,  // エリアの中心より右
                        area.y + area.height + 1.5f,  // エリアの上に配置
                        area.z
                    );
                    renderer->renderStar3D(starPos, glm::vec3(1.0f, 1.0f, 0.0f), 1.0f);
                    
                    // 鍵穴マークを最右側に表示
                    // glm::vec3 lockPos = glm::vec3(
                    //     area.x + 1.0f,  // 星の右側
                    //     area.y + area.height + 1.5f,  // エリアの上に配置
                    //     area.z
                    // );
                    // renderer->renderLock3D(lockPos, glm::vec3(0.6f, 0.3f, 0.3f), 1.5f);
                }
            }
        }
        
        // 操作説明
        std::string controlsText = "Controls: WASD=Move, SPACE=Jump, 0-5=Stage Select, LEFT/RIGHT=Next/Prev Stage, T=Speed Control, Q=Time Stop, H=Heart Feel";
        if (stageManager.getCurrentStage() == 0) {
            controlsText = "Controls: WASD=Move, SPACE=Select Stage, F=Camera Toggle, E=Easy Mode, R=Toggle Time Stop Skill, T=Toggle Double Jump Skill, Y=Toggle Heart Feel Skill";
        }
        renderer->renderText(controlsText, glm::vec2(10, height - 30), glm::vec3(0.8f, 0.8f, 0.8f));
        
        renderer->endFrame();
        
        // フレームレート制限
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        
        glfwPollEvents();
    }
    
    // クリーンアップ
    renderer->cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
