#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_renderer.h"
#include "../core/constants/game_constants.h"
#include "../gfx/camera_system.h"
#include "../gfx/texture_manager.h"
#include "../core/utils/ui_config_manager.h"
#include "../core/utils/resource_path.h"
#include "../gfx/minimap_renderer.h"
#include "../game/stage_editor.h"
#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
#include <GLFW/glfw3.h>
#include <algorithm>
#include <set>
#include <vector>

namespace GameLoop {
void GameRenderer::renderFrame(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
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
            
            if (gameState.ui.showPlayerNameInput) {
                gameStateUIRenderer->renderPlayerNameInput(width, height, gameState.ui.playerNameInput, gameState.ui.playerNameInputCursorPos, gameState.ui.titleScreenTimer);
            } else {
                gameStateUIRenderer->renderTitleScreen(width, height, gameState);
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
            
            renderer->endFrame();
            glfwPollEvents();  // イベントを処理してウィンドウを更新
            
            return;  // タイトル画面中は他の処理をスキップ
        }
        
        GameRenderer::prepareFrame(window, gameState, stageManager, renderer, width, height, deltaTime);
        
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
        
        GameRenderer::renderPlatforms(platformSystem, renderer, gameState, stageManager);
        
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
            GameRenderer::renderPlayer(gameState, renderer);
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
            
            if (gameState.ui.showMultiplayerMenu) {
                gameStateUIRenderer->renderMultiplayerMenu(width, height, gameState.multiplayer.isHost, 
                                                          gameState.multiplayer.isConnected, 
                                                          gameState.ui.isWaitingForConnection,
                                                          gameState.ui.connectionIP, 
                                                          gameState.ui.connectionPort);
            }
            
            if (gameState.ui.showRaceResultUI) {
                gameStateUIRenderer->renderRaceResultUI(width, height, gameState.ui.raceWinnerPlayerId, 
                                                        gameState.ui.raceWinnerTime, gameState.ui.raceLoserTime);
            }
            
            bool shouldShowAssist = gameState.ui.showStageSelectionAssist && 
                                   !gameState.ui.showUnlockConfirmUI && 
                                   !gameState.ui.showStarInsufficientUI &&
                                   !gameState.ui.showWarpTutorialUI &&
                                   !gameState.ui.showEasyModeExplanationUI &&
                                   !gameState.ui.showModeSelectionUI &&
                                   !gameState.ui.showTimeAttackSelectionUI &&  // タイムアタック選択UI表示中は非表示
                                   !gameState.ui.showStage0Tutorial &&  // チュートリアル表示中は非表示
                                   !gameState.ui.showLeaderboardUI;  // ランキングUI表示中は非表示
            bool isStageUnlocked = gameState.progress.unlockedStages[gameState.ui.assistTargetStage];
            gameStateUIRenderer->renderStageSelectionAssist(width, height, gameState.ui.assistTargetStage, shouldShowAssist, isStageUnlocked);
            
            // ランキングボードアシスト表示
            bool shouldShowLeaderboardAssist = gameState.ui.showLeaderboardAssist &&
                                               !gameState.ui.showUnlockConfirmUI &&
                                               !gameState.ui.showStarInsufficientUI &&
                                               !gameState.ui.showWarpTutorialUI &&
                                               !gameState.ui.showEasyModeExplanationUI &&
                                               !gameState.ui.showModeSelectionUI &&
                                               !gameState.ui.showTimeAttackSelectionUI &&
                                               !gameState.ui.showStageSelectionAssist &&
                                               !gameState.ui.showStage0Tutorial &&
                                               !gameState.ui.showLeaderboardUI;
            gameStateUIRenderer->renderLeaderboardAssist(width, height, shouldShowLeaderboardAssist);
            
            // ランキングUI表示
            if (gameState.ui.showLeaderboardUI) {
                gameStateUIRenderer->renderLeaderboardUI(width, height, 
                                                       gameState.ui.leaderboardTargetStage,
                                                       gameState.ui.leaderboardEntries,
                                                       gameState.ui.isLoadingLeaderboard,
                                                       gameState.ui.leaderboardSelectedIndex);
            }
            
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
        
        renderer->endFrame();
    }
void GameRenderer::prepareFrame(GLFWwindow* window, GameState& gameState, StageManager& stageManager,
                     std::unique_ptr<gfx::OpenGLRenderer>& renderer, int& width, int& height, float deltaTime) {
        renderer->beginFrameWithBackground(stageManager.getCurrentStage());
        
        if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::SHADOW_STAR && stageManager.getCurrentStage() != 0) {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        
        auto cameraConfig = CameraSystem::calculateCameraConfig(gameState, stageManager, window, deltaTime);
        CameraSystem::applyCameraConfig(renderer.get(), cameraConfig, window);
        
        auto [w, h] = CameraSystem::getWindowSize(window);
        width = w; height = h;
    }
void GameRenderer::renderPlatforms(PlatformSystem& platformSystem, 
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
        
        std::vector<bool> shadowVisibility = visibility;
        if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::SHADOW_STAR && stageManager.getCurrentStage() != 0) {
            for (size_t i = 0; i < shadowVisibility.size(); i++) {
                shadowVisibility[i] = false;
            }
            
            glm::vec3 playerSize = glm::vec3(1.0f, 2.0f, 1.0f);
            auto collisionResult = platformSystem.checkCollisionWithIndex(gameState.player.position, playerSize);
            int currentPlatformIndex = collisionResult.second;
            
            if (currentPlatformIndex >= 0 && currentPlatformIndex < static_cast<int>(shadowVisibility.size())) {
                shadowVisibility[currentPlatformIndex] = true;
            }
        }
        
        static GLuint staticPlatformTexture = 0;
        if (staticPlatformTexture == 0) {
            staticPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/static_platform.png"));
            if (staticPlatformTexture == 0) {
                staticPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/static_platform.png");
            }
        }
        static GLuint goalPlatformTexture = 0;
        if (goalPlatformTexture == 0) {
            goalPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/goal_platform.png"));
            if (goalPlatformTexture == 0) {
                goalPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/goal_platform.png");
            }
        }
        static GLuint startPlatformTexture = 0;
        if (startPlatformTexture == 0) {
            startPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/start_platform.png"));
            if (startPlatformTexture == 0) {
                startPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/start_platform.png");
            }
        }
        
        static GLuint movingPlatformTexture = 0;
        if (movingPlatformTexture == 0) {
            movingPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/moving_platform.png"));
            if (movingPlatformTexture == 0) {
                movingPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/moving_platform.png");
            }
        }

        static GLuint cyclingDisappearTexture = 0;
        if (cyclingDisappearTexture == 0) {
            cyclingDisappearTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/cyclingdisappearing_platform.png"));
            if (cyclingDisappearTexture == 0) {
                cyclingDisappearTexture = gfx::TextureManager::loadTexture("assets/textures/cyclingdisappearing_platform.png");
            }
        }

        static GLuint flyingPlatformTexture = 0;
        if (flyingPlatformTexture == 0) {
            flyingPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/flying_platform.png"));
            if (flyingPlatformTexture == 0) {
                flyingPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/flying_platform.png");
            }
        }
        
        static GLuint lockPlatformTexture = 0;
        if (lockPlatformTexture == 0) {
            lockPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/lock_platform.png"));
            if (lockPlatformTexture == 0) {
                lockPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/lock_platform.png");
            }
        }
        
        static GLuint unlockPlatformTexture = 0;
        if (unlockPlatformTexture == 0) {
            unlockPlatformTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/unlock_platform.png"));
            if (unlockPlatformTexture == 0) {
                unlockPlatformTexture = gfx::TextureManager::loadTexture("assets/textures/unlock_platform.png");
            }
        }
        
        for (size_t i = 0; i < positions.size(); i++) {
            bool shouldRender = (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::SHADOW_STAR) 
                                ? shadowVisibility[i] 
                                : visibility[i];
            if (!shouldRender || sizes[i].x <= 0 || sizes[i].y <= 0 || sizes[i].z <= 0) continue;
            
            if (isRotating[i]) {
                renderer->renderer3D.renderRotatedBox(positions[i], colors[i], sizes[i], rotationAxes[i], rotationAngles[i]);
            } else if (platformTypes[i] == "static" && (staticPlatformTexture != 0 || goalPlatformTexture != 0 || startPlatformTexture != 0 || lockPlatformTexture != 0 || unlockPlatformTexture != 0)) {
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
                renderer->renderer3D.renderTexturedBoxWithAlpha(positions[i], sizes[i], movingPlatformTexture, blinkAlphas[i]);
            } else if (platformTypes[i] == "cycle_disappearing" && cyclingDisappearTexture != 0) {
                renderer->renderer3D.renderTexturedBoxWithAlpha(positions[i], sizes[i], cyclingDisappearTexture, blinkAlphas[i]);
            } else if (platformTypes[i] == "flying" && flyingPlatformTexture != 0) {
                renderer->renderer3D.renderTexturedBoxWithAlpha(positions[i], sizes[i], flyingPlatformTexture, blinkAlphas[i]);
            } else {
                renderer->renderer3D.renderRealisticBox(positions[i], colors[i], sizes[i], blinkAlphas[i]);
            }
        }
    }
void GameRenderer::renderPlayer(GameState& gameState, 
                  std::unique_ptr<gfx::OpenGLRenderer>& renderer) {
    static GLuint playerTexture = 0;
    static GLuint playerFrontTexture = 0;
    
    if (playerTexture == 0) {
        playerTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/player.png"));
        if (playerTexture == 0) {
            playerTexture = gfx::TextureManager::loadTexture("assets/textures/player.png");
        }
    }
    
    if (playerFrontTexture == 0) {
        playerFrontTexture = gfx::TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/player_front.png"));
        if (playerFrontTexture == 0) {
            playerFrontTexture = gfx::TextureManager::loadTexture("assets/textures/player_front.png");
        }
    }
    
    if (playerTexture != 0 && playerFrontTexture != 0) {
        glm::vec3 playerSize = glm::vec3(GameConstants::PLAYER_SCALE);
        
        // ローカルプレイヤーを描画
        if (gameState.player.isShowingFrontTexture) {
            renderer->renderer3D.renderTexturedBox(gameState.player.position, playerSize, playerFrontTexture, playerTexture);
        } else {
            renderer->renderer3D.renderTexturedBox(gameState.player.position, playerSize, playerTexture);
        }
        
        // マルチプレイモードの場合、リモートプレイヤーも描画（少し透明度を下げて区別）
        if (gameState.multiplayer.isMultiplayerMode && gameState.multiplayer.isConnected) {
            float remotePlayerAlpha = 0.8f; // 少し透明度を下げてリモートプレイヤーを区別
            // 前面テクスチャと通常テクスチャの両方に対応するため、通常テクスチャを使用
            renderer->renderer3D.renderTexturedBoxWithAlpha(gameState.multiplayer.remotePlayer.position, playerSize, playerTexture, remotePlayerAlpha);
        }
    } else {
        renderer->renderer3D.renderCube(gameState.player.position, gameState.player.color, GameConstants::PLAYER_SCALE);
        
        // マルチプレイモードの場合、リモートプレイヤーも描画
        if (gameState.multiplayer.isMultiplayerMode && gameState.multiplayer.isConnected) {
            glm::vec3 remotePlayerColor = glm::vec3(0.2f, 0.8f, 1.0f);
            renderer->renderer3D.renderCube(gameState.multiplayer.remotePlayer.position, remotePlayerColor, GameConstants::PLAYER_SCALE);
        }
    }
}
} // namespace GameLoop
