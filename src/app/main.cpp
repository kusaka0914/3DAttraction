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
#include "../game/stage_manager.h"
#include "../game/platform_system.h"
#include "../game/cannon_system.h"
#include "../game/switch_system.h"
#include "../game/gravity_system.h"
#include "../physics/physics_system.h"
#include "../io/input_system.h"
#include "../game/game_constants.h"
#include "../core/error_handler.h"
#include "../game/camera_system.h"

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
        gravityStrength *= timeScale * GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_TIME_SCALE;
    }
    if(gameState.currentStage==0){
        gravityStrength *= GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_STAGE_0;
    }
    
    // 重力反転時は70%の強度
    if (gravityDirection.y > GameConstants::PhysicsConstants::GRAVITY_DIRECTION_THRESHOLD) {
        gravityStrength *= GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_INVERTED;
    }
    
    // バーストジャンプ中は重力を半分にする
    if (gameState.isBurstJumpActive && !gameState.hasUsedBurstJump) {
        gravityStrength *= GameConstants::PhysicsCalculationConstants::GRAVITY_MULTIPLIER_BURST_JUMP;
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
    std::map<int, KeyState> keyStates;
    for (int key : {GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, 
                    GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_ENTER, GLFW_KEY_R, GLFW_KEY_T,
                    GLFW_KEY_F, GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE}) {
        keyStates[key] = KeyState();
    }

    // ゲーム開始準備完了
    bool gameRunning = true;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastFrameTime = startTime;
    
    // ステージ開始時間を管理する関数
    auto resetStageStartTime = [&startTime]() {
        startTime = std::chrono::high_resolution_clock::now();
    };
    
    // チュートリアルステージの進行処理関数
    auto processTutorialProgress = [&](GLFWwindow* window, GameState& gameState, 
                                      const std::map<int, KeyState>& keyStates) {
        
        if (gameState.tutorialStepCompleted && keyStates.at(GLFW_KEY_ENTER).justPressed()) {
            // 次のステップに進む
            gameState.tutorialStep++;
            gameState.tutorialStepCompleted = false;
            gameState.tutorialStartPosition = gameState.playerPosition;
            
            // 次のステップのメッセージを設定
            switch (gameState.tutorialStep) {
                case 1: gameState.tutorialMessage = "MOVING LEFT: PRESS A"; break;
                case 2: gameState.tutorialMessage = "MOVING BACKWARD: PRESS S"; break;
                case 3: gameState.tutorialMessage = "MOVING RIGHT: PRESS D"; break;
                case 4: gameState.tutorialMessage = "JUMPING: PRESS SPACE"; break;
                case 5: gameState.tutorialMessage = "SPEED UP: PRESS T"; break;
                case 6: gameState.tutorialMessage = "LIFE"; break;
                case 7: gameState.tutorialMessage = "TIME LIMIT"; break;
                case 8: gameState.tutorialMessage = "STAR"; break;
                case 9: gameState.tutorialMessage = "ITEM"; break;
                case 10: gameState.tutorialMessage = "GOAL"; break;
                default: gameState.tutorialMessage = ""; break;
            }
        }
        
        // 現在のステップでの完了条件をチェック
        if (!gameState.tutorialStepCompleted) {
            bool stepCompleted = false;
            
            if (gameState.tutorialStep >= 6 && gameState.tutorialStep < 9) {
                // STEP 6以降は即座に完了（Enterで進むため）
                stepCompleted = true;
            } else if (gameState.tutorialStep == 4) {
                // SPACEキーのステップ：ジャンプが実行されたかチェック（velocity.yが正の値になったかどうか）
                stepCompleted = (gameState.playerVelocity.y > 0.0f);
            } else if (gameState.tutorialStep == 5) {
                // Tキーのステップ：速度変更が行われたかチェック
                stepCompleted = (gameState.timeScale != 1.0f);
            } else if (gameState.tutorialStep == 9) {
                // ステップ9：アイテムを3つ全て取った時に完了
                stepCompleted = (gameState.earnedItems >= gameState.totalItems);
            } else if (gameState.tutorialStep == 10) {
                // ステップ10：ゴール地点に乗った時に完了
                stepCompleted = gameState.isGoalReached;
            } else {
                // その他のステップ：移動距離をチェック
                float distance = glm::length(gameState.playerPosition - gameState.tutorialStartPosition);
                stepCompleted = (distance >= gameState.tutorialRequiredDistance);
            }
            
            if (stepCompleted) {
                gameState.tutorialStepCompleted = true;
            }
        }
    };
    
    // --------------------------
    //         ゲームループ
    // --------------------------
    while (!glfwWindowShouldClose(window) && gameRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
        
        // deltaTimeの異常値を制限（フレームレート低下時の問題を防ぐ）
        deltaTime = std::min(deltaTime, GameConstants::MAX_DELTA_TIME);
        
        lastFrameTime = currentTime;
        
        gameState.gameTime = std::chrono::duration<float>(currentTime - startTime).count();
        
        // 速度倍率を適用したdeltaTimeを計算（全ステージで有効）
        float scaledDeltaTime = deltaTime * gameState.timeScale;
        
        // Ready画面表示中の処理
        if (gameState.showReadyScreen) {
            // Ready画面表示中はゲームを一時停止
            renderer->beginFrameWithBackground(stageManager.getCurrentStage());
            
            // カメラ設定
            auto cameraConfig = CameraSystem::calculateCameraConfig(gameState, stageManager, window);
            CameraSystem::applyCameraConfig(renderer.get(), cameraConfig, window);
            
            // ウィンドウサイズを取得
            auto [width, height] = CameraSystem::getWindowSize(window);
            
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
                    renderer->renderer3D.renderRotatedBox(positions[i], colors[i], sizes[i], rotationAxes[i], rotationAngles[i]);
                } else {
                    renderer->renderer3D.renderRealisticBox(positions[i], colors[i], sizes[i], blinkAlphas[i]);
                }
            }
            
            // プレイヤーの描画
            renderer->renderer3D.renderCube(gameState.playerPosition, gameState.playerColor, GameConstants::PLAYER_SCALE);
            
            // Ready画面UIを描画
            gameStateUIRenderer->renderReadyScreen(width, height, gameState.readyScreenSpeedLevel, gameState.isFirstPersonMode);
            
            renderer->endFrame();
            
            // キー状態更新（Ready画面中でも必要）
            for (auto& [key, state] : keyStates) {
                state.update(glfwGetKey(window, key) == GLFW_PRESS);
            }
            
            // Ready画面でのキー入力処理
            if (keyStates[GLFW_KEY_T].justPressed()) {
                gameState.readyScreenSpeedLevel = (gameState.readyScreenSpeedLevel + 1) % 3;
            }
            
            if (keyStates[GLFW_KEY_F].justPressed()) {
                gameState.isFirstPersonMode = !gameState.isFirstPersonMode;
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
            }
            
            glfwPollEvents();
            continue; // Ready画面表示中は他の処理をスキップ
        }
        
        // カウントダウン中の処理
        if (gameState.isCountdownActive) {
            // カウントダウン中はゲームを一時停止
            renderer->beginFrameWithBackground(stageManager.getCurrentStage());
            
            // カメラ設定
            auto cameraConfig = CameraSystem::calculateCameraConfig(gameState, stageManager, window);
            CameraSystem::applyCameraConfig(renderer.get(), cameraConfig, window);
            
            // ウィンドウサイズを取得
            auto [width, height] = CameraSystem::getWindowSize(window);
            
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
                    renderer->renderer3D.renderRotatedBox(positions[i], colors[i], sizes[i], rotationAxes[i], rotationAngles[i]);
                } else {
                    renderer->renderer3D.renderRealisticBox(positions[i], colors[i], sizes[i], blinkAlphas[i]);
                }
            }
            
            // プレイヤーの描画
            renderer->renderer3D.renderCube(gameState.playerPosition, gameState.playerColor, GameConstants::PLAYER_SCALE);
            
            // カウントダウンUIを描画
            int count = (int)gameState.countdownTimer + 1;
            if (count > 0) {
                gameStateUIRenderer->renderCountdown(width, height, count);
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
        
        // エンディングシーケンスの処理
        if (gameState.isEndingSequence) {
            // エンディングシーケンス中はゲームを一時停止
            renderer->beginFrameWithBackground(stageManager.getCurrentStage());
            
            // カメラ設定
            auto cameraConfig = CameraSystem::calculateCameraConfig(gameState, stageManager, window);
            CameraSystem::applyCameraConfig(renderer.get(), cameraConfig, window);
            
            // ウィンドウサイズを取得
            auto [width, height] = CameraSystem::getWindowSize(window);
            
            // スタッフロールの表示
            if (gameState.showStaffRoll) {
                gameState.staffRollTimer += deltaTime;
                
                // スタッフロールが15秒間表示されたら、エンディングメッセージに切り替え
                if (gameState.staffRollTimer >= 14.0f) {
                    gameState.showStaffRoll = false;
                    gameState.showEndingMessage = true;
                    gameState.endingMessageTimer = 0.0f;
                    printf("Staff roll finished, showing ending message...\n");
                } else {
                    gameStateUIRenderer->renderStaffRoll(width, height, gameState.staffRollTimer);
                }
            }
            
            // エンディングメッセージの表示
            if (gameState.showEndingMessage) {
                gameState.endingMessageTimer += deltaTime;
                
                // エンディングメッセージが5秒間表示されたら、フィールドに戻る
                if (gameState.endingMessageTimer >= 5.0f) {
                    // エンディングシーケンス終了
                    gameState.isEndingSequence = false;
                    gameState.showStaffRoll = false;
                    gameState.showEndingMessage = false;
                    gameState.staffRollTimer = 0.0f;
                    gameState.endingMessageTimer = 0.0f;
                    
                    // フィールドに戻る
                    stageManager.goToStage(0, gameState, platformSystem);
                    gameState.playerPosition = glm::vec3(8, 0, 0);
                    gameState.playerVelocity = glm::vec3(0, 0, 0);
                    printf("Ending sequence finished, returning to field...\n");
                } else {
                    gameStateUIRenderer->renderEndingMessage(width, height, gameState.endingMessageTimer);
                }
            }
            
            // Enterキーでスキップ処理
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                printf("Ending sequence skipped by user\n");
                
                // エンディングシーケンス終了
                gameState.isEndingSequence = false;
                gameState.showStaffRoll = false;
                gameState.showEndingMessage = false;
                gameState.staffRollTimer = 0.0f;
                gameState.endingMessageTimer = 0.0f;
                
                // フィールドに戻る
                stageManager.goToStage(0, gameState, platformSystem);
                gameState.playerPosition = glm::vec3(8, 0, 0);
                gameState.playerVelocity = glm::vec3(0, 0, 0);
            }
            
            renderer->endFrame();
            
            // キー状態更新（エンディング中でも必要）
            for (auto& [key, state] : keyStates) {
                state.update(glfwGetKey(window, key) == GLFW_PRESS);
            }
            
            glfwPollEvents();
            continue; // エンディングシーケンス中は他の処理をスキップ
        }
        
        // 時間停止スキル発動時の処理
        if (gameState.isTimeStopped) {
            gameState.timeStopTimer -= deltaTime;
            if (gameState.timeStopTimer <= 0.0f) {
                gameState.isTimeStopped = false;
                gameState.timeStopTimer = 0.0f;
            }
        }
        
        // フリーカメラスキル発動時の処理
        if (gameState.isFreeCameraActive) {
            gameState.freeCameraTimer -= deltaTime;
            if (gameState.freeCameraTimer <= 0.0f) {
                gameState.isFreeCameraActive = false;
                gameState.freeCameraTimer = 0.0f;
                // マウスカーソルを表示する
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
        
        // バーストジャンプ遅延タイマーの更新（バーストジャンプ中の処理）
        if (gameState.burstJumpDelayTimer > 0.0f) {
            gameState.burstJumpDelayTimer -= deltaTime;
            if (gameState.burstJumpDelayTimer <= 0.0f) {
                gameState.isInBurstJumpAir = true;
                gameState.burstJumpDelayTimer = 0.0f;
            }
        }
        
        // 制限時間システムの更新（時間停止中は更新しない）
        if (!gameState.isStageCompleted && !gameState.isTimeUp && !gameState.isTimeStopped) {
            gameState.remainingTime -= deltaTime;
            
            // 時間切れ判定
            if (gameState.remainingTime <= 0.0f) {
                gameState.remainingTime = 0.0f;
                gameState.isTimeUp = true;
                gameState.isGameOver = true;
                gameState.gameOverTimer = 0.0f;
            }
        }
        
        // ゲームオーバータイマーの更新
        if (gameState.isGameOver) {
            gameState.gameOverTimer += deltaTime;
        }
        
        // システム更新（速度倍率を適用）
        platformSystem.update(scaledDeltaTime, gameState.playerPosition);
        GravitySystem::updateGravityZones(gameState, scaledDeltaTime);
        SwitchSystem::updateSwitches(gameState, scaledDeltaTime);
        CannonSystem::updateCannons(gameState, scaledDeltaTime);
        
        // エスケープキーでゲームを終了
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            gameRunning = false;
        }
        
        // チュートリアルステージでの入力制御
        bool tutorialInputEnabled = true;
        if (gameState.isTutorialStage) {
            if (gameState.tutorialStepCompleted) {
                tutorialInputEnabled = false;  // ステップ完了時は入力無効
            } else if (gameState.tutorialStep == 6) {
                tutorialInputEnabled = false;  // ステップ6は動けない
            } else if (gameState.tutorialStep >= 7) {
                tutorialInputEnabled = true;   // ステップ7以降は動ける（アイテム取得やゴール到達のため）
            } else {
                // 現在のステップ以外のキーを無効化
                switch (gameState.tutorialStep) {
                    case 0: // Wキーのステップ
                        tutorialInputEnabled = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
                        break;
                    case 1: // Aキーのステップ
                        tutorialInputEnabled = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
                        break;
                    case 2: // Sキーのステップ
                        tutorialInputEnabled = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
                        break;
                    case 3: // Dキーのステップ
                        tutorialInputEnabled = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
                        break;
                    case 4: // SPACEキーのステップ
                        tutorialInputEnabled = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
                        break;
                    case 5: // Tキーのステップ
                        tutorialInputEnabled = keyStates[GLFW_KEY_T].justPressed();
                        break;
                    default:
                        tutorialInputEnabled = true; // ステップ6以降は全て有効
                        break;
                }
            }
        }
        
        // キー状態更新
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
        
        // ステージ切り替え処理
        for (int key = GLFW_KEY_0; key <= GLFW_KEY_6; key++) {
            if (keyStates[key].justPressed()) {
                int stageNumber = key - GLFW_KEY_0;
                
                // ステージ0（ステージ選択フィールド）は常にアクセス可能
                if (stageNumber == 0) {
                    resetStageStartTime();
                    stageManager.goToStage(stageNumber, gameState, platformSystem);
                    gameState.showReadyScreen = false;
                    gameState.readyScreenShown = false;
                } else {
                    // その他のステージはロック状態をチェック
                    if (gameState.unlockedStages.count(stageNumber) && gameState.unlockedStages[stageNumber]) {
                        // 解放済み：ステージに移動
                        resetStageStartTime();
                        stageManager.goToStage(stageNumber, gameState, platformSystem);
                        gameState.readyScreenShown = false;
                        gameState.showReadyScreen = true;
                        gameState.readyScreenSpeedLevel = 0;
                    } else {
                        // 未解放：解放確認UIを表示
                        gameState.showUnlockConfirmUI = true;
                        gameState.unlockTargetStage = stageNumber;
                        
                        // 必要スター数を設定
                        if (stageNumber == 1) {
                            gameState.unlockRequiredStars = GameConstants::STAGE_1_COST;
                            printf("DEBUG: Setting STAGE_1_COST = %d\n", GameConstants::STAGE_1_COST);
                        } else if (stageNumber == 2) {
                            gameState.unlockRequiredStars = GameConstants::STAGE_2_COST;
                        } else if (stageNumber == 3) {
                            gameState.unlockRequiredStars = GameConstants::STAGE_3_COST;
                        } else if (stageNumber == 4) {
                            gameState.unlockRequiredStars = GameConstants::STAGE_4_COST;
                        } else if (stageNumber == 5) {
                            gameState.unlockRequiredStars = GameConstants::STAGE_5_COST;
                        }
                        
                        printf("Stage %d is locked! Required stars: %d, Current stars: %d\n", 
                               stageNumber, gameState.unlockRequiredStars, gameState.totalStars);
                        printf("DEBUG: showUnlockConfirmUI=%s, unlockTargetStage=%d, unlockRequiredStars=%d\n", 
                               gameState.showUnlockConfirmUI ? "true" : "false", 
                               gameState.unlockTargetStage, gameState.unlockRequiredStars);
                    }
                }
                
                // デバッグ：ステージ選択処理後の状態を確認
                if (gameState.showUnlockConfirmUI) {
                    printf("DEBUG: After stage selection - unlockRequiredStars=%d\n", gameState.unlockRequiredStars);
                }
                
                // 共通のリセット処理（ステージ移動時のみ）
                if (stageNumber == 0 || (gameState.unlockedStages.count(stageNumber) && gameState.unlockedStages[stageNumber])) {
                    gameState.timeScale = 1.0f;
                    gameState.timeScaleLevel = 0;
                    gameState.lives = 6;
                }
            }
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
        
        // スキル取得切り替え処理（ステージ0でのみ有効）
        if (stageManager.getCurrentStage() == 0) {
            // スキル取得キーマッピング
            struct SkillToggle {
                int key;
                bool& hasSkill;
                int& remainingUses;
                int maxUses;
                const char* skillName;
            };
            
            std::vector<SkillToggle> skillToggles = {
                {GLFW_KEY_R, gameState.hasTimeStopSkill, gameState.timeStopRemainingUses, gameState.timeStopMaxUses, "Time Stop"},
                {GLFW_KEY_T, gameState.hasDoubleJumpSkill, gameState.doubleJumpRemainingUses, gameState.doubleJumpMaxUses, "Double Jump"},
                {GLFW_KEY_Y, gameState.hasHeartFeelSkill, gameState.heartFeelRemainingUses, gameState.heartFeelMaxUses, "Heart Feel"},
                {GLFW_KEY_U, gameState.hasFreeCameraSkill, gameState.freeCameraRemainingUses, gameState.freeCameraMaxUses, "Free Camera"},
                {GLFW_KEY_I, gameState.hasBurstJumpSkill, gameState.burstJumpRemainingUses, gameState.burstJumpMaxUses, "Burst Jump"}
            };
            
            for (const auto& skill : skillToggles) {
                if (keyStates[skill.key].justPressed()) {
                    skill.hasSkill = !skill.hasSkill;
                    if (skill.hasSkill) {
                        skill.remainingUses = skill.maxUses;
                    }
                    printf("%s Skill: %s\n", skill.skillName, skill.hasSkill ? "ACQUIRED" : "NOT ACQUIRED");
                }
            }
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
        
        // フリーカメラスキル処理（Cキー）- 3人称モードでのみ有効
        if (keyStates[GLFW_KEY_C].justPressed() && gameState.hasFreeCameraSkill && !gameState.isFreeCameraActive && 
            gameState.freeCameraRemainingUses > 0 && !gameState.isFirstPersonView) {
            gameState.isFreeCameraActive = true;
            gameState.freeCameraTimer = gameState.freeCameraDuration;
            gameState.freeCameraRemainingUses--;
            
            // 現在のカメラ角度を保存（カメラ位置はプレイヤーに対して固定）
            gameState.freeCameraYaw = gameState.cameraYaw;
            gameState.freeCameraPitch = gameState.cameraPitch;
            
            // マウスカーソルを非表示にして自由に動かせるようにする
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            
            printf("フリーカメラ開始！残り使用回数: %d/%d\n", gameState.freeCameraRemainingUses, gameState.freeCameraMaxUses);
        }
        
        // バーストジャンプスキル処理（Bキー）
        if (keyStates[GLFW_KEY_B].justPressed() && gameState.hasBurstJumpSkill && !gameState.isBurstJumpActive && 
            gameState.burstJumpRemainingUses > 0) {
            gameState.isBurstJumpActive = true;
            gameState.hasUsedBurstJump = false;
            gameState.burstJumpRemainingUses--;
            
            printf("バーストジャンプ準備完了！残り使用回数: %d/%d\n", gameState.burstJumpRemainingUses, gameState.burstJumpMaxUses);
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
        
        // ステージ0初回入場チュートリアルUIでのキー入力処理
        if (gameState.showStage0Tutorial && stageManager.getCurrentStage() == 0) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.showStage0Tutorial = false;
                printf("Stage 0 tutorial dismissed.\n");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
                        returnPosition = glm::vec3(GameConstants::STAGE_AREAS[0].x, GameConstants::STAGE_AREAS[0].y, GameConstants::STAGE_AREAS[0].z-1);
                        break;
                    case 2: // ステージ2クリア後    
                        returnPosition = glm::vec3(GameConstants::STAGE_AREAS[1].x, GameConstants::STAGE_AREAS[1].y, GameConstants::STAGE_AREAS[1].z-1);
                        break;
                    case 3: // ステージ3クリア後
                        returnPosition = glm::vec3(GameConstants::STAGE_AREAS[2].x, GameConstants::STAGE_AREAS[2].y, GameConstants::STAGE_AREAS[2].z-1); // 青色エリア +1
                        break;
                    case 4: // ステージ4クリア後
                        returnPosition = glm::vec3(GameConstants::STAGE_AREAS[3].x, GameConstants::STAGE_AREAS[3].y, GameConstants::STAGE_AREAS[3].z-1); // 黄色エリア +1
                        break;
                    case 5: // ステージ5クリア後
                        returnPosition = glm::vec3(GameConstants::STAGE_AREAS[4].x, GameConstants::STAGE_AREAS[4].y, GameConstants::STAGE_AREAS[4].z-1); // マゼンタエリア +1
                        break;
                    default:
                        returnPosition = glm::vec3(GameConstants::STAGE_AREAS[0].x, GameConstants::STAGE_AREAS[0].y, GameConstants::STAGE_AREAS[0].z-1); // デフォルト位置
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
                gameState.playerPosition = glm::vec3(8, 2.0f, 0);
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
                    
                    // チュートリアルステージ用のearnedItemsも更新
                    if (gameState.isTutorialStage) {
                        gameState.earnedItems++;
                        printf("Tutorial: earnedItems = %d / %d\n", gameState.earnedItems, gameState.totalItems);
                    }
                    
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
        float gravityStrength = calculateGravityStrength(GameConstants::BASE_GRAVITY, deltaTime, gameState.timeScale, gravityDirection, gameState);
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
        float airResistance = (gameState.timeScale > 1.0f) ? GameConstants::AIR_RESISTANCE_FAST : GameConstants::AIR_RESISTANCE_NORMAL;
        gameState.playerVelocity *= airResistance;
        
        // ステージ解放確認UIでのキー入力処理
        if (gameState.showUnlockConfirmUI) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                // デバッグ：解放前の状態を出力
                printf("DEBUG: Before unlock - Target: %d, Required: %d, Current: %d\n", 
                       gameState.unlockTargetStage, gameState.unlockRequiredStars, gameState.totalStars);
                
                // 確認：星を消費してステージを解放
                gameState.totalStars -= gameState.unlockRequiredStars;
                gameState.unlockedStages[gameState.unlockTargetStage] = true;
                
                // デバッグ：解放後の状態を出力
                printf("DEBUG: After unlock - Target: %d, Required: %d, Current: %d\n", 
                       gameState.unlockTargetStage, gameState.unlockRequiredStars, gameState.totalStars);
                printf("Stage %d unlocked! Cost: %d stars, Remaining: %d stars\n", 
                       gameState.unlockTargetStage, gameState.unlockRequiredStars, gameState.totalStars);
                
                // 解放後すぐにステージに移動
                resetStageStartTime();
                gameState.lives = 6;
                stageManager.goToStage(gameState.unlockTargetStage, gameState, platformSystem);
                gameState.readyScreenShown = false;
                gameState.showReadyScreen = true;
                gameState.readyScreenSpeedLevel = 0;
                
                // 確認UIを閉じる
                gameState.showUnlockConfirmUI = false;
                gameState.unlockTargetStage = 0;
                gameState.unlockRequiredStars = 0;
            }
            
            if (keyStates[GLFW_KEY_Q].justPressed()) {
                // キャンセル：確認UIを閉じる
                gameState.showUnlockConfirmUI = false;
                gameState.unlockTargetStage = 0;
                gameState.unlockRequiredStars = 0;
                printf("Stage unlock cancelled\n");
            }
            
            // 確認UI中は他の操作を無効化
            // 入力処理をスキップ
        } else if (gameState.showStarInsufficientUI) {
            if (keyStates[GLFW_KEY_Q].justPressed()) {
                // 星不足警告UIを閉じる
                gameState.showStarInsufficientUI = false;
                gameState.insufficientTargetStage = 0;
                gameState.insufficientRequiredStars = 0;
                printf("Star insufficient warning closed\n");
            }
            
            // 星不足警告UI中は他の操作を無効化
            // 入力処理をスキップ
        } else {
            // 通常の入力処理（チュートリアルステージでも共通処理を使用）
            if (tutorialInputEnabled) {
                InputSystem::processInput(window, gameState, scaledDeltaTime);
                InputSystem::processJumpAndFloat(window, gameState, scaledDeltaTime, gravityDirection, platformSystem);
            }
            
                    // チュートリアルステージの進行処理
        if (gameState.isTutorialStage) {
            processTutorialProgress(window, gameState, keyStates);
        }
        

        }

        // 垂直位置更新
        gameState.playerPosition.y += gameState.playerVelocity.y * scaledDeltaTime;

        // 衝突判定
        glm::vec3 playerSize = GameConstants::PLAYER_SIZE;
        
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
                            
                            if(limitTime >= GameConstants::LONG_TIME_THRESHOLD){
                                if (remainingTime >= GameConstants::STAR_3_TIME_LONG) {
                                    gameState.earnedStars = 3;
                                } else if (remainingTime >= GameConstants::STAR_2_TIME_LONG) {
                                    gameState.earnedStars = 2;
                                } else {
                                    gameState.earnedStars = 1;
                                }
                            }else{
                                if (remainingTime >= GameConstants::STAR_3_TIME_SHORT) {
                                    gameState.earnedStars = 3;
                                } else if (remainingTime >= GameConstants::STAR_2_TIME_SHORT) {
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
                        
                            
                            

                            
                            // ステージ5クリア後の特別処理
                            if (currentStage == 5) {
                                // エンディングシーケンス開始
                                gameState.isEndingSequence = true;
                                gameState.showStaffRoll = true;
                                gameState.staffRollTimer = 0.0f;
                                printf("Stage 5 completed! Starting ending sequence...\n");
                            } else {
                                // 通常のステージクリアUI表示
                                gameState.showStageClearUI = true;
                            }
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
        
        // ステージ選択エリアの判定と操作アシスト表示
        if (stageManager.getCurrentStage() == 0) {
            // プレイヤーの位置でステージ選択エリアの判定
            int selectedStage = -1;
            for (int stage = 0; stage < 5; stage++) {
                const auto& stageArea = GameConstants::STAGE_AREAS[stage];
                if (gameState.playerPosition.x > stageArea.x - GameConstants::STAGE_SELECTION_RANGE && 
                    gameState.playerPosition.x < stageArea.x + GameConstants::STAGE_SELECTION_RANGE && 
                    gameState.playerPosition.z > stageArea.z - GameConstants::STAGE_SELECTION_RANGE && 
                    gameState.playerPosition.z < stageArea.z + GameConstants::STAGE_SELECTION_RANGE
                ) {
                    selectedStage = stage + 1;
                    break;
                }
            }
            
            // 操作アシストUIの表示制御
            if (selectedStage > 0) {
                gameState.showStageSelectionAssist = true;
                gameState.assistTargetStage = selectedStage;
                printf("Player in stage %d selection area, showing assist UI\n", selectedStage);
            } else {
                gameState.showStageSelectionAssist = false;
                gameState.assistTargetStage = 0;
            }
            
            // ステージ選択処理（統一版）
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                
                if (selectedStage > 0) {
                    if (selectedStage == 6) {
                        // ステージ6（チュートリアル）は常に解放済み
                        resetStageStartTime();
                        gameState.lives = 6;
                        stageManager.goToStage(selectedStage, gameState, platformSystem);
                        gameState.readyScreenShown = false;
                        gameState.showReadyScreen = true;
                        gameState.readyScreenSpeedLevel = 0;
                    } else {
                        // ステージ1-5の解放処理
                        bool isUnlocked = gameState.unlockedStages[selectedStage];
                        
                        if (isUnlocked) {
                            // 既に解放済み：ステージに移動
                            resetStageStartTime();
                            gameState.lives = 6;
                            stageManager.goToStage(selectedStage, gameState, platformSystem);
                            gameState.readyScreenShown = false;
                            gameState.showReadyScreen = true;
                            gameState.readyScreenSpeedLevel = 0;
                        } else {
                            // 未解放：星を使用して解放
                            int requiredStars = 0;
                            switch (selectedStage) {
                                case 1: requiredStars = GameConstants::STAGE_1_COST; break;
                                case 2: requiredStars = GameConstants::STAGE_2_COST; break;
                                case 3: requiredStars = GameConstants::STAGE_3_COST; break;
                                case 4: requiredStars = GameConstants::STAGE_4_COST; break;
                                case 5: requiredStars = GameConstants::STAGE_5_COST; break;
                            }
                            
                            if (gameState.totalStars >= requiredStars) {
                                // 確認UIを表示
                                gameState.showUnlockConfirmUI = true;
                                gameState.unlockTargetStage = selectedStage;
                                gameState.unlockRequiredStars = requiredStars;
                            } else {
                                // 星不足警告UIを表示
                                gameState.showStarInsufficientUI = true;
                                gameState.insufficientTargetStage = selectedStage;
                                gameState.insufficientRequiredStars = requiredStars;
                            }
                        }
                    }
                }
            }
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
            } else if (stageManager.getCurrentStage() == 0) {
                // ステージ0：ライフを減らさずに中央にリスポーン
                gameState.playerPosition = glm::vec3(8, 2.0f, 0); // ステージ0の中央にリスポーン
                gameState.playerVelocity = glm::vec3(0, 0, 0);
                printf("Stage 0: Fell off but no life lost. Respawned at center.\n");
            } else {
                // 通常モード：残機を減らす
                gameState.lives--; // 残機を1つ減らす
                printf("Fell off! Lives remaining: %d\n", gameState.lives);
                
                if (gameState.lives <= 0) {
                    // ゲームオーバー
                    gameState.isGameOver = true;
                    gameState.gameOverTimer = 0.0f;  // ゲームオーバータイマーを初期化
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
        auto cameraConfig = CameraSystem::calculateCameraConfig(gameState, stageManager, window);
        CameraSystem::applyCameraConfig(renderer.get(), cameraConfig, window);
        
        // ウィンドウサイズを取得
        auto [width, height] = CameraSystem::getWindowSize(window);
        
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
                renderer->renderer3D.renderRotatedBox(positions[i], colors[i], sizes[i], rotationAxes[i], rotationAngles[i]);
            } else {
                renderer->renderer3D.renderRealisticBox(positions[i], colors[i], sizes[i], blinkAlphas[i]);
            }
        }
        
        // 重力反転エリアの描画
        for (const auto& zone : gameState.gravityZones) {
            if (zone.isActive) {
                renderer->renderer3D.renderBoxWithAlpha(zone.position, GameConstants::Colors::GRAVITY_ZONE_COLOR, 
                                           zone.size, GameConstants::Colors::GRAVITY_ZONE_ALPHA);
            }
        }
        
        // アイテムの描画
        for (const auto& item : gameState.items) {
            if (!item.isCollected) {
                glm::vec3 itemPos = item.position + glm::vec3(0, item.bobHeight, 0);
                renderer->renderer3D.renderRotatedBox(itemPos, item.color, item.size, glm::vec3(0, 1, 0), item.rotationAngle);
            }
        }
        
        // プレイヤーの描画（1人称視点時は描画しない）
        if (!gameState.isFirstPersonView) {
            renderer->renderer3D.renderCube(gameState.playerPosition, gameState.playerColor, GameConstants::PLAYER_SCALE);
        }
        
        // チュートリアルステージのUI描画
        if (gameState.isTutorialStage && gameState.showTutorialUI) {
            gameStateUIRenderer->renderTutorialStageUI(width, height, gameState.tutorialMessage, gameState.tutorialStep, gameState.tutorialStepCompleted);
        }
    

        // UI（時間表示はrenderTimeUI関数で行うため削除）
        
        // ステージ情報
        const StageData* currentStageData = stageManager.getStageData(stageManager.getCurrentStage());
        if (currentStageData && stageManager.getCurrentStage()!=0 && stageManager.getCurrentStage()!=6) {
            // STAGEテキストを表示
            uiRenderer->renderText("STAGE " + std::to_string(stageManager.getCurrentStage()), 
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
        
    
        // 速度倍率の表示（全ステージで表示、チュートリアルステージでは条件付き）
        if(stageManager.getCurrentStage()!=0){
            bool shouldShowSpeedUI = true;
            if (stageManager.getCurrentStage() == 6 && gameState.tutorialStep >= 6 && gameState.tutorialStep < 9) {
                shouldShowSpeedUI = false; // ステップ6-8は速度倍率UIを非表示
            }
            
            if (shouldShowSpeedUI) {
                std::string speedText = std::to_string((int)gameState.timeScale) + "x";
                glm::vec3 speedColor = (gameState.timeScale > 1.0f) ? glm::vec3(1.0f, 0.8f, 0.2f) : glm::vec3(1.0f, 1.0f, 1.0f);
                uiRenderer->renderText(speedText, glm::vec2(880, 25), speedColor, 2.0f);

                std::string speedText2 =  "PRESS T";
                glm::vec3 speedColor2 = (gameState.timeScale > 1.0f) ? glm::vec3(1.0f, 0.8f, 0.2f) : glm::vec3(1.0f, 1.0f, 1.0f);
                uiRenderer->renderText(speedText2, glm::vec2(870, 65), speedColor2, 1.0f);
            }
        }
        // // 重力倍率の表示（速度倍率が1倍より大きい時のみ）
        // if (gameState.timeScale > 1.0f) {
        //     float gravityMultiplier = gameState.timeScale * gameState.timeScale;
        //     std::string gravityText = "Gravity: " + std::to_string((int)gravityMultiplier) + "x";
        //     renderer->renderText(gravityText, glm::vec2(10, 190), glm::vec3(1.0f, 0.6f, 0.2f));
        // }
        
        // 制限時間UIの表示（ステージ選択フィールドでは非表示、チュートリアルステージでは条件付き表示）
        if (stageManager.getCurrentStage() != 0) {
            // チュートリアルステージ（ステージ6）では、ステップ6をクリアするまでUIを非表示
            bool shouldShowUI = true;
            if (stageManager.getCurrentStage() == 6) {
                // チュートリアルステージでは、ステップ6をクリアした後（ステップ6が完了した後）にライフのUIのみ表示
                shouldShowUI = (gameState.tutorialStep > 5);
            }
            
            if (shouldShowUI) {
                if (stageManager.getCurrentStage() == 6 && gameState.tutorialStep == 6) {
                    // チュートリアルステージでステップ6はライフのUIと説明を表示
                    uiRenderer->renderLivesWithExplanation(gameState.lives);
                } else if (stageManager.getCurrentStage() == 6 && gameState.tutorialStep == 7) {
                    // チュートリアルステージでステップ7はライフと制限時間UIを表示
                    int currentStageStars = gameState.stageStars[stageManager.getCurrentStage()];
                    uiRenderer->renderLivesAndTimeUI(gameState.lives, gameState.remainingTime, gameState.timeLimit, gameState.earnedStars, currentStageStars);
                } else if (stageManager.getCurrentStage() == 6 && gameState.tutorialStep == 8) {
                    // チュートリアルステージでステップ8はライフ、制限時間、星を表示
                    int currentStageStars = gameState.stageStars[stageManager.getCurrentStage()];
                    uiRenderer->renderLivesTimeAndStarsUI(gameState.lives, gameState.remainingTime, gameState.timeLimit, gameState.earnedStars, currentStageStars);
                } else if (stageManager.getCurrentStage() == 6 && gameState.tutorialStep >= 9) {
                    // チュートリアルステージでステップ9以降は通常のUIを表示（全部あるやつ）
                    int currentStageStars = gameState.stageStars[stageManager.getCurrentStage()];
                    uiRenderer->renderTimeUI(gameState.remainingTime, gameState.timeLimit, gameState.earnedStars, currentStageStars, gameState.lives);
                } else {
                    // 通常のステージでは全てのUIを表示（エンディングシーケンス中は非表示）
                    if (!gameState.isEndingSequence) {
                        int currentStageStars = gameState.stageStars[stageManager.getCurrentStage()];
                        uiRenderer->renderTimeUI(gameState.remainingTime, gameState.timeLimit, gameState.earnedStars, currentStageStars, gameState.lives);
                        
                        // フリーカメラスキルUIを描画
                        uiRenderer->renderFreeCameraUI(gameState.hasFreeCameraSkill, gameState.isFreeCameraActive, gameState.freeCameraTimer, 
                                                    gameState.freeCameraRemainingUses, gameState.freeCameraMaxUses);
                        
                        // バーストジャンプスキルUIを描画
                        uiRenderer->renderBurstJumpUI(gameState.hasBurstJumpSkill, gameState.isBurstJumpActive, 
                                                   gameState.burstJumpRemainingUses, gameState.burstJumpMaxUses);
                        
                        // ハートフエールスキルUIを描画
                        uiRenderer->renderHeartFeelUI(gameState.hasHeartFeelSkill, gameState.heartFeelRemainingUses, 
                                                   gameState.heartFeelMaxUses, gameState.lives);
                        
                        // 二段ジャンプスキルUIを描画
                        uiRenderer->renderDoubleJumpUI(gameState.hasDoubleJumpSkill, gameState.isEasyMode, 
                                                    gameState.doubleJumpRemainingUses, gameState.doubleJumpMaxUses);
                        
                        // 時間停止スキルUIを描画
                        uiRenderer->renderTimeStopUI(gameState.hasTimeStopSkill, gameState.isTimeStopped, gameState.timeStopTimer, 
                                                  gameState.timeStopRemainingUses, gameState.timeStopMaxUses);
                    }
                }
            }
        }
        

        

        

        
        // ステージクリアUI（エンディングシーケンス中は非表示）
        if (gameState.showStageClearUI && !gameState.isEndingSequence) {
            // 背景とボックスを描画
            gameStateUIRenderer->renderStageClearBackground(width, height, gameState.clearTime, gameState.stageStars[stageManager.getCurrentStage()]);
        }
        
        // ステージ0での3D星の描画（ステージ選択エリアの上に固定）
        if (stageManager.getCurrentStage() == 0) {
            // GameConstantsのSTAGE_AREASを使用
            
            // 各ステージの星を描画
            for (int stage = 0; stage < 5; stage++) {
                const auto& area = GameConstants::STAGE_AREAS[stage];
                int stageNumber = stage + 1;
                
                // ステージ0（ステージ選択フィールド）は常に解放済み、ステージ1-5はロック状態をチェック
                bool isUnlocked = (stageNumber == 0) || gameState.unlockedStages[stageNumber];
                int requiredStars = 0;

                if(stageNumber == 1){
                    requiredStars = GameConstants::STAGE_1_COST;
                }else if (stageNumber == 2) {
                    requiredStars = GameConstants::STAGE_2_COST;
                } else if (stageNumber == 3) {
                    requiredStars = GameConstants::STAGE_3_COST;
                } else if (stageNumber == 4) {
                    requiredStars = GameConstants::STAGE_4_COST;
                } else if (stageNumber == 5) {
                    requiredStars = GameConstants::STAGE_5_COST;
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
                        
                        renderer->renderer3D.renderStar3D(starPos, starColor, 1.0f);
                    }
                } else {
                    // 未解禁：数値×星鍵穴の順番で横並びに表示
                    // 数値を左側に表示
                    glm::vec3 numberPos = glm::vec3(
                        area.x - 0.6f,  // エリアの中心より左
                        area.y + area.height + 1.5f,  // エリアの上に配置
                        area.z
                    );
                    renderer->renderer3D.renderNumber3D(numberPos, requiredStars, glm::vec3(1.0f, 1.0f, 0.0f), 1.0f);
                    
                    // ×記号を中央に表示
                    glm::vec3 xMarkPos = glm::vec3(
                        area.x,  // エリアの中心
                        area.y + area.height + 1.5f,  // エリアの上に配置
                        area.z
                    );
                    renderer->renderer3D.renderXMark3D(xMarkPos, glm::vec3(1.0f, 1.0f, 0.0f), 0.5f);
                    
                    // 星1個を右側に表示
                    glm::vec3 starPos = glm::vec3(
                        area.x + 0.6f,  // エリアの中心より右
                        area.y + area.height + 1.5f,  // エリアの上に配置
                        area.z
                    );
                    renderer->renderer3D.renderStar3D(starPos, glm::vec3(1.0f, 1.0f, 0.0f), 1.0f);
                }
            }
        }
        
        // ステージ解放確認UI
        if (gameState.showUnlockConfirmUI) {
            // 背景とUIを描画
            gameStateUIRenderer->renderUnlockConfirmBackground(width, height, gameState.unlockTargetStage, gameState.unlockRequiredStars, gameState.totalStars);
        }
        
        // 星不足警告UI
        if (gameState.showStarInsufficientUI) {
            // 背景とUIを描画
            gameStateUIRenderer->renderStarInsufficientBackground(width, height, gameState.insufficientTargetStage, gameState.insufficientRequiredStars, gameState.totalStars);
        }
        

        
        // ゲームオーバーUI
        if (gameState.isGameOver) {
            // 背景とUIを描画
            gameStateUIRenderer->renderGameOverBackground(width, height);
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
            
            uiRenderer->renderText("WORLD 1", glm::vec2(width/2 - 50, 30), glm::vec3(1, 1, 0), 1.5f);
            
            // 左上に星アイコンとトータル星数を表示
            uiRenderer->renderStar(glm::vec2(70, 70), glm::vec3(1.0f, 1.0f, 0.0f), 3.0f);
            uiRenderer->renderText("x " + std::to_string(gameState.totalStars), glm::vec2(72, 27), glm::vec3(1.0f, 1.0f, 0.0f), 1.5f);
            
            // 初回ステージ0入場チュートリアルUIの表示
            if (gameState.showStage0Tutorial) {
                gameStateUIRenderer->renderStage0Tutorial(width, height);
            }
            
            // 操作アシストUI（トータルスターのUIと同じタイミングで描画）
            // UNLOCK確認UIや星不足警告UIが表示されている時は非表示
            bool shouldShowAssist = gameState.showStageSelectionAssist && 
                                   !gameState.showUnlockConfirmUI && 
                                   !gameState.showStarInsufficientUI &&
                                   !gameState.showStage0Tutorial; // チュートリアル表示中は非表示
            bool isStageUnlocked = gameState.unlockedStages[gameState.assistTargetStage];
            gameStateUIRenderer->renderStageSelectionAssist(width, height, gameState.assistTargetStage, shouldShowAssist, isStageUnlocked);
            
            // 2Dモードを終了
            glEnable(GL_DEPTH_TEST);
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
        }
        
        // 操作説明
        std::string controlsText = "Controls: WASD=Move, SPACE=Jump, 0-5=Stage Select, LEFT/RIGHT=Next/Prev Stage, T=Speed Control, Q=Time Stop, H=Heart Feel, C=Free Camera, B=Burst Jump";
        if (stageManager.getCurrentStage() == 0) {
            controlsText = "Controls: WASD=Move, SPACE=Select Stage, F=Camera Toggle, E=Easy Mode, R=Toggle Time Stop Skill, T=Toggle Double Jump Skill, Y=Toggle Heart Feel Skill, U=Toggle Free Camera Skill, I=Toggle Burst Jump Skill";
        }
        uiRenderer->renderText(controlsText, glm::vec2(10, height - 30), glm::vec3(0.8f, 0.8f, 0.8f));
        
        renderer->endFrame();
        
        // フレームレート制限
        std::this_thread::sleep_for(std::chrono::milliseconds(GameConstants::FRAME_DELAY_MS));
        
        glfwPollEvents();
    }
    
    // クリーンアップ
    renderer->cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
