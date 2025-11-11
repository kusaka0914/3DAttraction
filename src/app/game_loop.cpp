#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_loop.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <variant>
#include <glm/gtc/matrix_transform.hpp>
#include "../core/constants/game_constants.h"
#include "../core/constants/debug_config.h"
#include "../gfx/camera_system.h"
#include "../gfx/texture_manager.h"
#include "../game/gravity_system.h"
#include "../game/switch_system.h"
#include "../game/cannon_system.h"
#include "../physics/physics_system.h"
#include "../core/utils/physics_utils.h"
#include "../io/input_system.h"
#include "../io/audio_manager.h"
#include "../gfx/minimap_renderer.h"
#include "tutorial_manager.h"
#include <set>
#include <map>

namespace GameLoop {

    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    // 共通ヘルパー: ステージ解放に必要なスター数を返す
    static int requiredStarsFor(int stageNumber) {
        switch (stageNumber) {
            case 1: return GameConstants::STAGE_1_COST;
            case 2: return GameConstants::STAGE_2_COST;
            case 3: return GameConstants::STAGE_3_COST;
            case 4: return GameConstants::STAGE_4_COST;
            case 5: return GameConstants::STAGE_5_COST;
            default: return 0;
        }
    }

    // 共通ヘルパー: ステージ選択エリアへのワープ機能
    static void teleportToStageArea(int stageNumber, GameState& gameState) {
        const auto& area = GameConstants::STAGE_AREAS[stageNumber - 1];
        
        // エリアの目の前にワープ（少し離れた位置）
        glm::vec3 teleportPosition = glm::vec3(
            area.x,  // エリアの前に2ユニット
            area.y + 1.0f,  // エリアと同じ高さ
            area.z - 2.0f
        );
        
        gameState.playerPosition = teleportPosition;
        printf("Teleported to Stage %d area\n", stageNumber);
    }

    // 共通ヘルパー: ステージ選択時の実処理（解放/移動/UI）
    static void processStageSelectionAction(
        int stageNumber,
        GameState& gameState,
        StageManager& stageManager,
        PlatformSystem& platformSystem,
        std::function<void()> resetStageStartTime,
        GLFWwindow* window
    ) {
        // ワープ機能説明UI表示中は入力を無視
        if (gameState.showWarpTutorialUI) {
            return;
        }
        if (stageNumber == 0) {
            // ステージ選択フィールドへ
            resetStageStartTime();
            stageManager.goToStage(0, gameState, platformSystem);
            gameState.showReadyScreen = false;
            gameState.readyScreenShown = false;
            gameState.timeScale = 1.0f;
            gameState.timeScaleLevel = 0;
            
            // ステージ選択フィールドではTPSモードに戻す
            gameState.isFirstPersonMode = false;
            gameState.isFirstPersonView = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            
            return;
        }

        // ロック状態のチェック
        const bool isUnlocked = (gameState.unlockedStages.count(stageNumber) && gameState.unlockedStages[stageNumber]);
        if (!isUnlocked) {
            // 未解放: 確認UIを表示
            gameState.showUnlockConfirmUI = true;
            gameState.unlockTargetStage = stageNumber;
            gameState.unlockRequiredStars = requiredStarsFor(stageNumber);
            return;
        }

        // 解放済み: ステージへ移動
        resetStageStartTime();
        gameState.lives = 6;
        stageManager.goToStage(stageNumber, gameState, platformSystem);
        gameState.readyScreenShown = false;
        gameState.showReadyScreen = true;
        gameState.readyScreenSpeedLevel = 0;
        gameState.timeScale = 1.0f;
        gameState.timeScaleLevel = 0;
    }

    void run(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
            PlatformSystem& platformSystem,
            std::unique_ptr<gfx::OpenGLRenderer>& renderer,
            std::unique_ptr<gfx::UIRenderer>& uiRenderer,
            std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
            std::map<int, InputUtils::KeyState>& keyStates,
            std::function<void()> resetStageStartTime,
            std::chrono::high_resolution_clock::time_point& startTime,
            io::AudioManager& audioManager) {
        
        // SE（効果音）の初期化
        if (gameState.audioEnabled) {
            audioManager.loadSFX("jump", "../assets/audio/se/jump.mp3");
            audioManager.loadSFX("clear", "../assets/audio/se/clear.mp3");
            audioManager.loadSFX("item", "../assets/audio/se/item.mp3");
            audioManager.loadSFX("on_ground", "../assets/audio/se/on_ground.mp3");
            audioManager.loadSFX("flying", "../assets/audio/se/flying.mp3");
            audioManager.loadSFX("countdown", "../assets/audio/se/countdown.mp3");
            audioManager.loadSFX("tutorial_ok", "../assets/audio/se/tutorial_ok.mp3");
            
            // 作業ディレクトリがプロジェクトルートの場合に対応
            if (!audioManager.loadSFX("jump", "assets/audio/se/jump.mp3")) {
                audioManager.loadSFX("jump", "../assets/audio/se/jump.mp3");
            }
            if (!audioManager.loadSFX("clear", "assets/audio/se/clear.mp3")) {
                audioManager.loadSFX("clear", "../assets/audio/se/clear.mp3");
            }
            if (!audioManager.loadSFX("item", "assets/audio/se/item.mp3")) {
                audioManager.loadSFX("item", "../assets/audio/se/item.mp3");
            }
            if (!audioManager.loadSFX("on_ground", "assets/audio/se/on_ground.mp3")) {
                audioManager.loadSFX("on_ground", "../assets/audio/se/on_ground.mp3");
            }
            if (!audioManager.loadSFX("flying", "assets/audio/se/flying.mp3")) {
                audioManager.loadSFX("flying", "../assets/audio/se/flying.mp3");
            }
            if (!audioManager.loadSFX("countdown", "assets/audio/se/countdown.mp3")) {
                audioManager.loadSFX("countdown", "../assets/audio/se/countdown.mp3");
            }
            if (!audioManager.loadSFX("tutorial_ok", "assets/audio/se/tutorial_ok.mp3")) {
                audioManager.loadSFX("tutorial_ok", "../assets/audio/se/tutorial_ok.mp3");
            }
        }
        
        auto lastFrameTime = startTime;
        bool gameRunning = true;

        while (!glfwWindowShouldClose(window) && gameRunning) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
            
            // deltaTimeの異常値を制限（フレームレート低下時の問題を防ぐ）
            deltaTime = std::min(deltaTime, GameConstants::MAX_DELTA_TIME);
            
            lastFrameTime = currentTime;
            
            gameState.gameTime = std::chrono::duration<float>(currentTime - startTime).count();
            
            // 速度倍率を適用したdeltaTimeを計算（全ステージで有効）
            float scaledDeltaTime = deltaTime * gameState.timeScale;
            
            // BGM管理：各ステージに対応するBGMを再生
            int currentStage = stageManager.getCurrentStage();
            if (gameState.audioEnabled && !gameState.isGoalReached) {
                std::string targetBGM = "";
                std::string bgmPath = "";
                
                // ステージに応じてBGMを決定
                switch (currentStage) {
                    case 0: // ステージ選択画面
                        targetBGM = "stage_select_field.mp3";
                        bgmPath = "../assets/audio/bgm/stage_select_field.mp3";
                        break;
                    case 1: // ステージ1
                        targetBGM = "stage1.mp3";
                        bgmPath = "../assets/audio/bgm/stage1.mp3";
                        break;
                    case 2: // ステージ2
                        targetBGM = "stage2.mp3";
                        bgmPath = "../assets/audio/bgm/stage2.mp3";
                        break;
                    case 3: // ステージ3
                        targetBGM = "stage3.mp3";
                        bgmPath = "../assets/audio/bgm/stage3.mp3";
                        break;
                    case 4: // ステージ4
                        targetBGM = "stage4.mp3";
                        bgmPath = "../assets/audio/bgm/stage4.mp3";
                        break;
                    case 5: // ステージ5
                        targetBGM = "stage5.mp3";
                        bgmPath = "../assets/audio/bgm/stage5.mp3";
                        break;
                    case 6: // チュートリアルステージ
                        targetBGM = "tutorial.mp3";
                        bgmPath = "../assets/audio/bgm/tutorial.mp3";
                        break;
                    default:
                        // その他のステージはBGMなし
                        break;
                }
                
                // BGMが必要なステージの場合
                if (!targetBGM.empty()) {
                    if (gameState.currentBGM != targetBGM) {
                        // 現在のBGMを停止
                        if (gameState.bgmPlaying) {
                            audioManager.stopBGM();
                            gameState.bgmPlaying = false;
                        }
                        
                        // 新しいBGMを読み込み・再生
                        if (audioManager.loadBGM(bgmPath)) {
                            audioManager.playBGM();
                            gameState.currentBGM = targetBGM;
                            gameState.bgmPlaying = true;
                            std::cout << "BGM started: " << targetBGM << std::endl;
                        } else {
                            // 作業ディレクトリがプロジェクトルートの場合に対応
                            std::string altPath = "assets/audio/bgm/" + targetBGM;
                            if (audioManager.loadBGM(altPath)) {
                                audioManager.playBGM();
                                gameState.currentBGM = targetBGM;
                                gameState.bgmPlaying = true;
                                std::cout << "BGM started (alternative path): " << targetBGM << std::endl;
                            }
                        }
                    }
                } else if (gameState.bgmPlaying) {
                    // BGMが不要なステージに移動した場合はBGMを停止
                    audioManager.stopBGM();
                    gameState.bgmPlaying = false;
                    gameState.currentBGM = "";
                    std::cout << "BGM stopped" << std::endl;
                }
            }

            // Ready画面表示中の処理
            if (gameState.showReadyScreen) {
                handleReadyScreen(window, gameState, stageManager, platformSystem, renderer, gameStateUIRenderer, keyStates, resetStageStartTime, audioManager);
                glfwPollEvents();
                continue; // Ready画面表示中は他の処理をスキップ
            }
            
            // カウントダウン中の処理
            if (gameState.isCountdownActive) {
                handleCountdown(window, gameState, stageManager, platformSystem, renderer, gameStateUIRenderer, keyStates, resetStageStartTime, deltaTime);
                glfwPollEvents();
                continue; // カウントダウン中は他の処理をスキップ
            }

            // エンディングシーケンスの処理
            if (gameState.isEndingSequence) {
                handleEndingSequence(window, gameState, stageManager, platformSystem, renderer, gameStateUIRenderer, keyStates, deltaTime);
                glfwPollEvents();
                continue; // エンディングシーケンス中は他の処理をスキップ
            }

            // エスケープキーでゲームを終了
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                gameRunning = false;
            }

            // ゲーム状態の更新
            updateGameState(window, gameState, stageManager, platformSystem, deltaTime, scaledDeltaTime, keyStates, resetStageStartTime, audioManager);

            // 描画
            renderFrame(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer);
            
            // フレームレート制限
            std::this_thread::sleep_for(std::chrono::milliseconds(GameConstants::FRAME_DELAY_MS));
            
            glfwPollEvents();
        }
    }

    void handleReadyScreen(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                          PlatformSystem& platformSystem,
                          std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                          std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                          std::map<int, InputUtils::KeyState>& keyStates,
                          std::function<void()> resetStageStartTime,
                          io::AudioManager& audioManager) {
        // Ready画面表示中はゲームを一時停止
        int width, height;
        prepareFrame(window, gameState, stageManager, renderer, width, height);
        
        // 通常のゲーム要素を描画（背景として）
        renderPlatforms(platformSystem, renderer);
        
        // プレイヤーの描画
        renderPlayer(gameState, renderer);
        
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
            
            // カウントダウン開始SEを再生
            if (gameState.audioEnabled) {
                audioManager.playSFX("countdown");
            }
            
            // カメラモードを設定
            gameState.isFirstPersonView = gameState.isFirstPersonMode;
            
            if (gameState.isFirstPersonMode) {
                gameState.cameraYaw = 90.0f;
                gameState.cameraPitch = -10.0f;
                gameState.firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            
            // 速度設定を適用
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
    }

    void handleCountdown(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                        PlatformSystem& platformSystem,
                        std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                        std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                        std::map<int, InputUtils::KeyState>& keyStates,
                        std::function<void()> resetStageStartTime,
                        float deltaTime) {
        
        // カウントダウン開始時に制限時間を再計算（モード変更に対応）
        if (!gameState.timeLimitApplied) {
            // 現在のステージの基本制限時間を取得
            float baseTimeLimit = gameState.timeLimit;
            
            // FPSモードとEASYモードのボーナスを適用
            if (gameState.isFirstPersonMode) {
                baseTimeLimit += GameConstants::FIRST_PERSON_TIME_BONUS;
                printf("COUNTDOWN: FPS MODE +%.1fs applied\n", GameConstants::FIRST_PERSON_TIME_BONUS);
            }
            if (gameState.isEasyMode) {
                baseTimeLimit += GameConstants::EASY_MODE_TIME_BONUS;
                printf("COUNTDOWN: EASY MODE +%.1fs applied\n", GameConstants::EASY_MODE_TIME_BONUS);
            }
            
            gameState.timeLimit = baseTimeLimit;
            gameState.remainingTime = baseTimeLimit;
            gameState.timeLimitApplied = true;
            printf("COUNTDOWN: Final time limit set to %.1f seconds\n", baseTimeLimit);
        }
        
        // カウントダウン中はゲームを一時停止
        int width, height;
        prepareFrame(window, gameState, stageManager, renderer, width, height);
        
        // 通常のゲーム要素を描画（背景として）
        renderPlatforms(platformSystem, renderer);
        
        // プレイヤーの描画
        renderPlayer(gameState, renderer);
        
        // カウントダウンUIを描画
        int count = (int)gameState.countdownTimer + 1;
        if (count > 0) {
            gameStateUIRenderer->renderCountdown(width, height, count);
        }
        
        renderer->endFrame();
        
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
        
        // カウントダウン処理
        gameState.countdownTimer -= deltaTime;
        
        if (gameState.countdownTimer <= 0.0f) {
            gameState.isCountdownActive = false;
            resetStageStartTime();
        }
    }

    void handleEndingSequence(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                             PlatformSystem& platformSystem,
                             std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                             std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                             std::map<int, InputUtils::KeyState>& keyStates,
                             float deltaTime) {
        // エンディングシーケンス中はゲームを一時停止
        int width, height;
        prepareFrame(window, gameState, stageManager, renderer, width, height);
        
        // スタッフロールの表示
        if (gameState.showStaffRoll) {
            gameState.staffRollTimer += deltaTime;
            
            // スタッフロールが15秒間表示されたら、エンディングメッセージに切り替え
            if (gameState.staffRollTimer >= 14.0f) {
                gameState.showStaffRoll = false;
                gameState.showEndingMessage = true;
                gameState.endingMessageTimer = 0.0f;
            } else {
                gameStateUIRenderer->renderStaffRoll(width, height, gameState.staffRollTimer);
            }
        }
        
        // エンディングメッセージの表示
        if (gameState.showEndingMessage) {
            gameState.endingMessageTimer += deltaTime;
            
            // エンディングメッセージが5秒間表示されたら、フィールドに戻る
            if (gameState.endingMessageTimer >= 5.0f) {
                gameState.isEndingSequence = false;
                gameState.showStaffRoll = false;
                gameState.showEndingMessage = false;
                gameState.staffRollTimer = 0.0f;
                gameState.endingMessageTimer = 0.0f;
                
                // フィールドに戻る
                stageManager.goToStage(0, gameState, platformSystem);
                gameState.playerPosition = glm::vec3(8, 0, 0);
                gameState.playerVelocity = glm::vec3(0, 0, 0);
                
                // ステージ選択フィールドではTPSモードに戻す
                gameState.isFirstPersonMode = false;
                gameState.isFirstPersonView = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                gameStateUIRenderer->renderEndingMessage(width, height, gameState.endingMessageTimer);
            }
        }
        
        // Enterキーでスキップ処理
        if (keyStates[GLFW_KEY_ENTER].justPressed()) {
            gameState.isEndingSequence = false;
            gameState.showStaffRoll = false;
            gameState.showEndingMessage = false;
            gameState.staffRollTimer = 0.0f;
            gameState.endingMessageTimer = 0.0f;
            
            stageManager.goToStage(0, gameState, platformSystem);
            gameState.playerPosition = glm::vec3(8, 0, 0);
            gameState.playerVelocity = glm::vec3(0, 0, 0);
            
            // ステージ選択フィールドではTPSモードに戻す
            gameState.isFirstPersonMode = false;
            gameState.isFirstPersonView = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        
        renderer->endFrame();
        
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
    }

    void updateGameState(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                        PlatformSystem& platformSystem, float deltaTime, float scaledDeltaTime,
                        std::map<int, InputUtils::KeyState>& keyStates,
                        std::function<void()> resetStageStartTime, io::AudioManager& audioManager) {
        // ファイル監視：ステージJSONファイルの変更をチェック（0.5秒ごと）
        static float fileCheckTimer = 0.0f;
        fileCheckTimer += deltaTime;
        if (fileCheckTimer >= 0.5f) {
            fileCheckTimer = 0.0f;
            stageManager.checkAndReloadStage(gameState, platformSystem);
            // テクスチャと音声の監視も同時に実行
            gfx::TextureManager::checkAndReloadTextures();
            audioManager.checkAndReloadAudio();
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
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
        
        // バーストジャンプ発動時の処理
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
        
        // FlyingPlatformの発動をチェックしてSEを再生
        static std::map<int, bool> lastFlyingState;
        const auto& platforms = platformSystem.getPlatforms();
        for (int i = 0; i < platforms.size(); i++) {
            std::visit([&](const auto& platform) {
                if constexpr (std::is_same_v<std::decay_t<decltype(platform)>, FlyingPlatform>) {
                    bool isCurrentlyFlying = platform.hasSpawned && platform.isFlying;
                    bool wasFlyingLastFrame = lastFlyingState[i];
                    
                    // 飛行状態がfalseからtrueに変わった時（新しく発動した時）にSEを再生
                    if (isCurrentlyFlying && !wasFlyingLastFrame) {
                        if (gameState.audioEnabled) {
                            audioManager.playSFX("flying");
                        }
                    }
                    
                    lastFlyingState[i] = isCurrentlyFlying;
                }
            }, platforms[i]);
        }
        
        platformSystem.update(scaledDeltaTime, gameState.playerPosition);
        GravitySystem::updateGravityZones(gameState, scaledDeltaTime);
        SwitchSystem::updateSwitches(gameState, scaledDeltaTime);
        CannonSystem::updateCannons(gameState, scaledDeltaTime);
        
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
        
        handleInputProcessing(window, gameState, stageManager, platformSystem, keyStates, resetStageStartTime, scaledDeltaTime, audioManager);
        updatePhysicsAndCollisions(window, gameState, stageManager, platformSystem, deltaTime, scaledDeltaTime, audioManager);
        updateItems(gameState, scaledDeltaTime, audioManager);
        handleStageSelectionArea(window, gameState, stageManager, platformSystem, resetStageStartTime);
    }

    void updatePhysicsAndCollisions(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                                   PlatformSystem& platformSystem, float deltaTime, float scaledDeltaTime, io::AudioManager& audioManager) {
        // 重力方向の取得
        glm::vec3 gravityDirection = glm::vec3(0, -1, 0);
        bool inGravityZone = PhysicsSystem::isPlayerInGravityZone(gameState, gameState.playerPosition, gravityDirection);
        
        float gravityStrength = PhysicsUtils::calculateGravityStrength(GameConstants::BASE_GRAVITY, deltaTime, gameState.timeScale, gravityDirection, gameState);
        glm::vec3 gravityForce = gravityDirection * gravityStrength;

        gameState.playerVelocity += gravityForce;
        
        // 速度倍率に応じて空気抵抗も調整
        float airResistance = (gameState.timeScale > 1.0f) ? GameConstants::AIR_RESISTANCE_FAST : GameConstants::AIR_RESISTANCE_NORMAL;
        gameState.playerVelocity *= airResistance;
        
        // 垂直位置更新
        gameState.playerPosition.y += gameState.playerVelocity.y * scaledDeltaTime;

        glm::vec3 playerSize = GameConstants::PLAYER_SIZE;
        
        SwitchSystem::checkSwitchCollision(gameState, gameState.playerPosition, playerSize);
        CannonSystem::checkCannonCollision(gameState, gameState.playerPosition, playerSize);
        
        // プラットフォーム衝突判定（インデックス付き）
        auto collisionResult = platformSystem.checkCollisionWithIndex(gameState.playerPosition, playerSize);
        PlatformVariant* currentPlatform = collisionResult.first;
        int currentPlatformIndex = collisionResult.second;
        
        // 足場衝突処理
        static bool wasOnPlatform = false;
        bool isOnPlatform = (currentPlatform != nullptr);
        
        if (currentPlatform != nullptr) {
            // お助けモード用：足場の中心位置とインデックスを記録
            if (gameState.isEasyMode) {
                std::visit([&](const auto& platform) {
                    gameState.lastPlatformPosition = platform.position;
                    gameState.lastPlatformIndex = currentPlatformIndex;
                    gameState.isTrackingPlatform = true;
                    
                    // 足場の種類を判定
                    std::string platformType = "Unknown";
                    if constexpr (std::is_same_v<decltype(platform), const StaticPlatform&>) {
                        platformType = "Static";
                    } else if constexpr (std::is_same_v<decltype(platform), const MovingPlatform&>) {
                        platformType = "Moving";
                    } else if constexpr (std::is_same_v<decltype(platform), const RotatingPlatform&>) {
                        platformType = "Rotating";
                    } else if constexpr (std::is_same_v<decltype(platform), const PatrollingPlatform&>) {
                        platformType = "Patrolling";
                    } else if constexpr (std::is_same_v<decltype(platform), const TeleportPlatform&>) {
                        platformType = "Teleport";
                    } else if constexpr (std::is_same_v<decltype(platform), const JumpPad&>) {
                        platformType = "JumpPad";
                    } else if constexpr (std::is_same_v<decltype(platform), const CycleDisappearingPlatform&>) {
                        platformType = "CycleDisappearing";
                    }
                }, *currentPlatform);
            }
            
            std::visit(overloaded{
                [&](const StaticPlatform& platform) {
                    if (!PhysicsSystem::checkPlatformCollisionHorizontal(gameState, gameState.playerPosition, playerSize)) {
                        PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    }
                    if (platform.color.r > 0.9f && platform.color.g > 0.9f && platform.color.b < 0.1f) {
                        if (!gameState.gameWon && gameState.collectedItems >= gameState.requiredItems) {
                            // BGMを停止してクリアSEを再生
                            if (gameState.audioEnabled) {
                                // BGMを停止
                                audioManager.stopBGM();
                                gameState.bgmPlaying = false;
                                gameState.currentBGM = "";
                                
                                // クリアSEを再生（他のSEの音量は変更しない）
                                audioManager.playSFX("clear");
                            }
                            
                            gameState.gameWon = true;
                            gameState.isStageCompleted = true;
                            gameState.isGoalReached = true;
                            gameState.clearTime = gameState.gameTime;
                            
                            float remainingTime = gameState.timeLimit - gameState.clearTime;
                            float limitTime = gameState.timeLimit;
                            
                            //制限時間の長さによって獲得する星数を変える                            
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
                            }
                        
                            // ステージ5クリア後の特別処理
                            if (currentStage == 5) {
                                // エンディング
                                gameState.isEndingSequence = true;
                                gameState.showStaffRoll = true;
                                gameState.staffRollTimer = 0.0f;
                            } else {
                                gameState.showStageClearUI = true;
                            }
                        }
                    }
                },
                [&](const MovingPlatform& platform) {
                    // 接地（重力方向に沿ってスナップ）
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    // 乗っているフラグを立てる
                    const_cast<MovingPlatform&>(platform).hasPlayerOnBoard = true;
                    // プレイヤー-足場同期（XZを足場の移動分だけ加算）
                    glm::vec3 platformMovement = platform.position - platform.previousPosition;
                    gameState.playerPosition.x += platformMovement.x;
                    gameState.playerPosition.z += platformMovement.z;
                },
                [&](const RotatingPlatform& platform) {
                    // 回転足場：プレイヤー位置をローカルに変換して回転を適用
                    glm::vec3 halfSize = platform.size * 0.5f;
                    glm::vec3 platformMin = platform.position - halfSize;
                    glm::vec3 platformMax = platform.position + halfSize;
                    bool onPlatform = (gameState.playerPosition.x >= platformMin.x && gameState.playerPosition.x <= platformMax.x &&
                                       gameState.playerPosition.z >= platformMin.z && gameState.playerPosition.z <= platformMax.z);
                    if (onPlatform) {
                        glm::vec3 localPlayerPos = gameState.playerPosition - platform.position;
                        if (glm::length(platform.rotationAxis - glm::vec3(0, 1, 0)) < 0.1f) {
                            float angle = glm::radians(platform.rotationSpeed * gameState.gameTime);
                            float cosAngle = cos(angle);
                            float sinAngle = sin(angle);
                            float newX = localPlayerPos.x * cosAngle - localPlayerPos.z * sinAngle;
                            float newZ = localPlayerPos.x * sinAngle + localPlayerPos.z * cosAngle;
                            gameState.playerPosition = platform.position + glm::vec3(newX, localPlayerPos.y, newZ);
                        } else if (glm::length(platform.rotationAxis - glm::vec3(1, 0, 0)) < 0.1f) {
                            float angle = glm::radians(platform.rotationSpeed * gameState.gameTime);
                            float cosAngle = cos(angle);
                            float sinAngle = sin(angle);
                            float newY = localPlayerPos.y * cosAngle - localPlayerPos.z * sinAngle;
                            float newZ = localPlayerPos.y * sinAngle + localPlayerPos.z * cosAngle;
                            gameState.playerPosition = platform.position + glm::vec3(localPlayerPos.x, newY, newZ);
                        }
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    } else {
                        PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    }
                },
                [&](const PatrollingPlatform& platform) {
                    // 巡回足場：接地 + 移動に合わせて同期、拡張範囲内なら中心に寄せる
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
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                        glm::vec3 platformMovement = platform.position - platform.previousPosition;
                        gameState.playerPosition.x += platformMovement.x;
                        gameState.playerPosition.z += platformMovement.z;
                        if (!onPlatform && inExtendedRange) {
                            glm::vec3 directionToCenter = platform.position - gameState.playerPosition;
                            directionToCenter.y = 0;
                            float distanceToCenter = glm::length(directionToCenter);
                            if (distanceToCenter > 0.1f) {
                                glm::vec3 normalizedDirection = glm::normalize(directionToCenter);
                                gameState.playerPosition += normalizedDirection * 0.5f * deltaTime;
                            }
                        }
                    } else {
                        PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    }
                },
                [&](const TeleportPlatform& platform) {
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    if (!platform.hasTeleported && platform.cooldownTimer <= 0.0f) {
                        gameState.playerPosition = platform.teleportDestination;
                        const_cast<TeleportPlatform&>(platform).hasTeleported = true;
                        const_cast<TeleportPlatform&>(platform).cooldownTimer = 2.0f;
                    }
                },
                [&](const JumpPad& platform) {
                    // ジャンプ台：接地後、上方向速度を与える
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    gameState.playerVelocity.y = platform.jumpPower;
                },
                [&](const CycleDisappearingPlatform& platform) {
                    // サイクル消失足場：可視時のみ接地
                    if (platform.isCurrentlyVisible) {
                        PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    }
                },
                [&](const DisappearingPlatform& platform) {
                    // 一時消失足場：通常接地
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                },
                [&](const FlyingPlatform& platform) {
                    // 飛来足場：通常接地
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                }
            }, *currentPlatform);
        } else {
        // プレイヤーが足場から離れた時の処理
            platformSystem.resetMovingPlatformFlags();
        }
        
        // 着地SEを再生（足場に着地した瞬間）
        if (isOnPlatform && !wasOnPlatform && gameState.audioEnabled) {
            audioManager.playSFX("on_ground");
        }
        wasOnPlatform = isOnPlatform;
        
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
                        }, platforms[gameState.lastPlatformIndex]);
                    } else {
                        gameState.playerPosition = gameState.lastPlatformPosition + glm::vec3(0, 2.0f, 0);
                    }
                } else {
                    gameState.playerPosition = gameState.lastPlatformPosition + glm::vec3(0, 2.0f, 0);
                }
                gameState.playerVelocity = glm::vec3(0, 0, 0);
            } else if (stageManager.getCurrentStage() == 0) {
                // ステージ選択フィールド：中央にリスポーン
                gameState.playerPosition = glm::vec3(8, 2.0f, 0);
                gameState.playerVelocity = glm::vec3(0, 0, 0);
            } else if (stageManager.getCurrentStage() == 6) {
                // チュートリアルステージ：ライフを減らさずに最初の位置にリスポーン
                gameState.playerPosition = gameState.tutorialStartPosition;
                gameState.playerVelocity = glm::vec3(0, 0, 0);
                printf("TUTORIAL: Respawned at start position (%.1f, %.1f, %.1f)\n", 
                       gameState.tutorialStartPosition.x, gameState.tutorialStartPosition.y, gameState.tutorialStartPosition.z);
            } else {
                // 通常モード：残機を減らす
                gameState.lives--;
                if (gameState.lives <= 0) {
                    gameState.isGameOver = true;
                    gameState.gameOverTimer = 0.0f;
                } else {
                    // 残機がある場合はチェックポイントにリセット（アイテムは保持）
                    if (gameState.lastCheckpointItemId != -1) {
                        gameState.playerPosition = gameState.lastCheckpoint;
                    } else {
                        gameState.playerPosition = glm::vec3(0, 6.0f, -25.0f);
                    }
                    gameState.playerVelocity = glm::vec3(0, 0, 0);
                }
            }
        }
    }

    void updateItems(GameState& gameState, float scaledDeltaTime, io::AudioManager& audioManager) {
        for (auto& item : gameState.items) {
            if (!item.isCollected) {
                // アイテムの回転
                item.rotationAngle += scaledDeltaTime * 90.0f;
                if (item.rotationAngle >= 360.0f) {
                    item.rotationAngle -= 360.0f;
                }
                
                // アイテムの上下の揺れ
                item.bobTimer += scaledDeltaTime;
                item.bobHeight = sin(item.bobTimer * 2.0f) * 0.2f;
                
                // プレイヤーとの距離チェック（アイテム収集）
                float distance = glm::length(gameState.playerPosition - item.position);
                if (distance < 1.5f) { // 収集範囲
                    // アイテム取得SEを再生
                    if (gameState.audioEnabled) {
                        audioManager.playSFX("item");
                    }
                    
                    item.isCollected = true;
                    gameState.collectedItems++;
                    
                    if (gameState.isTutorialStage) {
                        gameState.earnedItems++;
                    }
                    
                    // チェックポイントを更新
                    gameState.lastCheckpoint = item.position;
                    gameState.lastCheckpointItemId = item.itemId;
                }
            }
        }
    }

    void handleStageSelectionArea(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                                 PlatformSystem& platformSystem, std::function<void()> resetStageStartTime) {
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
            } else {
                gameState.showStageSelectionAssist = false;
                gameState.assistTargetStage = 0;
            }
            
            // ステージ選択処理（ワープUI表示中は無効）
            if (!gameState.showWarpTutorialUI && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                if (selectedStage > 0) {
                    processStageSelectionAction(selectedStage, gameState, stageManager, platformSystem, resetStageStartTime, window);
                }
            }
        }
    }

    void renderFrame(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                    PlatformSystem& platformSystem,
                    std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                    std::unique_ptr<gfx::UIRenderer>& uiRenderer,
                    std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer) {
        int width, height;
        prepareFrame(window, gameState, stageManager, renderer, width, height);
        
        // UIレンダラーにウィンドウサイズを設定（スケーリング用）
        uiRenderer->setWindowSize(width, height);
        
        // 足場の描画
        renderPlatforms(platformSystem, renderer);
        
        // 重力反転エリアの描画
        for (const auto& zone : gameState.gravityZones) {
            if (zone.isActive) {
                renderer->renderer3D.renderBoxWithAlpha(zone.position, GameConstants::Colors::GRAVITY_ZONE_COLOR, 
                                           zone.size, GameConstants::Colors::GRAVITY_ZONE_ALPHA);
            }
        }
        
        // スイッチの描画
        for (const auto& switch_obj : gameState.switches) {
            glm::vec3 switchColor = switch_obj.color;
            if (switch_obj.isPressed) {
                switchColor *= 0.7f;
            }
            renderer->renderer3D.renderCube(switch_obj.position, switchColor, switch_obj.size.x);
        }
        
        // 大砲の描画
        for (const auto& cannon : gameState.cannons) {
            if (cannon.isActive) {
                glm::vec3 color = cannon.color;
                if (cannon.cooldownTimer > 0.0f) {
                    color *= 0.5f;
                }
                renderer->renderer3D.renderCube(cannon.position, color, cannon.size.x);
            }
        }
        
        // アイテム用のテクスチャを読み込み
        static GLuint itemFirstTexture = 0;
        if (itemFirstTexture == 0) {
            itemFirstTexture = gfx::TextureManager::loadTexture("../assets/textures/item_first.png");
            if (itemFirstTexture == 0) {
                itemFirstTexture = gfx::TextureManager::loadTexture("assets/textures/item_first.png");
            }
        }
        
        static GLuint itemSecondTexture = 0;
        if (itemSecondTexture == 0) {
            itemSecondTexture = gfx::TextureManager::loadTexture("../assets/textures/item_second.png");
            if (itemSecondTexture == 0) {
                itemSecondTexture = gfx::TextureManager::loadTexture("assets/textures/item_second.png");
            }
        }
        
        static GLuint itemThirdTexture = 0;
        if (itemThirdTexture == 0) {
            itemThirdTexture = gfx::TextureManager::loadTexture("../assets/textures/item_third.png");
            if (itemThirdTexture == 0) {
                itemThirdTexture = gfx::TextureManager::loadTexture("assets/textures/item_third.png");
            }
        }
        
        // アイテムの描画
        for (const auto& item : gameState.items) {
            if (!item.isCollected) {
                glm::vec3 itemPos = item.position + glm::vec3(0, item.bobHeight, 0);
                
                // アイテムの色に応じてテクスチャを選択
                bool isRedItem = (item.color.r > 0.9f && item.color.g < 0.1f && item.color.b < 0.1f);
                bool isGreenItem = (item.color.r < 0.1f && item.color.g > 0.9f && item.color.b < 0.1f);
                bool isBlueItem = (item.color.r < 0.1f && item.color.g < 0.1f && item.color.b > 0.9f);
                
                if (isRedItem && itemFirstTexture != 0) {
                    // 赤色アイテム（item_first.png）
                    renderer->renderer3D.renderTexturedRotatedBox(itemPos, item.size, itemFirstTexture, glm::vec3(0, 1, 0), item.rotationAngle);
                } else if (isGreenItem && itemSecondTexture != 0) {
                    // 緑色アイテム（item_second.png）
                    renderer->renderer3D.renderTexturedRotatedBox(itemPos, item.size, itemSecondTexture, glm::vec3(0, 1, 0), item.rotationAngle);
                } else if (isBlueItem && itemThirdTexture != 0) {
                    // 青色アイテム（item_third.png）
                    renderer->renderer3D.renderTexturedRotatedBox(itemPos, item.size, itemThirdTexture, glm::vec3(0, 1, 0), item.rotationAngle);
                } else {
                    // テクスチャがない場合は従来通り色ベースで描画
                    renderer->renderer3D.renderRotatedBox(itemPos, item.color, item.size, glm::vec3(0, 1, 0), item.rotationAngle);
                }
            }
        }
        
        // プレイヤーの描画（1人称視点時は描画しない）
        if (!gameState.isFirstPersonView) {
            renderPlayer(gameState, renderer);
        }
        
        // チュートリアルステージのUI描画
        if (gameState.isTutorialStage && gameState.showTutorialUI) {
            gameStateUIRenderer->renderTutorialStageUI(width, height, gameState.tutorialMessage, gameState.tutorialStep, gameState.tutorialStepCompleted);
        }
    
        // ステージ情報
        const StageData* currentStageData = stageManager.getStageData(stageManager.getCurrentStage());
        if (currentStageData && stageManager.getCurrentStage()!=0) {
            // チュートリアルステージの場合は「TUTORIAL」を表示
            if (stageManager.getCurrentStage() == 6) {
                uiRenderer->renderText("TUTORIAL", 
                                   glm::vec2(15, 30), glm::vec3(1, 1, 0), 2.0f);
            } else {
                // 通常のステージは「STAGE X」を表示
                uiRenderer->renderText("STAGE " + std::to_string(stageManager.getCurrentStage()), 
                                   glm::vec2(30, 30), glm::vec3(1, 1, 0), 2.0f);
            }
        }
        
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
                        
                        // ミニマップを描画（ステージ0とエンディングシーケンス中は非表示）
                        if (stageManager.getCurrentStage() != 0 && !gameState.isEndingSequence) {
                            static gfx::MinimapRenderer minimapRenderer;
                            minimapRenderer.render(gameState, platformSystem, width, height, stageManager.getCurrentStage(), 30.0f);
                        }
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
        
        // ワープ機能説明UI
        if (gameState.showWarpTutorialUI) {
            // 背景とUIを描画
            gameStateUIRenderer->renderWarpTutorialBackground(width, height, gameState.warpTutorialStage);
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
            
            // 星数の右にEASY/NORMAL表示
            std::string modeText = gameState.isEasyMode ? "EASY" : "NORMAL";
            glm::vec3 modeColor = gameState.isEasyMode ? glm::vec3(0.2f, 0.8f, 0.2f) : glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec2 modeTextPosition = gameState.isEasyMode ? glm::vec2(155, 20) : glm::vec2(140, 20);
            uiRenderer->renderText(modeText, modeTextPosition, modeColor, 1.5f);
            
            // EASY/NORMALの下にPRESS E表示
            uiRenderer->renderText("PRESS E", glm::vec2(155, 50), glm::vec3(1.0f, 1.0f, 1.0f), 0.8f);
            
            // 初回ステージ0入場チュートリアルUIの表示
            if (gameState.showStage0Tutorial) {
                gameStateUIRenderer->renderStage0Tutorial(width, height);
            }
            
            // EASYモード説明UIの表示
            if (gameState.showEasyModeExplanationUI) {
                gameStateUIRenderer->renderEasyModeExplanationUI(width, height);
            }
            
            // モード選択UIの表示
            if (gameState.showModeSelectionUI) {
                gameStateUIRenderer->renderModeSelectionUI(width, height, gameState.isEasyMode);
            }
            
            // 操作アシストUI（トータルスターのUIと同じタイミングで描画）
            // UNLOCK確認UIや星不足警告UIが表示されている時は非表示
            bool shouldShowAssist = gameState.showStageSelectionAssist && 
                                   !gameState.showUnlockConfirmUI && 
                                   !gameState.showStarInsufficientUI &&
                                   !gameState.showWarpTutorialUI &&
                                   !gameState.showEasyModeExplanationUI &&
                                   !gameState.showModeSelectionUI &&
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
            controlsText = "Controls: WASD=Move, SPACE=Select Stage, 1-5=Teleport to Stage Area, F=Camera Toggle, E=Easy Mode, R=Toggle Time Stop Skill, T=Toggle Double Jump Skill, Y=Toggle Heart Feel Skill, U=Toggle Free Camera Skill, I=Toggle Burst Jump Skill";
        }
        uiRenderer->renderText(controlsText, glm::vec2(10, height - 30), glm::vec3(0.8f, 0.8f, 0.8f));
        
        renderer->endFrame();
    }

    void handleInputProcessing(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                              PlatformSystem& platformSystem, 
                              std::map<int, InputUtils::KeyState>& keyStates,
                              std::function<void()> resetStageStartTime, float scaledDeltaTime, io::AudioManager& audioManager) {
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
        
        // チュートリアルステップ完了時のSE再生
        static bool lastTutorialStepCompleted = false;
        if (gameState.isTutorialStage && gameState.tutorialStepCompleted && !lastTutorialStepCompleted) {
            if (gameState.audioEnabled) {
                audioManager.playSFX("tutorial_ok");
            }
        }
        lastTutorialStepCompleted = gameState.tutorialStepCompleted;
        
        // ステージ切り替え処理（ワープUI表示中は数値キー無効）
        for (int key = GLFW_KEY_0; key <= GLFW_KEY_6; key++) {
            if (keyStates[key].justPressed()) {
                if (gameState.showWarpTutorialUI) { continue; }
                int stageNumber = key - GLFW_KEY_0;
                
                // ステージ選択フィールドでの1-5キーはワープ機能に変更
                if (stageManager.getCurrentStage() == 0 && stageNumber >= 1 && stageNumber <= 5) {
                    if (gameState.unlockedStages[stageNumber]) {
                        // ステージ選択エリアの目の前にワープ
                        teleportToStageArea(stageNumber, gameState);
                    }
                } else {
                    // 通常のステージ選択処理
                    processStageSelectionAction(stageNumber, gameState, stageManager, platformSystem, resetStageStartTime, window);
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
                    break;
                case 1:
                    gameState.timeScale = 2.0f;
                    break;
                case 2:
                    gameState.timeScale = 3.0f;
                    break;
            }
        }
        
        // お助けモード切り替え処理（Eキー）- ステージ0のみ
        // モード選択UI表示中は無効化（モード選択UIで処理するため）
        if (keyStates[GLFW_KEY_E].justPressed() && stageManager.getCurrentStage() == 0 && !gameState.showModeSelectionUI) {
            gameState.isEasyMode = !gameState.isEasyMode;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // スキル取得切り替え処理（ステージ0のみ）
        if (stageManager.getCurrentStage() == 0) {
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
                }
            }
        }
        
        // スキル使用処理
        if (keyStates[GLFW_KEY_Q].justPressed() && gameState.hasTimeStopSkill && !gameState.isTimeStopped && gameState.timeStopRemainingUses > 0) {
            gameState.isTimeStopped = true;
            gameState.timeStopTimer = gameState.timeStopDuration;
            gameState.timeStopRemainingUses--;
        }
        
        if (keyStates[GLFW_KEY_H].justPressed() && gameState.hasHeartFeelSkill && gameState.heartFeelRemainingUses > 0 && gameState.lives < 6) {
            gameState.lives++;
            gameState.heartFeelRemainingUses--;
        }
        
        if (keyStates[GLFW_KEY_C].justPressed() && gameState.hasFreeCameraSkill && !gameState.isFreeCameraActive && 
            gameState.freeCameraRemainingUses > 0 && !gameState.isFirstPersonView) {
            gameState.isFreeCameraActive = true;
            gameState.freeCameraTimer = gameState.freeCameraDuration;
            gameState.freeCameraRemainingUses--;
            
            gameState.freeCameraYaw = gameState.cameraYaw;
            gameState.freeCameraPitch = gameState.cameraPitch;
            
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        
        if (keyStates[GLFW_KEY_B].justPressed() && gameState.hasBurstJumpSkill && !gameState.isBurstJumpActive && 
            gameState.burstJumpRemainingUses > 0 && !gameState.isInBurstJumpAir) {
            gameState.isBurstJumpActive = true;
            gameState.hasUsedBurstJump = false;
            gameState.burstJumpRemainingUses--;
        }
        
        // カメラ切り替え処理（Fキー）- ステージ0のみ
        if (keyStates[GLFW_KEY_F].justPressed() && stageManager.getCurrentStage() == 0) {
            gameState.isFirstPersonView = !gameState.isFirstPersonView;
            if (gameState.isFirstPersonView) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                gameState.cameraYaw = 90.0f;
                gameState.cameraPitch = -10.0f;
                gameState.firstMouse = true;
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
        
        // 初回入場チュートリアルUI処理
        if (gameState.showStage0Tutorial && stageManager.getCurrentStage() == 0) {
            // 直前のENTERを離すまで受け付けない
            if (gameState.blockEnterUntilReleased) {
                if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                    gameState.blockEnterUntilReleased = false; // 離されたのを確認
                }
                // 離されるまでここで処理終了（自動遷移を防止）
                return;
            }

            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.showStage0Tutorial = false;
                gameState.showEasyModeExplanationUI = true; // EASYモード説明UIを表示
                // 直前のENTER押下を飲み込むため、離されるまでブロック
                gameState.blockEnterUntilReleased = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        // EASYモード説明UI処理
        if (gameState.showEasyModeExplanationUI) {
            // 直前のENTERを離すまで受け付けない
            if (gameState.blockEnterUntilReleased) {
                if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                    gameState.blockEnterUntilReleased = false; // 離されたのを確認
                }
                // 離されるまでここで処理終了（自動遷移を防止）
                return;
            }

            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.showEasyModeExplanationUI = false;
                gameState.showModeSelectionUI = true; // モード選択UIを表示
                // 直前のENTER押下を飲み込むため、離されるまでブロック
                gameState.blockEnterUntilReleased = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        // モード選択UI処理
        if (gameState.showModeSelectionUI) {
            // Eキーでモード切り替え（blockEnterUntilReleasedの影響を受けない）
            if (keyStates[GLFW_KEY_E].justPressed()) {
                printf("DEBUG: E key pressed in mode selection UI\n");
                gameState.isEasyMode = !gameState.isEasyMode;
                printf("DEBUG: isEasyMode changed to: %s\n", gameState.isEasyMode ? "true" : "false");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            // 直前のENTERを離すまで受け付けない
            if (gameState.blockEnterUntilReleased) {
                if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                    gameState.blockEnterUntilReleased = false; // 離されたのを確認
                }
                // 離されるまでここで処理終了（自動遷移を防止）
                return;
            }
            
            // ENTERキーで確定
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.showModeSelectionUI = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        // ステージクリアUI処理
        if (gameState.showStageClearUI) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                int clearedStage = stageManager.getCurrentStage();
                
                stageManager.completeStage(clearedStage);
                stageManager.goToStage(0, gameState, platformSystem);
                gameState.timeScale = 1.0f;
                gameState.timeScaleLevel = 0;
                
                // ステージ選択フィールドではTPSモードに戻す
                gameState.isFirstPersonMode = false;
                gameState.isFirstPersonView = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                
                glm::vec3 returnPosition;
                switch (clearedStage) {
                    case 1: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[0].x, GameConstants::STAGE_AREAS[0].y, GameConstants::STAGE_AREAS[0].z-1); break;
                    case 2: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[1].x, GameConstants::STAGE_AREAS[1].y, GameConstants::STAGE_AREAS[1].z-1); break;
                    case 3: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[2].x, GameConstants::STAGE_AREAS[2].y, GameConstants::STAGE_AREAS[2].z-1); break;
                    case 4: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[3].x, GameConstants::STAGE_AREAS[3].y, GameConstants::STAGE_AREAS[3].z-1); break;
                    case 5: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[4].x, GameConstants::STAGE_AREAS[4].y, GameConstants::STAGE_AREAS[4].z-1); break;
                    default: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[0].x, GameConstants::STAGE_AREAS[0].y, GameConstants::STAGE_AREAS[0].z-1); break;
                }
                
                gameState.playerPosition = returnPosition;
                gameState.playerVelocity = glm::vec3(0, 0, 0);
                
                gameState.showStageClearUI = false;
                gameState.gameWon = false;
                gameState.isGoalReached = false;
            }
            
            if (keyStates[GLFW_KEY_R].justPressed()) {
                resetStageStartTime();
                stageManager.loadStage(stageManager.getCurrentStage(), gameState, platformSystem);
                gameState.showStageClearUI = false;
                gameState.gameWon = false;
                gameState.isGoalReached = false;
                gameState.timeScale = 1.0f;
                gameState.timeScaleLevel = 0;
                gameState.lives = 6;
                gameState.showReadyScreen = true;
                gameState.readyScreenShown = false;
                gameState.readyScreenSpeedLevel = 0;
            }
        }
        
        // ゲームオーバーUI処理
        if (gameState.isGameOver) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                stageManager.goToStage(0, gameState, platformSystem);
                gameState.timeScale = 1.0f;
                gameState.timeScaleLevel = 0;
                gameState.isGameOver = false;
                gameState.isTimeUp = false;
                gameState.remainingTime = gameState.timeLimit;
                
                gameState.playerPosition = glm::vec3(8, 2.0f, 0);
                gameState.playerVelocity = glm::vec3(0, 0, 0);
                
                // ステージ選択フィールドではTPSモードに戻す
                gameState.isFirstPersonMode = false;
                gameState.isFirstPersonView = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            
            if (keyStates[GLFW_KEY_R].justPressed()) {
                resetStageStartTime();
                stageManager.loadStage(stageManager.getCurrentStage(), gameState, platformSystem);
                gameState.isGameOver = false;
                gameState.isTimeUp = false;
                gameState.remainingTime = gameState.timeLimit;
                gameState.timeScale = 1.0f;
                gameState.timeScaleLevel = 0;
                gameState.lives = 6;
                gameState.playerVelocity = glm::vec3(0, 0, 0);
                gameState.lastCheckpoint = glm::vec3(0, 30.0f, 0);
                gameState.lastCheckpointItemId = -1;
                gameState.showReadyScreen = true;
                gameState.readyScreenShown = false;
                gameState.readyScreenSpeedLevel = 0;
            }
        }

        // 星不足警告UI処理
        if (gameState.showStarInsufficientUI) {
            if (keyStates[GLFW_KEY_SPACE].justPressed()) {
                gameState.showStarInsufficientUI = false;
                gameState.insufficientTargetStage = 0;
                gameState.insufficientRequiredStars = 0;
            }
        }
        
        // ステージ解放確認UI処理
        if (gameState.showUnlockConfirmUI && !gameState.showStarInsufficientUI) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                // 星数チェック
                if (gameState.totalStars >= gameState.unlockRequiredStars) {
                    // 十分な星がある場合のみアンロック
                    gameState.totalStars -= gameState.unlockRequiredStars;
                    gameState.unlockedStages[gameState.unlockTargetStage] = true;
                    
                    // ワープ機能説明UIを表示（ステージには入らない）
                    gameState.showWarpTutorialUI = true;
                    gameState.warpTutorialStage = gameState.unlockTargetStage;
                    // 直前のENTER押下がそのまま伝播しないよう、離されるまでブロック
                    gameState.blockEnterUntilReleased = true;
                    
                    gameState.showUnlockConfirmUI = false;
                    gameState.unlockTargetStage = 0;
                    gameState.unlockRequiredStars = 0;
                } else {
                    // 星不足の場合は警告UIを表示
                    gameState.showUnlockConfirmUI = false;
                    gameState.showStarInsufficientUI = true;
                    gameState.insufficientTargetStage = gameState.unlockTargetStage;
                    gameState.insufficientRequiredStars = gameState.unlockRequiredStars;
                    gameState.unlockTargetStage = 0;
                    gameState.unlockRequiredStars = 0;
                }
            }
            
            if (keyStates[GLFW_KEY_SPACE].justPressed()) {
                gameState.showUnlockConfirmUI = false;
                gameState.unlockTargetStage = 0;
                gameState.unlockRequiredStars = 0;
            }
        }
        
        // ワープ機能説明UI処理
        if (gameState.showWarpTutorialUI) {
            // 直前のENTERが離されるまで処理をブロック
            if (gameState.blockEnterUntilReleased) {
                if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                    gameState.blockEnterUntilReleased = false;
                }
                return;
            }
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                // ステージに入る
                resetStageStartTime();
                gameState.lives = 6;
                stageManager.goToStage(gameState.warpTutorialStage, gameState, platformSystem);
                gameState.readyScreenShown = false;
                gameState.showReadyScreen = true;
                gameState.readyScreenSpeedLevel = 0;
                
                gameState.showWarpTutorialUI = false;
                gameState.warpTutorialStage = 0;
            }
        }
        
        // 通常の入力処理（UI表示中は移動を無効化）
        // チュートリアルステージ（ステージ6）ではshowStage0Tutorialは無視
        bool isUIBlockingMovement = gameState.showUnlockConfirmUI || 
                                   gameState.showStarInsufficientUI || 
                                   gameState.showWarpTutorialUI || 
                                   gameState.isGameOver || 
                                   gameState.showStageClearUI || 
                                   gameState.showEasyModeExplanationUI ||
                                   gameState.showModeSelectionUI ||
                                   (gameState.showStage0Tutorial && stageManager.getCurrentStage() == 0);
        
        if (tutorialInputEnabled && !isUIBlockingMovement) {
            glm::vec3 gravityDirection = glm::vec3(0, -1, 0);
            InputSystem::processInput(window, gameState, scaledDeltaTime);
            InputSystem::processJumpAndFloat(window, gameState, scaledDeltaTime, gravityDirection, platformSystem, audioManager);
        }
        
        // チュートリアルステージの進行処理
        if (gameState.isTutorialStage) {
            TutorialManager::processTutorialProgress(window, gameState, keyStates);
        }
    }

    // 共通の描画関数の実装
    void prepareFrame(GLFWwindow* window, GameState& gameState, StageManager& stageManager,
                     std::unique_ptr<gfx::OpenGLRenderer>& renderer, int& width, int& height) {
        renderer->beginFrameWithBackground(stageManager.getCurrentStage());
        
        // カメラ設定
        auto cameraConfig = CameraSystem::calculateCameraConfig(gameState, stageManager, window);
        CameraSystem::applyCameraConfig(renderer.get(), cameraConfig, window);
        
        // ウィンドウサイズを取得
        auto [w, h] = CameraSystem::getWindowSize(window);
        width = w; height = h;
    }

    void renderPlatforms(PlatformSystem& platformSystem, 
                        std::unique_ptr<gfx::OpenGLRenderer>& renderer) {
        auto positions = platformSystem.getPositions();
        auto sizes = platformSystem.getSizes();
        auto colors = platformSystem.getColors();
        auto visibility = platformSystem.getVisibility();
        auto isRotating = platformSystem.getIsRotating();
        auto rotationAngles = platformSystem.getRotationAngles();
        auto rotationAxes = platformSystem.getRotationAxes();
        auto blinkAlphas = platformSystem.getBlinkAlphas();
        auto platformTypes = platformSystem.getPlatformTypes();
        
        // static足場用のテクスチャを読み込み
        static GLuint staticPlatformTexture = 0;
        if (staticPlatformTexture == 0) {
            staticPlatformTexture = gfx::TextureManager::loadTexture("../assets/textures/static_platform.png");
            if (staticPlatformTexture == 0) {
                // 作業ディレクトリがプロジェクトルートの場合に対応
                staticPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/static_platform.png");
            }
        }
        // ゴール（黄色）足場用のテクスチャを読み込み
        static GLuint goalPlatformTexture = 0;
        if (goalPlatformTexture == 0) {
            goalPlatformTexture = gfx::TextureManager::loadTexture("../assets/textures/goal_platform.png");
            if (goalPlatformTexture == 0) {
                goalPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/goal_platform.png");
            }
        }
        // スタート（緑色）足場用のテクスチャを読み込み
        static GLuint startPlatformTexture = 0;
        if (startPlatformTexture == 0) {
            startPlatformTexture = gfx::TextureManager::loadTexture("../assets/textures/start_platform.png");
            if (startPlatformTexture == 0) {
                startPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/start_platform.png");
            }
        }
        
        // moving足場用のテクスチャを読み込み
        static GLuint movingPlatformTexture = 0;
        if (movingPlatformTexture == 0) {
            movingPlatformTexture = gfx::TextureManager::loadTexture("../assets/textures/moving_platform.png");
            if (movingPlatformTexture == 0) {
                movingPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/moving_platform.png");
            }
        }

        // サイクル消失足場用のテクスチャを読み込み
        static GLuint cyclingDisappearTexture = 0;
        if (cyclingDisappearTexture == 0) {
            cyclingDisappearTexture = gfx::TextureManager::loadTexture("../assets/textures/cyclingdisappearing_platform.png");
            if (cyclingDisappearTexture == 0) {
                cyclingDisappearTexture = gfx::TextureManager::loadTexture("assets/textures/cyclingdisappearing_platform.png");
            }
        }

        // 飛来足場用のテクスチャを読み込み
        static GLuint flyingPlatformTexture = 0;
        if (flyingPlatformTexture == 0) {
            flyingPlatformTexture = gfx::TextureManager::loadTexture("../assets/textures/flying_platform.png");
            if (flyingPlatformTexture == 0) {
                flyingPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/flying_platform.png");
            }
        }
        
        // ロック状態の足場用のテクスチャを読み込み
        static GLuint lockPlatformTexture = 0;
        if (lockPlatformTexture == 0) {
            lockPlatformTexture = gfx::TextureManager::loadTexture("../assets/textures/lock_platform.png");
            if (lockPlatformTexture == 0) {
                lockPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/lock_platform.png");
            }
        }
        
        // アンロック状態の足場用のテクスチャを読み込み
        static GLuint unlockPlatformTexture = 0;
        if (unlockPlatformTexture == 0) {
            unlockPlatformTexture = gfx::TextureManager::loadTexture("../assets/textures/unlock_platform.png");
            if (unlockPlatformTexture == 0) {
                unlockPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/unlock_platform.png");
            }
        }
        
        for (size_t i = 0; i < positions.size(); i++) {
            if (!visibility[i] || sizes[i].x <= 0 || sizes[i].y <= 0 || sizes[i].z <= 0) continue;
            
            if (isRotating[i]) {
                renderer->renderer3D.renderRotatedBox(positions[i], colors[i], sizes[i], rotationAxes[i], rotationAngles[i]);
            } else if (platformTypes[i] == "static" && (staticPlatformTexture != 0 || goalPlatformTexture != 0 || startPlatformTexture != 0 || lockPlatformTexture != 0 || unlockPlatformTexture != 0)) {
                // 色に応じてテクスチャを選択
                bool isGoalColor = (colors[i].r > 0.9f && colors[i].g > 0.9f && colors[i].b < 0.1f);
                bool isStartColor = (colors[i].r < 0.1f && colors[i].g > 0.9f && colors[i].b < 0.1f);
                bool isUnlockColor = (colors[i].r < 0.3f && colors[i].g > 0.9f && colors[i].b < 0.3f); // 緑色（アンロック済み）
                bool isLockColor = (colors[i].r > 0.4f && colors[i].g > 0.4f && colors[i].b > 0.4f && 
                                   colors[i].r < 0.6f && colors[i].g < 0.6f && colors[i].b < 0.6f); // 灰色（ロック中）
                
                GLuint tex = staticPlatformTexture; // デフォルト
                if (isGoalColor && goalPlatformTexture != 0) {
                    tex = goalPlatformTexture;
                } else if (isStartColor && startPlatformTexture != 0) {
                    tex = startPlatformTexture;
                } else if (isUnlockColor && unlockPlatformTexture != 0) {
                    tex = unlockPlatformTexture;
                } else if (isLockColor && lockPlatformTexture != 0) {
                    tex = lockPlatformTexture;
                }
                
                renderer->renderer3D.renderTexturedBoxWithAlpha(positions[i], sizes[i], tex, blinkAlphas[i]);
            } else if ((platformTypes[i] == "moving" || platformTypes[i] == "patrolling") && movingPlatformTexture != 0) {
                // moving足場とpatrolling足場はテクスチャを使用
                renderer->renderer3D.renderTexturedBoxWithAlpha(positions[i], sizes[i], movingPlatformTexture, blinkAlphas[i]);
            } else if (platformTypes[i] == "cycle_disappearing" && cyclingDisappearTexture != 0) {
                // サイクル消失足場は専用テクスチャを使用（点滅アルファに対応）
                renderer->renderer3D.renderTexturedBoxWithAlpha(positions[i], sizes[i], cyclingDisappearTexture, blinkAlphas[i]);
            } else if (platformTypes[i] == "flying" && flyingPlatformTexture != 0) {
                // 飛来足場は専用テクスチャを使用
                renderer->renderer3D.renderTexturedBoxWithAlpha(positions[i], sizes[i], flyingPlatformTexture, blinkAlphas[i]);
            } else {
                // その他の足場は従来通り色ベースで描画
                renderer->renderer3D.renderRealisticBox(positions[i], colors[i], sizes[i], blinkAlphas[i]);
            }
        }
    }

void renderPlayer(GameState& gameState, 
                  std::unique_ptr<gfx::OpenGLRenderer>& renderer) {
    // プレイヤー用のテクスチャを読み込み
    static GLuint playerTexture = 0;
    static GLuint playerFrontTexture = 0;
    
    if (playerTexture == 0) {
        playerTexture = gfx::TextureManager::loadTexture("../assets/textures/player.png");
        if (playerTexture == 0) {
            // 作業ディレクトリがプロジェクトルートの場合に対応
            playerTexture = gfx::TextureManager::loadTexture("assets/textures/player.png");
        }
    }
    
    if (playerFrontTexture == 0) {
        playerFrontTexture = gfx::TextureManager::loadTexture("../assets/textures/player_front.png");
        if (playerFrontTexture == 0) {
            // 作業ディレクトリがプロジェクトルートの場合に対応
            playerFrontTexture = gfx::TextureManager::loadTexture("assets/textures/player_front.png");
        }
    }
    
    if (playerTexture != 0 && playerFrontTexture != 0) {
        // テクスチャ付きでプレイヤーを描画
        glm::vec3 playerSize = glm::vec3(GameConstants::PLAYER_SCALE);
        
        if (gameState.isShowingFrontTexture) {
            // フロントテクスチャ表示時：前面にplayer_front.png、他の面にplayer.png
            renderer->renderer3D.renderTexturedBox(gameState.playerPosition, playerSize, playerFrontTexture, playerTexture);
        } else {
            // 通常時：全ての面にplayer.png
            renderer->renderer3D.renderTexturedBox(gameState.playerPosition, playerSize, playerTexture);
        }
    } else {
        // テクスチャがない場合は従来通り色ベースで描画
        renderer->renderer3D.renderCube(gameState.playerPosition, gameState.playerColor, GameConstants::PLAYER_SCALE);
    }
}
}
