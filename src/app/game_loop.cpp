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
#include "../core/utils/ui_config_manager.h"
#include "../core/utils/resource_path.h"
#include "../io/input_system.h"
#include "../io/audio_manager.h"
#include "../gfx/minimap_renderer.h"
#include "../game/replay_manager.h"
#include "../game/save_manager.h"
#include "../game/stage_editor.h"
#include "tutorial_manager.h"
#include <set>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

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
            
            // SECRET STARモードをリセット
            gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
            
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

        // 解放済み: ステージをクリアしたことがあるかチェック（星を1個以上獲得しているか）
        bool hasClearedStage = (gameState.stageStars.count(stageNumber) > 0 && gameState.stageStars[stageNumber] > 0);
        
        if (hasClearedStage) {
            // クリア済み: タイムアタック選択UIを表示（ステージに入る前に）
            gameState.showTimeAttackSelectionUI = true;
            gameState.modeSelectionTargetStage = stageNumber;
            gameState.blockEnterUntilReleased = true;  // 直前のENTERを無視
            // モード選択UI表示時に、selectedSecretStarTypeを初期化
            gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
            gameState.isTimeAttackMode = false;
        } else {
            // 初めて入る場合: 直接ready画面へ（タイムアタック選択UIは表示しない）
        resetStageStartTime();
        gameState.lives = 6;
        stageManager.goToStage(stageNumber, gameState, platformSystem);
        gameState.readyScreenShown = false;
        gameState.showReadyScreen = true;
        gameState.readyScreenSpeedLevel = 0;
        gameState.timeScale = 1.0f;
        gameState.timeScaleLevel = 0;
            gameState.isTimeAttackMode = false;  // 初めて入る場合はNORMALモード
        }
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
            audioManager.loadSFX("jump", ResourcePath::getResourcePath("assets/audio/se/jump.ogg"));
            audioManager.loadSFX("clear", ResourcePath::getResourcePath("assets/audio/se/clear.ogg"));
            audioManager.loadSFX("item", ResourcePath::getResourcePath("assets/audio/se/item.ogg"));
            audioManager.loadSFX("on_ground", ResourcePath::getResourcePath("assets/audio/se/on_ground.ogg"));
            audioManager.loadSFX("flying", ResourcePath::getResourcePath("assets/audio/se/flying.ogg"));
            audioManager.loadSFX("countdown", ResourcePath::getResourcePath("assets/audio/se/countdown.ogg"));
            audioManager.loadSFX("tutorial_ok", ResourcePath::getResourcePath("assets/audio/se/tutorial_ok.ogg"));
            audioManager.loadSFX("damage", ResourcePath::getResourcePath("assets/audio/se/damage.ogg"));
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
            
            // BGM管理：各ステージに対応するBGMを再生（タイトル画面中はスキップ）
            if (!gameState.showTitleScreen) {
            int currentStage = stageManager.getCurrentStage();
            // タイトル画面表示中はステージBGMを再生しない
            if (gameState.audioEnabled && !gameState.isGoalReached && !gameState.showTitleScreen) {
                std::string targetBGM = "";
                std::string bgmPath = "";
                
                // ステージに応じてBGMを決定
                switch (currentStage) {
                    case 0: // ステージ選択画面
                        targetBGM = "stage_select_field.ogg";
                        bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage_select_field.ogg");
                        break;
                    case 1: // ステージ1
                        targetBGM = "stage1.ogg";
                        bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage1.ogg");
                        break;
                    case 2: // ステージ2
                        targetBGM = "stage2.ogg";
                        bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage2.ogg");
                        break;
                    case 3: // ステージ3
                        targetBGM = "stage3.ogg";
                        bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage3.ogg");
                        break;
                    case 4: // ステージ4
                        targetBGM = "stage4.ogg";
                        bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage4.ogg");
                        break;
                    case 5: // ステージ5
                        targetBGM = "stage5.ogg";
                        bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage5.ogg");
                        break;
                    case 6: // チュートリアルステージ
                        targetBGM = "tutorial.ogg";
                        bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/tutorial.ogg");
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
            }

            // Ready画面表示中の処理
            if (gameState.showReadyScreen) {
                handleReadyScreen(window, gameState, stageManager, platformSystem, renderer, gameStateUIRenderer, keyStates, resetStageStartTime, audioManager, deltaTime);
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
                // ゲームデータをセーブ
                SaveManager::saveGameData(gameState);
                gameRunning = false;
            }

            // タイトル画面の処理
            if (gameState.showTitleScreen) {
                // ステージ遷移アニメーションの処理（タイトル画面中でも必要）
                if (gameState.isTransitioning) {
                    const float FADE_DURATION = 0.5f;  // フェード時間（秒）
                    gameState.transitionTimer += deltaTime;
                    
                    if (gameState.transitionType == GameState::TransitionType::FADE_OUT) {
                        // フェードアウト中
                        if (gameState.transitionTimer >= FADE_DURATION) {
                            // フェードアウト完了：実際のステージ遷移を実行
                            if (gameState.pendingStageTransition >= 0) {
                                // タイトル画面を非表示
                                gameState.showTitleScreen = false;
                                
                                // チュートリアルステージを読み込む
                                stageManager.loadStage(6, gameState, platformSystem);
                                gameState.showReadyScreen = false;
                                gameState.readyScreenShown = true;
                                gameState.isCountdownActive = false;
                                gameState.countdownTimer = 0.0f;
                                resetStageStartTime();
                                
                                // フェードインに切り替え
                                gameState.transitionType = GameState::TransitionType::FADE_IN;
                                gameState.transitionTimer = 0.0f;
                            }
                        }
                    } else if (gameState.transitionType == GameState::TransitionType::FADE_IN) {
                        // フェードイン中
                        if (gameState.transitionTimer >= FADE_DURATION) {
                            // フェードイン完了：遷移終了
                            gameState.isTransitioning = false;
                            gameState.transitionType = GameState::TransitionType::NONE;
                            gameState.transitionTimer = 0.0f;
                            gameState.pendingStageTransition = -1;
                            gameState.pendingReadyScreen = false;
                            gameState.pendingSkipCountdown = false;
                        }
                    }
                }
                
                // タイトル画面のアニメーションタイマーを更新
                gameState.titleScreenTimer += deltaTime;
                
                // フェーズの遷移
                const float BACKGROUND_FADE_DURATION = 1.5f;  // 背景フェードイン時間
                const float LOGO_ANIMATION_DURATION = 1.0f;    // ロゴアニメーション時間
                
                if (gameState.titleScreenPhase == GameState::TitleScreenPhase::BACKGROUND_FADE_IN) {
                    if (gameState.titleScreenTimer >= BACKGROUND_FADE_DURATION) {
                        gameState.titleScreenPhase = GameState::TitleScreenPhase::LOGO_ANIMATION;
                        gameState.titleScreenTimer = 0.0f;  // タイマーリセット
                    }
                } else if (gameState.titleScreenPhase == GameState::TitleScreenPhase::LOGO_ANIMATION) {
                    if (gameState.titleScreenTimer >= LOGO_ANIMATION_DURATION) {
                        gameState.titleScreenPhase = GameState::TitleScreenPhase::SHOW_TEXT;
                        gameState.titleScreenTimer = 0.0f;  // タイマーリセット
                    }
                }
                
                // タイトル画面のBGMを再生
                if (gameState.audioEnabled) {
                    std::string titleBGM = "title.ogg";
                    if (gameState.currentBGM != titleBGM) {
                        // 現在のBGMを停止
                        if (gameState.bgmPlaying) {
                            audioManager.stopBGM();
                            gameState.bgmPlaying = false;
                        }
                        
                        // タイトルBGMを読み込み・再生
                        std::string bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/title.ogg");
                        if (audioManager.loadBGM(bgmPath)) {
                            audioManager.playBGM();
                            gameState.currentBGM = titleBGM;
                            gameState.bgmPlaying = true;
                            std::cout << "Title BGM started: " << titleBGM << std::endl;
                        }
                    }
                }
                
                // キー状態更新
                for (auto& [key, state] : keyStates) {
                    state.update(glfwGetKey(window, key) == GLFW_PRESS);
                }
                
                // ENTERキーでゲーム開始（テキスト表示中のみ）
                if (keyStates[GLFW_KEY_ENTER].justPressed() && 
                    gameState.titleScreenPhase == GameState::TitleScreenPhase::SHOW_TEXT &&
                    !gameState.isTransitioning) {
                    // タイトルBGMを停止
                    if (gameState.bgmPlaying && gameState.currentBGM == "title.ogg") {
                        audioManager.stopBGM();
                        gameState.bgmPlaying = false;
                        gameState.currentBGM = "";
                    }
                    
                    // チュートリアル（ステージ6）がクリア済みかチェック
                    bool hasClearedTutorial = (gameState.stageStars.count(6) > 0 && gameState.stageStars[6] > 0);
                    
                    if (hasClearedTutorial) {
                        // クリア済み: ステージ選択フィールド（ステージ0）に直接遷移
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
                        
                        // SECRET STARモードをリセット
                        gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
                    } else {
                        // 未クリア: 遷移アニメーションを開始してチュートリアルへ
                        gameState.isTransitioning = true;
                        gameState.transitionType = GameState::TransitionType::FADE_OUT;
                        gameState.transitionTimer = 0.0f;
                        gameState.pendingStageTransition = 6;  // チュートリアルステージ
                        gameState.pendingReadyScreen = false;  // Ready画面をスキップ
                        gameState.pendingSkipCountdown = true;  // カウントダウンをスキップ
                    }
                }
                
                // 描画
                renderFrame(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer, keyStates, deltaTime);
                
                glfwPollEvents();
                continue;  // タイトル画面中は他の処理をスキップ
            }

            // ゲーム状態の更新
            updateGameState(window, gameState, stageManager, platformSystem, deltaTime, scaledDeltaTime, keyStates, resetStageStartTime, audioManager);

            // 描画
            renderFrame(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer, keyStates, deltaTime);
            
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
                          io::AudioManager& audioManager,
                          float deltaTime) {
        // ファイル監視：UI設定ファイルの変更をチェック（0.5秒ごと）
        static float readyScreenFileCheckTimer = 0.0f;
        readyScreenFileCheckTimer += deltaTime;
        if (readyScreenFileCheckTimer >= 0.5f) {
            readyScreenFileCheckTimer = 0.0f;
            UIConfig::UIConfigManager::getInstance().checkAndReloadConfig();
        }
        
        // Ready画面表示中はゲームを一時停止
        int width, height;
        prepareFrame(window, gameState, stageManager, renderer, width, height);
        
        // 通常のゲーム要素を描画（背景として）
        renderPlatforms(platformSystem, renderer, gameState, stageManager);
        
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
        
        // FPS/TPS切り替えはready画面では無効化
        
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
        renderPlatforms(platformSystem, renderer, gameState, stageManager);
        
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
            
            // MAX_SPEED_STARの場合は時間倍率を3倍に固定
            if (gameState.selectedSecretStarType == GameState::SecretStarType::MAX_SPEED_STAR) {
                gameState.timeScale = 3.0f;
                gameState.timeScaleLevel = 2;  // 3倍に設定
            }
            
            // IMMERSIVE_STARの場合はFPSモードを有効化
            if (gameState.selectedSecretStarType == GameState::SecretStarType::IMMERSIVE_STAR) {
                gameState.isFirstPersonView = true;
                gameState.cameraYaw = 90.0f;
                gameState.cameraPitch = -10.0f;
                gameState.firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            
            // タイムアタックモードの場合、開始時刻を記録
            if (gameState.isTimeAttackMode) {
                gameState.timeAttackStartTime = gameState.gameTime;
                gameState.currentTimeAttackTime = 0.0f;
                
                // リプレイ記録を開始
                gameState.isRecordingReplay = true;
                gameState.replayBuffer.clear();
                gameState.replayRecordTimer = 0.0f;
                
                // 最初のフレームを記録
                GameState::ReplayFrame firstFrame;
                firstFrame.timestamp = 0.0f;
                firstFrame.playerPosition = gameState.playerPosition;
                firstFrame.playerVelocity = gameState.playerVelocity;
                // アイテムの収集状態を記録
                firstFrame.itemCollectedStates.clear();
                for (const auto& item : gameState.items) {
                    firstFrame.itemCollectedStates.push_back(item.isCollected);
                }
                gameState.replayBuffer.push_back(firstFrame);
                
                printf("TIME ATTACK: Started at %.2f\n", gameState.timeAttackStartTime);
                printf("REPLAY: Recording started\n");
            }
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
                
                // ゲームクリア済みフラグを設定
                gameState.isGameCleared = true;
                gameState.showSecretStarExplanationUI = true;
                
                // フィールドに戻る
                stageManager.goToStage(0, gameState, platformSystem);
                gameState.playerPosition = glm::vec3(8, 0, 0);
                gameState.playerVelocity = glm::vec3(0, 0, 0);
                
                // ステージ選択フィールドではTPSモードに戻す
                gameState.isFirstPersonMode = false;
                gameState.isFirstPersonView = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                
                // SECRET STARモードをリセット
                gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
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
            
            // ゲームクリア済みフラグを設定
            gameState.isGameCleared = true;
            gameState.showSecretStarExplanationUI = true;
            
            stageManager.goToStage(0, gameState, platformSystem);
            gameState.playerPosition = glm::vec3(8, 0, 0);
            gameState.playerVelocity = glm::vec3(0, 0, 0);
            
            // ステージ選択フィールドではTPSモードに戻す
            gameState.isFirstPersonMode = false;
            gameState.isFirstPersonView = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            
            // SECRET STARモードをリセット
            gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
        }
        
        renderer->endFrame();
        
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
    }

    // リプレイ再生用のクリアタイムを保存
    float savedClearTime = 0.0f;

    void updateGameState(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                        PlatformSystem& platformSystem, float deltaTime, float scaledDeltaTime,
                        std::map<int, InputUtils::KeyState>& keyStates,
                        std::function<void()> resetStageStartTime, io::AudioManager& audioManager) {
        // エディタ状態（静的変数として保持、GameStateと共有）
        static EditorState editorState;
        gameState.editorState = &editorState;
        
        // エディタ入力処理（常に実行）
        StageEditor::processEditorInput(window, gameState, editorState, platformSystem, stageManager, deltaTime);
        
        // エディタモード中はカメラ移動のみ有効化（通常のゲームロジックはスキップ）
        if (editorState.isActive) {
            // エディタモード中でもカメラ移動を有効化
            InputSystem::processInput(window, gameState, deltaTime);
            return;
        }
        
        // ステージ遷移アニメーションの処理（タイトル→チュートリアルのみ）
        if (gameState.isTransitioning) {
            const float FADE_DURATION = 0.5f;  // フェード時間（秒）
            gameState.transitionTimer += deltaTime;
            
            if (gameState.transitionType == GameState::TransitionType::FADE_OUT) {
                // フェードアウト中
                if (gameState.transitionTimer >= FADE_DURATION) {
                    // フェードアウト完了：チュートリアルステージを読み込む
                    if (gameState.pendingStageTransition == 6) {
                        // タイトル画面を非表示
                        gameState.showTitleScreen = false;
                        
                        // チュートリアルステージを読み込む
                        stageManager.loadStage(6, gameState, platformSystem);
                        gameState.showReadyScreen = false;
                        gameState.readyScreenShown = true;
                        gameState.isCountdownActive = false;
                        gameState.countdownTimer = 0.0f;
                        resetStageStartTime();
                        
                        // フェードインに切り替え
                        gameState.transitionType = GameState::TransitionType::FADE_IN;
                        gameState.transitionTimer = 0.0f;
                    }
                }
            } else if (gameState.transitionType == GameState::TransitionType::FADE_IN) {
                // フェードイン中
                if (gameState.transitionTimer >= FADE_DURATION) {
                    // フェードイン完了：遷移終了
                    gameState.isTransitioning = false;
                    gameState.transitionType = GameState::TransitionType::NONE;
                    gameState.transitionTimer = 0.0f;
                    gameState.pendingStageTransition = -1;
                    gameState.pendingReadyScreen = false;
                    gameState.pendingSkipCountdown = false;
                }
            }
            
            // 遷移中は他の処理をスキップ
            return;
        }
        
        // ファイル監視：ステージJSONファイルの変更をチェック（0.5秒ごと）
        // エディタモード中はホットリロードを無効化（エディタで編集中の内容が上書きされないように）
        if (!editorState.isActive) {
            static float fileCheckTimer = 0.0f;
            fileCheckTimer += deltaTime;
            if (fileCheckTimer >= 0.5f) {
                fileCheckTimer = 0.0f;
                stageManager.checkAndReloadStage(gameState, platformSystem);
                // テクスチャと音声の監視も同時に実行
                gfx::TextureManager::checkAndReloadTextures();
                audioManager.checkAndReloadAudio();
                // UI設定ファイルの監視も実行
                UIConfig::UIConfigManager::getInstance().checkAndReloadConfig();
            }
        }
        
        // 時間停止スキル発動時の処理
        if (gameState.isTimeStopped) {
            gameState.timeStopTimer -= deltaTime;
            if (gameState.timeStopTimer <= 0.0f) {
                gameState.isTimeStopped = false;
                gameState.timeStopTimer = 0.0f;
            }
        }
        
        // フリーカメラスキル発動時の処理（エディタモード中はタイマーを減らさない）
        if (gameState.isFreeCameraActive && !editorState.isActive) {
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
        
        // リプレイ再生モードの処理
        if (gameState.isReplayMode) {
            // 左右キーで巻き戻し/早送り（ポーズ中でも少しずつ移動可能）
            float rewindSpeed = 0.0f;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                if (gameState.isReplayPaused) {
                    rewindSpeed = -0.5f;  // ポーズ中はゆっくり巻き戻し
                } else {
                    rewindSpeed = -2.0f;  // 通常時は2倍速で巻き戻し
                }
            } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                if (gameState.isReplayPaused) {
                    rewindSpeed = 0.5f;   // ポーズ中はゆっくり早送り
                } else {
                    rewindSpeed = 2.0f;   // 通常時は2倍速で早送り
                }
            }
            
            if (!gameState.isReplayPaused && rewindSpeed == 0.0f) {
                // 通常再生
                gameState.replayPlaybackTime += deltaTime * gameState.replayPlaybackSpeed;
            } else if (rewindSpeed != 0.0f) {
                // 巻き戻し/早送り（ポーズ中も含む）
                gameState.replayPlaybackTime += deltaTime * rewindSpeed * gameState.replayPlaybackSpeed;
                // 時間が負にならないように制限
                if (gameState.replayPlaybackTime < 0.0f) {
                    gameState.replayPlaybackTime = 0.0f;
                }
            }
            
            // リプレイデータから現在時刻に該当するフレームを取得して補間
            if (!gameState.currentReplay.frames.empty()) {
                // 最後のフレームを超えた場合はリプレイ終了
                if (gameState.replayPlaybackTime >= gameState.currentReplay.frames.back().timestamp) {
                    gameState.replayPlaybackTime = gameState.currentReplay.frames.back().timestamp;
                    
                    // リプレイ終了：クリア画面に戻る
                    gameState.isReplayMode = false;
                    gameState.isReplayPaused = false;
                    gameState.showStageClearUI = true;
                    gameState.isStageCompleted = true;
                    gameState.isGoalReached = true;
                    // タイムアタックモードではcurrentTimeAttackTimeを復元
                    if (gameState.isTimeAttackMode) {
                        gameState.currentTimeAttackTime = savedClearTime;  // 保存しておいたクリアタイムを復元
                    } else {
                        gameState.clearTime = savedClearTime;  // 通常モード用
                    }
                    gameState.replayPlaybackTime = 0.0f;
                    
                    // 最後のフレームの位置を設定
                    const auto& lastFrame = gameState.currentReplay.frames.back();
                    gameState.playerPosition = lastFrame.playerPosition;
                    gameState.playerVelocity = lastFrame.playerVelocity;
                    
                    printf("REPLAY: Finished, returning to stage clear screen\n");
                } else {
                    // 2つのフレーム間を補間
                    for (size_t i = 0; i < gameState.currentReplay.frames.size() - 1; i++) {
                        const auto& frame1 = gameState.currentReplay.frames[i];
                        const auto& frame2 = gameState.currentReplay.frames[i + 1];
                        
                        if (gameState.replayPlaybackTime >= frame1.timestamp && 
                            gameState.replayPlaybackTime <= frame2.timestamp) {
                            float t = (gameState.replayPlaybackTime - frame1.timestamp) / 
                                     (frame2.timestamp - frame1.timestamp);
                            t = std::clamp(t, 0.0f, 1.0f);
                            
                            gameState.playerPosition = glm::mix(frame1.playerPosition, frame2.playerPosition, t);
                            gameState.playerVelocity = glm::mix(frame1.playerVelocity, frame2.playerVelocity, t);
                            
                            // アイテムの収集状態を復元（より近いフレームの状態を使用）
                            if (!frame1.itemCollectedStates.empty() && frame1.itemCollectedStates.size() == gameState.items.size() &&
                                !frame2.itemCollectedStates.empty() && frame2.itemCollectedStates.size() == gameState.items.size()) {
                                // フレーム1とフレーム2の間で、より近い方の状態を使用
                                // 巻き戻し時：frame1の状態を使用（過去の状態に戻す）
                                // 早送り時：frame2の状態を使用（未来の状態に進める）
                                bool useFrame1 = (t < 0.5f);
                                
                                for (size_t j = 0; j < gameState.items.size() && j < frame1.itemCollectedStates.size(); j++) {
                                    bool shouldBeCollected = useFrame1 ? frame1.itemCollectedStates[j] : frame2.itemCollectedStates[j];
                                    
                                    // 現在の状態と目標状態が異なる場合のみ更新
                                    if (gameState.items[j].isCollected != shouldBeCollected) {
                                        if (shouldBeCollected) {
                                            // アイテムを収集状態にする
                                            gameState.items[j].isCollected = true;
                                            gameState.collectedItems++;
                                        } else {
                                            // アイテムを未収集状態に戻す（復活）
                                            gameState.items[j].isCollected = false;
                                            gameState.collectedItems--;
                                        }
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
        
        // リプレイ記録の処理（タイムアタックモード中）
        if (gameState.isRecordingReplay && gameState.isTimeAttackMode) {
            if (!gameState.isCountdownActive && gameState.timeAttackStartTime > 0.0f) {
                gameState.replayRecordTimer += deltaTime;
                
                // 一定間隔でフレームを記録
                if (gameState.replayRecordTimer >= gameState.REPLAY_RECORD_INTERVAL) {
                    GameState::ReplayFrame frame;
                    frame.timestamp = gameState.currentTimeAttackTime;
                    frame.playerPosition = gameState.playerPosition;
                    frame.playerVelocity = gameState.playerVelocity;
                    // アイテムの収集状態を記録
                    frame.itemCollectedStates.clear();
                    for (const auto& item : gameState.items) {
                        frame.itemCollectedStates.push_back(item.isCollected);
                    }
                    gameState.replayBuffer.push_back(frame);
                    
                    gameState.replayRecordTimer = 0.0f;
                }
            }
        }
        
        // タイムアタックモードの処理
        if (gameState.isTimeAttackMode) {
            // タイムアタックモード：カウントダウン終了後、経過時間を計測
            if (!gameState.isCountdownActive && gameState.timeAttackStartTime > 0.0f) {
                if (!gameState.isStageCompleted && !gameState.isTimeStopped) {
                    gameState.currentTimeAttackTime += deltaTime;
                }
            }
        } else {
            // 通常モード：制限時間システムの更新（時間停止中は更新しない）
        if (!gameState.isStageCompleted && !gameState.isTimeUp && !gameState.isTimeStopped) {
            gameState.remainingTime -= deltaTime;
            
            if (gameState.remainingTime <= 0.0f) {
                gameState.remainingTime = 0.0f;
                gameState.isTimeUp = true;
                gameState.isGameOver = true;
                gameState.gameOverTimer = 0.0f;
                }
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
        
        // リプレイモード中は物理演算をスキップ（位置はリプレイデータから設定される）
        // ゲームオーバー時も物理演算をスキップ（無限落下を防ぐ）
        if (!gameState.isReplayMode && !gameState.isGameOver) {
        updatePhysicsAndCollisions(window, gameState, stageManager, platformSystem, deltaTime, scaledDeltaTime, audioManager);
        } else {
            // リプレイモード中またはゲームオーバー中でも足場の更新は必要（視覚的な動きのため）
            if (!gameState.isGameOver) {
                platformSystem.update(scaledDeltaTime, gameState.playerPosition);
            }
        }
        
        // アイテムの更新（リプレイモード中はポーズ中でない場合のみ、または通常モード）
        if (!gameState.isReplayMode || !gameState.isReplayPaused) {
        updateItems(gameState, scaledDeltaTime, audioManager);
        }
        handleStageSelectionArea(window, gameState, stageManager, platformSystem, resetStageStartTime);
    }

    void updatePhysicsAndCollisions(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                                   PlatformSystem& platformSystem, float deltaTime, float scaledDeltaTime, io::AudioManager& audioManager) {
        // ゲームオーバー時は物理演算をスキップ
        if (gameState.isGameOver) {
            return;
        }
        
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
                            
                            // リプレイモード中はクリアタイムを更新しない
                            if (!gameState.isReplayMode) {
                            gameState.clearTime = gameState.gameTime;
                            }
                            
                            int currentStage = stageManager.getCurrentStage();
                            
                            // タイムアタックモードの場合
                            if (gameState.isTimeAttackMode) {
                                float clearTime = gameState.currentTimeAttackTime;
                                
                                // リプレイ記録を停止
                                if (gameState.isRecordingReplay) {
                                    // 最後のフレームを記録
                                    GameState::ReplayFrame lastFrame;
                                    lastFrame.timestamp = clearTime;
                                    lastFrame.playerPosition = gameState.playerPosition;
                                    lastFrame.playerVelocity = gameState.playerVelocity;
                                    gameState.replayBuffer.push_back(lastFrame);
                                    
                                    gameState.isRecordingReplay = false;
                                    printf("REPLAY: Recording stopped (%zu frames)\n", gameState.replayBuffer.size());
                                }
                                
                                // ベストタイムを更新
                                gameState.isNewRecord = false;
                                bool shouldSaveReplay = false;
                                if (gameState.timeAttackRecords.find(currentStage) == gameState.timeAttackRecords.end() ||
                                    clearTime < gameState.timeAttackRecords[currentStage]) {
                                    gameState.timeAttackRecords[currentStage] = clearTime;
                                    gameState.isNewRecord = true;
                                    shouldSaveReplay = true;
                                    printf("TIME ATTACK: New record for stage %d: %.2fs\n", currentStage, clearTime);
                                } else {
                                    printf("TIME ATTACK: Stage %d cleared in %.2fs (Best: %.2fs)\n", 
                                           currentStage, clearTime, gameState.timeAttackRecords[currentStage]);
                                }
                                
                                // ベストタイム更新時のみリプレイを保存
                                if (shouldSaveReplay && !gameState.replayBuffer.empty()) {
                                    GameState::ReplayData replayData;
                                    replayData.stageNumber = currentStage;
                                    replayData.clearTime = clearTime;
                                    replayData.frames = gameState.replayBuffer;
                                    replayData.frameRate = gameState.REPLAY_RECORD_INTERVAL;
                                    
                                    // 現在の日時を文字列として取得
                                    auto now = std::time(nullptr);
                                    std::stringstream ss;
                                    ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
                                    replayData.recordedDate = ss.str();
                                    
                                    ReplayManager::saveReplay(replayData, currentStage);
                                }
                                
                                // タイムアタックモードでは星数は常に3（ベストタイム更新時のみ）
                                gameState.earnedStars = gameState.isNewRecord ? 3 : 0;
                            } else {
                                // 通常モード：制限時間の長さによって獲得する星数を変える
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
                            int oldStars = gameState.stageStars[currentStage];
                            int starDifference = gameState.earnedStars - oldStars;
                            
                            if (starDifference > 0) {
                                gameState.stageStars[currentStage] = gameState.earnedStars;
                                gameState.totalStars += starDifference;
                                }
                            }
                        
                            // SECRET STARモードでクリアした場合、クリア状況を記録（ゴール到達時）
                            if (gameState.selectedSecretStarType != GameState::SecretStarType::NONE && currentStage > 0) {
                                gameState.secretStarCleared[currentStage].insert(gameState.selectedSecretStarType);
                                // ゲームデータをセーブ（SECRET STARのクリア状況を含む）
                                SaveManager::saveGameData(gameState);
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
        // ゲームオーバー中は落下判定をスキップ（無限ループを防ぐ）
        if (gameState.playerPosition.y < 0 && !gameState.isGameOver) {
            // ダメージSEを再生
            if (gameState.audioEnabled) {
                audioManager.playSFX("on_ground");  // damageSEとしてon_groundを使用
            }
            
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
                
                // ダメージSEを再生
                if (gameState.audioEnabled) {
                    audioManager.playSFX("damage");
                }
                
                if (gameState.lives <= 0) {
                    gameState.isGameOver = true;
                    gameState.gameOverTimer = 0.0f;
                    // ゲームオーバー時も位置をリセット（無限落下を防ぐ）
                    if (gameState.lastCheckpointItemId != -1) {
                        gameState.playerPosition = gameState.lastCheckpoint;
                    } else {
                        gameState.playerPosition = glm::vec3(0, 6.0f, -25.0f);
                    }
                    gameState.playerVelocity = glm::vec3(0, 0, 0);
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
                    std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                    std::map<int, InputUtils::KeyState>& keyStates,
                    float deltaTime) {
        // エディタ状態をGameStateから取得（updateGameStateと共有）
        EditorState* editorState = gameState.editorState;
        if (!editorState) {
            // エディタ状態が初期化されていない場合はスキップ
            static EditorState defaultEditorState;
            editorState = &defaultEditorState;
        }
        
        // ファイル監視：UI設定ファイルの変更をチェック（0.5秒ごと）
        static float gameUIFileCheckTimer = 0.0f;
        gameUIFileCheckTimer += deltaTime;
        if (gameUIFileCheckTimer >= 0.5f) {
            gameUIFileCheckTimer = 0.0f;
            UIConfig::UIConfigManager::getInstance().checkAndReloadConfig();
        }
        
        int width, height;
        
        // タイトル画面の処理（背景を描画しない）
        if (gameState.showTitleScreen) {
            // タイトル画面の場合は背景を描画せず、直接タイトル画面を描画
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // 黒でクリア
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // タイトル画面を描画
            gameStateUIRenderer->renderTitleScreen(width, height, gameState);
            
            // ステージ遷移アニメーションのオーバーレイを描画（タイトル画面中でも必要）
            if (gameState.isTransitioning) {
                const float FADE_DURATION = 0.5f;
                float alpha = 0.0f;
                
                if (gameState.transitionType == GameState::TransitionType::FADE_OUT) {
                    // フェードアウト：0.0 → 1.0
                    alpha = std::min(1.0f, gameState.transitionTimer / FADE_DURATION);
                } else if (gameState.transitionType == GameState::TransitionType::FADE_IN) {
                    // フェードイン：1.0 → 0.0
                    alpha = 1.0f - std::min(1.0f, gameState.transitionTimer / FADE_DURATION);
                }
                
                // 黒いオーバーレイを描画
                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                glLoadIdentity();
                glOrtho(0, width, height, 0, -1, 1);
                
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glLoadIdentity();
                
                glDisable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(0.0f, 0.0f, 0.0f, alpha);
                
                glBegin(GL_QUADS);
                glVertex2f(0, 0);
                glVertex2f(width, 0);
                glVertex2f(width, height);
                glVertex2f(0, height);
                glEnd();
                
                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST);
                
                glMatrixMode(GL_PROJECTION);
                glPopMatrix();
                glMatrixMode(GL_MODELVIEW);
                glPopMatrix();
            }
            
            // フレームバッファをスワップ（Macで確実に表示されるように）
            renderer->endFrame();
            glfwPollEvents();  // イベントを処理してウィンドウを更新
            
            return;  // タイトル画面中は他の処理をスキップ
        }
        
        prepareFrame(window, gameState, stageManager, renderer, width, height);
        
        // エディタモード中はグリッドとプレビューを表示
        if (editorState->isActive) {
            CameraConfig cameraConfig = CameraSystem::calculateCameraConfig(gameState, stageManager, window);
            StageEditor::renderGrid(cameraConfig.position, editorState->gridSize, 20);
            
            // プレビュー位置にオブジェクトを表示
            if (editorState->showPreview && editorState->currentMode == EditorMode::PLACE) {
                glPushMatrix();
                glTranslatef(editorState->previewPosition.x, editorState->previewPosition.y, editorState->previewPosition.z);
                glColor3f(0.5f, 1.0f, 0.5f);  // 薄い緑色
                glBegin(GL_LINE_LOOP);
                float halfSize = 2.0f;
                glVertex3f(-halfSize, 0, -halfSize);
                glVertex3f( halfSize, 0, -halfSize);
                glVertex3f( halfSize, 0,  halfSize);
                glVertex3f(-halfSize, 0,  halfSize);
                glEnd();
                glPopMatrix();
            }
        }
        
        // UIレンダラーにウィンドウサイズを設定（スケーリング用）
        uiRenderer->setWindowSize(width, height);
        
        // 足場の描画
        renderPlatforms(platformSystem, renderer, gameState, stageManager);
        
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
            itemFirstTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/item_first.png"));
            if (itemFirstTexture == 0) {
                itemFirstTexture = gfx::TextureManager::loadTexture("assets/textures/item_first.png");
            }
        }
        
        static GLuint itemSecondTexture = 0;
        if (itemSecondTexture == 0) {
            itemSecondTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/item_second.png"));
            if (itemSecondTexture == 0) {
                itemSecondTexture = gfx::TextureManager::loadTexture("assets/textures/item_second.png");
            }
        }
        
        static GLuint itemThirdTexture = 0;
        if (itemThirdTexture == 0) {
            itemThirdTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/item_third.png"));
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
    
        // ステージ情報（リプレイモード中は非表示）
        if (!gameState.isReplayMode) {
        const StageData* currentStageData = stageManager.getStageData(stageManager.getCurrentStage());
        if (currentStageData && stageManager.getCurrentStage()!=0) {
            // チュートリアルステージの場合は「TUTORIAL」を表示
            auto& uiConfig = UIConfig::UIConfigManager::getInstance();
            auto stageInfoConfig = uiConfig.getStageInfoConfig();
            glm::vec2 stageInfoPos = uiConfig.calculatePosition(stageInfoConfig.position, width, height);
            
            if (stageManager.getCurrentStage() == 6) {
                uiRenderer->renderText("TUTORIAL", 
                                   stageInfoPos, stageInfoConfig.color, stageInfoConfig.scale);
            } else {
                // 通常のステージは「STAGE X」を表示
                uiRenderer->renderText("STAGE " + std::to_string(stageManager.getCurrentStage()), 
                                   stageInfoPos, stageInfoConfig.color, stageInfoConfig.scale);
                }
            }
        }
        
        
        // 速度倍率の表示（全ステージで表示、チュートリアルステージでは条件付き、タイムアタックモードでは非表示）
        if(stageManager.getCurrentStage()!=0){
            bool shouldShowSpeedUI = true;
            if (stageManager.getCurrentStage() == 6 && gameState.tutorialStep >= 6 && gameState.tutorialStep < 9) {
                shouldShowSpeedUI = false; // ステップ6-8は速度倍率UIを非表示
            }
            if (gameState.isTimeAttackMode) {
                shouldShowSpeedUI = false; // タイムアタックモードでは非表示（renderTimeAttackUI内で表示されるため）
            }
            
            if (shouldShowSpeedUI) {
                auto& uiConfig = UIConfig::UIConfigManager::getInstance();
                auto speedConfig = uiConfig.getSpeedDisplayConfig();
                auto pressTConfig = uiConfig.getPressTConfig();
                
                std::string speedText = std::to_string((int)gameState.timeScale) + "x";
                glm::vec3 speedColor = (gameState.timeScale > 1.0f) ? glm::vec3(1.0f, 0.8f, 0.2f) : speedConfig.color;
                glm::vec2 speedPos = uiConfig.calculatePosition(speedConfig.position, width, height);
                uiRenderer->renderText(speedText, speedPos, speedColor, speedConfig.scale);
                
                // PRESS T表示（リプレイモード中でない場合のみ）
                if (!gameState.isReplayMode) {
                    std::string speedText2 = "PRESS T";
                    glm::vec3 speedColor2 = (gameState.timeScale > 1.0f) ? glm::vec3(1.0f, 0.8f, 0.2f) : pressTConfig.color;
                    glm::vec2 pressTPos = uiConfig.calculatePosition(pressTConfig.position, width, height);
                    uiRenderer->renderText(speedText2, pressTPos, speedColor2, pressTConfig.scale);
                }
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
                    int currentStage = stageManager.getCurrentStage();
                    int secretStarType = (gameState.selectedSecretStarType != GameState::SecretStarType::NONE) ? 
                                         static_cast<int>(gameState.selectedSecretStarType) - 1 : -1;  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                    std::map<int, std::set<int>> secretStarClearedInt;
                    for (const auto& [stage, types] : gameState.secretStarCleared) {
                        std::set<int> typesInt;
                        for (auto type : types) {
                            typesInt.insert(static_cast<int>(type) - 1);  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                        }
                        secretStarClearedInt[stage] = typesInt;
                    }
                    uiRenderer->renderLivesTimeAndStarsUI(gameState.lives, gameState.remainingTime, gameState.timeLimit, gameState.earnedStars, currentStageStars,
                                                          currentStage, secretStarType, secretStarClearedInt);
                } else if (stageManager.getCurrentStage() == 6 && gameState.tutorialStep >= 9) {
                    // チュートリアルステージでステップ9以降は通常のUIを表示（全部あるやつ）
                    int currentStageStars = gameState.stageStars[stageManager.getCurrentStage()];
                    int currentStage = stageManager.getCurrentStage();
                    int secretStarType = (gameState.selectedSecretStarType != GameState::SecretStarType::NONE) ? 
                                         static_cast<int>(gameState.selectedSecretStarType) - 1 : -1;  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                    std::map<int, std::set<int>> secretStarClearedInt;
                    for (const auto& [stage, types] : gameState.secretStarCleared) {
                        std::set<int> typesInt;
                        for (auto type : types) {
                            typesInt.insert(static_cast<int>(type) - 1);  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                        }
                        secretStarClearedInt[stage] = typesInt;
                    }
                    uiRenderer->renderTimeUI(gameState.remainingTime, gameState.timeLimit, gameState.earnedStars, currentStageStars, gameState.lives,
                                              currentStage, secretStarType, secretStarClearedInt);
                } else {
                    // 通常のステージでは全てのUIを表示（エンディングシーケンス中とリプレイモード中は一部非表示）
                    if (!gameState.isEndingSequence) {
                        int currentStageStars = gameState.stageStars[stageManager.getCurrentStage()];
                        
                        // タイムアタックモードかどうかでUIを切り替え（リプレイモード中でもタイムは表示）
                        if (gameState.isTimeAttackMode) {
                            float bestTime = 0.0f;
                            if (gameState.timeAttackRecords.find(stageManager.getCurrentStage()) != gameState.timeAttackRecords.end()) {
                                bestTime = gameState.timeAttackRecords[stageManager.getCurrentStage()];
                            }
                            
                            // リプレイモード中のタイムアタック時間を計算
                            float displayTime = gameState.currentTimeAttackTime;
                            if (gameState.isReplayMode) {
                                // リプレイ中の場合は、リプレイの再生時間に基づいてタイムを計算
                                displayTime = gameState.replayPlaybackTime;
                            }
                            
                            // リプレイモード中は星とライフのUIを非表示にするため、earnedStarsとcurrentStageStars、livesを0に設定
                            if (gameState.isReplayMode) {
                                uiRenderer->renderTimeAttackUI(displayTime, bestTime, 0, 0, 0, gameState.timeScale, true);
                            } else {
                                uiRenderer->renderTimeAttackUI(gameState.currentTimeAttackTime, bestTime, gameState.earnedStars, currentStageStars, gameState.lives, gameState.timeScale, false);
                            }
                        } else {
                            // 通常モードではリプレイ中でもUIを表示しない（制限時間はリプレイ中は不要）
                            if (!gameState.isReplayMode) {
                                int currentStage = stageManager.getCurrentStage();
                                int secretStarType = (gameState.selectedSecretStarType != GameState::SecretStarType::NONE) ? 
                                                     static_cast<int>(gameState.selectedSecretStarType) - 1 : -1;  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                                std::map<int, std::set<int>> secretStarClearedInt;
                                for (const auto& [stage, types] : gameState.secretStarCleared) {
                                    std::set<int> typesInt;
                                    for (auto type : types) {
                                        typesInt.insert(static_cast<int>(type) - 1);  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                                    }
                                    secretStarClearedInt[stage] = typesInt;
                                }
                                uiRenderer->renderTimeUI(gameState.remainingTime, gameState.timeLimit, gameState.earnedStars, currentStageStars, gameState.lives,
                                                          currentStage, secretStarType, secretStarClearedInt);
                            }
                        }
                        
                        // スキルUIはリプレイモード中は非表示
                        if (!gameState.isReplayMode) {
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
                        
                        // ミニマップを描画（ステージ0とエンディングシーケンス中は非表示、リプレイモード中でも表示）
                        if (stageManager.getCurrentStage() != 0 && !gameState.isEndingSequence) {
                            static gfx::MinimapRenderer minimapRenderer;
                            minimapRenderer.render(gameState, platformSystem, width, height, stageManager.getCurrentStage(), 30.0f);
                        }
                    }
                }
            }
        }
        
        // ステージクリアUI（エンディングシーケンス中とリプレイモード中は非表示）
        if (gameState.showStageClearUI && !gameState.isEndingSequence && !gameState.isReplayMode) {
            // タイムアタックモードの場合は専用のUIを表示
            if (gameState.isTimeAttackMode) {
                float clearTime = gameState.currentTimeAttackTime;  // タイムアタックモードではcurrentTimeAttackTimeを使用
                float bestTime = 0.0f;
                if (gameState.timeAttackRecords.find(stageManager.getCurrentStage()) != gameState.timeAttackRecords.end()) {
                    bestTime = gameState.timeAttackRecords[stageManager.getCurrentStage()];
                }
                gameStateUIRenderer->renderTimeAttackClearBackground(width, height, clearTime, bestTime, gameState.isNewRecord);
            } else {
                // 通常モードの場合は従来のUIを表示
                int currentStage = stageManager.getCurrentStage();
                gameStateUIRenderer->renderStageClearBackground(width, height, gameState.clearTime, gameState.stageStars[currentStage], false,
                                                                 currentStage, gameState.selectedSecretStarType, gameState.secretStarCleared);
            }
        }
        
        // リプレイモード中のUI表示
        if (gameState.isReplayMode) {
            uiRenderer->begin2DMode();
            
            auto& uiConfig = UIConfig::UIConfigManager::getInstance();
            
            // 巻き戻し/早送り状態を判定
            bool isRewinding = false;
            bool isFastForwarding = false;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                isRewinding = true;
            } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                isFastForwarding = true;
            }
            
            // 常時表示されるマークとPRESSキー表示（画面下、REPLAY SPEEDの上）
            // 巻き戻しマーク「<<」（巻き戻し中は色が変わる）
            auto rewindMarkConfig = uiConfig.getReplayRewindMarkAlwaysConfig();
            glm::vec2 rewindMarkPos = uiConfig.calculatePosition(rewindMarkConfig.position, width, height);
            glm::vec3 rewindMarkColor = isRewinding ? rewindMarkConfig.activeColor : rewindMarkConfig.color;
            uiRenderer->renderText("<<", rewindMarkPos, rewindMarkColor, rewindMarkConfig.scale);
            
            // PRESS A表示
            auto pressAConfig = uiConfig.getReplayPressAConfig();
            glm::vec2 pressAPos = uiConfig.calculatePosition(pressAConfig.position, width, height);
            uiRenderer->renderText("PRESS A", pressAPos, pressAConfig.color, pressAConfig.scale);
            
            // ポーズマーク「||」（ポーズ中は色が変わる）
            auto pauseMarkConfig = uiConfig.getReplayPauseMarkAlwaysConfig();
            glm::vec2 pauseMarkPos = uiConfig.calculatePosition(pauseMarkConfig.position, width, height);
            glm::vec3 pauseMarkColor = gameState.isReplayPaused ? pauseMarkConfig.activeColor : pauseMarkConfig.color;
            uiRenderer->renderText("||", pauseMarkPos, pauseMarkColor, pauseMarkConfig.scale);
            
            // PRESS SPACE表示
            auto pressSpaceConfig = uiConfig.getReplayPressSpaceConfig();
            glm::vec2 pressSpacePos = uiConfig.calculatePosition(pressSpaceConfig.position, width, height);
            uiRenderer->renderText("PRESS SPACE", pressSpacePos, pressSpaceConfig.color, pressSpaceConfig.scale);
            
            // 早送りマーク「>>»（早送り中は色が変わる）
            auto fastForwardMarkConfig = uiConfig.getReplayFastForwardMarkAlwaysConfig();
            glm::vec2 fastForwardMarkPos = uiConfig.calculatePosition(fastForwardMarkConfig.position, width, height);
            glm::vec3 fastForwardMarkColor = isFastForwarding ? fastForwardMarkConfig.activeColor : fastForwardMarkConfig.color;
            uiRenderer->renderText(">>", fastForwardMarkPos, fastForwardMarkColor, fastForwardMarkConfig.scale);
            
            // PRESS D表示
            auto pressDConfig = uiConfig.getReplayPressDConfig();
            glm::vec2 pressDPos = uiConfig.calculatePosition(pressDConfig.position, width, height);
            uiRenderer->renderText("PRESS D", pressDPos, pressDConfig.color, pressDConfig.scale);
            
            // REPLAY SPEEDラベル（速度表示の上）
            auto speedLabelConfig = uiConfig.getReplaySpeedLabelConfig();
            glm::vec2 speedLabelPos = uiConfig.calculatePosition(speedLabelConfig.position, width, height);
            uiRenderer->renderText("REPLAY SPEED", speedLabelPos, speedLabelConfig.color, speedLabelConfig.scale);
            
            // 速度表示（画面中央で画面下寄り）
            std::string speedText;
            if (gameState.replayPlaybackSpeed == 0.3f) {
                speedText = "0.3x";
            } else if (gameState.replayPlaybackSpeed == 0.5f) {
                speedText = "0.5x";
            } else {
                speedText = "1.0x";
            }
            auto speedConfig = uiConfig.getReplaySpeedDisplayConfig();
            glm::vec2 speedPos = uiConfig.calculatePosition(speedConfig.position, width, height);
            uiRenderer->renderText(speedText, speedPos, speedConfig.color, speedConfig.scale);
            
            // PRESS T表示（速度表示の下）
            auto pressTConfig = uiConfig.getReplayPressTConfig();
            glm::vec2 pressTPos = uiConfig.calculatePosition(pressTConfig.position, width, height);
            uiRenderer->renderText("PRESS T", pressTPos, pressTConfig.color, pressTConfig.scale);
            
            // 操作説明
            auto instructionsConfig = uiConfig.getReplayInstructionsConfig();
            glm::vec2 instructionsPos = uiConfig.calculatePosition(instructionsConfig.position, width, height);
            // 画面下からのオフセットを考慮
            if (instructionsConfig.position.useRelative && instructionsConfig.position.offsetY < 0) {
                instructionsPos.y = height + instructionsConfig.position.offsetY;
            }
            uiRenderer->renderText("SPACE: Pause/Resume  A/D: Rewind/FastForward  T: Speed  ESC: Exit", 
                                  instructionsPos, instructionsConfig.color, instructionsConfig.scale);
            
            uiRenderer->end2DMode();
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
                    
                    // クリア済みの場合、SECRET STAR用の星を追加表示（ゲームクリア後のみ表示）
                    bool hasClearedStage = (gameState.isGameCleared && gameState.stageStars.count(stageNumber) > 0 && gameState.stageStars[stageNumber] > 0);
                    if (hasClearedStage) {
                        // SECRET STARタイプの順序: MAX_SPEED_STAR, SHADOW_STAR, IMMERSIVE_STAR（左から右）
                        std::vector<GameState::SecretStarType> secretStarTypes = {
                            GameState::SecretStarType::MAX_SPEED_STAR,
                            GameState::SecretStarType::SHADOW_STAR,
                            GameState::SecretStarType::IMMERSIVE_STAR
                        };
                        
                        // 各SECRET STARタイプの色定義
                        std::vector<glm::vec3> secretStarColors = {
                            glm::vec3(0.2f, 0.8f, 1.0f),  // MAX_SPEED_STAR: 水色
                            glm::vec3(0.1f, 0.1f, 0.1f),  // SHADOW_STAR: 黒
                            glm::vec3(1.0f, 0.4f, 0.8f)   // IMMERSIVE_STAR: ピンク
                        };
                        
                        glm::vec3 inactiveColor = glm::vec3(0.5f, 0.5f, 0.5f); // 灰色（未獲得）
                        
                        // 現在のステージでクリア済みのSECRET STARタイプを取得
                        std::set<GameState::SecretStarType> clearedTypes;
                        if (gameState.secretStarCleared.count(stageNumber) > 0) {
                            clearedTypes = gameState.secretStarCleared.at(stageNumber);
                        }
                        
                        // SECRET STAR用の星を通常の星の下に表示（左から右に：MAX_SPEED_STAR, SHADOW_STAR, IMMERSIVE_STAR）
                        for (int i = 0; i < 3; i++) {
                            bool isCleared = (clearedTypes.count(secretStarTypes[i]) > 0);
                            glm::vec3 starColor = isCleared ? secretStarColors[i] : inactiveColor;
                            
                            // 星の位置をエリアの中心から計算（通常の星の下、左から右に並べる）
                            glm::vec3 starPos = glm::vec3(
                                area.x - 0.8f + (2 - i) * 0.8f,  // 順序を逆にして左から右に：MAX_SPEED_STAR, SHADOW_STAR, IMMERSIVE_STAR
                                area.y + area.height + 1.5f - 0.7f,  // 通常の星の下に配置
                                area.z
                            );
                            
                            renderer->renderer3D.renderStar3D(starPos, starColor, 1.0f);
                        }
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
            
            auto& uiConfig = UIConfig::UIConfigManager::getInstance();
            
            // WORLD 1表示
            auto worldTitleConfig = uiConfig.getWorldTitleConfig();
            glm::vec2 worldTitlePos = uiConfig.calculatePosition(worldTitleConfig.position, width, height);
            uiRenderer->renderText("WORLD 1", worldTitlePos, worldTitleConfig.color, worldTitleConfig.scale);
            
            // 左上に星アイコンとトータル星数を表示
            auto starIconConfig = uiConfig.getStarIconConfig();
            glm::vec2 starIconPos = uiConfig.calculatePosition(starIconConfig.position, width, height);
            uiRenderer->renderStar(starIconPos, starIconConfig.color, starIconConfig.scale);
            auto starCountConfig = uiConfig.getStarCountConfig();
            glm::vec2 starCountPos = uiConfig.calculatePosition(starCountConfig.position, width, height);
            uiRenderer->renderText("x " + std::to_string(gameState.totalStars), starCountPos, starCountConfig.color, starCountConfig.scale);
            
            // 星数の右にEASY/NORMAL表示（個別の設定を使用）
            if (gameState.isEasyMode) {
                auto easyModeTextConfig = uiConfig.getEasyModeTextConfig();
                glm::vec2 easyModeTextPosition = uiConfig.calculatePosition(easyModeTextConfig.position, width, height);
                uiRenderer->renderText("EASY", easyModeTextPosition, easyModeTextConfig.color, easyModeTextConfig.scale);
            } else {
                auto normalModeTextConfig = uiConfig.getNormalModeTextConfig();
                glm::vec2 normalModeTextPosition = uiConfig.calculatePosition(normalModeTextConfig.position, width, height);
                uiRenderer->renderText("NORMAL", normalModeTextPosition, normalModeTextConfig.color, normalModeTextConfig.scale);
            }
            
            // EASY/NORMALの下にPRESS E表示
            auto pressEConfig = uiConfig.getPressEConfig();
            glm::vec2 pressEPos = uiConfig.calculatePosition(pressEConfig.position, width, height);
            uiRenderer->renderText("PRESS E", pressEPos, pressEConfig.color, pressEConfig.scale);
            
            // 初回ステージ0入場チュートリアルUIの表示
            if (gameState.showStage0Tutorial) {
                gameStateUIRenderer->renderStage0Tutorial(width, height);
            }
            
            // EASYモード説明UIの表示
            if (gameState.showEasyModeExplanationUI) {
                gameStateUIRenderer->renderEasyModeExplanationUI(width, height);
            }
            
            // EASY/NORMAL選択UIの表示（ステージ選択フィールド用）
            if (gameState.showModeSelectionUI) {
                gameStateUIRenderer->renderEasyModeSelectionUI(width, height, gameState.isEasyMode);
            }
            
            // NORMAL/TIME ATTACK選択UIの表示（ステージ入場時用）
            if (gameState.showTimeAttackSelectionUI) {
                gameStateUIRenderer->renderTimeAttackSelectionUI(width, height, gameState.isTimeAttackMode, gameState.isGameCleared, gameState.selectedSecretStarType);
            }
            
            // SECRET STAR説明UIの表示
            if (gameState.showSecretStarExplanationUI) {
                gameStateUIRenderer->renderSecretStarExplanationUI(width, height);
            }
            
            // SECRET STAR選択UIの表示
            if (gameState.showSecretStarSelectionUI) {
                gameStateUIRenderer->renderSecretStarSelectionUI(width, height, gameState.selectedSecretStarType);
            }
            
            // 操作アシストUI（トータルスターのUIと同じタイミングで描画）
            // UNLOCK確認UIや星不足警告UIが表示されている時は非表示
            bool shouldShowAssist = gameState.showStageSelectionAssist && 
                                   !gameState.showUnlockConfirmUI && 
                                   !gameState.showStarInsufficientUI &&
                                   !gameState.showWarpTutorialUI &&
                                   !gameState.showEasyModeExplanationUI &&
                                   !gameState.showModeSelectionUI &&
                                   !gameState.showTimeAttackSelectionUI &&  // タイムアタック選択UI表示中は非表示
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
        auto& uiConfig = UIConfig::UIConfigManager::getInstance();
        auto controlsTextConfig = uiConfig.getControlsTextConfig();
        glm::vec2 controlsTextPos = uiConfig.calculatePosition(controlsTextConfig.position, width, height);
        uiRenderer->renderText(controlsText, controlsTextPos, controlsTextConfig.color, controlsTextConfig.scale);
        
        // エディタUIと選択ハイライトを描画
        if (editorState && editorState->isActive) {
            StageEditor::renderSelectionHighlight(*editorState, platformSystem, gameState);
            StageEditor::renderEditorUI(window, *editorState, gameState, uiRenderer);
        }
        
        // ステージ遷移アニメーションのオーバーレイを描画
        if (gameState.isTransitioning) {
            const float FADE_DURATION = 0.5f;
            float alpha = 0.0f;
            
            if (gameState.transitionType == GameState::TransitionType::FADE_OUT) {
                // フェードアウト：0.0 → 1.0
                alpha = std::min(1.0f, gameState.transitionTimer / FADE_DURATION);
            } else if (gameState.transitionType == GameState::TransitionType::FADE_IN) {
                // フェードイン：1.0 → 0.0
                alpha = 1.0f - std::min(1.0f, gameState.transitionTimer / FADE_DURATION);
            }
            
            // 黒いオーバーレイを描画
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, width, height, 0, -1, 1);
            
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.0f, 0.0f, 0.0f, alpha);
            
            glBegin(GL_QUADS);
            glVertex2f(0, 0);
            glVertex2f(width, 0);
            glVertex2f(width, height);
            glVertex2f(0, height);
            glEnd();
            
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
        }
        
        renderer->endFrame();
    }

    void handleInputProcessing(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                              PlatformSystem& platformSystem, 
                              std::map<int, InputUtils::KeyState>& keyStates,
                              std::function<void()> resetStageStartTime, float scaledDeltaTime, io::AudioManager& audioManager) {
        // リプレイモード中は入力処理をスキップ（一部のコントロールキーは除く）
        if (gameState.isReplayMode) {
            // リプレイコントロール
            // スペースキーで一時停止/再開
            if (keyStates[GLFW_KEY_SPACE].justPressed()) {
                gameState.isReplayPaused = !gameState.isReplayPaused;
                printf("REPLAY: %s\n", gameState.isReplayPaused ? "Paused" : "Resumed");
            }
            
            // Tキーでリプレイ速度切り替え（0.3x → 0.5x → 1x → 0.3x...）
            if (keyStates[GLFW_KEY_T].justPressed()) {
                if (gameState.replayPlaybackSpeed < 0.4f) {
                    gameState.replayPlaybackSpeed = 0.5f;
                } else if (gameState.replayPlaybackSpeed < 0.6f) {
                    gameState.replayPlaybackSpeed = 1.0f;
                } else {
                    gameState.replayPlaybackSpeed = 0.3f;
                }
                printf("REPLAY: Speed changed to %.1fx\n", gameState.replayPlaybackSpeed);
            }
            
            // ESCキーで終了
            if (keyStates[GLFW_KEY_ESCAPE].justPressed()) {
                gameState.isReplayMode = false;
                gameState.isReplayPaused = false;
                gameState.replayPlaybackTime = 0.0f;
                printf("REPLAY: Stopped\n");
            }
            return;  // リプレイモード中は他の入力処理をスキップ
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
        
        // チュートリアルステップ完了時のSE再生
        static bool lastTutorialStepCompleted = false;
        if (gameState.isTutorialStage && gameState.tutorialStepCompleted && !lastTutorialStepCompleted) {
            if (gameState.audioEnabled) {
                audioManager.playSFX("tutorial_ok");
            }
        }
        lastTutorialStepCompleted = gameState.tutorialStepCompleted;
        
        // ステージ切り替え処理（ワープUI表示中は数値キー無効）
        // チュートリアルステージではステージ切り替えを許可（ESCで終了できるようにするため）
        for (int key = GLFW_KEY_0; key <= GLFW_KEY_6; key++) {
            if (keyStates[key].justPressed()) {
                if (gameState.showWarpTutorialUI) { continue; }
                int stageNumber = key - GLFW_KEY_0;
                
                // チュートリアルステージ（ステージ6）では、0キーでステージ選択フィールドに戻れるようにする
                if (gameState.isTutorialStage && stageNumber == 0) {
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
                    
                    // SECRET STARモードをリセット
                    gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
                    
                    // UIフラグをリセット
                    gameState.showModeSelectionUI = false;
                    gameState.showTimeAttackSelectionUI = false;
                    gameState.showSecretStarSelectionUI = false;
                    gameState.showSecretStarExplanationUI = false;
                    continue;
                }
                
                // ステージ選択フィールドでの1-5キーはワープ機能に変更
                if (stageManager.getCurrentStage() == 0 && stageNumber >= 1 && stageNumber <= 5) {
                    if (gameState.unlockedStages[stageNumber]) {
                        // ステージ選択エリアの目の前にワープ
                        teleportToStageArea(stageNumber, gameState);
                    }
                } else if (!gameState.isTutorialStage) {
                    // 通常のステージ選択処理（チュートリアルステージ以外）
                    processStageSelectionAction(stageNumber, gameState, stageManager, platformSystem, resetStageStartTime, window);
                }
            }
        }
        
        // 速度制御処理（Tキー）- ステージ0以外で有効（モード選択UI表示中は無効、MAX_SPEED_STAR時は無効）
        // チュートリアル中でも速度変更は常に可能（ただし、チュートリアルステップ5以外ではtutorialInputEnabledのチェックが必要）
        // UI処理より前に実行することで、UI表示中でも速度変更が可能（ただし、UI処理でTキーが消費される場合は速度変更は実行されない）
        int currentStage = stageManager.getCurrentStage();
        bool canUseTKey = tutorialInputEnabled || (gameState.isTutorialStage);
        // MAX_SPEED_STARの場合は時間倍率を変更できない
        if (keyStates[GLFW_KEY_T].justPressed() && currentStage != 0 && canUseTKey && !gameState.showModeSelectionUI && !gameState.showTimeAttackSelectionUI && !gameState.showSecretStarSelectionUI && gameState.selectedSecretStarType != GameState::SecretStarType::MAX_SPEED_STAR) {
                gameState.timeScaleLevel = (gameState.timeScaleLevel + 1) % 3;
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
        if (keyStates[GLFW_KEY_E].justPressed() && stageManager.getCurrentStage() == 0 && !gameState.showModeSelectionUI && !gameState.showTimeAttackSelectionUI) {
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
                // Tキーはタイムアタックモード切り替えに使用するため、Double JumpスキルはYキーに変更
                {GLFW_KEY_Y, gameState.hasDoubleJumpSkill, gameState.doubleJumpRemainingUses, gameState.doubleJumpMaxUses, "Double Jump"},
                // YキーがDouble Jumpに使われたため、Heart FeelスキルはUキーに変更
                {GLFW_KEY_U, gameState.hasHeartFeelSkill, gameState.heartFeelRemainingUses, gameState.heartFeelMaxUses, "Heart Feel"},
                // UキーがHeart Feelに使われたため、Free CameraスキルはIキーに変更
                {GLFW_KEY_I, gameState.hasFreeCameraSkill, gameState.freeCameraRemainingUses, gameState.freeCameraMaxUses, "Free Camera"},
                // IキーがFree Cameraに使われたため、Burst JumpスキルはOキーに変更
                {GLFW_KEY_O, gameState.hasBurstJumpSkill, gameState.burstJumpRemainingUses, gameState.burstJumpMaxUses, "Burst Jump"}
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
        
        // EASY/NORMAL選択UI処理（ステージ選択フィールド用）
        if (gameState.showModeSelectionUI) {
            // TキーでNORMAL/EASYモード切り替え（blockEnterUntilReleasedの影響を受けない）
            if (keyStates[GLFW_KEY_T].justPressed()) {
                gameState.isEasyMode = !gameState.isEasyMode;
                printf("MODE SELECTION: %s\n", gameState.isEasyMode ? "EASY" : "NORMAL");
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
            
            // ENTERキーで確定してステージ選択フィールドに戻る（ステージに入らない）
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.showModeSelectionUI = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        // SECRET STAR説明UI処理
        if (gameState.showSecretStarExplanationUI) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.showSecretStarExplanationUI = false;
            }
        }
        
        // SECRET STAR選択UI処理（最優先）
        if (gameState.showSecretStarSelectionUI) {
            // SECRET STAR選択UI表示時に、selectedSecretStarTypeがNONEの場合はMAX_SPEED_STARに初期化
            if (gameState.selectedSecretStarType == GameState::SecretStarType::NONE) {
                gameState.selectedSecretStarType = GameState::SecretStarType::MAX_SPEED_STAR;
            }
            
            // TキーでSECRET STARタイプ切り替え（blockEnterUntilReleasedの影響を受けない）
            if (keyStates[GLFW_KEY_T].justPressed()) {
                if (gameState.selectedSecretStarType == GameState::SecretStarType::MAX_SPEED_STAR) {
                    gameState.selectedSecretStarType = GameState::SecretStarType::SHADOW_STAR;
                } else if (gameState.selectedSecretStarType == GameState::SecretStarType::SHADOW_STAR) {
                    gameState.selectedSecretStarType = GameState::SecretStarType::IMMERSIVE_STAR;
                } else if (gameState.selectedSecretStarType == GameState::SecretStarType::IMMERSIVE_STAR) {
                    gameState.selectedSecretStarType = GameState::SecretStarType::MAX_SPEED_STAR;
                } else {
                    // その他の場合はMAX_SPEED_STARに初期化
                    gameState.selectedSecretStarType = GameState::SecretStarType::MAX_SPEED_STAR;
                }
                printf("SECRET STAR SELECTION: %d (0=NONE, 1=MAX_SPEED, 2=SHADOW, 3=IMMERSIVE)\n", static_cast<int>(gameState.selectedSecretStarType));
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            // 直前のENTERを離すまで受け付けない
            if (gameState.blockEnterUntilReleased) {
                if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                    gameState.blockEnterUntilReleased = false; // 離されたのを確認
                    printf("SECRET STAR: ENTER released, blockEnterUntilReleased = false\n");
                }
                // 離されるまでここで処理終了（自動遷移を防止）
                // ただし、Tキーの処理は上で実行済みなので、returnしても問題ない
                return;
            }
            
            // ENTERキーで確定してカウントダウンを開始（ready画面をスキップ）
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                printf("SECRET STAR: ENTER pressed, selectedType = %d\n", static_cast<int>(gameState.selectedSecretStarType));
                int targetStage = gameState.modeSelectionTargetStage;
                gameState.showSecretStarSelectionUI = false;
                gameState.showTimeAttackSelectionUI = false;  // 念のためタイムアタック選択UIも無効化
                gameState.modeSelectionTargetStage = 0;
                gameState.blockEnterUntilReleased = false;  // ブロックを解除
                
                // 選択したSECRET STARモードでステージへ移動
                resetStageStartTime();
                gameState.lives = 6;
                stageManager.goToStage(targetStage, gameState, platformSystem);
                
                // SECRET STARモードの初期化
                gameState.isTimeAttackMode = false;  // SECRET STARモード時はタイムアタックモードを無効化
                
                // MAX_SPEED_STARの場合はready画面をスキップしてカウントダウンを開始
                if (gameState.selectedSecretStarType == GameState::SecretStarType::MAX_SPEED_STAR) {
                    gameState.readyScreenShown = true;  // ready画面をスキップしたことを記録
                    gameState.showReadyScreen = false;  // ready画面を表示しない
                    gameState.isCountdownActive = true;  // カウントダウンを開始
                    gameState.countdownTimer = 3.0f;  // カウントダウンタイマーを設定
                    gameState.readyScreenSpeedLevel = 0;
                    
                    // 時間倍率を3倍に固定
                    gameState.timeScale = 3.0f;
                    gameState.timeScaleLevel = 2;  // 3倍に設定
                } else {
                    // SHADOW_STARとIMMERSIVE_STARの場合は通常通りready画面を表示
                    gameState.showReadyScreen = true;
                    gameState.readyScreenShown = false;
                    gameState.isCountdownActive = false;
                    gameState.countdownTimer = 0.0f;
                    gameState.readyScreenSpeedLevel = 0;
                    
                    gameState.timeScale = 1.0f;
                    gameState.timeScaleLevel = 0;
                }
                
                // IMMERSIVE_STARの場合はFPSモードを有効化
                if (gameState.selectedSecretStarType == GameState::SecretStarType::IMMERSIVE_STAR) {
                    gameState.isFirstPersonMode = true;
                    gameState.isFirstPersonView = true;
                    gameState.cameraYaw = 90.0f;
                    gameState.cameraPitch = -10.0f;
                    gameState.firstMouse = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                } else {
                    gameState.isFirstPersonMode = false;
                    gameState.isFirstPersonView = false;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            return;  // SECRET STAR選択UI表示中は他のUI処理をスキップ
        }
        
        // NORMAL/TIME ATTACK選択UI処理（ステージ入場時用）
        if (gameState.showTimeAttackSelectionUI) {
            // TキーでNORMAL/TIME ATTACK/SECRET STARモード切り替え（blockEnterUntilReleasedの影響を受けない）
            if (keyStates[GLFW_KEY_T].justPressed()) {
                if (gameState.isGameCleared) {
                    // 3択: NORMAL -> TIME ATTACK -> SECRET STAR -> NORMAL
                    if (!gameState.isTimeAttackMode && gameState.selectedSecretStarType == GameState::SecretStarType::NONE) {
                        gameState.isTimeAttackMode = true;
                    } else if (gameState.isTimeAttackMode && gameState.selectedSecretStarType == GameState::SecretStarType::NONE) {
                        gameState.isTimeAttackMode = false;
                        gameState.selectedSecretStarType = GameState::SecretStarType::MAX_SPEED_STAR;
                    } else {
                        gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
                    }
                } else {
                    // 2択: NORMAL <-> TIME ATTACK
                    gameState.isTimeAttackMode = !gameState.isTimeAttackMode;
                    // NORMALモードに戻った場合は、selectedSecretStarTypeをリセット
                    if (!gameState.isTimeAttackMode) {
                        gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
                    }
                }
                printf("MODE SELECTION: %s\n", 
                    gameState.isTimeAttackMode ? "TIME ATTACK" : 
                    (gameState.selectedSecretStarType != GameState::SecretStarType::NONE ? "SECRET STAR" : "NORMAL"));
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
                // SECRET STARが選択されている場合は、SECRET STAR選択UIを表示
                if (gameState.selectedSecretStarType != GameState::SecretStarType::NONE) {
                    // modeSelectionTargetStageを保持したままSECRET STAR選択UIへ
                    gameState.showTimeAttackSelectionUI = false;
                    gameState.showSecretStarSelectionUI = true;
                    gameState.blockEnterUntilReleased = true;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    return;
                }
                
                // NORMALまたはTIME ATTACKが選択されている場合はready画面へ進む
                // NORMALモードの場合は、selectedSecretStarTypeを確実にNONEにリセット
                if (!gameState.isTimeAttackMode) {
                    gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
                }
                
                int targetStage = gameState.modeSelectionTargetStage;
                gameState.showTimeAttackSelectionUI = false;
                gameState.modeSelectionTargetStage = 0;
                gameState.blockEnterUntilReleased = false;  // ブロックを解除
                
                // 選択したモードでステージへ移動
                resetStageStartTime();
                gameState.lives = 6;
                stageManager.goToStage(targetStage, gameState, platformSystem);
                gameState.readyScreenShown = false;
                gameState.showReadyScreen = true;
                gameState.readyScreenSpeedLevel = 0;
                gameState.timeScale = 1.0f;
                gameState.timeScaleLevel = 0;
                
                // タイムアタックモードの初期化
                if (gameState.isTimeAttackMode) {
                    gameState.currentTimeAttackTime = 0.0f;
                    gameState.timeAttackStartTime = 0.0f;
                    gameState.isNewRecord = false;
                }
            }
        }
        
        // ステージクリアUI処理
        if (gameState.showStageClearUI) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                int clearedStage = stageManager.getCurrentStage();
                
                // SECRET STARのクリア状況は既にゴール到達時に記録済みなので、ここでは何もしない
                
                stageManager.completeStage(clearedStage);
                stageManager.goToStage(0, gameState, platformSystem);
                gameState.timeScale = 1.0f;
                gameState.timeScaleLevel = 0;
                
                // ステージ選択フィールドではTPSモードに戻す
                gameState.isFirstPersonMode = false;
                gameState.isFirstPersonView = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                
                // SECRET STARモードをリセット
                gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
                
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
                // リトライ処理（タイムアタックモードでも通常モードと同じ）
                resetStageStartTime();
                stageManager.loadStage(stageManager.getCurrentStage(), gameState, platformSystem);
                gameState.showStageClearUI = false;
                gameState.gameWon = false;
                gameState.isGoalReached = false;
                gameState.timeScale = 1.0f;
                gameState.timeScaleLevel = 0;
                gameState.lives = 6;
                // タイムアタックモードの初期化
                if (gameState.isTimeAttackMode) {
                    gameState.currentTimeAttackTime = 0.0f;
                    gameState.timeAttackStartTime = 0.0f;
                    gameState.isNewRecord = false;
                }
                // リトライ時は既にステージに入っているので、直接ready画面へ
                gameState.readyScreenShown = false;
                gameState.showReadyScreen = true;
                gameState.readyScreenSpeedLevel = 0;
            }
            
            if (keyStates[GLFW_KEY_SPACE].justPressed()) {
                // タイムアタックモードでリプレイが存在する場合はリプレイ再生
                if (gameState.isTimeAttackMode) {
                    int currentStage = stageManager.getCurrentStage();
                    if (ReplayManager::replayExists(currentStage)) {
                        // リプレイデータを読み込み
                        if (ReplayManager::loadReplay(gameState.currentReplay, currentStage)) {
                            // ステージを再読み込み（リプレイ再生用）
                            stageManager.goToStage(currentStage, gameState, platformSystem);
                            
                            // 現在のクリアタイムを保存（リプレイ完了時に復元するため）
                            savedClearTime = gameState.currentTimeAttackTime;
                            
                            // クリアタイムをリプレイデータから設定（更新を防ぐため）
                            gameState.clearTime = gameState.currentReplay.clearTime;
                            
                            gameState.isReplayMode = true;
                            gameState.isReplayPaused = false;
                            gameState.replayPlaybackTime = 0.0f;
                            gameState.replayPlaybackSpeed = 1.0f;  // 初期速度は1.0x
                            gameState.showStageClearUI = false;
                            gameState.isStageCompleted = false;
                            gameState.isGoalReached = false;
                            printf("REPLAY: Started playback for stage %d (Saved clear time: %.2fs)\n", currentStage, savedClearTime);
                            return;  // リプレイ再生開始時は他の処理をスキップ
                        }
                    }
                }
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
                
                // SECRET STARモードをリセット
                gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
                
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
                // リトライ時は既にステージに入っているので、直接ready画面へ
                gameState.readyScreenShown = false;
                gameState.showReadyScreen = true;
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
                int targetStage = gameState.warpTutorialStage;
                gameState.showWarpTutorialUI = false;
                gameState.warpTutorialStage = 0;
                gameState.blockEnterUntilReleased = false;  // ブロックを解除
                
                // ステージをクリアしたことがあるかチェック（星を1個以上獲得しているか）
                bool hasClearedStage = (gameState.stageStars.count(targetStage) > 0 && gameState.stageStars[targetStage] > 0);
                
                if (hasClearedStage) {
                    // クリア済み: タイムアタック選択UIを表示（ステージに入る前に）
                    gameState.showTimeAttackSelectionUI = true;
                    gameState.modeSelectionTargetStage = targetStage;
                    gameState.blockEnterUntilReleased = true;
                    // モード選択UI表示時に、selectedSecretStarTypeを初期化
                    gameState.selectedSecretStarType = GameState::SecretStarType::NONE;
                    gameState.isTimeAttackMode = false;
                } else {
                    // 初めて入る場合: 直接ready画面へ（タイムアタック選択UIは表示しない）
                resetStageStartTime();
                gameState.lives = 6;
                    stageManager.goToStage(targetStage, gameState, platformSystem);
                gameState.readyScreenShown = false;
                gameState.showReadyScreen = true;
                gameState.readyScreenSpeedLevel = 0;
                    gameState.timeScale = 1.0f;
                    gameState.timeScaleLevel = 0;
                    gameState.isTimeAttackMode = false;  // 初めて入る場合はNORMALモード
                }
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
                                   gameState.showTimeAttackSelectionUI ||
                                   gameState.showSecretStarExplanationUI ||
                                   gameState.showSecretStarSelectionUI ||
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
        
        // SHADOW_STARモードの場合は背景を黒にする
        if (gameState.selectedSecretStarType == GameState::SecretStarType::SHADOW_STAR && stageManager.getCurrentStage() != 0) {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        
        // カメラ設定
        auto cameraConfig = CameraSystem::calculateCameraConfig(gameState, stageManager, window);
        CameraSystem::applyCameraConfig(renderer.get(), cameraConfig, window);
        
        // ウィンドウサイズを取得
        auto [w, h] = CameraSystem::getWindowSize(window);
        width = w; height = h;
    }

    void renderPlatforms(PlatformSystem& platformSystem, 
                        std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                        GameState& gameState,
                        StageManager& stageManager) {
        auto positions = platformSystem.getPositions();
        auto sizes = platformSystem.getSizes();
        auto colors = platformSystem.getColors();
        auto visibility = platformSystem.getVisibility();
        auto isRotating = platformSystem.getIsRotating();
        auto rotationAngles = platformSystem.getRotationAngles();
        auto rotationAxes = platformSystem.getRotationAxes();
        auto blinkAlphas = platformSystem.getBlinkAlphas();
        auto platformTypes = platformSystem.getPlatformTypes();
        
        // SHADOW_STARモードの場合、プレイヤーが乗っている足場だけを表示（ステージ0では適用しない）
        std::vector<bool> shadowVisibility = visibility;
        if (gameState.selectedSecretStarType == GameState::SecretStarType::SHADOW_STAR && stageManager.getCurrentStage() != 0) {
            // すべての足場を一旦非表示にする
            for (size_t i = 0; i < shadowVisibility.size(); i++) {
                shadowVisibility[i] = false;
            }
            
            // プレイヤーが乗っている足場を判定
            glm::vec3 playerSize = glm::vec3(1.0f, 2.0f, 1.0f);
            auto collisionResult = platformSystem.checkCollisionWithIndex(gameState.playerPosition, playerSize);
            int currentPlatformIndex = collisionResult.second;
            
            // プレイヤーが乗っている足場を表示
            if (currentPlatformIndex >= 0 && currentPlatformIndex < static_cast<int>(shadowVisibility.size())) {
                shadowVisibility[currentPlatformIndex] = true;
            }
        }
        
        // static足場用のテクスチャを読み込み
        static GLuint staticPlatformTexture = 0;
        if (staticPlatformTexture == 0) {
            staticPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/static_platform.png"));
            if (staticPlatformTexture == 0) {
                // 作業ディレクトリがプロジェクトルートの場合に対応
                staticPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/static_platform.png");
            }
        }
        // ゴール（黄色）足場用のテクスチャを読み込み
        static GLuint goalPlatformTexture = 0;
        if (goalPlatformTexture == 0) {
            goalPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/goal_platform.png"));
            if (goalPlatformTexture == 0) {
                goalPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/goal_platform.png");
            }
        }
        // スタート（緑色）足場用のテクスチャを読み込み
        static GLuint startPlatformTexture = 0;
        if (startPlatformTexture == 0) {
            startPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/start_platform.png"));
            if (startPlatformTexture == 0) {
                startPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/start_platform.png");
            }
        }
        
        // moving足場用のテクスチャを読み込み
        static GLuint movingPlatformTexture = 0;
        if (movingPlatformTexture == 0) {
            movingPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/moving_platform.png"));
            if (movingPlatformTexture == 0) {
                movingPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/moving_platform.png");
            }
        }

        // サイクル消失足場用のテクスチャを読み込み
        static GLuint cyclingDisappearTexture = 0;
        if (cyclingDisappearTexture == 0) {
            cyclingDisappearTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/cyclingdisappearing_platform.png"));
            if (cyclingDisappearTexture == 0) {
                cyclingDisappearTexture = gfx::TextureManager::loadTexture("assets/textures/cyclingdisappearing_platform.png");
            }
        }

        // 飛来足場用のテクスチャを読み込み
        static GLuint flyingPlatformTexture = 0;
        if (flyingPlatformTexture == 0) {
            flyingPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/flying_platform.png"));
            if (flyingPlatformTexture == 0) {
                flyingPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/flying_platform.png");
            }
        }
        
        // ロック状態の足場用のテクスチャを読み込み
        static GLuint lockPlatformTexture = 0;
        if (lockPlatformTexture == 0) {
            lockPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/lock_platform.png"));
            if (lockPlatformTexture == 0) {
                lockPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/lock_platform.png");
            }
        }
        
        // アンロック状態の足場用のテクスチャを読み込み
        static GLuint unlockPlatformTexture = 0;
        if (unlockPlatformTexture == 0) {
            unlockPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/unlock_platform.png"));
            if (unlockPlatformTexture == 0) {
                unlockPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/unlock_platform.png");
            }
        }
        
        for (size_t i = 0; i < positions.size(); i++) {
            // SHADOW_STARモードの場合はshadowVisibilityを使用、それ以外は通常のvisibilityを使用
            bool shouldRender = (gameState.selectedSecretStarType == GameState::SecretStarType::SHADOW_STAR) 
                                ? shadowVisibility[i] 
                                : visibility[i];
            if (!shouldRender || sizes[i].x <= 0 || sizes[i].y <= 0 || sizes[i].z <= 0) continue;
            
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
        playerTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/player.png"));
        if (playerTexture == 0) {
            // 作業ディレクトリがプロジェクトルートの場合に対応
            playerTexture = gfx::TextureManager::loadTexture("assets/textures/player.png");
        }
    }
    
    if (playerFrontTexture == 0) {
        playerFrontTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/player_front.png"));
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
