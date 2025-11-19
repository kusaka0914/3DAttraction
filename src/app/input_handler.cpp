#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "input_handler.h"
#include "../core/constants/game_constants.h"
#include "../io/input_system.h"
#include "../game/replay_manager.h"
#include "../game/save_manager.h"
#include "tutorial_manager.h"
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>

namespace GameLoop {

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

static void teleportToStageArea(int stageNumber, GameState& gameState) {
    const auto& area = GameConstants::STAGE_AREAS[stageNumber - 1];
    
    glm::vec3 teleportPosition = glm::vec3(
        area.x,  // エリアの前に2ユニット
        area.y + 1.0f,  // エリアと同じ高さ
        area.z - 2.0f
    );
    
    gameState.player.position = teleportPosition;
    printf("Teleported to Stage %d area\n", stageNumber);
}

void InputHandler::processStageSelectionAction(
    int stageNumber,
    GameState& gameState,
    StageManager& stageManager,
    PlatformSystem& platformSystem,
    std::function<void()> resetStageStartTime,
    GLFWwindow* window
) {
    if (gameState.ui.showWarpTutorialUI) {
        return;
    }
    if (stageNumber == 0) {
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
        
        return;
    }

    const bool isUnlocked = (gameState.progress.unlockedStages.count(stageNumber) && gameState.progress.unlockedStages[stageNumber]);
    if (!isUnlocked) {
        gameState.ui.showUnlockConfirmUI = true;
        gameState.ui.unlockTargetStage = stageNumber;
        gameState.ui.unlockRequiredStars = requiredStarsFor(stageNumber);
        return;
    }

    bool hasClearedStage = (gameState.progress.stageStars.count(stageNumber) > 0 && gameState.progress.stageStars[stageNumber] > 0);
    
    if (hasClearedStage) {
        gameState.ui.showTimeAttackSelectionUI = true;
        gameState.ui.modeSelectionTargetStage = stageNumber;
        gameState.ui.blockEnterUntilReleased = true;  // 直前のENTERを無視
        gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
        gameState.progress.isTimeAttackMode = false;
    } else {
        resetStageStartTime();
        gameState.progress.lives = 6;
        stageManager.goToStage(stageNumber, gameState, platformSystem);
        gameState.ui.readyScreenShown = false;
        gameState.ui.showReadyScreen = true;
        gameState.ui.readyScreenSpeedLevel = 0;
        gameState.progress.timeScale = 1.0f;
        gameState.progress.timeScaleLevel = 0;
        gameState.progress.isTimeAttackMode = false;  // 初めて入る場合はNORMALモード
    }
}

void InputHandler::handleInputProcessing(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                              PlatformSystem& platformSystem, 
                              std::map<int, InputUtils::KeyState>& keyStates,
                              std::function<void()> resetStageStartTime, float scaledDeltaTime, io::AudioManager& audioManager) {
        // デバッグ: handleInputProcessingが呼ばれていることを確認（ステージ0の時のみ、初回のみ）
        static bool debugLogged = false;
        if (!debugLogged && stageManager.getCurrentStage() == 0) {
            printf("DEBUG: handleInputProcessing called for stage 0, keyStates size: %zu\n", keyStates.size());
            debugLogged = true;
        }
        
        // マルチプレイモードの開始（ステージ選択画面でMキー）- リーダーボードUI表示中でも動作
        if (keyStates.find(GLFW_KEY_M) != keyStates.end()) {
            if (keyStates[GLFW_KEY_M].justPressed() && stageManager.getCurrentStage() == 0 && !gameState.ui.showModeSelectionUI && !gameState.ui.showTimeAttackSelectionUI) {
                gameState.ui.showMultiplayerMenu = !gameState.ui.showMultiplayerMenu;
                printf("Multiplayer menu toggled: %s\n", gameState.ui.showMultiplayerMenu ? "ON" : "OFF");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } else if (keyStates[GLFW_KEY_M].justPressed()) {
                // 条件が満たされていない場合のデバッグ
                printf("DEBUG: M key pressed but conditions not met: stage=%d, showModeSelectionUI=%d, showTimeAttackSelectionUI=%d\n",
                       stageManager.getCurrentStage(), gameState.ui.showModeSelectionUI, gameState.ui.showTimeAttackSelectionUI);
            }
        } else {
            // MキーがkeyStatesに登録されていない場合のデバッグ（初回のみ）
            static bool warnedOnce = false;
            if (!warnedOnce && stageManager.getCurrentStage() == 0) {
                printf("DEBUG: GLFW_KEY_M not found in keyStates map!\n");
                warnedOnce = true;
            }
        }
        
        // リーダーボードUI表示中はプレイヤー移動を無効化
        if (gameState.ui.showLeaderboardUI) {
            return;
        }
        
        if (gameState.replay.isReplayMode) {
            // デバッグ: リプレイモード中の入力処理が呼ばれていることを確認
            static int debugInputCount = 0;
            debugInputCount++;
            if (debugInputCount % 60 == 0) {  // 60フレームごとにログ出力（約1秒）
                printf("REPLAY INPUT: Mode active - paused: %s, speed: %.1fx, playbackTime: %.2f\n",
                       gameState.replay.isReplayPaused ? "true" : "false",
                       gameState.replay.replayPlaybackSpeed,
                       gameState.replay.replayPlaybackTime);
            }
            
            if (keyStates[GLFW_KEY_SPACE].justPressed()) {
                gameState.replay.isReplayPaused = !gameState.replay.isReplayPaused;
                printf("REPLAY: %s\n", gameState.replay.isReplayPaused ? "Paused" : "Resumed");
            }
            
            if (keyStates[GLFW_KEY_T].justPressed()) {
                if (gameState.replay.replayPlaybackSpeed < 0.4f) {
                    gameState.replay.replayPlaybackSpeed = 0.5f;
                } else if (gameState.replay.replayPlaybackSpeed < 0.6f) {
                    gameState.replay.replayPlaybackSpeed = 1.0f;
                } else {
                    gameState.replay.replayPlaybackSpeed = 0.3f;
                }
                printf("REPLAY: Speed changed to %.1fx\n", gameState.replay.replayPlaybackSpeed);
            }
            
            if (keyStates[GLFW_KEY_ESCAPE].justPressed()) {
                gameState.replay.isReplayMode = false;
                gameState.replay.isReplayPaused = false;
                gameState.replay.replayPlaybackTime = 0.0f;
                printf("REPLAY: Stopped\n");
            }
            return;  // リプレイモード中は他の入力処理をスキップ
        }
        
        bool tutorialInputEnabled = true;
        if (gameState.progress.isTutorialStage) {
            if (gameState.progress.tutorialStepCompleted) {
                tutorialInputEnabled = false;  // ステップ完了時は入力無効
            } else if (gameState.progress.tutorialStep == 6) {
                tutorialInputEnabled = false;  // ステップ6は動けない
            } else if (gameState.progress.tutorialStep >= 7) {
                tutorialInputEnabled = true;   // ステップ7以降は動ける（アイテム取得やゴール到達のため）
            } else {
                switch (gameState.progress.tutorialStep) {
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
        
        static bool lastTutorialStepCompleted = false;
        if (gameState.progress.isTutorialStage && gameState.progress.tutorialStepCompleted && !lastTutorialStepCompleted) {
            if (gameState.audioEnabled) {
                audioManager.playSFX("tutorial_ok");
            }
        }
        lastTutorialStepCompleted = gameState.progress.tutorialStepCompleted;
        
        for (int key = GLFW_KEY_0; key <= GLFW_KEY_6; key++) {
            if (keyStates[key].justPressed()) {
                if (gameState.ui.showWarpTutorialUI) { continue; }
                int stageNumber = key - GLFW_KEY_0;
                
                if (gameState.progress.isTutorialStage && stageNumber == 0) {
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
                    
                    gameState.ui.showModeSelectionUI = false;
                    gameState.ui.showTimeAttackSelectionUI = false;
                    gameState.ui.showSecretStarSelectionUI = false;
                    gameState.ui.showSecretStarExplanationUI = false;
                    continue;
                }
                if (gameState.ui.showLeaderboardUI) {
                    gameState.ui.showLeaderboardUI = false;
                    gameState.ui.leaderboardEntries.clear();
                    continue;
                }
                
                if (stageManager.getCurrentStage() == 0 && stageNumber >= 1 && stageNumber <= 5) {
                    if (gameState.progress.unlockedStages[stageNumber]) {
                        teleportToStageArea(stageNumber, gameState);
                    }
                } else if (!gameState.progress.isTutorialStage) {
                    // マルチプレイモードの場合、両方のプレイヤーが同じステージで開始
                    if (gameState.multiplayer.isMultiplayerMode && gameState.multiplayer.isConnected) {
                        // マルチプレイモードでは、ホストがステージを選択すると両方のプレイヤーが開始
                        if (gameState.multiplayer.isHost) {
                            processStageSelectionAction(stageNumber, gameState, stageManager, platformSystem, resetStageStartTime, window);
                            // リモートプレイヤーも同じステージで開始（状態を同期）
                            gameState.multiplayer.remotePlayer.position = gameState.player.position;
                            gameState.multiplayer.remotePlayer.velocity = glm::vec3(0, 0, 0);
                            gameState.multiplayer.isRaceStarted = true;
                        }
                    } else {
                        processStageSelectionAction(stageNumber, gameState, stageManager, platformSystem, resetStageStartTime, window);
                    }
                }
            }
        }
        
        int currentStage = stageManager.getCurrentStage();
        bool canUseTKey = tutorialInputEnabled || (gameState.progress.isTutorialStage);
        if (keyStates[GLFW_KEY_T].justPressed() && currentStage != 0 && canUseTKey && !gameState.ui.showModeSelectionUI && !gameState.ui.showTimeAttackSelectionUI && !gameState.ui.showSecretStarSelectionUI && gameState.progress.selectedSecretStarType != GameProgressState::SecretStarType::MAX_SPEED_STAR) {
                gameState.progress.timeScaleLevel = (gameState.progress.timeScaleLevel + 1) % 3;
            switch (gameState.progress.timeScaleLevel) {
                case 0:
                    gameState.progress.timeScale = 1.0f;
                    break;
                case 1:
                    gameState.progress.timeScale = 2.0f;
                    break;
                case 2:
                    gameState.progress.timeScale = 3.0f;
                    break;
            }
        }
        
        if (keyStates[GLFW_KEY_E].justPressed() && stageManager.getCurrentStage() == 0 && !gameState.ui.showModeSelectionUI && !gameState.ui.showTimeAttackSelectionUI) {
            gameState.progress.isEasyMode = !gameState.progress.isEasyMode;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // マルチプレイモードの開始（ステージ選択画面でMキー）- 上で既に処理済み
        
        if (stageManager.getCurrentStage() == 0) {
            struct SkillToggle {
                int key;
                bool& hasSkill;
                int& remainingUses;
                int maxUses;
                const char* skillName;
            };
            
            std::vector<SkillToggle> skillToggles = {
                {GLFW_KEY_R, gameState.skills.hasTimeStopSkill, gameState.skills.timeStopRemainingUses, gameState.skills.timeStopMaxUses, "Time Stop"},
                {GLFW_KEY_Y, gameState.skills.hasDoubleJumpSkill, gameState.skills.doubleJumpRemainingUses, gameState.skills.doubleJumpMaxUses, "Double Jump"},
                {GLFW_KEY_U, gameState.skills.hasHeartFeelSkill, gameState.skills.heartFeelRemainingUses, gameState.skills.heartFeelMaxUses, "Heart Feel"},
                {GLFW_KEY_I, gameState.skills.hasFreeCameraSkill, gameState.skills.freeCameraRemainingUses, gameState.skills.freeCameraMaxUses, "Free Camera"},
                {GLFW_KEY_O, gameState.skills.hasBurstJumpSkill, gameState.skills.burstJumpRemainingUses, gameState.skills.burstJumpMaxUses, "Burst Jump"}
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
        
        bool isSecretStarMode = (gameState.progress.selectedSecretStarType != GameProgressState::SecretStarType::NONE);
        bool isTimeAttackMode = gameState.progress.isTimeAttackMode;
        
        if (keyStates[GLFW_KEY_Q].justPressed() && !isSecretStarMode && !isTimeAttackMode && gameState.skills.hasTimeStopSkill && !gameState.skills.isTimeStopped && gameState.skills.timeStopRemainingUses > 0) {
            gameState.skills.isTimeStopped = true;
            gameState.skills.timeStopTimer = gameState.skills.timeStopDuration;
            gameState.skills.timeStopRemainingUses--;
        }
        
        if (keyStates[GLFW_KEY_H].justPressed() && !isSecretStarMode && !isTimeAttackMode && gameState.skills.hasHeartFeelSkill && gameState.skills.heartFeelRemainingUses > 0 && gameState.progress.lives < 6) {
            gameState.progress.lives++;
            gameState.skills.heartFeelRemainingUses--;
        }
        
        if (keyStates[GLFW_KEY_C].justPressed() && !isSecretStarMode && !isTimeAttackMode && gameState.skills.hasFreeCameraSkill && !gameState.skills.isFreeCameraActive && 
            gameState.skills.freeCameraRemainingUses > 0 && !gameState.camera.isFirstPersonView) {
            gameState.skills.isFreeCameraActive = true;
            gameState.skills.freeCameraTimer = gameState.skills.freeCameraDuration;
            gameState.skills.freeCameraRemainingUses--;
            
            gameState.skills.freeCameraYaw = gameState.camera.yaw;
            gameState.skills.freeCameraPitch = gameState.camera.pitch;
            
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        
        if (keyStates[GLFW_KEY_B].justPressed() && !isSecretStarMode && !isTimeAttackMode && gameState.skills.hasBurstJumpSkill && !gameState.skills.isBurstJumpActive && 
            gameState.skills.burstJumpRemainingUses > 0 && !gameState.skills.isInBurstJumpAir && stageManager.getCurrentStage() != 0) {
            gameState.skills.isBurstJumpActive = true;
            gameState.skills.hasUsedBurstJump = false;
            gameState.skills.burstJumpRemainingUses--;
        }
        
        if (keyStates[GLFW_KEY_F].justPressed() && stageManager.getCurrentStage() == 0) {
            gameState.camera.isFirstPersonView = !gameState.camera.isFirstPersonView;
            if (gameState.camera.isFirstPersonView) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                gameState.camera.yaw = 90.0f;
                gameState.camera.pitch = -10.0f;
                gameState.camera.firstMouse = true;
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
        
        if (gameState.ui.showStage0Tutorial && stageManager.getCurrentStage() == 0) {
            if (gameState.ui.blockEnterUntilReleased) {
                if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                    gameState.ui.blockEnterUntilReleased = false; // 離されたのを確認
                }
                return;
            }

            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.ui.showStage0Tutorial = false;
                gameState.ui.showEasyModeExplanationUI = true; // EASYモード説明UIを表示
                gameState.ui.blockEnterUntilReleased = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        if (gameState.ui.showEasyModeExplanationUI) {
            if (gameState.ui.blockEnterUntilReleased) {
                if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                    gameState.ui.blockEnterUntilReleased = false; // 離されたのを確認
                }
                return;
            }

            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.ui.showEasyModeExplanationUI = false;
                gameState.ui.showModeSelectionUI = true; // モード選択UIを表示
                gameState.ui.blockEnterUntilReleased = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        if (gameState.ui.showModeSelectionUI) {
            if (keyStates[GLFW_KEY_T].justPressed()) {
                gameState.progress.isEasyMode = !gameState.progress.isEasyMode;
                printf("MODE SELECTION: %s\n", gameState.progress.isEasyMode ? "EASY" : "NORMAL");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            if (gameState.ui.blockEnterUntilReleased) {
                if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                    gameState.ui.blockEnterUntilReleased = false; // 離されたのを確認
                }
                return;
            }
            
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.ui.showModeSelectionUI = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        if (gameState.ui.showSecretStarExplanationUI) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.ui.showSecretStarExplanationUI = false;
            }
        }
        
        if (gameState.ui.showRaceResultUI) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                gameState.ui.showRaceResultUI = false;
                gameState.multiplayer.isRaceFinished = false;
                gameState.multiplayer.winnerPlayerId = -1;
                gameState.multiplayer.winnerTime = 0.0f;
                gameState.multiplayer.loserTime = 0.0f;
                gameState.ui.raceWinnerPlayerId = -1;
                gameState.ui.raceWinnerTime = 0.0f;
                gameState.ui.raceLoserTime = 0.0f;
                // フィールドに戻る
                if (stageManager.getCurrentStage() != 0) {
                    GameLoop::InputHandler::returnToField(window, gameState, stageManager, platformSystem, -1);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        if (gameState.ui.showSecretStarSelectionUI) {
            if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::NONE) {
                gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::MAX_SPEED_STAR;
            }
            
            if (keyStates[GLFW_KEY_T].justPressed()) {
                if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::MAX_SPEED_STAR) {
                    gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::SHADOW_STAR;
                } else if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::SHADOW_STAR) {
                    gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::IMMERSIVE_STAR;
                } else if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::IMMERSIVE_STAR) {
                    gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::MAX_SPEED_STAR;
                } else {
                    gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::MAX_SPEED_STAR;
                }
                printf("SECRET STAR SELECTION: %d (0=NONE, 1=MAX_SPEED, 2=SHADOW, 3=IMMERSIVE)\n", static_cast<int>(gameState.progress.selectedSecretStarType));
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            if (gameState.ui.blockEnterUntilReleased) {
                if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                    gameState.ui.blockEnterUntilReleased = false; // 離されたのを確認
                    printf("SECRET STAR: ENTER released, blockEnterUntilReleased = false\n");
                }
                return;
            }
            
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                printf("SECRET STAR: ENTER pressed, selectedType = %d\n", static_cast<int>(gameState.progress.selectedSecretStarType));
                int targetStage = gameState.ui.modeSelectionTargetStage;
                gameState.ui.showSecretStarSelectionUI = false;
                gameState.ui.showTimeAttackSelectionUI = false;  // 念のためタイムアタック選択UIも無効化
                gameState.ui.modeSelectionTargetStage = 0;
                gameState.ui.blockEnterUntilReleased = false;  // ブロックを解除
                
                resetStageStartTime();
                gameState.progress.lives = 6;
                stageManager.goToStage(targetStage, gameState, platformSystem);
                
                gameState.progress.isTimeAttackMode = false;  // SECRET STARモード時はタイムアタックモードを無効化
                
                if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::MAX_SPEED_STAR) {
                    gameState.ui.readyScreenShown = true;  // ready画面をスキップしたことを記録
                    gameState.ui.showReadyScreen = false;  // ready画面を表示しない
                    gameState.ui.isCountdownActive = true;  // カウントダウンを開始
                    gameState.ui.countdownTimer = 3.0f;  // カウントダウンタイマーを設定
                    gameState.ui.readyScreenSpeedLevel = 0;
                    
                    gameState.progress.timeScale = 3.0f;
                    gameState.progress.timeScaleLevel = 2;  // 3倍に設定
                } else {
                    gameState.ui.showReadyScreen = true;
                    gameState.ui.readyScreenShown = false;
                    gameState.ui.isCountdownActive = false;
                    gameState.ui.countdownTimer = 0.0f;
                    gameState.ui.readyScreenSpeedLevel = 0;
                    
                    gameState.progress.timeScale = 1.0f;
                    gameState.progress.timeScaleLevel = 0;
                }
                
                if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::IMMERSIVE_STAR) {
                    gameState.camera.isFirstPersonMode = true;
                    gameState.camera.isFirstPersonView = true;
                    gameState.camera.yaw = 90.0f;
                    gameState.camera.pitch = -10.0f;
                    gameState.camera.firstMouse = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                } else {
                    gameState.camera.isFirstPersonMode = false;
                    gameState.camera.isFirstPersonView = false;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            return;  // SECRET STAR選択UI表示中は他のUI処理をスキップ
        }
        
        if (gameState.ui.showTimeAttackSelectionUI) {
            if (keyStates[GLFW_KEY_T].justPressed()) {
                if (gameState.progress.isGameCleared) {
                    if (!gameState.progress.isTimeAttackMode && gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::NONE) {
                        gameState.progress.isTimeAttackMode = true;
                    } else if (gameState.progress.isTimeAttackMode && gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::NONE) {
                        gameState.progress.isTimeAttackMode = false;
                        gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::MAX_SPEED_STAR;
                    } else {
                        gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
                    }
                } else {
                    gameState.progress.isTimeAttackMode = !gameState.progress.isTimeAttackMode;
                    if (!gameState.progress.isTimeAttackMode) {
                        gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
                    }
                }
                printf("MODE SELECTION: %s\n", 
                    gameState.progress.isTimeAttackMode ? "TIME ATTACK" : 
                    (gameState.progress.selectedSecretStarType != GameProgressState::SecretStarType::NONE ? "SECRET STAR" : "NORMAL"));
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            if (gameState.ui.blockEnterUntilReleased) {
                if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                    gameState.ui.blockEnterUntilReleased = false; // 離されたのを確認
                }
                return;
            }
            
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                if (gameState.progress.selectedSecretStarType != GameProgressState::SecretStarType::NONE) {
                    gameState.ui.showTimeAttackSelectionUI = false;
                    gameState.ui.showSecretStarSelectionUI = true;
                    gameState.ui.blockEnterUntilReleased = true;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    return;
                }
                
                if (!gameState.progress.isTimeAttackMode) {
                    gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
                }
                
                int targetStage = gameState.ui.modeSelectionTargetStage;
                gameState.ui.showTimeAttackSelectionUI = false;
                gameState.ui.modeSelectionTargetStage = 0;
                gameState.ui.blockEnterUntilReleased = false;  // ブロックを解除
                
                resetStageStartTime();
                gameState.progress.lives = 6;
                stageManager.goToStage(targetStage, gameState, platformSystem);
                gameState.ui.readyScreenShown = false;
                gameState.ui.showReadyScreen = true;
                gameState.ui.readyScreenSpeedLevel = 0;
                gameState.progress.timeScale = 1.0f;
                gameState.progress.timeScaleLevel = 0;
                
                if (gameState.progress.isTimeAttackMode) {
                    gameState.progress.currentTimeAttackTime = 0.0f;
                    gameState.progress.timeAttackStartTime = 0.0f;
                    gameState.progress.isNewRecord = false;
                }
            }
        }
        
        if (gameState.ui.showStageClearUI) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                int clearedStage = stageManager.getCurrentStage();
                stageManager.completeStage(clearedStage);
                
                InputHandler::returnToField(window, gameState, stageManager, platformSystem, clearedStage);
                
                gameState.ui.showStageClearUI = false;
                // フィールドに戻る際にオンラインリプレイフラグをリセット
                gameState.replay.isOnlineReplay = false;
            }
            
            // オンラインリプレイの場合はリトライ機能を無効化
            if (keyStates[GLFW_KEY_R].justPressed() && !gameState.replay.isOnlineReplay) {
                resetStageStartTime();
                stageManager.loadStage(stageManager.getCurrentStage(), gameState, platformSystem);
                gameState.ui.showStageClearUI = false;
                gameState.progress.gameWon = false;
                gameState.progress.isGoalReached = false;
                gameState.progress.timeScale = 1.0f;
                gameState.progress.timeScaleLevel = 0;
                gameState.progress.lives = 6;
                if (gameState.progress.isTimeAttackMode) {
                    gameState.progress.currentTimeAttackTime = 0.0f;
                    gameState.progress.timeAttackStartTime = 0.0f;
                    gameState.progress.isNewRecord = false;
                }
                gameState.ui.readyScreenShown = false;
                gameState.ui.showReadyScreen = true;
                gameState.ui.readyScreenSpeedLevel = 0;
            }
            
            if (keyStates[GLFW_KEY_SPACE].justPressed()) {
                if (gameState.progress.isTimeAttackMode) {
                    int currentStage = stageManager.getCurrentStage();
                    
                    // オンラインリプレイの場合は、currentReplayをそのまま使用
                    if (gameState.replay.isOnlineReplay && !gameState.replay.currentReplay.frames.empty()) {
                        printf("REPLAY: Restarting online replay for stage %d (Clear time: %.2fs, frames: %zu)\n", 
                               currentStage, gameState.replay.currentReplay.clearTime, gameState.replay.currentReplay.frames.size());
                        
                        resetStageStartTime();  // ゲームタイムをリセット
                        stageManager.goToStage(currentStage, gameState, platformSystem);
                        
                        // リプレイ開始時はclearTimeを0に初期化（リプレイ終了時にreplayPlaybackTimeが設定される）
                        gameState.progress.clearTime = 0.0f;
                        gameState.progress.isTimeAttackMode = true;
                        gameState.progress.currentTimeAttackTime = 0.0f;
                        gameState.progress.timeAttackStartTime = 0.0f;
                        
                        gameState.replay.isReplayMode = true;
                        gameState.replay.isReplayPaused = false;
                        gameState.replay.replayPlaybackTime = 0.0f;
                        gameState.replay.previousReplayPlaybackTime = 0.0f;  // 前フレームの時間を初期化
                        gameState.replay.replayPlaybackSpeed = 1.0f;  // 初期速度は1.0x
                        // isOnlineReplayフラグは維持
                        
                        // リプレイ開始時に最初のフレームの位置を設定
                        if (!gameState.replay.currentReplay.frames.empty()) {
                            const auto& firstFrame = gameState.replay.currentReplay.frames[0];
                            gameState.player.position = firstFrame.playerPosition;
                            gameState.player.velocity = firstFrame.playerVelocity;
                        }
                        
                        gameState.ui.showStageClearUI = false;
                        gameState.progress.isStageCompleted = false;
                        gameState.progress.isGoalReached = false;
                        printf("REPLAY: Started online replay playback for stage %d (Clear time: %.2fs)\n", 
                               currentStage, gameState.replay.currentReplay.clearTime);
                        return;  // リプレイ再生開始時は他の処理をスキップ
                    }
                    // ローカルリプレイの場合は従来通り
                    else if (ReplayManager::replayExists(currentStage)) {
                        if (ReplayManager::loadReplay(gameState.replay.currentReplay, currentStage)) {
                            resetStageStartTime();  // ゲームタイムをリセット
                            stageManager.goToStage(currentStage, gameState, platformSystem);
                            
                            // リプレイ開始時はclearTimeを0に初期化（リプレイ終了時にreplayPlaybackTimeが設定される）
                            gameState.progress.clearTime = 0.0f;
                            
                            gameState.replay.isReplayMode = true;
                            gameState.replay.isReplayPaused = false;
                            gameState.replay.replayPlaybackTime = 0.0f;
                            gameState.replay.previousReplayPlaybackTime = 0.0f;  // 前フレームの時間を初期化
                            gameState.replay.replayPlaybackSpeed = 1.0f;  // 初期速度は1.0x
                            gameState.replay.isOnlineReplay = false;  // ローカルリプレイの場合はフラグをリセット
                            gameState.ui.showStageClearUI = false;
                            gameState.progress.isStageCompleted = false;
                            gameState.progress.isGoalReached = false;
                            printf("REPLAY: Started local replay playback for stage %d (Clear time: %.2fs)\n", currentStage, gameState.replay.currentReplay.clearTime);
                            return;  // リプレイ再生開始時は他の処理をスキップ
                        }
                    }
                }
            }
        }
        
        if (gameState.progress.isGameOver) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                stageManager.goToStage(0, gameState, platformSystem);
                gameState.progress.timeScale = 1.0f;
                gameState.progress.timeScaleLevel = 0;
                gameState.progress.isGameOver = false;
                gameState.progress.isTimeUp = false;
                gameState.progress.remainingTime = gameState.progress.timeLimit;
                
                gameState.player.position = glm::vec3(8, 2.0f, 0);
                gameState.player.velocity = glm::vec3(0, 0, 0);
                
                gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
                
                gameState.camera.isFirstPersonMode = false;
                gameState.camera.isFirstPersonView = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            
            if (keyStates[GLFW_KEY_R].justPressed()) {
                resetStageStartTime();
                stageManager.loadStage(stageManager.getCurrentStage(), gameState, platformSystem);
                gameState.progress.isGameOver = false;
                gameState.progress.isTimeUp = false;
                gameState.progress.remainingTime = gameState.progress.timeLimit;
                gameState.progress.timeScale = 1.0f;
                gameState.progress.timeScaleLevel = 0;
                gameState.progress.lives = 6;
                gameState.player.velocity = glm::vec3(0, 0, 0);
                gameState.player.lastCheckpoint = glm::vec3(0, 30.0f, 0);
                gameState.player.lastCheckpointItemId = -1;
                gameState.ui.readyScreenShown = false;
                gameState.ui.showReadyScreen = true;
                gameState.ui.readyScreenSpeedLevel = 0;
                
                // Immersiveモードの場合はisFirstPersonModeとisFirstPersonViewを再設定（loadStageでリセットされるため）
                if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::IMMERSIVE_STAR) {
                    gameState.camera.isFirstPersonMode = true;
                    gameState.camera.isFirstPersonView = true;
                    gameState.camera.yaw = 90.0f;
                    gameState.camera.pitch = -10.0f;
                    gameState.camera.firstMouse = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
            }
        }

        if (gameState.ui.showStarInsufficientUI) {
            if (keyStates[GLFW_KEY_SPACE].justPressed()) {
                gameState.ui.showStarInsufficientUI = false;
                gameState.ui.insufficientTargetStage = 0;
                gameState.ui.insufficientRequiredStars = 0;
            }
        }
        
        if (gameState.ui.showUnlockConfirmUI && !gameState.ui.showStarInsufficientUI) {
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                if (gameState.progress.totalStars >= gameState.ui.unlockRequiredStars) {
                    gameState.progress.totalStars -= gameState.ui.unlockRequiredStars;
                    gameState.progress.unlockedStages[gameState.ui.unlockTargetStage] = true;
                    
                    gameState.ui.showWarpTutorialUI = true;
                    gameState.ui.warpTutorialStage = gameState.ui.unlockTargetStage;
                    gameState.ui.blockEnterUntilReleased = true;
                    
                    gameState.ui.showUnlockConfirmUI = false;
                    gameState.ui.unlockTargetStage = 0;
                    gameState.ui.unlockRequiredStars = 0;
                } else {
                    gameState.ui.showUnlockConfirmUI = false;
                    gameState.ui.showStarInsufficientUI = true;
                    gameState.ui.insufficientTargetStage = gameState.ui.unlockTargetStage;
                    gameState.ui.insufficientRequiredStars = gameState.ui.unlockRequiredStars;
                    gameState.ui.unlockTargetStage = 0;
                    gameState.ui.unlockRequiredStars = 0;
                }
            }
            
            if (keyStates[GLFW_KEY_SPACE].justPressed()) {
                gameState.ui.showUnlockConfirmUI = false;
                gameState.ui.unlockTargetStage = 0;
                gameState.ui.unlockRequiredStars = 0;
            }
        }
        
        if (gameState.ui.showWarpTutorialUI) {
            if (gameState.ui.blockEnterUntilReleased) {
                if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                    gameState.ui.blockEnterUntilReleased = false;
                }
                return;
            }
            if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                int targetStage = gameState.ui.warpTutorialStage;
                gameState.ui.showWarpTutorialUI = false;
                gameState.ui.warpTutorialStage = 0;
                gameState.ui.blockEnterUntilReleased = false;  // ブロックを解除
                
                bool hasClearedStage = (gameState.progress.stageStars.count(targetStage) > 0 && gameState.progress.stageStars[targetStage] > 0);
                
                if (hasClearedStage) {
                    gameState.ui.showTimeAttackSelectionUI = true;
                    gameState.ui.modeSelectionTargetStage = targetStage;
                    gameState.ui.blockEnterUntilReleased = true;
                    gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
                    gameState.progress.isTimeAttackMode = false;
                } else {
                resetStageStartTime();
                gameState.progress.lives = 6;
                    stageManager.goToStage(targetStage, gameState, platformSystem);
                gameState.ui.readyScreenShown = false;
                gameState.ui.showReadyScreen = true;
                gameState.ui.readyScreenSpeedLevel = 0;
                    gameState.progress.timeScale = 1.0f;
                    gameState.progress.timeScaleLevel = 0;
                    gameState.progress.isTimeAttackMode = false;  // 初めて入る場合はNORMALモード
                }
            }
        }
        
        bool isUIBlockingMovement = gameState.ui.showUnlockConfirmUI || 
                                   gameState.ui.showStarInsufficientUI || 
                                   gameState.ui.showWarpTutorialUI || 
                                   gameState.progress.isGameOver || 
                                   gameState.ui.showStageClearUI || 
                                   gameState.ui.showEasyModeExplanationUI ||
                                   gameState.ui.showModeSelectionUI ||
                                   gameState.ui.showTimeAttackSelectionUI ||
                                   gameState.ui.showSecretStarExplanationUI ||
                                   gameState.ui.showSecretStarSelectionUI ||
                                   (gameState.ui.showStage0Tutorial && stageManager.getCurrentStage() == 0);
        
        if (tutorialInputEnabled && !isUIBlockingMovement) {
            glm::vec3 gravityDirection = glm::vec3(0, -1, 0);
            InputSystem::processInput(window, gameState, scaledDeltaTime);
            InputSystem::processJumpAndFloat(window, gameState, scaledDeltaTime, gravityDirection, platformSystem, audioManager);
        }
        
        if (gameState.progress.isTutorialStage) {
            TutorialManager::processTutorialProgress(window, gameState, keyStates);
        }
    }
    
    void InputHandler::returnToField(
        GLFWwindow* window,
        GameState& gameState,
        StageManager& stageManager,
        PlatformSystem& platformSystem,
        int clearedStage
    ) {
        stageManager.goToStage(0, gameState, platformSystem);
        
        // 共通のリセット処理
        gameState.progress.timeScale = 1.0f;
        gameState.progress.timeScaleLevel = 0;
        gameState.camera.isFirstPersonMode = false;
        gameState.camera.isFirstPersonView = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
        gameState.progress.gameWon = false;
        gameState.progress.isGoalReached = false;
        
        // プレイヤー位置の設定
        if (clearedStage > 0 && clearedStage <= 5) {
            glm::vec3 returnPosition;
            switch (clearedStage) {
                case 1: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[0].x, GameConstants::STAGE_AREAS[0].y, GameConstants::STAGE_AREAS[0].z-1); break;
                case 2: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[1].x, GameConstants::STAGE_AREAS[1].y, GameConstants::STAGE_AREAS[1].z-1); break;
                case 3: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[2].x, GameConstants::STAGE_AREAS[2].y, GameConstants::STAGE_AREAS[2].z-1); break;
                case 4: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[3].x, GameConstants::STAGE_AREAS[3].y, GameConstants::STAGE_AREAS[3].z-1); break;
                case 5: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[4].x, GameConstants::STAGE_AREAS[4].y, GameConstants::STAGE_AREAS[4].z-1); break;
                default: returnPosition = glm::vec3(GameConstants::STAGE_AREAS[0].x, GameConstants::STAGE_AREAS[0].y, GameConstants::STAGE_AREAS[0].z-1); break;
            }
            gameState.player.position = returnPosition;
        } else {
            // デフォルト位置（スタッフロール終了時など）
            gameState.player.position = glm::vec3(8, 0, 0);
        }
        gameState.player.velocity = glm::vec3(0, 0, 0);
    }
} // namespace GameLoop
