#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_loop.h"
#include "game_updater.h"
#include "game_renderer.h"
#include "input_handler.h"
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
#include "../game/online_leaderboard_manager.h"
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


    void run(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
            PlatformSystem& platformSystem,
            std::unique_ptr<gfx::OpenGLRenderer>& renderer,
            std::unique_ptr<gfx::UIRenderer>& uiRenderer,
            std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
            std::map<int, InputUtils::KeyState>& keyStates,
            std::function<void()> resetStageStartTime,
            std::chrono::high_resolution_clock::time_point& startTime,
            io::AudioManager& audioManager) {
        
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
            
            deltaTime = std::min(deltaTime, GameConstants::MAX_DELTA_TIME);
            
            lastFrameTime = currentTime;
            
            // リプレイモード中はgameTimeを更新しない（リプレイのタイムスタンプを使用するため）
            if (!gameState.replay.isReplayMode) {
                gameState.progress.gameTime = std::chrono::duration<float>(currentTime - startTime).count();
            }
            
            float scaledDeltaTime = deltaTime * gameState.progress.timeScale;
            
            if (!gameState.ui.showTitleScreen) {
            int currentStage = stageManager.getCurrentStage();
            // フェードアウト中はBGMを再生しない
            bool shouldPlayStageBGM = gameState.audioEnabled && 
                                     !gameState.progress.isGoalReached && 
                                     !gameState.ui.showTitleScreen &&
                                     !gameState.ui.isTransitioning &&
                                     gameState.ui.transitionType != UIState::TransitionType::FADE_OUT;
            
            if (shouldPlayStageBGM) {
                std::string targetBGM = "";
                std::string bgmPath = "";
                
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
                        break;
                }
                
                if (!targetBGM.empty()) {
                    if (gameState.currentBGM != targetBGM) {
                        if (gameState.bgmPlaying) {
                            audioManager.stopBGM();
                            gameState.bgmPlaying = false;
                        }
                        
                        if (audioManager.loadBGM(bgmPath)) {
                            audioManager.playBGM();
                            gameState.currentBGM = targetBGM;
                            gameState.bgmPlaying = true;
                            std::cout << "BGM started: " << targetBGM << std::endl;
                        }
                    }
                } else if (gameState.bgmPlaying) {
                    audioManager.stopBGM();
                    gameState.bgmPlaying = false;
                    gameState.currentBGM = "";
                    std::cout << "BGM stopped" << std::endl;
                }
            }
            }

            if (gameState.ui.showReadyScreen) {
                handleReadyScreen(window, gameState, stageManager, platformSystem, renderer, gameStateUIRenderer, keyStates, resetStageStartTime, audioManager, deltaTime);
                glfwPollEvents();
                continue; // Ready画面表示中は他の処理をスキップ
            }
            
            if (gameState.ui.isCountdownActive) {
                handleCountdown(window, gameState, stageManager, platformSystem, renderer, gameStateUIRenderer, keyStates, resetStageStartTime, deltaTime);
                glfwPollEvents();
                continue; // カウントダウン中は他の処理をスキップ
            }

            if (gameState.ui.isEndingSequence) {
                handleEndingSequence(window, gameState, stageManager, platformSystem, renderer, gameStateUIRenderer, keyStates, deltaTime);
                glfwPollEvents();
                continue; // エンディングシーケンス中は他の処理をスキップ
            }

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                SaveManager::saveGameData(gameState);
                gameRunning = false;
            }

            if (gameState.ui.showTitleScreen) {
                if (gameState.ui.isTransitioning) {
                    const float FADE_DURATION = 0.5f;  // フェード時間（秒）
                    gameState.ui.transitionTimer += deltaTime;
                    
                    if (gameState.ui.transitionType == UIState::TransitionType::FADE_OUT) {
                        if (gameState.ui.transitionTimer >= FADE_DURATION) {
                            if (gameState.ui.pendingStageTransition >= 0) {
                                // 次のフレームでshowTitleScreen = falseにしてステージを読み込むようにフラグを設定
                                gameState.ui.pendingFadeIn = true;
                            }
                        }
                    }
                    // フェードアウトのみで遷移を完了する（フェードインは行わない）
                }
                
                // pendingFadeInがtrueの場合、showTitleScreen = falseにしてステージを読み込む
                bool justTransitioned = false;
                if (gameState.ui.pendingFadeIn && gameState.ui.isTransitioning) {
                    gameState.ui.showTitleScreen = false;
                    justTransitioned = true;
                    
                    int targetStage = gameState.ui.pendingStageTransition;
                    if (targetStage == 6) {
                        // チュートリアルステージ
                        stageManager.loadStage(6, gameState, platformSystem);
                        gameState.ui.showReadyScreen = false;
                        gameState.ui.readyScreenShown = true;
                        gameState.ui.isCountdownActive = false;
                        gameState.ui.countdownTimer = 0.0f;
                        resetStageStartTime();
                    } else if (targetStage == 0) {
                        // ステージ選択画面
                        resetStageStartTime();
                        stageManager.goToStage(0, gameState, platformSystem);
                        gameState.ui.showReadyScreen = false;
                        gameState.ui.readyScreenShown = false;
                        gameState.progress.timeScale = 1.0f;
                        gameState.progress.timeScaleLevel = 0;
                        
                        gameState.camera.isFirstPersonMode = false;
                        gameState.camera.isFirstPersonView = false;
                        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        
                        gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
                    }
                    // フェードアウトのみで遷移を完了（フェードインは行わない）
                    gameState.ui.pendingFadeIn = false;
                    gameState.ui.isTransitioning = false;
                    gameState.ui.transitionType = UIState::TransitionType::NONE;
                    gameState.ui.transitionTimer = 0.0f;
                    gameState.ui.pendingStageTransition = -1;
                    gameState.ui.pendingReadyScreen = false;
                    gameState.ui.pendingSkipCountdown = false;
                    gameState.ui.stageLoadedForFadeIn = false;
                }
                
                gameState.ui.titleScreenTimer += deltaTime;
                
                const float BACKGROUND_FADE_DURATION = 1.5f;  // 背景フェードイン時間
                const float LOGO_ANIMATION_DURATION = 1.0f;    // ロゴアニメーション時間
                
                if (gameState.ui.titleScreenPhase == UIState::TitleScreenPhase::BACKGROUND_FADE_IN) {
                    if (gameState.ui.titleScreenTimer >= BACKGROUND_FADE_DURATION) {
                        gameState.ui.titleScreenPhase = UIState::TitleScreenPhase::LOGO_ANIMATION;
                        gameState.ui.titleScreenTimer = 0.0f;  // タイマーリセット
                    }
                } else if (gameState.ui.titleScreenPhase == UIState::TitleScreenPhase::LOGO_ANIMATION) {
                    if (gameState.ui.titleScreenTimer >= LOGO_ANIMATION_DURATION) {
                        gameState.ui.titleScreenPhase = UIState::TitleScreenPhase::SHOW_TEXT;
                        gameState.ui.titleScreenTimer = 0.0f;  // タイマーリセット
                    }
                }
                
                // フェードアウト中やpendingFadeInがtrue、または遷移予定がある場合はBGMを再生しない
                // また、このフレームで遷移が完了した場合（justTransitioned）もBGMを再生しない
                bool shouldPlayBGM = gameState.audioEnabled && 
                                    !gameState.ui.isTransitioning && 
                                    !gameState.ui.pendingFadeIn &&
                                    !justTransitioned &&
                                    gameState.ui.transitionType != UIState::TransitionType::FADE_OUT &&
                                    gameState.ui.pendingStageTransition < 0;
                
                if (shouldPlayBGM) {
                    std::string titleBGM = "title.ogg";
                    if (gameState.currentBGM != titleBGM) {
                        if (gameState.bgmPlaying) {
                            audioManager.stopBGM();
                            gameState.bgmPlaying = false;
                        }
                        
                        std::string bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/title.ogg");
                        if (audioManager.loadBGM(bgmPath)) {
                            audioManager.playBGM();
                            gameState.currentBGM = titleBGM;
                            gameState.bgmPlaying = true;
                            std::cout << "Title BGM started: " << titleBGM << std::endl;
                        }
                    }
                } else {
                    // フェードアウト中など、BGMを再生すべきでない場合は停止する
                    if (gameState.bgmPlaying && gameState.currentBGM == "title.ogg") {
                        audioManager.stopBGM();
                        gameState.bgmPlaying = false;
                        gameState.currentBGM = "";
                    }
                }
                
                for (auto& [key, state] : keyStates) {
                    state.update(glfwGetKey(window, key) == GLFW_PRESS);
                }
                
                // 名前入力画面の処理
                if (gameState.ui.showPlayerNameInput) {
                    // Backspaceキーで文字削除
                    if (keyStates[GLFW_KEY_BACKSPACE].justPressed()) {
                        if (!gameState.ui.playerNameInput.empty() && gameState.ui.playerNameInputCursorPos > 0) {
                            gameState.ui.playerNameInput.erase(gameState.ui.playerNameInputCursorPos - 1, 1);
                            gameState.ui.playerNameInputCursorPos--;
                        }
                    }
                    
                    // ローマ字（A-Z, a-z）の入力（常に大文字で保存・表示）
                    for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
                        if (keyStates[key].justPressed()) {
                            if (gameState.ui.playerNameInput.length() < 12) {
                                // 常に大文字で保存
                                char c = 'A' + (key - GLFW_KEY_A);
                                gameState.ui.playerNameInput.insert(gameState.ui.playerNameInputCursorPos, 1, c);
                                gameState.ui.playerNameInputCursorPos++;
                            }
                        }
                    }
                    
                    // Enterキーで確定
                    if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                        // 名前が空の場合は"Player"を設定
                        std::string finalName = gameState.ui.playerNameInput.empty() ? "Player" : gameState.ui.playerNameInput;
                        printf("ONLINE: Setting player name to: [%s] (length: %zu)\n", finalName.c_str(), finalName.length());
                        OnlineLeaderboardManager::setPlayerName(finalName);
                        
                        // プレイヤー名をセーブデータに保存
                        SaveManager::saveGameData(gameState);
                        printf("SAVE: Saved player name to save file: [%s]\n", finalName.c_str());
                        
                        gameState.ui.showPlayerNameInput = false;
                        
                        // チュートリアルに遷移
                        gameState.ui.isTransitioning = true;
                        gameState.ui.transitionType = UIState::TransitionType::FADE_OUT;
                        gameState.ui.transitionTimer = 0.0f;
                        gameState.ui.pendingStageTransition = 6;  // チュートリアルステージ
                        gameState.ui.pendingReadyScreen = false;  // Ready画面をスキップ
                        gameState.ui.pendingSkipCountdown = true;  // カウントダウンをスキップ
                    }
                } else if (keyStates[GLFW_KEY_ENTER].justPressed() && 
                    gameState.ui.titleScreenPhase == UIState::TitleScreenPhase::SHOW_TEXT &&
                    !gameState.ui.isTransitioning) {
                    if (gameState.bgmPlaying && gameState.currentBGM == "title.ogg") {
                        audioManager.stopBGM();
                        gameState.bgmPlaying = false;
                        gameState.currentBGM = "";
                    }
                    
                    // 名前入力画面を表示（初回のみ、または名前が未設定の場合）
                    bool hasClearedTutorial = (gameState.progress.stageStars.count(6) > 0 && gameState.progress.stageStars[6] > 0) ||
                                             (gameState.progress.unlockedStages.count(1) > 0 && gameState.progress.unlockedStages[1]) ||
                                             !gameState.ui.showStage0Tutorial;
                    
                    std::string currentPlayerName = OnlineLeaderboardManager::getPlayerName();
                    bool isPlayerNameSet = !currentPlayerName.empty() && currentPlayerName != "Player";
                    
                    // 初回（チュートリアル未クリア）または名前が未設定の場合のみ名前入力画面を表示
                    if (!hasClearedTutorial || !isPlayerNameSet) {
                        gameState.ui.showPlayerNameInput = true;
                        // 既に名前が設定されている場合はそれを表示
                        if (isPlayerNameSet) {
                            gameState.ui.playerNameInput = currentPlayerName;
                            gameState.ui.playerNameInputCursorPos = currentPlayerName.length();
                        } else {
                            gameState.ui.playerNameInput = "";
                            gameState.ui.playerNameInputCursorPos = 0;
                        }
                    } else {
                        // 既にクリア済みで名前も設定済みの場合は直接ステージ選択画面へ
                        gameState.ui.isTransitioning = true;
                        gameState.ui.transitionType = UIState::TransitionType::FADE_OUT;
                        gameState.ui.transitionTimer = 0.0f;
                        gameState.ui.pendingStageTransition = 0;  // ステージ選択画面
                        gameState.ui.pendingReadyScreen = false;  // Ready画面をスキップ
                        gameState.ui.pendingSkipCountdown = true;  // カウントダウンをスキップ
                    }
                }
                
                GameLoop::GameRenderer::renderFrame(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer, keyStates, deltaTime);
                
                glfwPollEvents();
                // showTitleScreen = falseになった場合は、次のフレームで新しいステージを描画するためcontinueしない
                if (gameState.ui.showTitleScreen) {
                    continue;  // タイトル画面中は他の処理をスキップ
                }
            }

            updateGameState(window, gameState, stageManager, platformSystem, deltaTime, scaledDeltaTime, keyStates, resetStageStartTime, audioManager);

            GameLoop::GameRenderer::renderFrame(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer, keyStates, deltaTime);
            
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
        static float readyScreenFileCheckTimer = 0.0f;
        readyScreenFileCheckTimer += deltaTime;
        if (readyScreenFileCheckTimer >= 0.5f) {
            readyScreenFileCheckTimer = 0.0f;
            UIConfig::UIConfigManager::getInstance().checkAndReloadConfig();
        }
        
        // Immersiveモードの場合は、Ready画面でもFPS視点を維持
        if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::IMMERSIVE_STAR) {
            if (!gameState.camera.isFirstPersonView || !gameState.camera.isFirstPersonMode) {
                gameState.camera.isFirstPersonMode = true;
                gameState.camera.isFirstPersonView = true;
                gameState.camera.yaw = 90.0f;
                gameState.camera.pitch = -10.0f;
                gameState.camera.firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
        
        int width, height;
        GameLoop::GameRenderer::prepareFrame(window, gameState, stageManager, renderer, width, height, deltaTime);
        
        GameLoop::GameRenderer::renderPlatforms(platformSystem, renderer, gameState, stageManager);
        
        GameLoop::GameRenderer::renderPlayer(gameState, renderer);
        
        gameStateUIRenderer->renderReadyScreen(width, height, gameState.ui.readyScreenSpeedLevel, gameState.camera.isFirstPersonMode);
        
        renderer->endFrame();
        
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
        
        if (keyStates[GLFW_KEY_T].justPressed()) {
            gameState.ui.readyScreenSpeedLevel = (gameState.ui.readyScreenSpeedLevel + 1) % 3;
        }
        
        
        if (keyStates[GLFW_KEY_ENTER].justPressed()) {
            gameState.ui.showReadyScreen = false;
            gameState.ui.isCountdownActive = true;
            gameState.ui.countdownTimer = 3.0f;
            
            if (gameState.audioEnabled) {
                audioManager.playSFX("countdown");
            }
            
            gameState.camera.isFirstPersonView = gameState.camera.isFirstPersonMode;
            
            if (gameState.camera.isFirstPersonMode) {
                gameState.camera.yaw = 90.0f;
                gameState.camera.pitch = -10.0f;
                gameState.camera.firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            
            switch (gameState.ui.readyScreenSpeedLevel) {
                case 0:
                    gameState.progress.timeScale = 1.0f;
                    gameState.progress.timeScaleLevel = 0;
                    break;
                case 1:
                    gameState.progress.timeScale = 2.0f;
                    gameState.progress.timeScaleLevel = 1;
                    break;
                case 2:
                    gameState.progress.timeScale = 3.0f;
                    gameState.progress.timeScaleLevel = 2;
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
        
        if (!gameState.progress.timeLimitApplied) {
            float baseTimeLimit = gameState.progress.timeLimit;
            
            if (gameState.camera.isFirstPersonMode) {
                baseTimeLimit += GameConstants::FIRST_PERSON_TIME_BONUS;
                printf("COUNTDOWN: FPS MODE +%.1fs applied\n", GameConstants::FIRST_PERSON_TIME_BONUS);
            }
            if (gameState.progress.isEasyMode) {
                baseTimeLimit += GameConstants::EASY_MODE_TIME_BONUS;
                printf("COUNTDOWN: EASY MODE +%.1fs applied\n", GameConstants::EASY_MODE_TIME_BONUS);
            }
            
            gameState.progress.timeLimit = baseTimeLimit;
            gameState.progress.remainingTime = baseTimeLimit;
            gameState.progress.timeLimitApplied = true;
            printf("COUNTDOWN: Final time limit set to %.1f seconds\n", baseTimeLimit);
        }
        
        int width, height;
        GameLoop::GameRenderer::prepareFrame(window, gameState, stageManager, renderer, width, height, deltaTime);
        
        GameLoop::GameRenderer::renderPlatforms(platformSystem, renderer, gameState, stageManager);
        
        GameLoop::GameRenderer::renderPlayer(gameState, renderer);
        
        int count = (int)gameState.ui.countdownTimer + 1;
        if (count > 0) {
            gameStateUIRenderer->renderCountdown(width, height, count);
        }
        
        renderer->endFrame();
        
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
        
        gameState.ui.countdownTimer -= deltaTime;
        
        if (gameState.ui.countdownTimer <= 0.0f) {
            gameState.ui.isCountdownActive = false;
            resetStageStartTime();
            
            if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::MAX_SPEED_STAR) {
                gameState.progress.timeScale = 3.0f;
                gameState.progress.timeScaleLevel = 2;  // 3倍に設定
            }
            
            if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::IMMERSIVE_STAR) {
                gameState.camera.isFirstPersonView = true;
                gameState.camera.yaw = 90.0f;
                gameState.camera.pitch = -10.0f;
                gameState.camera.firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            
            if (gameState.progress.isTimeAttackMode) {
                gameState.progress.timeAttackStartTime = gameState.progress.gameTime;
                gameState.progress.currentTimeAttackTime = 0.0f;
                
                gameState.replay.isRecordingReplay = true;
                gameState.replay.replayBuffer.clear();
                gameState.replay.replayRecordTimer = 0.0f;
                
                ReplayFrame firstFrame;
                firstFrame.timestamp = 0.0f;
                firstFrame.playerPosition = gameState.player.position;
                firstFrame.playerVelocity = gameState.player.velocity;
                firstFrame.itemCollectedStates.clear();
                for (const auto& item : gameState.items.items) {
                    firstFrame.itemCollectedStates.push_back(item.isCollected);
                }
                gameState.replay.replayBuffer.push_back(firstFrame);
                
                printf("TIME ATTACK: Started at %.2f\n", gameState.progress.timeAttackStartTime);
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
        int width, height;
        GameLoop::GameRenderer::prepareFrame(window, gameState, stageManager, renderer, width, height, deltaTime);
        
        if (gameState.ui.showStaffRoll) {
            gameState.ui.staffRollTimer += deltaTime;
            
            if (gameState.ui.staffRollTimer >= 14.0f) {
                gameState.ui.showStaffRoll = false;
                gameState.ui.showEndingMessage = true;
                gameState.ui.endingMessageTimer = 0.0f;
            } else {
                gameStateUIRenderer->renderStaffRoll(width, height, gameState.ui.staffRollTimer);
            }
        }
        
        if (gameState.ui.showEndingMessage) {
            gameState.ui.endingMessageTimer += deltaTime;
            
            if (gameState.ui.endingMessageTimer >= 5.0f) {
                gameState.ui.isEndingSequence = false;
                gameState.ui.showStaffRoll = false;
                gameState.ui.showEndingMessage = false;
                gameState.ui.staffRollTimer = 0.0f;
                gameState.ui.endingMessageTimer = 0.0f;
                
                gameState.progress.isGameCleared = true;
                if (!gameState.progress.hasShownSecretStarExplanationUI) {
                    gameState.ui.showSecretStarExplanationUI = true;
                    gameState.progress.hasShownSecretStarExplanationUI = true;
                }
                
                GameLoop::InputHandler::returnToField(window, gameState, stageManager, platformSystem, -1);
            } else {
                gameStateUIRenderer->renderEndingMessage(width, height, gameState.ui.endingMessageTimer);
            }
        }
        
        if (keyStates[GLFW_KEY_ENTER].justPressed()) {
            gameState.ui.isEndingSequence = false;
            gameState.ui.showStaffRoll = false;
            gameState.ui.showEndingMessage = false;
            gameState.ui.staffRollTimer = 0.0f;
            gameState.ui.endingMessageTimer = 0.0f;
            
            gameState.progress.isGameCleared = true;
            if (!gameState.progress.hasShownSecretStarExplanationUI) {
                gameState.ui.showSecretStarExplanationUI = true;
                gameState.progress.hasShownSecretStarExplanationUI = true;
            }
            
            GameLoop::InputHandler::returnToField(window, gameState, stageManager, platformSystem, -1);
        }
        
        renderer->endFrame();
        
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
    }

    float savedClearTime = 0.0f;

    void updateGameState(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                        PlatformSystem& platformSystem, float deltaTime, float scaledDeltaTime,
                        std::map<int, InputUtils::KeyState>& keyStates,
                        std::function<void()> resetStageStartTime, io::AudioManager& audioManager) {
        GameLoop::GameUpdater::updateGameState(window, gameState, stageManager, platformSystem, deltaTime, scaledDeltaTime, keyStates, resetStageStartTime, audioManager);
    }


    void updatePhysicsAndCollisions(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                                   PlatformSystem& platformSystem, float deltaTime, float scaledDeltaTime, io::AudioManager& audioManager) {
        GameLoop::GameUpdater::updatePhysicsAndCollisions(window, gameState, stageManager, platformSystem, deltaTime, scaledDeltaTime, audioManager);
    }

    void updateItems(GameState& gameState, float scaledDeltaTime, io::AudioManager& audioManager) {
        GameLoop::GameUpdater::updateItems(gameState, scaledDeltaTime, audioManager);
    }


    void handleStageSelectionArea(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                                 PlatformSystem& platformSystem, std::function<void()> resetStageStartTime,
                                 std::map<int, InputUtils::KeyState>& keyStates, float deltaTime) {
        if (stageManager.getCurrentStage() == 0) {
            int selectedStage = -1;
            for (int stage = 0; stage < 5; stage++) {
                const auto& stageArea = GameConstants::STAGE_AREAS[stage];
                if (gameState.player.position.x > stageArea.x - GameConstants::STAGE_SELECTION_RANGE && 
                    gameState.player.position.x < stageArea.x + GameConstants::STAGE_SELECTION_RANGE && 
                    gameState.player.position.z > stageArea.z - GameConstants::STAGE_SELECTION_RANGE && 
                    gameState.player.position.z < stageArea.z + GameConstants::STAGE_SELECTION_RANGE
                ) {
                    selectedStage = stage + 1;
                    break;
                }
            }
            
            if (selectedStage > 0) {
                gameState.ui.showStageSelectionAssist = true;
                gameState.ui.assistTargetStage = selectedStage;
            } else {
                gameState.ui.showStageSelectionAssist = false;
                gameState.ui.assistTargetStage = 0;
            }
            
            if (!gameState.ui.showWarpTutorialUI && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                if (selectedStage > 0) {
                    InputHandler::processStageSelectionAction(selectedStage, gameState, stageManager, platformSystem, resetStageStartTime, window);
                }
            }
            
            // ランキングボードの検出（JSONから読み込んだ位置を使用）
            const glm::vec3& leaderboardPos = gameState.ui.leaderboardPosition;
            bool isNearLeaderboard = 
                gameState.player.position.x > leaderboardPos.x - GameConstants::LEADERBOARD_SELECTION_RANGE &&
                gameState.player.position.x < leaderboardPos.x + GameConstants::LEADERBOARD_SELECTION_RANGE &&
                gameState.player.position.z > leaderboardPos.z - GameConstants::LEADERBOARD_SELECTION_RANGE &&
                gameState.player.position.z < leaderboardPos.z + GameConstants::LEADERBOARD_SELECTION_RANGE;
            
            if (isNearLeaderboard) {
                gameState.ui.showLeaderboardAssist = true;
            } else {
                gameState.ui.showLeaderboardAssist = false;
            }
            
            // リーダーボードUI表示中の処理
            if (gameState.ui.showLeaderboardUI) {
                // ローディング中でエラーが発生した場合の自動リトライ
                if (gameState.ui.isLoadingLeaderboard && gameState.ui.leaderboardEntries.empty()) {
                    const float RETRY_INTERVAL = 2.0f;  // 2秒ごとにリトライ
                    const float TIMEOUT_SECONDS = 10.0f;  // 10秒でタイムアウト
                    
                    gameState.ui.leaderboardRetryTimer += deltaTime;
                    
                    // 10秒経過したらタイムアウト
                    if (gameState.ui.leaderboardRetryTimer >= TIMEOUT_SECONDS) {
                        printf("ONLINE: Timeout after %.1f seconds, showing NO RECORDS\n", TIMEOUT_SECONDS);
                        gameState.ui.isLoadingLeaderboard = false;
                        gameState.ui.leaderboardRetryTimer = 0.0f;
                        gameState.ui.leaderboardRetryCount = 0;
                    } else if (gameState.ui.leaderboardRetryTimer >= RETRY_INTERVAL) {
                        // 2秒ごとにリトライ（最大5回まで）
                        const int MAX_RETRY_COUNT = 5;
                        if (gameState.ui.leaderboardRetryCount < MAX_RETRY_COUNT) {
                            gameState.ui.leaderboardRetryTimer = 0.0f;
                            gameState.ui.leaderboardRetryCount++;
                            
                            printf("ONLINE: Retrying to fetch leaderboard for stage %d (attempt %d/%d)\n", 
                                   gameState.ui.leaderboardTargetStage, 
                                   gameState.ui.leaderboardRetryCount, 
                                   MAX_RETRY_COUNT);
                            
                            // リトライ
                            OnlineLeaderboardManager::fetchLeaderboard(
                                gameState.ui.leaderboardTargetStage, 
                                [&gameState](const std::vector<LeaderboardEntry>& entries) {
                                    if (!entries.empty()) {
                                        printf("ONLINE: Loaded leaderboard (%zu entries)\n", entries.size());
                                        gameState.ui.leaderboardEntries = entries;
                                        gameState.ui.isLoadingLeaderboard = false;
                                        gameState.ui.leaderboardRetryCount = 0;
                                        gameState.ui.leaderboardRetryTimer = 0.0f;
                                    } else {
                                        // まだ失敗している場合は、isLoadingLeaderboardをtrueのままにしてリトライを続ける
                                        printf("ONLINE: Failed to load leaderboard, will retry...\n");
                                    }
                                }
                            );
                        }
                    }
                }
                
                // A/Dキーでステージ切り替え
                if (keyStates[GLFW_KEY_A].justPressed() || keyStates[GLFW_KEY_LEFT].justPressed()) {
                    int currentStage = gameState.ui.leaderboardTargetStage;
                    int newStage = currentStage - 1;
                    if (newStage < 1) {
                        newStage = 5;  // 1-5の範囲でループ
                    }
                    gameState.ui.leaderboardTargetStage = newStage;
                    gameState.ui.isLoadingLeaderboard = true;
                    gameState.ui.leaderboardEntries.clear();
                    gameState.ui.leaderboardRetryCount = 0;
                    gameState.ui.leaderboardRetryTimer = 0.0f;
                    
                    // 新しいステージのランキングを取得
                    OnlineLeaderboardManager::fetchLeaderboard(newStage, [&gameState, newStage](const std::vector<LeaderboardEntry>& entries) {
                        if (!entries.empty()) {
                            printf("ONLINE: Loaded leaderboard for stage %d (%zu entries)\n", newStage, entries.size());
                            gameState.ui.leaderboardEntries = entries;
                            gameState.ui.isLoadingLeaderboard = false;
                            gameState.ui.leaderboardRetryCount = 0;
                            gameState.ui.leaderboardRetryTimer = 0.0f;
                        } else {
                            // エラー時はLOADINGを続ける（リトライロジックが処理する）
                            printf("ONLINE: Failed to load leaderboard for stage %d, will retry...\n", newStage);
                        }
                    });
                }
                
                if (keyStates[GLFW_KEY_D].justPressed() || keyStates[GLFW_KEY_RIGHT].justPressed()) {
                    int currentStage = gameState.ui.leaderboardTargetStage;
                    int newStage = currentStage + 1;
                    if (newStage > 5) {
                        newStage = 1;  // 1-5の範囲でループ
                    }
                    gameState.ui.leaderboardTargetStage = newStage;
                    gameState.ui.isLoadingLeaderboard = true;
                    gameState.ui.leaderboardEntries.clear();
                    gameState.ui.leaderboardRetryCount = 0;
                    gameState.ui.leaderboardRetryTimer = 0.0f;
                    
                    // 新しいステージのランキングを取得
                    OnlineLeaderboardManager::fetchLeaderboard(newStage, [&gameState, newStage](const std::vector<LeaderboardEntry>& entries) {
                        if (!entries.empty()) {
                            printf("ONLINE: Loaded leaderboard for stage %d (%zu entries)\n", newStage, entries.size());
                            gameState.ui.leaderboardEntries = entries;
                            gameState.ui.isLoadingLeaderboard = false;
                            gameState.ui.leaderboardRetryCount = 0;
                            gameState.ui.leaderboardRetryTimer = 0.0f;
                        } else {
                            // エラー時はLOADINGを続ける（リトライロジックが処理する）
                            printf("ONLINE: Failed to load leaderboard for stage %d, will retry...\n", newStage);
                        }
                    });
                }
                
                // W/Sキーでエントリ選択
                if (keyStates[GLFW_KEY_W].justPressed() || keyStates[GLFW_KEY_UP].justPressed()) {
                    if (!gameState.ui.leaderboardEntries.empty()) {
                        gameState.ui.leaderboardSelectedIndex--;
                        if (gameState.ui.leaderboardSelectedIndex < 0) {
                            gameState.ui.leaderboardSelectedIndex = static_cast<int>(gameState.ui.leaderboardEntries.size()) - 1;
                        }
                    }
                }
                
                if (keyStates[GLFW_KEY_S].justPressed() || keyStates[GLFW_KEY_DOWN].justPressed()) {
                    if (!gameState.ui.leaderboardEntries.empty()) {
                        gameState.ui.leaderboardSelectedIndex++;
                        if (gameState.ui.leaderboardSelectedIndex >= static_cast<int>(gameState.ui.leaderboardEntries.size())) {
                            gameState.ui.leaderboardSelectedIndex = 0;
                        }
                    }
                }
                
                // Spaceキーでリプレイを視聴
                if (keyStates[GLFW_KEY_SPACE].justPressed()) {
                    printf("ONLINE: Space key pressed in leaderboard UI\n");
                    if (gameState.ui.leaderboardEntries.empty()) {
                        printf("ONLINE: No entries available\n");
                    } else {
                        int selectedIndex = gameState.ui.leaderboardSelectedIndex;
                        printf("ONLINE: Selected index: %d, Total entries: %zu\n", 
                               selectedIndex, gameState.ui.leaderboardEntries.size());
                        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(gameState.ui.leaderboardEntries.size())) {
                            const auto& selectedEntry = gameState.ui.leaderboardEntries[selectedIndex];
                            printf("ONLINE: Selected entry - ID: %d, hasReplay: %s, playerName: %s\n", 
                                   selectedEntry.id, selectedEntry.hasReplay ? "true" : "false", selectedEntry.playerName.c_str());
                            if (selectedEntry.hasReplay && selectedEntry.id > 0) {
                                printf("ONLINE: Loading replay for entry ID %d\n", selectedEntry.id);
                                gameState.ui.isLoadingReplay = true;
                                
                                // リプレイを取得（コールバック内でステージを読み込む）
                                OnlineLeaderboardManager::fetchReplay(selectedEntry.id, [&gameState](const ReplayData* replayData) {
                                    if (replayData != nullptr) {
                                        printf("ONLINE: Replay loaded successfully (%zu frames, stage %d)\n", 
                                               replayData->frames.size(), replayData->stageNumber);
                                        
                                        // リプレイデータをコピーして保存
                                        gameState.replay.currentReplay = *replayData;
                                        gameState.replay.pendingReplayStage = replayData->stageNumber;
                                        gameState.replay.pendingReplayLoad = true;
                                        gameState.replay.isOnlineReplay = true;  // オンラインリプレイフラグを設定
                                        
                                        printf("ONLINE: Replay data prepared - pendingReplayLoad: %s, pendingReplayStage: %d\n",
                                               gameState.replay.pendingReplayLoad ? "true" : "false",
                                               gameState.replay.pendingReplayStage);
                                        
                                        // ランキングUIを閉じる
                                        gameState.ui.showLeaderboardUI = false;
                                        gameState.ui.leaderboardEntries.clear();
                                        gameState.ui.leaderboardTargetStage = 0;
                                        gameState.ui.leaderboardSelectedIndex = 0;
                                        
                                        printf("ONLINE: Replay data prepared for stage %d (Clear time: %.2fs)\n", 
                                               replayData->stageNumber, replayData->clearTime);
                                    } else {
                                        printf("ONLINE: Failed to load replay - replayData is nullptr\n");
                                    }
                                    gameState.ui.isLoadingReplay = false;
                                });
                            } else {
                                // リプレイがない場合のメッセージ
                                if (!selectedEntry.hasReplay) {
                                    printf("ONLINE: No replay available for selected entry (rank %d, player: %s, id: %d)\n",
                                           selectedIndex + 1, selectedEntry.playerName.c_str(), selectedEntry.id);
                                    printf("ONLINE: This record was submitted before replay feature was implemented, or replay data was not saved.\n");
                                } else {
                                    printf("ONLINE: Replay data exists but ID is invalid (id: %d)\n", selectedEntry.id);
                                }
                            }
                        } else {
                            printf("ONLINE: Invalid selected index: %d\n", selectedIndex);
                        }
                    }
                }
                
                // Enterキーで閉じる
                if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                    gameState.ui.showLeaderboardUI = false;
                    gameState.ui.leaderboardEntries.clear();
                    gameState.ui.leaderboardTargetStage = 0;
                    gameState.ui.leaderboardSelectedIndex = 0;
                    gameState.ui.isLoadingLeaderboard = false;
                    gameState.ui.leaderboardRetryCount = 0;
                    gameState.ui.leaderboardRetryTimer = 0.0f;
                }
            } else if (!gameState.ui.showWarpTutorialUI && 
                !gameState.ui.showUnlockConfirmUI &&
                !gameState.ui.showStarInsufficientUI &&
                !gameState.ui.showStageSelectionAssist &&
                keyStates[GLFW_KEY_ENTER].justPressed()) {
                // ランキングボードでEnterキーを押したときの処理
                if (isNearLeaderboard && !gameState.ui.showLeaderboardUI && !gameState.ui.isLoadingLeaderboard) {
                    // ステージ選択UIを表示して、どのステージのランキングを見るか選択させる
                    // まずは現在選択中のステージ（または最後にクリアしたステージ）のランキングを表示
                    int targetStage = gameState.ui.assistTargetStage > 0 ? gameState.ui.assistTargetStage : 1;
                    gameState.ui.leaderboardTargetStage = targetStage;
                    gameState.ui.isLoadingLeaderboard = true;
                    gameState.ui.showLeaderboardUI = true;
                    gameState.ui.leaderboardEntries.clear();
                    gameState.ui.leaderboardSelectedIndex = 0;
                    gameState.ui.leaderboardRetryCount = 0;
                    gameState.ui.leaderboardRetryTimer = 0.0f;
                    
                    // ランキングを取得
                    OnlineLeaderboardManager::fetchLeaderboard(targetStage, [&gameState, targetStage](const std::vector<LeaderboardEntry>& entries) {
                        if (!entries.empty()) {
                            printf("ONLINE: Loaded leaderboard for stage %d (%zu entries)\n", targetStage, entries.size());
                            for (size_t i = 0; i < entries.size(); i++) {
                                printf("ONLINE: Entry %zu - playerName: [%s] (length: %zu), time: %.2f\n", 
                                       i + 1, entries[i].playerName.c_str(), entries[i].playerName.length(), entries[i].time);
                            }
                            gameState.ui.leaderboardEntries = entries;
                            gameState.ui.isLoadingLeaderboard = false;
                            gameState.ui.leaderboardRetryCount = 0;
                            gameState.ui.leaderboardRetryTimer = 0.0f;
                        } else {
                            // エラー時はLOADINGを続ける（リトライロジックが処理する）
                            printf("ONLINE: Failed to load leaderboard for stage %d, will retry...\n", targetStage);
                        }
                    });
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
        GameLoop::GameRenderer::renderFrame(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer, keyStates, deltaTime);
    }

    void prepareFrame(GLFWwindow* window, GameState& gameState, StageManager& stageManager,
                     std::unique_ptr<gfx::OpenGLRenderer>& renderer, int& width, int& height, float deltaTime) {
        GameLoop::GameRenderer::prepareFrame(window, gameState, stageManager, renderer, width, height, deltaTime);
    }

    void renderPlatforms(PlatformSystem& platformSystem, 
                        std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                        GameState& gameState,
                        StageManager& stageManager) {
        GameLoop::GameRenderer::renderPlatforms(platformSystem, renderer, gameState, stageManager);
    }

    void renderPlayer(GameState& gameState, std::unique_ptr<gfx::OpenGLRenderer>& renderer) {
        GameLoop::GameRenderer::renderPlayer(gameState, renderer);
    }

    void _old_renderFrame_removed(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                    PlatformSystem& platformSystem,
                    std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                    std::unique_ptr<gfx::UIRenderer>& uiRenderer,
                    std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                    std::map<int, InputUtils::KeyState>& keyStates,
                    float deltaTime) {
        EditorState* editorState = gameState.editorState;
        if (!editorState) {
            static EditorState defaultEditorState;
            editorState = &defaultEditorState;
        }
        
        static float gameUIFileCheckTimer = 0.0f;
        gameUIFileCheckTimer += deltaTime;
        if (gameUIFileCheckTimer >= 0.5f) {
            gameUIFileCheckTimer = 0.0f;
            UIConfig::UIConfigManager::getInstance().checkAndReloadConfig();
        }
        
        int width, height;
        
        if (gameState.ui.showTitleScreen) {
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // 黒でクリア
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            gameStateUIRenderer->renderTitleScreen(width, height, gameState);
            
            if (gameState.ui.isTransitioning) {
                const float FADE_DURATION = 0.5f;
                float alpha = 0.0f;
                
                if (gameState.ui.transitionType == UIState::TransitionType::FADE_OUT) {
                    alpha = std::min(1.0f, gameState.ui.transitionTimer / FADE_DURATION);
                } else if (gameState.ui.transitionType == UIState::TransitionType::FADE_IN) {
                    alpha = 1.0f - std::min(1.0f, gameState.ui.transitionTimer / FADE_DURATION);
                }
                
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
            glfwPollEvents();  // イベントを処理してウィンドウを更新
            
            return;  // タイトル画面中は他の処理をスキップ
        }
        
        prepareFrame(window, gameState, stageManager, renderer, width, height, deltaTime);
        
        if (editorState->isActive) {
            CameraConfig cameraConfig = CameraSystem::calculateCameraConfig(gameState, stageManager, window, deltaTime);
            StageEditor::renderGrid(cameraConfig.position, editorState->gridSize, 20);
            
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
        
        uiRenderer->setWindowSize(width, height);
        
        renderPlatforms(platformSystem, renderer, gameState, stageManager);
        
        for (const auto& zone : gameState.gravityZones) {
            if (zone.isActive) {
                renderer->renderer3D.renderBoxWithAlpha(zone.position, GameConstants::Colors::GRAVITY_ZONE_COLOR, 
                                           zone.size, GameConstants::Colors::GRAVITY_ZONE_ALPHA);
            }
        }
        
        for (const auto& switch_obj : gameState.switches) {
            glm::vec3 switchColor = switch_obj.color;
            if (switch_obj.isPressed) {
                switchColor *= 0.7f;
            }
            renderer->renderer3D.renderCube(switch_obj.position, switchColor, switch_obj.size.x);
        }
        
        for (const auto& cannon : gameState.cannons) {
            if (cannon.isActive) {
                glm::vec3 color = cannon.color;
                if (cannon.cooldownTimer > 0.0f) {
                    color *= 0.5f;
                }
                renderer->renderer3D.renderCube(cannon.position, color, cannon.size.x);
            }
        }
        
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
        
        for (const auto& item : gameState.items.items) {
            if (!item.isCollected) {
                glm::vec3 itemPos = item.position + glm::vec3(0, item.bobHeight, 0);
                
                bool isRedItem = (item.color.r > 0.9f && item.color.g < 0.1f && item.color.b < 0.1f);
                bool isGreenItem = (item.color.r < 0.1f && item.color.g > 0.9f && item.color.b < 0.1f);
                bool isBlueItem = (item.color.r < 0.1f && item.color.g < 0.1f && item.color.b > 0.9f);
                
                if (isRedItem && itemFirstTexture != 0) {
                    renderer->renderer3D.renderTexturedRotatedBox(itemPos, item.size, itemFirstTexture, glm::vec3(0, 1, 0), item.rotationAngle);
                } else if (isGreenItem && itemSecondTexture != 0) {
                    renderer->renderer3D.renderTexturedRotatedBox(itemPos, item.size, itemSecondTexture, glm::vec3(0, 1, 0), item.rotationAngle);
                } else if (isBlueItem && itemThirdTexture != 0) {
                    renderer->renderer3D.renderTexturedRotatedBox(itemPos, item.size, itemThirdTexture, glm::vec3(0, 1, 0), item.rotationAngle);
                } else {
                    renderer->renderer3D.renderRotatedBox(itemPos, item.color, item.size, glm::vec3(0, 1, 0), item.rotationAngle);
                }
            }
        }
        
        if (!gameState.camera.isFirstPersonView) {
            renderPlayer(gameState, renderer);
        }
        
        if (gameState.progress.isTutorialStage && gameState.ui.showTutorialUI) {
            gameStateUIRenderer->renderTutorialStageUI(width, height, gameState.ui.tutorialMessage, gameState.progress.tutorialStep, gameState.progress.tutorialStepCompleted);
        }
    
        if (!gameState.replay.isReplayMode) {
        const StageData* currentStageData = stageManager.getStageData(stageManager.getCurrentStage());
        if (currentStageData && stageManager.getCurrentStage()!=0) {
            auto& uiConfig = UIConfig::UIConfigManager::getInstance();
            auto stageInfoConfig = uiConfig.getStageInfoConfig();
            glm::vec2 stageInfoPos = uiConfig.calculatePosition(stageInfoConfig.position, width, height);
            
            if (stageManager.getCurrentStage() == 6) {
                uiRenderer->renderText("TUTORIAL", 
                                   stageInfoPos, stageInfoConfig.color, stageInfoConfig.scale);
            } else {
                uiRenderer->renderText("STAGE " + std::to_string(stageManager.getCurrentStage()), 
                                   stageInfoPos, stageInfoConfig.color, stageInfoConfig.scale);
                }
            }
        }
        
        
        if(stageManager.getCurrentStage()!=0){
            bool shouldShowSpeedUI = true;
            if (stageManager.getCurrentStage() == 6 && gameState.progress.tutorialStep >= 6 && gameState.progress.tutorialStep < 9) {
                shouldShowSpeedUI = false; // ステップ6-8は速度倍率UIを非表示
            }
            if (gameState.progress.isTimeAttackMode) {
                shouldShowSpeedUI = false; // タイムアタックモードでは非表示（renderTimeAttackUI内で表示されるため）
            }
            
            if (shouldShowSpeedUI) {
                auto& uiConfig = UIConfig::UIConfigManager::getInstance();
                auto speedConfig = uiConfig.getSpeedDisplayConfig();
                auto pressTConfig = uiConfig.getPressTConfig();
                
                std::string speedText = std::to_string((int)gameState.progress.timeScale) + "x";
                glm::vec3 speedColor = (gameState.progress.timeScale > 1.0f) ? glm::vec3(1.0f, 0.8f, 0.2f) : speedConfig.color;
                glm::vec2 speedPos = uiConfig.calculatePosition(speedConfig.position, width, height);
                uiRenderer->renderText(speedText, speedPos, speedColor, speedConfig.scale);
                
                if (!gameState.replay.isReplayMode) {
                    std::string speedText2 = "PRESS T";
                    glm::vec3 speedColor2 = (gameState.progress.timeScale > 1.0f) ? glm::vec3(1.0f, 0.8f, 0.2f) : pressTConfig.color;
                    glm::vec2 pressTPos = uiConfig.calculatePosition(pressTConfig.position, width, height);
                    uiRenderer->renderText(speedText2, pressTPos, speedColor2, pressTConfig.scale);
                }
            }
        }
        
        if (stageManager.getCurrentStage() != 0) {
            bool shouldShowUI = true;
            if (stageManager.getCurrentStage() == 6) {
                shouldShowUI = (gameState.progress.tutorialStep > 5);
            }
            
            if (shouldShowUI) {
                if (stageManager.getCurrentStage() == 6 && gameState.progress.tutorialStep == 6) {
                    uiRenderer->renderLivesWithExplanation(gameState.progress.lives);
                } else if (stageManager.getCurrentStage() == 6 && gameState.progress.tutorialStep == 7) {
                    int currentStageStars = gameState.progress.stageStars[stageManager.getCurrentStage()];
                    uiRenderer->renderLivesAndTimeUI(gameState.progress.lives, gameState.progress.remainingTime, gameState.progress.timeLimit, gameState.progress.earnedStars, currentStageStars);
                } else if (stageManager.getCurrentStage() == 6 && gameState.progress.tutorialStep == 8) {
                    int currentStageStars = gameState.progress.stageStars[stageManager.getCurrentStage()];
                    int currentStage = stageManager.getCurrentStage();
                    int secretStarType = (gameState.progress.selectedSecretStarType != GameProgressState::SecretStarType::NONE) ? 
                                         static_cast<int>(gameState.progress.selectedSecretStarType) - 1 : -1;  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                    std::map<int, std::set<int>> secretStarClearedInt;
                    for (const auto& [stage, types] : gameState.progress.secretStarCleared) {
                        std::set<int> typesInt;
                        for (auto type : types) {
                            typesInt.insert(static_cast<int>(type) - 1);  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                        }
                        secretStarClearedInt[stage] = typesInt;
                    }
                    uiRenderer->renderLivesTimeAndStarsUI(gameState.progress.lives, gameState.progress.remainingTime, gameState.progress.timeLimit, gameState.progress.earnedStars, currentStageStars,
                                                          currentStage, secretStarType, secretStarClearedInt);
                } else if (stageManager.getCurrentStage() == 6 && gameState.progress.tutorialStep >= 9) {
                    int currentStageStars = gameState.progress.stageStars[stageManager.getCurrentStage()];
                    int currentStage = stageManager.getCurrentStage();
                    int secretStarType = (gameState.progress.selectedSecretStarType != GameProgressState::SecretStarType::NONE) ? 
                                         static_cast<int>(gameState.progress.selectedSecretStarType) - 1 : -1;  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                    std::map<int, std::set<int>> secretStarClearedInt;
                    for (const auto& [stage, types] : gameState.progress.secretStarCleared) {
                        std::set<int> typesInt;
                        for (auto type : types) {
                            typesInt.insert(static_cast<int>(type) - 1);  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                        }
                        secretStarClearedInt[stage] = typesInt;
                    }
                    uiRenderer->renderTimeUI(gameState.progress.remainingTime, gameState.progress.timeLimit, gameState.progress.earnedStars, currentStageStars, gameState.progress.lives,
                                              currentStage, secretStarType, secretStarClearedInt);
                } else {
                    if (!gameState.ui.isEndingSequence) {
                        int currentStageStars = gameState.progress.stageStars[stageManager.getCurrentStage()];
                        
                        if (gameState.progress.isTimeAttackMode) {
                            float bestTime = 0.0f;
                            if (gameState.progress.timeAttackRecords.find(stageManager.getCurrentStage()) != gameState.progress.timeAttackRecords.end()) {
                                bestTime = gameState.progress.timeAttackRecords[stageManager.getCurrentStage()];
                            }
                            
                            float displayTime = gameState.progress.currentTimeAttackTime;
                            if (gameState.replay.isReplayMode) {
                                displayTime = gameState.replay.replayPlaybackTime;
                            }
                            
                            if (gameState.replay.isReplayMode) {
                                uiRenderer->renderTimeAttackUI(displayTime, bestTime, 0, 0, 0, gameState.progress.timeScale, true);
                            } else {
                                uiRenderer->renderTimeAttackUI(gameState.progress.currentTimeAttackTime, bestTime, gameState.progress.earnedStars, currentStageStars, gameState.progress.lives, gameState.progress.timeScale, false);
                            }
                        } else {
                            if (!gameState.replay.isReplayMode) {
                                int currentStage = stageManager.getCurrentStage();
                                int secretStarType = (gameState.progress.selectedSecretStarType != GameProgressState::SecretStarType::NONE) ? 
                                                     static_cast<int>(gameState.progress.selectedSecretStarType) - 1 : -1;  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                                std::map<int, std::set<int>> secretStarClearedInt;
                                for (const auto& [stage, types] : gameState.progress.secretStarCleared) {
                                    std::set<int> typesInt;
                                    for (auto type : types) {
                                        typesInt.insert(static_cast<int>(type) - 1);  // NONE=-1, MAX_SPEED_STAR=0, SHADOW_STAR=1, IMMERSIVE_STAR=2
                                    }
                                    secretStarClearedInt[stage] = typesInt;
                                }
                                uiRenderer->renderTimeUI(gameState.progress.remainingTime, gameState.progress.timeLimit, gameState.progress.earnedStars, currentStageStars, gameState.progress.lives,
                                                          currentStage, secretStarType, secretStarClearedInt);
                            }
                        }
                        
                        if (!gameState.replay.isReplayMode && gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::NONE && !gameState.progress.isTimeAttackMode) {
                        uiRenderer->renderFreeCameraUI(gameState.skills.hasFreeCameraSkill, gameState.skills.isFreeCameraActive, gameState.skills.freeCameraTimer, 
                                                    gameState.skills.freeCameraRemainingUses, gameState.skills.freeCameraMaxUses);
                        
                        uiRenderer->renderBurstJumpUI(gameState.skills.hasBurstJumpSkill, gameState.skills.isBurstJumpActive, 
                                                   gameState.skills.burstJumpRemainingUses, gameState.skills.burstJumpMaxUses);
                        
                        uiRenderer->renderHeartFeelUI(gameState.skills.hasHeartFeelSkill, gameState.skills.heartFeelRemainingUses, 
                                                   gameState.skills.heartFeelMaxUses, gameState.progress.lives);
                        
                        uiRenderer->renderDoubleJumpUI(gameState.skills.hasDoubleJumpSkill, gameState.progress.isEasyMode, 
                                                    gameState.skills.doubleJumpRemainingUses, gameState.skills.doubleJumpMaxUses);
                        
                        uiRenderer->renderTimeStopUI(gameState.skills.hasTimeStopSkill, gameState.skills.isTimeStopped, gameState.skills.timeStopTimer, 
                                                  gameState.skills.timeStopRemainingUses, gameState.skills.timeStopMaxUses);
                        }
                        
                        if (stageManager.getCurrentStage() != 0 && !gameState.ui.isEndingSequence) {
                            static gfx::MinimapRenderer minimapRenderer;
                            minimapRenderer.render(gameState, platformSystem, width, height, stageManager.getCurrentStage(), 30.0f);
                        }
                    }
                }
            }
        }
        
        if (gameState.ui.showStageClearUI && !gameState.ui.isEndingSequence && !gameState.replay.isReplayMode) {
            if (gameState.progress.isTimeAttackMode) {
                float clearTime = gameState.progress.currentTimeAttackTime;  // タイムアタックモードではcurrentTimeAttackTimeを使用
                float bestTime = 0.0f;
                if (gameState.progress.timeAttackRecords.find(stageManager.getCurrentStage()) != gameState.progress.timeAttackRecords.end()) {
                    bestTime = gameState.progress.timeAttackRecords[stageManager.getCurrentStage()];
                }
                gameStateUIRenderer->renderTimeAttackClearBackground(width, height, clearTime, bestTime, gameState.progress.isNewRecord, gameState.replay.isOnlineReplay);
            } else {
                int currentStage = stageManager.getCurrentStage();
                gameStateUIRenderer->renderStageClearBackground(width, height, gameState.progress.clearTime, gameState.progress.stageStars[currentStage], false,
                                                                 currentStage, gameState.progress.selectedSecretStarType, gameState.progress.secretStarCleared, gameState.replay.isOnlineReplay);
            }
        }
        
        if (gameState.replay.isReplayMode) {
            // デバッグ: リプレイモード中のUI描画が呼ばれていることを確認
            static int debugRenderCount = 0;
            debugRenderCount++;
            if (debugRenderCount % 60 == 0) {  // 60フレームごとにログ出力（約1秒）
                printf("REPLAY UI: Rendering replay UI - paused: %s, speed: %.1fx, playbackTime: %.2f\n",
                       gameState.replay.isReplayPaused ? "true" : "false",
                       gameState.replay.replayPlaybackSpeed,
                       gameState.replay.replayPlaybackTime);
            }
            
            uiRenderer->begin2DMode();
            
            auto& uiConfig = UIConfig::UIConfigManager::getInstance();
            
            bool isRewinding = false;
            bool isFastForwarding = false;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                isRewinding = true;
            } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                isFastForwarding = true;
            }
            
            auto rewindMarkConfig = uiConfig.getReplayRewindMarkAlwaysConfig();
            glm::vec2 rewindMarkPos = uiConfig.calculatePosition(rewindMarkConfig.position, width, height);
            glm::vec3 rewindMarkColor = isRewinding ? rewindMarkConfig.activeColor : rewindMarkConfig.color;
            uiRenderer->renderText("<<", rewindMarkPos, rewindMarkColor, rewindMarkConfig.scale);
            
            auto pressAConfig = uiConfig.getReplayPressAConfig();
            glm::vec2 pressAPos = uiConfig.calculatePosition(pressAConfig.position, width, height);
            uiRenderer->renderText("PRESS A", pressAPos, pressAConfig.color, pressAConfig.scale);
            
            auto pauseMarkConfig = uiConfig.getReplayPauseMarkAlwaysConfig();
            glm::vec2 pauseMarkPos = uiConfig.calculatePosition(pauseMarkConfig.position, width, height);
            glm::vec3 pauseMarkColor = gameState.replay.isReplayPaused ? pauseMarkConfig.activeColor : pauseMarkConfig.color;
            uiRenderer->renderText("||", pauseMarkPos, pauseMarkColor, pauseMarkConfig.scale);
            
            auto pressSpaceConfig = uiConfig.getReplayPressSpaceConfig();
            glm::vec2 pressSpacePos = uiConfig.calculatePosition(pressSpaceConfig.position, width, height);
            uiRenderer->renderText("PRESS SPACE", pressSpacePos, pressSpaceConfig.color, pressSpaceConfig.scale);
            
            auto fastForwardMarkConfig = uiConfig.getReplayFastForwardMarkAlwaysConfig();
            glm::vec2 fastForwardMarkPos = uiConfig.calculatePosition(fastForwardMarkConfig.position, width, height);
            glm::vec3 fastForwardMarkColor = isFastForwarding ? fastForwardMarkConfig.activeColor : fastForwardMarkConfig.color;
            uiRenderer->renderText(">>", fastForwardMarkPos, fastForwardMarkColor, fastForwardMarkConfig.scale);
            
            auto pressDConfig = uiConfig.getReplayPressDConfig();
            glm::vec2 pressDPos = uiConfig.calculatePosition(pressDConfig.position, width, height);
            uiRenderer->renderText("PRESS D", pressDPos, pressDConfig.color, pressDConfig.scale);
            
            auto speedLabelConfig = uiConfig.getReplaySpeedLabelConfig();
            glm::vec2 speedLabelPos = uiConfig.calculatePosition(speedLabelConfig.position, width, height);
            uiRenderer->renderText("REPLAY SPEED", speedLabelPos, speedLabelConfig.color, speedLabelConfig.scale);
            
            std::string speedText;
            if (gameState.replay.replayPlaybackSpeed == 0.3f) {
                speedText = "0.3x";
            } else if (gameState.replay.replayPlaybackSpeed == 0.5f) {
                speedText = "0.5x";
            } else {
                speedText = "1.0x";
            }
            auto speedConfig = uiConfig.getReplaySpeedDisplayConfig();
            glm::vec2 speedPos = uiConfig.calculatePosition(speedConfig.position, width, height);
            uiRenderer->renderText(speedText, speedPos, speedConfig.color, speedConfig.scale);
            
            auto pressTConfig = uiConfig.getReplayPressTConfig();
            glm::vec2 pressTPos = uiConfig.calculatePosition(pressTConfig.position, width, height);
            uiRenderer->renderText("PRESS T", pressTPos, pressTConfig.color, pressTConfig.scale);
            
            auto instructionsConfig = uiConfig.getReplayInstructionsConfig();
            glm::vec2 instructionsPos = uiConfig.calculatePosition(instructionsConfig.position, width, height);
            if (instructionsConfig.position.useRelative && instructionsConfig.position.offsetY < 0) {
                instructionsPos.y = height + instructionsConfig.position.offsetY;
            }
            uiRenderer->renderText("SPACE: Pause/Resume  A/D: Rewind/FastForward  T: Speed  ESC: Exit", 
                                  instructionsPos, instructionsConfig.color, instructionsConfig.scale);
            
            uiRenderer->end2DMode();
        }
        
        if (stageManager.getCurrentStage() == 0) {
            
            for (int stage = 0; stage < 5; stage++) {
                const auto& area = GameConstants::STAGE_AREAS[stage];
                int stageNumber = stage + 1;
                
                bool isUnlocked = (stageNumber == 0) || gameState.progress.unlockedStages[stageNumber];
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
                    for (int i = 0; i < 3; i++) {
                        glm::vec3 starColor = (i < gameState.progress.stageStars[stageNumber]) ? 
                            glm::vec3(1.0f, 1.0f, 0.0f) :  // 黄色（獲得済み）
                            glm::vec3(0.5f, 0.5f, 0.5f);   // 灰色（未獲得）
                        
                        glm::vec3 starPos = glm::vec3(
                            area.x - 0.8f + i * 0.8f,  // 左から右に並べる
                            area.y + area.height + 1.5f,  // エリアの上に配置
                            area.z
                        );
                        
                        renderer->renderer3D.renderStar3D(starPos, starColor, 1.0f);
                    }
                    
                    bool hasClearedStage = (gameState.progress.isGameCleared && gameState.progress.stageStars.count(stageNumber) > 0 && gameState.progress.stageStars[stageNumber] > 0);
                    if (hasClearedStage) {
                        std::vector<GameProgressState::SecretStarType> secretStarTypes = {
                            GameProgressState::SecretStarType::MAX_SPEED_STAR,
                            GameProgressState::SecretStarType::SHADOW_STAR,
                            GameProgressState::SecretStarType::IMMERSIVE_STAR
                        };
                        
                        std::vector<glm::vec3> secretStarColors = {
                            glm::vec3(0.2f, 0.8f, 1.0f),  // MAX_SPEED_STAR: 水色
                            glm::vec3(0.1f, 0.1f, 0.1f),  // SHADOW_STAR: 黒
                            glm::vec3(1.0f, 0.4f, 0.8f)   // IMMERSIVE_STAR: ピンク
                        };
                        
                        glm::vec3 inactiveColor = glm::vec3(0.5f, 0.5f, 0.5f); // 灰色（未獲得）
                        
                        std::set<GameProgressState::SecretStarType> clearedTypes;
                        if (gameState.progress.secretStarCleared.count(stageNumber) > 0) {
                            clearedTypes = gameState.progress.secretStarCleared.at(stageNumber);
                        }
                        
                        for (int i = 0; i < 3; i++) {
                            bool isCleared = (clearedTypes.count(secretStarTypes[i]) > 0);
                            glm::vec3 starColor = isCleared ? secretStarColors[i] : inactiveColor;
                            
                            glm::vec3 starPos = glm::vec3(
                                area.x - 0.8f + (2 - i) * 0.8f,  // 順序を逆にして左から右に：MAX_SPEED_STAR, SHADOW_STAR, IMMERSIVE_STAR
                                area.y + area.height + 1.5f - 0.7f,  // 通常の星の下に配置
                                area.z
                            );
                            
                            renderer->renderer3D.renderStar3D(starPos, starColor, 1.0f);
                        }
                    }
                } else {
                    glm::vec3 numberPos = glm::vec3(
                        area.x - 0.6f,  // エリアの中心より左
                        area.y + area.height + 1.5f,  // エリアの上に配置
                        area.z
                    );
                    renderer->renderer3D.renderNumber3D(numberPos, requiredStars, glm::vec3(1.0f, 1.0f, 0.0f), 1.0f);
                    
                    glm::vec3 xMarkPos = glm::vec3(
                        area.x,  // エリアの中心
                        area.y + area.height + 1.5f,  // エリアの上に配置
                        area.z
                    );
                    renderer->renderer3D.renderXMark3D(xMarkPos, glm::vec3(1.0f, 1.0f, 0.0f), 0.5f);
                    
                    glm::vec3 starPos = glm::vec3(
                        area.x + 0.6f,  // エリアの中心より右
                        area.y + area.height + 1.5f,  // エリアの上に配置
                        area.z
                    );
                    renderer->renderer3D.renderStar3D(starPos, glm::vec3(1.0f, 1.0f, 0.0f), 1.0f);
                }
            }
        }
        
        if (gameState.ui.showUnlockConfirmUI) {
            gameStateUIRenderer->renderUnlockConfirmBackground(width, height, gameState.ui.unlockTargetStage, gameState.ui.unlockRequiredStars, gameState.progress.totalStars);
        }
        
        if (gameState.ui.showStarInsufficientUI) {
            gameStateUIRenderer->renderStarInsufficientBackground(width, height, gameState.ui.insufficientTargetStage, gameState.ui.insufficientRequiredStars, gameState.progress.totalStars);
        }
        
        if (gameState.ui.showWarpTutorialUI) {
            gameStateUIRenderer->renderWarpTutorialBackground(width, height, gameState.ui.warpTutorialStage);
        }
        
        if (gameState.progress.isGameOver) {
            gameStateUIRenderer->renderGameOverBackground(width, height);
        }
        
        if (stageManager.getCurrentStage() == 0) {
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, width, height, 0, -1, 1);
            
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            
            glDisable(GL_DEPTH_TEST);
            
            auto& uiConfig = UIConfig::UIConfigManager::getInstance();
            
            auto worldTitleConfig = uiConfig.getWorldTitleConfig();
            glm::vec2 worldTitlePos = uiConfig.calculatePosition(worldTitleConfig.position, width, height);
            uiRenderer->renderText("WORLD 1", worldTitlePos, worldTitleConfig.color, worldTitleConfig.scale);
           
            auto starIconConfig = uiConfig.getStarIconConfig();
            glm::vec2 starIconPos = uiConfig.calculatePosition(starIconConfig.position, width, height);
            uiRenderer->renderStar(starIconPos, starIconConfig.color, starIconConfig.scale);
            auto starCountConfig = uiConfig.getStarCountConfig();
            glm::vec2 starCountPos = uiConfig.calculatePosition(starCountConfig.position, width, height);
            uiRenderer->renderText("x " + std::to_string(gameState.progress.totalStars), starCountPos, starCountConfig.color, starCountConfig.scale);
            
            if (gameState.progress.isEasyMode) {
                auto easyModeTextConfig = uiConfig.getEasyModeTextConfig();
                glm::vec2 easyModeTextPosition = uiConfig.calculatePosition(easyModeTextConfig.position, width, height);
                uiRenderer->renderText("EASY", easyModeTextPosition, easyModeTextConfig.color, easyModeTextConfig.scale);
            } else {
                auto normalModeTextConfig = uiConfig.getNormalModeTextConfig();
                glm::vec2 normalModeTextPosition = uiConfig.calculatePosition(normalModeTextConfig.position, width, height);
                uiRenderer->renderText("NORMAL", normalModeTextPosition, normalModeTextConfig.color, normalModeTextConfig.scale);
            }
            
            auto pressEConfig = uiConfig.getPressEConfig();
            glm::vec2 pressEPos = uiConfig.calculatePosition(pressEConfig.position, width, height);
            uiRenderer->renderText("PRESS E", pressEPos, pressEConfig.color, pressEConfig.scale);
            
            if (gameState.ui.showStage0Tutorial) {
                gameStateUIRenderer->renderStage0Tutorial(width, height);
            }
            
            if (gameState.ui.showEasyModeExplanationUI) {
                gameStateUIRenderer->renderEasyModeExplanationUI(width, height);
            }
            
            if (gameState.ui.showModeSelectionUI) {
                gameStateUIRenderer->renderEasyModeSelectionUI(width, height, gameState.progress.isEasyMode);
            }
            
            if (gameState.ui.showTimeAttackSelectionUI) {
                gameStateUIRenderer->renderTimeAttackSelectionUI(width, height, gameState.progress.isTimeAttackMode, gameState.progress.isGameCleared, gameState.progress.selectedSecretStarType);
            }
            
            if (gameState.ui.showSecretStarExplanationUI) {
                gameStateUIRenderer->renderSecretStarExplanationUI(width, height);
            }
            
            if (gameState.ui.showSecretStarSelectionUI) {
                gameStateUIRenderer->renderSecretStarSelectionUI(width, height, gameState.progress.selectedSecretStarType);
            }
            
            bool shouldShowAssist = gameState.ui.showStageSelectionAssist && 
                                   !gameState.ui.showUnlockConfirmUI && 
                                   !gameState.ui.showStarInsufficientUI &&
                                   !gameState.ui.showWarpTutorialUI &&
                                   !gameState.ui.showEasyModeExplanationUI &&
                                   !gameState.ui.showModeSelectionUI &&
                                   !gameState.ui.showTimeAttackSelectionUI &&  // タイムアタック選択UI表示中は非表示
                                   !gameState.ui.showStage0Tutorial; // チュートリアル表示中は非表示
            bool isStageUnlocked = gameState.progress.unlockedStages[gameState.ui.assistTargetStage];
            gameStateUIRenderer->renderStageSelectionAssist(width, height, gameState.ui.assistTargetStage, shouldShowAssist, isStageUnlocked);
            
            // ステージ選択画面でESCキー情報を表示
            if (stageManager.getCurrentStage() == 0 && 
                !gameState.ui.showUnlockConfirmUI && 
                !gameState.ui.showStarInsufficientUI &&
                !gameState.ui.showWarpTutorialUI &&
                !gameState.ui.showEasyModeExplanationUI &&
                !gameState.ui.showModeSelectionUI &&
                !gameState.ui.showTimeAttackSelectionUI &&
                !gameState.ui.showSecretStarSelectionUI &&
                !gameState.ui.showStage0Tutorial) {
                gameStateUIRenderer->renderStageSelectionEscKeyInfo(width, height);
            }
            
            glEnable(GL_DEPTH_TEST);
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
        }
        
        std::string controlsText = "Controls: WASD=Move, SPACE=Jump, 0-5=Stage Select, LEFT/RIGHT=Next/Prev Stage, T=Speed Control, Q=Time Stop, H=Heart Feel, C=Free Camera, B=Burst Jump";
        if (stageManager.getCurrentStage() == 0) {
            controlsText = "Controls: WASD=Move, SPACE=Select Stage, 1-5=Teleport to Stage Area, F=Camera Toggle, E=Easy Mode, R=Toggle Time Stop Skill, T=Toggle Double Jump Skill, Y=Toggle Heart Feel Skill, U=Toggle Free Camera Skill, I=Toggle Burst Jump Skill";
        }
        auto& uiConfig = UIConfig::UIConfigManager::getInstance();
        auto controlsTextConfig = uiConfig.getControlsTextConfig();
        glm::vec2 controlsTextPos = uiConfig.calculatePosition(controlsTextConfig.position, width, height);
        uiRenderer->renderText(controlsText, controlsTextPos, controlsTextConfig.color, controlsTextConfig.scale);
        
        if (editorState && editorState->isActive) {
            StageEditor::renderSelectionHighlight(*editorState, platformSystem, gameState);
            StageEditor::renderEditorUI(window, *editorState, gameState, uiRenderer);
        }
        
        if (gameState.ui.isTransitioning) {
            const float FADE_DURATION = 0.5f;
            float alpha = 0.0f;
            
            if (gameState.ui.transitionType == UIState::TransitionType::FADE_OUT) {
                alpha = std::min(1.0f, gameState.ui.transitionTimer / FADE_DURATION);
            } else if (gameState.ui.transitionType == UIState::TransitionType::FADE_IN) {
                alpha = 1.0f - std::min(1.0f, gameState.ui.transitionTimer / FADE_DURATION);
            }
            
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
    }
} // namespace GameLoop
