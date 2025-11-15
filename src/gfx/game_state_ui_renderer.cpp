#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state_ui_renderer.h"
#include "../core/utils/ui_config_manager.h"
#include "texture_manager.h"
#include "../core/utils/resource_path.h"
#include "../game/game_state.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace gfx {

GameStateUIRenderer::GameStateUIRenderer() {
    font.initialize();
}

GameStateUIRenderer::~GameStateUIRenderer() {
}

void GameStateUIRenderer::renderTutorialStageUI(int width, int height, const std::string& message, int currentStep, bool stepCompleted) {
    font.initialize();
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    
    auto stepTextConfig = uiConfig.getTutorialStepTextConfig();
    glm::vec2 stepTextPos = uiConfig.calculatePosition(stepTextConfig.position, width, height);
    std::string stepText = "STEP " + std::to_string(currentStep + 1) + "/11";
    renderText(stepText, stepTextPos, stepTextConfig.color, stepTextConfig.scale);

    if(currentStep == 5){
        auto explainText1Config = uiConfig.getTutorialStep5ExplainText1Config();
        glm::vec2 explainText1Pos = uiConfig.calculatePosition(explainText1Config.position, width, height);
        std::string explainText = "YOUR SPEED AND THE SPEED OF THE GIMMICK WILL CHANGE !";
        renderText(explainText, explainText1Pos, explainText1Config.color, explainText1Config.scale);
    }else if(currentStep == 9){
        auto explainText1Config = uiConfig.getTutorialStep9ExplainText1Config();
        glm::vec2 explainText1Pos = uiConfig.calculatePosition(explainText1Config.position, width, height);
        std::string explainText = "THERE ARE THREE ITEMS ON THE STAGE !";
        renderText(explainText, explainText1Pos, explainText1Config.color, explainText1Config.scale);
        
        auto explainText2Config = uiConfig.getTutorialStep9ExplainText2Config();
        glm::vec2 explainText2Pos = uiConfig.calculatePosition(explainText2Config.position, width, height);
        std::string explainText2 = "BRONZE, SILVER, AND GOLD !";
        renderText(explainText2, explainText2Pos, explainText2Config.color, explainText2Config.scale);
        
        auto explainText3Config = uiConfig.getTutorialStep9ExplainText3Config();
        glm::vec2 explainText3Pos = uiConfig.calculatePosition(explainText3Config.position, width, height);
        std::string explainText3 = "COLLECT THEM ALL TO REACH THE GOAL !";
        renderText(explainText3, explainText3Pos, explainText3Config.color, explainText3Config.scale);
        
        auto explainText4Config = uiConfig.getTutorialStep9ExplainText4Config();
        glm::vec2 explainText4Pos = uiConfig.calculatePosition(explainText4Config.position, width, height);
        std::string explainText4 = "LET'S GO AND COLLECT THEM !";
        renderText(explainText4, explainText4Pos, explainText4Config.color, explainText4Config.scale);
    }else if(currentStep == 10){
        auto explainText1Config = uiConfig.getTutorialStep10ExplainText1Config();
        glm::vec2 explainText1Pos = uiConfig.calculatePosition(explainText1Config.position, width, height);
        std::string explainText = "THE YELLOW BLOCK IS THE GOAL !";
        renderText(explainText, explainText1Pos, explainText1Config.color, explainText1Config.scale);
        
        auto explainText2Config = uiConfig.getTutorialStep10ExplainText2Config();
        glm::vec2 explainText2Pos = uiConfig.calculatePosition(explainText2Config.position, width, height);
        std::string explainText2 = "TOUCH THE GOAL WITH THREE ITEMS TO CLEAR THE STAGE !";
        renderText(explainText2, explainText2Pos, explainText2Config.color, explainText2Config.scale);
        
        auto explainText3Config = uiConfig.getTutorialStep10ExplainText3Config();
        glm::vec2 explainText3Pos = uiConfig.calculatePosition(explainText3Config.position, width, height);
        std::string explainText3 = "LET'S TRY IT!";
        renderText(explainText3, explainText3Pos, explainText3Config.color, explainText3Config.scale);
    }
    
    auto messageConfig = uiConfig.getTutorialMessageConfigForStep(currentStep);
    glm::vec2 messagePos = uiConfig.calculatePosition(messageConfig.position, width, height);
    glm::vec3 messageColor = stepCompleted ? messageConfig.completedColor : messageConfig.color;
    renderText(message, messagePos, messageColor, messageConfig.scale);
    
    if (stepCompleted) {
        auto pressEnterConfig = uiConfig.getTutorialPressEnterConfig();
        glm::vec2 pressEnterPos = uiConfig.calculatePosition(pressEnterConfig.position, width, height);
        if (currentStep != 10) {
            renderText("PRESS ENTER TO CONTINUE", pressEnterPos, pressEnterConfig.color, pressEnterConfig.scale);
        }
    }
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderStageClearBackground(int width, int height, float clearTime, int earnedStars, bool isTimeAttackMode,
                                                      int currentStage, GameProgressState::SecretStarType selectedSecretStarType,
                                                      const std::map<int, std::set<GameProgressState::SecretStarType>>& secretStarCleared) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    if (earnedStars == 3) {
        auto completedConfig = uiConfig.getStageClearCompletedTextConfig();
        glm::vec2 completedPos = uiConfig.calculatePosition(completedConfig.position, width, height);
        renderText("STAGE COMPLETED!", completedPos, completedConfig.color, completedConfig.scale);
    } else {
        auto clearConfig = uiConfig.getStageClearClearTextConfig();
        glm::vec2 clearPos = uiConfig.calculatePosition(clearConfig.position, width, height);
        renderText("STAGE CLEAR!", clearPos, clearConfig.color, clearConfig.scale);
    }
    
    int clearTimeInt = static_cast<int>(clearTime);
    int clearTimeDecimal = static_cast<int>((clearTime - clearTimeInt) * 100);
    std::string clearTimeText = std::to_string(clearTimeInt) + "." + 
                               (clearTimeDecimal < 10 ? "0" : "") + std::to_string(clearTimeDecimal) + "s";
    auto clearTimeConfig = uiConfig.getStageClearClearTimeConfig();
    glm::vec2 clearTimePos = uiConfig.calculatePosition(clearTimeConfig.position, width, height);
    renderText("CLEAR TIME: " + clearTimeText, clearTimePos, clearTimeConfig.color, clearTimeConfig.scale);
    
    if (!isTimeAttackMode) {
        auto starsConfig = uiConfig.getStageClearStarsConfig();
        glm::vec2 starsBasePos = uiConfig.calculatePosition(starsConfig.position, width, height);
        
        if (selectedSecretStarType != GameProgressState::SecretStarType::NONE && currentStage > 0) {
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
            if (secretStarCleared.count(currentStage) > 0) {
                clearedTypes = secretStarCleared.at(currentStage);
            }
            
            for (int i = 0; i < 3; i++) {
                glm::vec2 starPos = glm::vec2(starsBasePos.x + i * starsConfig.spacing, starsBasePos.y);
                bool isCleared = (clearedTypes.count(secretStarTypes[i]) > 0);
                glm::vec3 starColor = isCleared ? secretStarColors[i] : inactiveColor;
                renderStar(starPos, starColor, starsConfig.scale, width, height);
            }
        } else {
            for (int i = 0; i < 3; i++) {
                glm::vec2 starPos = glm::vec2(starsBasePos.x + i * starsConfig.spacing, starsBasePos.y);
                glm::vec3 starColor = (i < earnedStars) ? starsConfig.selectedColor : starsConfig.unselectedColor;
                renderStar(starPos, starColor, starsConfig.scale, width, height);
            }
        }
    }
    
    auto returnFieldConfig = uiConfig.getStageClearReturnFieldConfig();
    glm::vec2 returnFieldPos = uiConfig.calculatePosition(returnFieldConfig.position, width, height);
    renderText("RETURN FIELD: ENTER", returnFieldPos, returnFieldConfig.color, returnFieldConfig.scale);
    
    auto retryConfig = uiConfig.getStageClearRetryConfig();
    glm::vec2 retryPos = uiConfig.calculatePosition(retryConfig.position, width, height);
    renderText("RETRY: R", retryPos, retryConfig.color, retryConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderTimeAttackClearBackground(int width, int height, float clearTime, float bestTime, bool isNewRecord) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    
    int clearTimeInt = static_cast<int>(clearTime);
    int clearTimeMinutes = clearTimeInt / 60;
    int clearTimeSeconds = clearTimeInt % 60;
    int clearTimeDecimal = static_cast<int>((clearTime - clearTimeInt) * 100);
    std::string clearTimeText = (clearTimeMinutes > 0 ? std::to_string(clearTimeMinutes) + ":" : "") + 
                                (clearTimeSeconds < 10 ? "0" : "") + std::to_string(clearTimeSeconds) + "." +
                                (clearTimeDecimal < 10 ? "0" : "") + std::to_string(clearTimeDecimal) + "s";
    auto clearTimeConfig = uiConfig.getTimeAttackClearClearTimeConfig();
    glm::vec2 clearTimePos = uiConfig.calculatePosition(clearTimeConfig.position, width, height);
    renderText("CLEAR TIME: " + clearTimeText, clearTimePos, clearTimeConfig.color, clearTimeConfig.scale);
    
    if (bestTime > 0.0f) {
        int bestTimeInt = static_cast<int>(bestTime);
        int bestTimeMinutes = bestTimeInt / 60;
        int bestTimeSeconds = bestTimeInt % 60;
        int bestTimeDecimal = static_cast<int>((bestTime - bestTimeInt) * 100);
        std::string bestTimeText = (bestTimeMinutes > 0 ? std::to_string(bestTimeMinutes) + ":" : "") + 
                                   (bestTimeSeconds < 10 ? "0" : "") + std::to_string(bestTimeSeconds) + "." +
                                   (bestTimeDecimal < 10 ? "0" : "") + std::to_string(bestTimeDecimal) + "s";
        auto bestTimeConfig = uiConfig.getTimeAttackClearBestTimeConfig();
        glm::vec2 bestTimePos = uiConfig.calculatePosition(bestTimeConfig.position, width, height);
        renderText("BEST TIME: " + bestTimeText, bestTimePos, bestTimeConfig.color, bestTimeConfig.scale);
    }
    
    if (isNewRecord) {
        auto newRecordConfig = uiConfig.getTimeAttackClearNewRecordConfig();
        glm::vec2 newRecordPos = uiConfig.calculatePosition(newRecordConfig.position, width, height);
        renderText("NEW RECORD!", newRecordPos, newRecordConfig.color, newRecordConfig.scale);
    }
    
    auto returnFieldConfig = uiConfig.getTimeAttackClearReturnFieldConfig();
    glm::vec2 returnFieldPos = uiConfig.calculatePosition(returnFieldConfig.position, width, height);
    renderText("RETURN FIELD: ENTER", returnFieldPos, returnFieldConfig.color, returnFieldConfig.scale);
    
    auto retryConfig = uiConfig.getTimeAttackClearRetryConfig();
    glm::vec2 retryPos = uiConfig.calculatePosition(retryConfig.position, width, height);
    renderText("RETRY: R", retryPos, retryConfig.color, retryConfig.scale);
    
    auto replayConfig = uiConfig.getTimeAttackClearReplayConfig();
    glm::vec2 replayPos = uiConfig.calculatePosition(replayConfig.position, width, height);
    renderText("REPLAY: SPACE", replayPos, replayConfig.color, replayConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderWarpTutorialBackground(int width, int height, int targetStage) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    auto titleConfig = uiConfig.getWarpTutorialTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("STAGE " + std::to_string(targetStage) + " HAS BEEN UNLOCKED.", titlePos, titleConfig.color, titleConfig.scale);
    
    auto desc1Config = uiConfig.getWarpTutorialDescription1Config();
    glm::vec2 desc1Pos = uiConfig.calculatePosition(desc1Config.position, width, height);
    renderText("YOU CAN WARP TO THE STAGE", desc1Pos, desc1Config.color, desc1Config.scale);
    
    auto desc2Config = uiConfig.getWarpTutorialDescription2Config();
    glm::vec2 desc2Pos = uiConfig.calculatePosition(desc2Config.position, width, height);
    renderText("BY PRESSING KEY " + std::to_string(targetStage) + ".", desc2Pos, desc2Config.color, desc2Config.scale);
    
    auto enterButtonConfig = uiConfig.getWarpTutorialEnterButtonConfig();
    glm::vec2 enterButtonPos = uiConfig.calculatePosition(enterButtonConfig.position, width, height);
    renderText("ENTER STAGE: ENTER", enterButtonPos, enterButtonConfig.color, enterButtonConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderGameOverBackground(int width, int height) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    auto gameOverTextConfig = uiConfig.getGameOverTextConfig();
    glm::vec2 gameOverTextPos = uiConfig.calculatePosition(gameOverTextConfig.position, width, height);
    renderText("GAME OVER!", gameOverTextPos, gameOverTextConfig.color, gameOverTextConfig.scale);
    
    auto returnFieldConfig = uiConfig.getGameOverReturnFieldConfig();
    glm::vec2 returnFieldPos = uiConfig.calculatePosition(returnFieldConfig.position, width, height);
    renderText("RETURN FIELD: ENTER", returnFieldPos, returnFieldConfig.color, returnFieldConfig.scale);
    
    auto retryConfig = uiConfig.getGameOverRetryConfig();
    glm::vec2 retryPos = uiConfig.calculatePosition(retryConfig.position, width, height);
    renderText("RETRY: R", retryPos, retryConfig.color, retryConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderEndingMessage(int width, int height, float timer) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    auto thankYouConfig = uiConfig.getEndingThankYouConfig();
    glm::vec2 thankYouPos = uiConfig.calculatePosition(thankYouConfig.position, width, height);
    renderText("THANK YOU FOR PLAYING!", thankYouPos, thankYouConfig.color, thankYouConfig.scale);
    
    auto congratulationsConfig = uiConfig.getEndingCongratulationsConfig();
    glm::vec2 congratulationsPos = uiConfig.calculatePosition(congratulationsConfig.position, width, height);
    renderText("AND CONGRATULATIONS ON CONQUERING WORLD 1!", congratulationsPos, congratulationsConfig.color, congratulationsConfig.scale);
    
    auto seeYouAgainConfig = uiConfig.getEndingSeeYouAgainConfig();
    glm::vec2 seeYouAgainPos = uiConfig.calculatePosition(seeYouAgainConfig.position, width, height);
    renderText("SEE YOU AGAIN SOMEWHERE!", seeYouAgainPos, seeYouAgainConfig.color, seeYouAgainConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderUnlockConfirmBackground(int width, int height, int targetStage, int requiredStars, int currentStars) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    auto titleConfig = uiConfig.getUnlockConfirmTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("UNLOCK STAGE " + std::to_string(targetStage) + " ?", titlePos, titleConfig.color, titleConfig.scale);
    
    auto requiredStarsConfig = uiConfig.getUnlockConfirmRequiredStarsConfig();
    glm::vec2 requiredStarsPos = uiConfig.calculatePosition(requiredStarsConfig.position, width, height);
    renderText("YOU MUST USE " + std::to_string(requiredStars) + " STARS !", requiredStarsPos, requiredStarsConfig.color, requiredStarsConfig.scale);
    
    auto unlockButtonConfig = uiConfig.getUnlockConfirmUnlockButtonConfig();
    glm::vec2 unlockButtonPos = uiConfig.calculatePosition(unlockButtonConfig.position, width, height);
    renderText("UNLOCK: ENTER", unlockButtonPos, unlockButtonConfig.color, unlockButtonConfig.scale);
    
    auto cancelButtonConfig = uiConfig.getUnlockConfirmCancelButtonConfig();
    glm::vec2 cancelButtonPos = uiConfig.calculatePosition(cancelButtonConfig.position, width, height);
    renderText("CANCEL: SPACE", cancelButtonPos, cancelButtonConfig.color, cancelButtonConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderStarInsufficientBackground(int width, int height, int targetStage, int requiredStars, int currentStars) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    auto titleConfig = uiConfig.getStarInsufficientTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("YOU CAN'T UNLOCK STAGE " + std::to_string(targetStage) + " !", titlePos, titleConfig.color, titleConfig.scale);
    
    auto requiredStarsConfig = uiConfig.getStarInsufficientRequiredStarsConfig();
    glm::vec2 requiredStarsPos = uiConfig.calculatePosition(requiredStarsConfig.position, width, height);
    renderText("YOU NEED " + std::to_string(requiredStars) + " STARS !", requiredStarsPos, requiredStarsConfig.color, requiredStarsConfig.scale);
    
    auto collectStarsConfig = uiConfig.getStarInsufficientCollectStarsConfig();
    glm::vec2 collectStarsPos = uiConfig.calculatePosition(collectStarsConfig.position, width, height);
    renderText("COLLECT STARS ON OTHER STAGES !", collectStarsPos, collectStarsConfig.color, collectStarsConfig.scale);
    
    auto okButtonConfig = uiConfig.getStarInsufficientOkButtonConfig();
    glm::vec2 okButtonPos = uiConfig.calculatePosition(okButtonConfig.position, width, height);
    renderText("OK: SPACE", okButtonPos, okButtonConfig.color, okButtonConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderTitleScreen(int width, int height, const GameState& gameState) {
    font.initialize();
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    float backgroundAlpha = 1.0f;
    
    std::string titleBgPath = ResourcePath::getResourcePath("assets/textures/title_bg.png");
    GLuint titleBgTexture = TextureManager::loadTexture(titleBgPath);
    if (titleBgTexture != 0) {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, backgroundAlpha);
        TextureManager::bindTexture(titleBgTexture);
        
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(width, 0);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(width, height);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0, height);
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    } else {
        printf("WARNING: Failed to load title background texture from: %s\n", titleBgPath.c_str());
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, backgroundAlpha);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(width, 0);
        glVertex2f(width, height);
        glVertex2f(0, height);
        glEnd();
        glDisable(GL_BLEND);
    }
    
    if (gameState.ui.titleScreenPhase == UIState::TitleScreenPhase::LOGO_ANIMATION ||
        gameState.ui.titleScreenPhase == UIState::TitleScreenPhase::SHOW_TEXT) {
        
        auto logoConfig = uiConfig.getTitleLogoConfig();
        glm::vec2 targetLogoPos = uiConfig.calculatePosition(logoConfig.position, width, height);
        
        float logoAlpha = 1.0f;
        float logoScale = 1.0f;
        glm::vec2 logoPos = targetLogoPos;
        
        if (gameState.ui.titleScreenPhase == UIState::TitleScreenPhase::LOGO_ANIMATION) {
            const float LOGO_ANIMATION_DURATION = 1.0f;
            float progress = std::min(1.0f, gameState.ui.titleScreenTimer / LOGO_ANIMATION_DURATION);
            
            float easeOut = 1.0f - pow(1.0f - progress, 3.0f);
            
            logoAlpha = progress;
            
            float slideOffset = height * 0.3f;  // 画面下から30%の位置から開始
            logoPos.y = targetLogoPos.y + slideOffset * (1.0f - easeOut);
            
            float baseScale = 0.3f + (easeOut * 0.7f);
            
            float bounceProgress = progress;
            if (progress > 0.7f) {
                float bouncePhase = (progress - 0.7f) / 0.3f;
                float bounce = sinf(bouncePhase * 3.14159f) * 0.15f * (1.0f - bouncePhase);
                logoScale = baseScale + bounce;
            } else {
                logoScale = baseScale;
            }
        }
        
        GLuint titleLogoTexture = TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/title_logo.png"));
        if (titleLogoTexture != 0) {
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 1.0f, 1.0f, logoAlpha);
            TextureManager::bindTexture(titleLogoTexture);
            
            float logoWidth = 400.0f * logoConfig.scale * logoScale;
            float logoHeight = 400.0f * logoConfig.scale * logoScale;
            
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(logoPos.x - logoWidth / 2, logoPos.y - logoHeight / 2);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(logoPos.x + logoWidth / 2, logoPos.y - logoHeight / 2);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(logoPos.x + logoWidth / 2, logoPos.y + logoHeight / 2);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(logoPos.x - logoWidth / 2, logoPos.y + logoHeight / 2);
            glEnd();
            
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
        }
    }
    
    if (gameState.ui.titleScreenPhase == UIState::TitleScreenPhase::SHOW_TEXT) {
        auto startButtonConfig = uiConfig.getTitleStartButtonConfig();
        glm::vec2 startButtonPos = uiConfig.calculatePosition(startButtonConfig.position, width, height);
        
        const float TEXT_FADE_DURATION = 0.3f;
        float textAlpha = 1.0f;
        if (gameState.ui.titleScreenTimer < TEXT_FADE_DURATION) {
            textAlpha = gameState.ui.titleScreenTimer / TEXT_FADE_DURATION;
        }
        
        if (gameState.ui.titleScreenTimer >= TEXT_FADE_DURATION) {
            float blinkSpeed = 2.0f;  // 点滅速度
            float blinkTime = gameState.ui.titleScreenTimer - TEXT_FADE_DURATION;
            float blink = 0.7f + 0.3f * (0.5f + 0.5f * sinf(blinkTime * blinkSpeed * 3.14159f));
            textAlpha = blink;
        }
        
        glm::vec3 textColor = startButtonConfig.color * textAlpha;
        renderText("PLAY START : ENTER", startButtonPos, textColor, startButtonConfig.scale);
    }
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderReadyScreen(int width, int height, int speedLevel, bool isFirstPersonMode) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    auto readyTextConfig = uiConfig.getReadyTextConfig();
    glm::vec2 readyTextPos = uiConfig.calculatePosition(readyTextConfig.position, width, height);
    renderText("READY", glm::vec2(readyTextPos.x * scaleX, readyTextPos.y * scaleY), readyTextConfig.color, readyTextConfig.scale);
    
    auto playSpeedLabelConfig = uiConfig.getPlaySpeedLabelConfig();
    glm::vec2 playSpeedLabelPos = uiConfig.calculatePosition(playSpeedLabelConfig.position, width, height);
    renderText("PLAY SPEED", glm::vec2(playSpeedLabelPos.x * scaleX, playSpeedLabelPos.y * scaleY), playSpeedLabelConfig.color, playSpeedLabelConfig.scale);
    
    auto speedOptionsConfig = uiConfig.getSpeedOptionsConfig();
    glm::vec2 speedOptionsBasePos = uiConfig.calculatePosition(speedOptionsConfig.position, width, height);
    std::vector<std::string> speedTexts = {"1x", "2x", "3x"};
    for (int i = 0; i < 3; i++) {
        glm::vec3 color = (i == speedLevel) ? speedOptionsConfig.selectedColor : speedOptionsConfig.unselectedColor;
        float xPos = (speedOptionsBasePos.x + i * speedOptionsConfig.spacing) * scaleX;
        float yPos = speedOptionsBasePos.y * scaleY;
        renderText(speedTexts[i], glm::vec2(xPos, yPos), color, speedOptionsConfig.scale);
    }
    
    auto readyPressTConfig = uiConfig.getReadyPressTConfig();
    glm::vec2 readyPressTPos = uiConfig.calculatePosition(readyPressTConfig.position, width, height);
    renderText("PRESS T", glm::vec2(readyPressTPos.x * scaleX, readyPressTPos.y * scaleY), readyPressTConfig.color, readyPressTConfig.scale);
    
    auto confirmConfig = uiConfig.getConfirmConfig();
    glm::vec2 confirmPos = uiConfig.calculatePosition(confirmConfig.position, width, height);
    renderText("CONFIRM: ENTER", glm::vec2(confirmPos.x * scaleX, confirmPos.y * scaleY), confirmConfig.color, confirmConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderCountdown(int width, int height, int count) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    auto countdownConfig = uiConfig.getCountdownNumberConfig();
    glm::vec2 countdownPos = uiConfig.calculatePosition(countdownConfig.position, width, height);
    
    std::string countText = std::to_string(count);
    renderText(countText, countdownPos, countdownConfig.color, countdownConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderStage0Tutorial(int width, int height) {
    font.initialize();
    
    // 2D描画モードに切り替え（difficulty selectionと同じ方法）
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
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto line1Config = uiConfig.getStage0TutorialLine1Config();
    glm::vec2 line1Pos = uiConfig.calculatePosition(line1Config.position, width, height);
    renderText("THIS IS THE WORLD.", line1Pos, line1Config.color, line1Config.scale);
    
    auto line2Config = uiConfig.getStage0TutorialLine2Config();
    glm::vec2 line2Pos = uiConfig.calculatePosition(line2Config.position, width, height);
    renderText("THERE ARE ENTRANCES TO EACH STAGE.", line2Pos, line2Config.color, line2Config.scale);
    
    auto line3Config = uiConfig.getStage0TutorialLine3Config();
    glm::vec2 line3Pos = uiConfig.calculatePosition(line3Config.position, width, height);
    renderText("EACH STAGE IS LOCKED AT FIRST", line3Pos, line3Config.color, line3Config.scale);
    
    auto line4Config = uiConfig.getStage0TutorialLine4Config();
    glm::vec2 line4Pos = uiConfig.calculatePosition(line4Config.position, width, height);
    renderText("SO YOU NEED TO UNLOCK IT USING STARS YOU'VE EARNED.", line4Pos, line4Config.color, line4Config.scale);
    
    auto line5Config = uiConfig.getStage0TutorialLine5Config();
    glm::vec2 line5Pos = uiConfig.calculatePosition(line5Config.position, width, height);
    renderText("COLLECT LOTS OF STARS TO PROGRESS!", line5Pos, line5Config.color, line5Config.scale);
    
    auto okButtonConfig = uiConfig.getStage0TutorialOkButtonConfig();
    glm::vec2 okButtonPos = uiConfig.calculatePosition(okButtonConfig.position, width, height);
    renderText("OK : ENTER", okButtonPos, okButtonConfig.color, okButtonConfig.scale);
    
    // 3D描画モードに戻す（difficulty selectionと同じ方法）
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderStageSelectionAssist(int width, int height, int targetStage, bool isVisible, bool isUnlocked) {
    if (!isVisible) {
        return; // 非表示の場合は何も描画しない
    }
    
    font.initialize();
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    auto assistConfig = uiConfig.getStageSelectionAssistTextConfig();
    glm::vec2 assistPos = uiConfig.calculatePosition(assistConfig.position, width, height);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    std::string assistText;
    if (targetStage == 6 || isUnlocked) {
        assistText = "ENTER STAGE " + std::to_string(targetStage) + " : ENTER";
    } else { 
        assistText = "UNLOCK STAGE " + std::to_string(targetStage) + " : ENTER";
    }
    
    renderText(assistText, assistPos, assistConfig.color, assistConfig.scale);
    
    glDisable(GL_BLEND);
}

void GameStateUIRenderer::renderStageSelectionEscKeyInfo(int width, int height) {
    font.initialize();
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    auto escKeyInfoConfig = uiConfig.getStageSelectionEscKeyInfoConfig();
    glm::vec2 escKeyInfoPos = uiConfig.calculatePosition(escKeyInfoConfig.position, width, height);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    std::string escKeyInfoText = "SAVE DATA/QUIT: ESC";
    renderText(escKeyInfoText, escKeyInfoPos, escKeyInfoConfig.color, escKeyInfoConfig.scale);
    
    glDisable(GL_BLEND);
}

void GameStateUIRenderer::renderStaffRoll(int width, int height, float timer) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    float scrollOffset = timer * uiConfig.getStaffRollSpacing(); // 設定ファイルから間隔を取得
    float spacing = uiConfig.getStaffRollSpacing();
    
    auto skipConfig = uiConfig.getStaffRollSkipConfig();
    glm::vec2 skipPos = uiConfig.calculatePosition(skipConfig.position, width, height);
    renderText("SKIP : ENTER", skipPos, skipConfig.color, skipConfig.scale);
    
    auto titleConfig = uiConfig.getStaffRollTitleConfig();
    glm::vec2 titleBasePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("STAFF ROLL", glm::vec2(titleBasePos.x, titleBasePos.y - scrollOffset), titleConfig.color, titleConfig.scale);
    
    auto roleConfig = uiConfig.getStaffRollRoleConfig();
    auto nameConfig = uiConfig.getStaffRollNameConfig();
    glm::vec2 roleBasePos = uiConfig.calculatePosition(roleConfig.position, width, height);
    glm::vec2 nameBasePos = uiConfig.calculatePosition(nameConfig.position, width, height);
    
    renderText("DIRECTOR", glm::vec2(roleBasePos.x, roleBasePos.y - scrollOffset + spacing), roleConfig.color, roleConfig.scale);
    renderText("TAKUMI KUSAKA", glm::vec2(nameBasePos.x, nameBasePos.y - scrollOffset + spacing), nameConfig.color, nameConfig.scale);
    
    renderText("PROGRAMMER", glm::vec2(roleBasePos.x, roleBasePos.y - scrollOffset + spacing * 2), roleConfig.color, roleConfig.scale);
    renderText("TAKUMI KUSAKA", glm::vec2(nameBasePos.x, nameBasePos.y - scrollOffset + spacing * 2), nameConfig.color, nameConfig.scale);
    
    renderText("GRAPHICS", glm::vec2(roleBasePos.x, roleBasePos.y - scrollOffset + spacing * 3), roleConfig.color, roleConfig.scale);
    renderText("TAKUMI KUSAKA", glm::vec2(nameBasePos.x, nameBasePos.y - scrollOffset + spacing * 3), nameConfig.color, nameConfig.scale);
    
    renderText("SOUND", glm::vec2(roleBasePos.x, roleBasePos.y - scrollOffset + spacing * 4), roleConfig.color, roleConfig.scale);
    renderText("TAKUMI KUSAKA", glm::vec2(nameBasePos.x, nameBasePos.y - scrollOffset + spacing * 4), nameConfig.color, nameConfig.scale);
    
    renderText("GAMEDESIGN", glm::vec2(roleBasePos.x, roleBasePos.y - scrollOffset + spacing * 5), roleConfig.color, roleConfig.scale);
    renderText("TAKUMI KUSAKA", glm::vec2(nameBasePos.x, nameBasePos.y - scrollOffset + spacing * 5), nameConfig.color, nameConfig.scale);
    
    renderText("DEBUG", glm::vec2(roleBasePos.x, roleBasePos.y - scrollOffset + spacing * 6), roleConfig.color, roleConfig.scale);
    renderText("TAKUMI KUSAKA", glm::vec2(nameBasePos.x, nameBasePos.y - scrollOffset + spacing * 6), nameConfig.color, nameConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::begin2DMode() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1280, 720, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
}

void GameStateUIRenderer::end2DMode() {
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1280, 720, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    glColor3f(color.r, color.g, color.b);
    
    float currentX = position.x;
    float charWidth = GameConstants::RenderConstants::CHAR_WIDTH * scale;
    float charHeight = GameConstants::RenderConstants::CHAR_HEIGHT * scale;
    float spaceWidth = GameConstants::RenderConstants::SPACE_WIDTH * scale;  // スペース幅を増加
    
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        
        if (c == ' ') {
            currentX += spaceWidth;
            continue;
        }
        
        renderBitmapChar(c, glm::vec2(currentX, position.y), color, scale);
        currentX += charWidth + GameConstants::RenderConstants::CHAR_SPACING * scale;  // 文字間隔を増加
    }
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale) {
    if (!font.hasCharacter(c)) {
        return;
    }
    
    const std::vector<bool>& charData = font.getCharacter(c);
    
    glColor3f(color.r, color.g, color.b);
    
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 8; x++) {
            size_t index = y * 8 + x;
            if (index < charData.size() && charData[index]) {
                float pixelX = position.x + x * scale;
                float pixelY = position.y + y * scale;
                
                glBegin(GL_QUADS);
                glVertex2f(pixelX, pixelY);
                glVertex2f(pixelX + scale, pixelY);
                glVertex2f(pixelX + scale, pixelY + scale);
                glVertex2f(pixelX, pixelY + scale);
                glEnd();
            }
        }
    }
}

void GameStateUIRenderer::renderStar(const glm::vec2& position, const glm::vec3& color, float scale, int width, int height) {
    float scaleX = static_cast<float>(width) / 1280.0f;
    float scaleY = static_cast<float>(height) / 720.0f;
    float scaledScale = scale * std::min(scaleX, scaleY);
    
    glColor3f(color.r, color.g, color.b);
    
    float centerX = position.x;
    float centerY = position.y;
    
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; i++) {
        float angle1 = i * 72.0f * 3.14159f / 180.0f;
        float angle2 = (i + 2) * 72.0f * 3.14159f / 180.0f;
        
        float x1 = centerX + cos(angle1) * 12.0f * scaledScale;
        float y1 = centerY + sin(angle1) * 12.0f * scaledScale;
        
        float x2 = centerX + cos(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scaledScale;
        float y2 = centerY + sin(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scaledScale;
        
        float x3 = centerX + cos(angle2) * 12.0f * scaledScale;
        float y3 = centerY + sin(angle2) * 12.0f * scaledScale;
        
        glVertex2f(centerX, centerY);  // 中心点
        glVertex2f(x1, y1);            // 外側の点1
        glVertex2f(x2, y2);            // 内側の点
        
        glVertex2f(centerX, centerY);  // 中心点
        glVertex2f(x2, y2);            // 内側の点
        glVertex2f(x3, y3);            // 外側の点2
    }
    glEnd();
}

void GameStateUIRenderer::renderEasyModeExplanationUI(int width, int height) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    auto titleConfig = uiConfig.getEasyModeExplanationTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("DIFFICULTY SELECTION", titlePos, titleConfig.color, titleConfig.scale);
    
    auto desc1Config = uiConfig.getEasyModeExplanationDescription1Config();
    glm::vec2 desc1Pos = uiConfig.calculatePosition(desc1Config.position, width, height);
    renderText("YOU CAN CHOOSE BETWEEN EASY AND NORMAL MODES.", desc1Pos, desc1Config.color, desc1Config.scale);
    
    auto desc2Config = uiConfig.getEasyModeExplanationDescription2Config();
    glm::vec2 desc2Pos = uiConfig.calculatePosition(desc2Config.position, width, height);
    renderText("IN EASY MODE,", desc2Pos, desc2Config.color, desc2Config.scale);
    
    auto item1Config = uiConfig.getEasyModeExplanationItem1Config();
    glm::vec2 item1Pos = uiConfig.calculatePosition(item1Config.position, width, height);
    renderText("1. NO LIVES ARE LOST.", item1Pos, item1Config.color, item1Config.scale);
    
    auto item2Config = uiConfig.getEasyModeExplanationItem2Config();
    glm::vec2 item2Pos = uiConfig.calculatePosition(item2Config.position, width, height);
    renderText("2. THE TIME LIMIT IS INCREASED BY 20 SECONDS.", item2Pos, item2Config.color, item2Config.scale);
    
    auto item3Config = uiConfig.getEasyModeExplanationItem3Config();
    glm::vec2 item3Pos = uiConfig.calculatePosition(item3Config.position, width, height);
    renderText("3. PRESS SPACE TWICE TO JUMP AGAIN IN THE AIR.", item3Pos, item3Config.color, item3Config.scale);
    
    auto item4aConfig = uiConfig.getEasyModeExplanationItem4aConfig();
    glm::vec2 item4aPos = uiConfig.calculatePosition(item4aConfig.position, width, height);
    renderText("4. IF YOU FALL, YOU CAN RESUME FROM THE PLATFORM", item4aPos, item4aConfig.color, item4aConfig.scale);
    
    auto item4bConfig = uiConfig.getEasyModeExplanationItem4bConfig();
    glm::vec2 item4bPos = uiConfig.calculatePosition(item4bConfig.position, width, height);
    renderText("   YOU WERE LAST STANDING ON.", item4bPos, item4bConfig.color, item4bConfig.scale);
    
    auto okButtonConfig = uiConfig.getEasyModeExplanationOkButtonConfig();
    glm::vec2 okButtonPos = uiConfig.calculatePosition(okButtonConfig.position, width, height);
    renderText("OK : ENTER", okButtonPos, okButtonConfig.color, okButtonConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderEasyModeSelectionUI(int width, int height, bool isEasyMode) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    auto titleConfig = uiConfig.getModeSelectionTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("SELECT MODE", titlePos, titleConfig.color, titleConfig.scale);
    
    auto normalConfig = uiConfig.getModeSelectionNormalTextConfig();
    glm::vec2 normalPos = uiConfig.calculatePosition(normalConfig.position, width, height);
    glm::vec3 normalColor = isEasyMode ? normalConfig.unselectedColor : normalConfig.selectedColor;
    renderText("NORMAL", normalPos, normalColor, normalConfig.scale);
    
    auto easyConfig = uiConfig.getModeSelectionEasyTextConfig();
    glm::vec2 easyPos = uiConfig.calculatePosition(easyConfig.position, width, height);
    glm::vec3 easyColor = isEasyMode ? easyConfig.selectedColor : easyConfig.unselectedColor;
    renderText("EASY", easyPos, easyColor, easyConfig.scale);
    
    auto pressTConfig = uiConfig.getModeSelectionPressTConfig();
    glm::vec2 pressTPos = uiConfig.calculatePosition(pressTConfig.position, width, height);
    renderText("PRESS T", pressTPos, pressTConfig.color, pressTConfig.scale);
    
    auto confirmConfig = uiConfig.getModeSelectionConfirmConfig();
    glm::vec2 confirmPos = uiConfig.calculatePosition(confirmConfig.position, width, height);
    renderText("CONFIRM: ENTER", confirmPos, confirmConfig.color, confirmConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderTimeAttackSelectionUI(int width, int height, bool isTimeAttackMode, bool isGameCleared, GameProgressState::SecretStarType secretStarType) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    auto titleConfig = uiConfig.getModeSelectionTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("SELECT MODE", titlePos, titleConfig.color, titleConfig.scale);
    
    auto normalConfig = uiConfig.getModeSelectionNormalTextConfig();
    glm::vec2 normalPos = uiConfig.calculatePosition(normalConfig.position, width, height);
    
    auto timeAttackConfig = uiConfig.getModeSelectionTimeAttackTextConfig();
    glm::vec2 timeAttackPos = uiConfig.calculatePosition(timeAttackConfig.position, width, height);
    
    if (isGameCleared) {
        bool isSecretStarSelected = (secretStarType != GameProgressState::SecretStarType::NONE);
        
        glm::vec3 normalColor = (isTimeAttackMode || isSecretStarSelected) ? normalConfig.unselectedColor : normalConfig.selectedColor;
        renderText("NORMAL", normalPos, normalColor, normalConfig.scale);
        
        glm::vec3 timeAttackColor = isTimeAttackMode ? timeAttackConfig.selectedColor : timeAttackConfig.unselectedColor;
        renderText("TIME ATTACK", timeAttackPos, timeAttackColor, timeAttackConfig.scale);
        
        auto secretStarConfig = uiConfig.getModeSelectionSecretStarTextConfig();
        glm::vec2 secretStarPos = uiConfig.calculatePosition(secretStarConfig.position, width, height);
        glm::vec3 secretStarColor = isSecretStarSelected ? secretStarConfig.selectedColor : secretStarConfig.unselectedColor;
        renderText("SECRET STAR", secretStarPos, secretStarColor, secretStarConfig.scale);
    } else {
        glm::vec3 normalColor = isTimeAttackMode ? normalConfig.unselectedColor : normalConfig.selectedColor;
        renderText("NORMAL", normalPos, normalColor, normalConfig.scale);
        
        glm::vec3 timeAttackColor = isTimeAttackMode ? timeAttackConfig.selectedColor : timeAttackConfig.unselectedColor;
        renderText("TIME ATTACK", timeAttackPos, timeAttackColor, timeAttackConfig.scale);
    }
    
    auto pressTConfig = uiConfig.getModeSelectionPressTConfig();
    glm::vec2 pressTPos = uiConfig.calculatePosition(pressTConfig.position, width, height);
    renderText("PRESS T", pressTPos, pressTConfig.color, pressTConfig.scale);
    
    auto confirmConfig = uiConfig.getModeSelectionConfirmConfig();
    glm::vec2 confirmPos = uiConfig.calculatePosition(confirmConfig.position, width, height);
    renderText("CONFIRM: ENTER", confirmPos, confirmConfig.color, confirmConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderSecretStarExplanationUI(int width, int height) {
    font.initialize();
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    
    auto line1Config = uiConfig.getSecretStarExplanationLine1Config();
    glm::vec2 line1Pos = uiConfig.calculatePosition(line1Config.position, width, height);
    renderText("SECRET STARS NOW APPEAR IN EACH STAGE.", line1Pos, line1Config.color, line1Config.scale);
    
    auto line2Config = uiConfig.getSecretStarExplanationLine2Config();
    glm::vec2 line2Pos = uiConfig.calculatePosition(line2Config.position, width, height);
    renderText("THERE ARE THREE TYPES OF SECRET STARS.", line2Pos, line2Config.color, line2Config.scale);
    
    auto line3Config = uiConfig.getSecretStarExplanationLine3Config();
    glm::vec2 line3Pos = uiConfig.calculatePosition(line3Config.position, width, height);
    renderText("1. MAX SPEED STAR: THE WORLD'S SPEED IS ALWAYS TRIPLED AND CANNOT BE CHANGED.", line3Pos, line3Config.color, line3Config.scale);
    
    auto line3bConfig = uiConfig.getSecretStarExplanationLine3bConfig();
    glm::vec2 line3bPos = uiConfig.calculatePosition(line3bConfig.position, width, height);
    renderText("AIM FOR THE GOAL IN A WORLD OF LIGHTNING SPEED.", line3bPos, line3bConfig.color, line3bConfig.scale);
    
    auto line4Config = uiConfig.getSecretStarExplanationLine4Config();
    glm::vec2 line4Pos = uiConfig.calculatePosition(line4Config.position, width, height);
    renderText("2. SHADOW STAR: THE STAGE IS ENGULFED IN DARKNESS,", line4Pos, line4Config.color, line4Config.scale);
    
    auto line4bConfig = uiConfig.getSecretStarExplanationLine4bConfig();
    glm::vec2 line4bPos = uiConfig.calculatePosition(line4bConfig.position, width, height);
    renderText("AND YOU CAN ONLY SEE THE GROUND BENEATH YOUR FEET.", line4bPos, line4bConfig.color, line4bConfig.scale);
    
    auto line4cConfig = uiConfig.getSecretStarExplanationLine4cConfig();
    glm::vec2 line4cPos = uiConfig.calculatePosition(line4cConfig.position, width, height);
    renderText("USE THE MINIMAP AND YOUR MEMORY TO TAKE EACH STEP TOWARDS THE GOAL.", line4cPos, line4cConfig.color, line4cConfig.scale);
    
    auto line5Config = uiConfig.getSecretStarExplanationLine5Config();
    glm::vec2 line5Pos = uiConfig.calculatePosition(line5Config.position, width, height);
    renderText("3. IMMERSIVE STAR: SWITCH TO A FIRST-PERSON PERSPECTIVE.", line5Pos, line5Config.color, line5Config.scale);
    
    auto line5bConfig = uiConfig.getSecretStarExplanationLine5bConfig();
    glm::vec2 line5bPos = uiConfig.calculatePosition(line5bConfig.position, width, height);
    renderText("ENJOY THE IMMERSIVE EXPERIENCE", line5bPos, line5bConfig.color, line5bConfig.scale);
    
    auto okButtonConfig = uiConfig.getSecretStarExplanationOkButtonConfig();
    glm::vec2 okButtonPos = uiConfig.calculatePosition(okButtonConfig.position, width, height);
    renderText("OK : ENTER", okButtonPos, okButtonConfig.color, okButtonConfig.scale);
    
    glDisable(GL_BLEND);
}

void GameStateUIRenderer::renderSecretStarSelectionUI(int width, int height, GameProgressState::SecretStarType selectedType) {
    font.initialize();
    
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_BLEND);
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    auto titleConfig = uiConfig.getSecretStarSelectionTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("SELECT SECRET STAR", titlePos, titleConfig.color, titleConfig.scale);
    
    glm::vec3 maxSpeedColor = glm::vec3(0.4f, 0.8f, 1.0f);  // 水色
    glm::vec3 shadowColor = glm::vec3(0.2f, 0.2f, 0.2f);   // 黒
    glm::vec3 immersiveColor = glm::vec3(1.0f, 0.6f, 0.8f); // ピンク
    glm::vec3 unselectedColor = glm::vec3(0.5f, 0.5f, 0.5f); // グレー（未選択）
    
    auto star1PosConfig = uiConfig.getSecretStarSelectionStar1Config();
    auto star2PosConfig = uiConfig.getSecretStarSelectionStar2Config();
    auto star3PosConfig = uiConfig.getSecretStarSelectionStar3Config();
    
    glm::vec2 star1Pos = uiConfig.calculatePosition(star1PosConfig, width, height);
    glm::vec2 star2Pos = uiConfig.calculatePosition(star2PosConfig, width, height);
    glm::vec2 star3Pos = uiConfig.calculatePosition(star3PosConfig, width, height);
    
    glm::vec3 star1Color = (selectedType == GameProgressState::SecretStarType::MAX_SPEED_STAR) ? maxSpeedColor : unselectedColor;
    glm::vec3 star2Color = (selectedType == GameProgressState::SecretStarType::SHADOW_STAR) ? shadowColor : unselectedColor;
    glm::vec3 star3Color = (selectedType == GameProgressState::SecretStarType::IMMERSIVE_STAR) ? immersiveColor : unselectedColor;
    
    float starScale = 2.0f;
    
    renderStar(star1Pos, star1Color, starScale, width, height);
    renderStar(star2Pos, star2Color, starScale, width, height);
    renderStar(star3Pos, star3Color, starScale, width, height);
    
    auto name1Config = uiConfig.getSecretStarSelectionName1Config();
    auto name2Config = uiConfig.getSecretStarSelectionName2Config();
    auto name3Config = uiConfig.getSecretStarSelectionName3Config();
    
    glm::vec2 name1Pos = uiConfig.calculatePosition(name1Config.position, width, height);
    glm::vec2 name2Pos = uiConfig.calculatePosition(name2Config.position, width, height);
    glm::vec2 name3Pos = uiConfig.calculatePosition(name3Config.position, width, height);
    
    glm::vec3 name1Color = (selectedType == GameProgressState::SecretStarType::MAX_SPEED_STAR) ? name1Config.selectedColor : name1Config.unselectedColor;
    glm::vec3 name2Color = (selectedType == GameProgressState::SecretStarType::SHADOW_STAR) ? name2Config.selectedColor : name2Config.unselectedColor;
    glm::vec3 name3Color = (selectedType == GameProgressState::SecretStarType::IMMERSIVE_STAR) ? name3Config.selectedColor : name3Config.unselectedColor;
    
    renderText("MAX SPEED STAR", name1Pos, name1Color, name1Config.scale);
    renderText("SHADOW STAR", name2Pos, name2Color, name2Config.scale);
    renderText("IMMERSIVE STAR", name3Pos, name3Color, name3Config.scale);
    
    auto pressTConfig = uiConfig.getSecretStarSelectionPressTConfig();
    glm::vec2 pressTPos = uiConfig.calculatePosition(pressTConfig.position, width, height);
    renderText("PRESS T", pressTPos, pressTConfig.color, pressTConfig.scale);
    
    auto confirmConfig = uiConfig.getSecretStarSelectionConfirmConfig();
    glm::vec2 confirmPos = uiConfig.calculatePosition(confirmConfig.position, width, height);
    renderText("CONFIRM: ENTER", confirmPos, confirmConfig.color, confirmConfig.scale);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

} // namespace gfx
