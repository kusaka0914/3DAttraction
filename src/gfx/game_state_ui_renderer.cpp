#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state_ui_renderer.h"
#include "../core/utils/ui_config_manager.h"
#include <iostream>
#include <cmath>

namespace gfx {

GameStateUIRenderer::GameStateUIRenderer() {
    font.initialize();
}

GameStateUIRenderer::~GameStateUIRenderer() {
}

void GameStateUIRenderer::renderTutorialStageUI(int width, int height, const std::string& message, int currentStep, bool stepCompleted) {
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 2D描画モードに切り替え
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
    
    // 画面中央より少し下に表示
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    
    // ステップ表示
    auto stepTextConfig = uiConfig.getTutorialStepTextConfig();
    glm::vec2 stepTextPos = uiConfig.calculatePosition(stepTextConfig.position, width, height);
    std::string stepText = "STEP " + std::to_string(currentStep + 1) + "/11";
    renderText(stepText, stepTextPos, stepTextConfig.color, stepTextConfig.scale);

    // ステップ固有の説明テキスト表示
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
        std::string explainText2 = "RED, BLUE, AND GREEN !";
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
    
    // メッセージ表示
    auto messageConfig = uiConfig.getTutorialMessageConfig();
    glm::vec2 messagePos = uiConfig.calculatePosition(messageConfig.position, width, height);
    glm::vec3 messageColor = stepCompleted ? messageConfig.completedColor : messageConfig.color;
    renderText(message, messagePos, messageColor, messageConfig.scale);
    
    // ステップ完了時のメッセージ
    if (stepCompleted) {
        auto pressEnterConfig = uiConfig.getTutorialPressEnterConfig();
        glm::vec2 pressEnterPos = uiConfig.calculatePosition(pressEnterConfig.position, width, height);
        if (currentStep == 9) {
            // ステップ9の場合は位置を少し調整
            pressEnterPos.x += 200.0f;
            pressEnterPos.y -= 45.0f;
        }
        renderText("PRESS ENTER TO CONTINUE", pressEnterPos, pressEnterConfig.color, pressEnterConfig.scale);
    }
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderStageClearBackground(int width, int height, float clearTime, int earnedStars) {
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 背景オーバーレイ（半透明の黒）
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
    
    // ステージクリアメッセージ
    if (earnedStars == 3) {
        auto completedConfig = uiConfig.getStageClearCompletedTextConfig();
        glm::vec2 completedPos = uiConfig.calculatePosition(completedConfig.position, width, height);
        renderText("STAGE COMPLETED!", completedPos, completedConfig.color, completedConfig.scale);
    } else {
        auto clearConfig = uiConfig.getStageClearClearTextConfig();
        glm::vec2 clearPos = uiConfig.calculatePosition(clearConfig.position, width, height);
        renderText("STAGE CLEAR!", clearPos, clearConfig.color, clearConfig.scale);
    }
    
    // クリアタイム（少数第2位まで表示）
    int clearTimeInt = static_cast<int>(clearTime);
    int clearTimeDecimal = static_cast<int>((clearTime - clearTimeInt) * 100);
    std::string clearTimeText = std::to_string(clearTimeInt) + "." + 
                               (clearTimeDecimal < 10 ? "0" : "") + std::to_string(clearTimeDecimal) + "s";
    auto clearTimeConfig = uiConfig.getStageClearClearTimeConfig();
    glm::vec2 clearTimePos = uiConfig.calculatePosition(clearTimeConfig.position, width, height);
    renderText("CLEAR TIME: " + clearTimeText, clearTimePos, clearTimeConfig.color, clearTimeConfig.scale);
    
    // 星の表示（実際の星を3つ表示）
    auto starsConfig = uiConfig.getStageClearStarsConfig();
    glm::vec2 starsBasePos = uiConfig.calculatePosition(starsConfig.position, width, height);
    for (int i = 0; i < 3; i++) {
        glm::vec2 starPos = glm::vec2(starsBasePos.x + i * starsConfig.spacing, starsBasePos.y);
        glm::vec3 starColor = (i < earnedStars) ? starsConfig.selectedColor : starsConfig.unselectedColor;
        renderStar(starPos, starColor, starsConfig.scale, width, height);
    }
    
    // ステージ選択フィールドに戻るボタン
    auto returnFieldConfig = uiConfig.getStageClearReturnFieldConfig();
    glm::vec2 returnFieldPos = uiConfig.calculatePosition(returnFieldConfig.position, width, height);
    renderText("RETURN FIELD: ENTER", returnFieldPos, returnFieldConfig.color, returnFieldConfig.scale);
    
    // リトライボタン
    auto retryConfig = uiConfig.getStageClearRetryConfig();
    glm::vec2 retryPos = uiConfig.calculatePosition(retryConfig.position, width, height);
    renderText("RETRY: R", retryPos, retryConfig.color, retryConfig.scale);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderWarpTutorialBackground(int width, int height, int targetStage) {
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 背景オーバーレイ（半透明の黒）
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
    
    // タイトル
    auto titleConfig = uiConfig.getWarpTutorialTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("STAGE " + std::to_string(targetStage) + " HAS BEEN UNLOCKED.", titlePos, titleConfig.color, titleConfig.scale);
    
    // ワープ機能の説明
    auto desc1Config = uiConfig.getWarpTutorialDescription1Config();
    glm::vec2 desc1Pos = uiConfig.calculatePosition(desc1Config.position, width, height);
    renderText("YOU CAN WARP TO THE STAGE", desc1Pos, desc1Config.color, desc1Config.scale);
    
    auto desc2Config = uiConfig.getWarpTutorialDescription2Config();
    glm::vec2 desc2Pos = uiConfig.calculatePosition(desc2Config.position, width, height);
    renderText("BY PRESSING KEY " + std::to_string(targetStage) + ".", desc2Pos, desc2Config.color, desc2Config.scale);
    
    // ステージに入るボタン
    auto enterButtonConfig = uiConfig.getWarpTutorialEnterButtonConfig();
    glm::vec2 enterButtonPos = uiConfig.calculatePosition(enterButtonConfig.position, width, height);
    renderText("ENTER STAGE: ENTER", enterButtonPos, enterButtonConfig.color, enterButtonConfig.scale);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderGameOverBackground(int width, int height) {
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 背景オーバーレイ（半透明の黒）
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
    
    // ゲームオーバーメッセージ
    auto gameOverTextConfig = uiConfig.getGameOverTextConfig();
    glm::vec2 gameOverTextPos = uiConfig.calculatePosition(gameOverTextConfig.position, width, height);
    renderText("GAME OVER!", gameOverTextPos, gameOverTextConfig.color, gameOverTextConfig.scale);
    
    // ステージ選択フィールドに戻るボタン
    auto returnFieldConfig = uiConfig.getGameOverReturnFieldConfig();
    glm::vec2 returnFieldPos = uiConfig.calculatePosition(returnFieldConfig.position, width, height);
    renderText("RETURN FIELD: ENTER", returnFieldPos, returnFieldConfig.color, returnFieldConfig.scale);
    
    // リトライボタン
    auto retryConfig = uiConfig.getGameOverRetryConfig();
    glm::vec2 retryPos = uiConfig.calculatePosition(retryConfig.position, width, height);
    renderText("RETRY: R", retryPos, retryConfig.color, retryConfig.scale);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderEndingMessage(int width, int height, float timer) {
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 黒い背景で暗転
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    // メッセージ1
    auto thankYouConfig = uiConfig.getEndingThankYouConfig();
    glm::vec2 thankYouPos = uiConfig.calculatePosition(thankYouConfig.position, width, height);
    renderText("THANK YOU FOR PLAYING!", thankYouPos, thankYouConfig.color, thankYouConfig.scale);
    
    // メッセージ2
    auto congratulationsConfig = uiConfig.getEndingCongratulationsConfig();
    glm::vec2 congratulationsPos = uiConfig.calculatePosition(congratulationsConfig.position, width, height);
    renderText("AND CONGRATULATIONS ON CONQUERING WORLD 1!", congratulationsPos, congratulationsConfig.color, congratulationsConfig.scale);
    
    // メッセージ3
    auto seeYouAgainConfig = uiConfig.getEndingSeeYouAgainConfig();
    glm::vec2 seeYouAgainPos = uiConfig.calculatePosition(seeYouAgainConfig.position, width, height);
    renderText("SEE YOU AGAIN SOMEWHERE!", seeYouAgainPos, seeYouAgainConfig.color, seeYouAgainConfig.scale);
    
    // スキッププロンプトを右上に表示
    auto skipConfig = uiConfig.getEndingSkipConfig();
    glm::vec2 skipPos = uiConfig.calculatePosition(skipConfig.position, width, height);
    renderText("SKIP : ENTER", skipPos, skipConfig.color, skipConfig.scale);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderUnlockConfirmBackground(int width, int height, int targetStage, int requiredStars, int currentStars) {
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 背景オーバーレイ（半透明の黒）
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
    
    // タイトル
    auto titleConfig = uiConfig.getUnlockConfirmTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("UNLOCK STAGE " + std::to_string(targetStage) + " ?", titlePos, titleConfig.color, titleConfig.scale);
    
    // 必要な星数
    auto requiredStarsConfig = uiConfig.getUnlockConfirmRequiredStarsConfig();
    glm::vec2 requiredStarsPos = uiConfig.calculatePosition(requiredStarsConfig.position, width, height);
    renderText("YOU MUST USE " + std::to_string(requiredStars) + " STARS !", requiredStarsPos, requiredStarsConfig.color, requiredStarsConfig.scale);
    
    // 確認ボタン
    auto unlockButtonConfig = uiConfig.getUnlockConfirmUnlockButtonConfig();
    glm::vec2 unlockButtonPos = uiConfig.calculatePosition(unlockButtonConfig.position, width, height);
    renderText("UNLOCK: ENTER", unlockButtonPos, unlockButtonConfig.color, unlockButtonConfig.scale);
    
    // キャンセルボタン
    auto cancelButtonConfig = uiConfig.getUnlockConfirmCancelButtonConfig();
    glm::vec2 cancelButtonPos = uiConfig.calculatePosition(cancelButtonConfig.position, width, height);
    renderText("CANCEL: SPACE", cancelButtonPos, cancelButtonConfig.color, cancelButtonConfig.scale);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderStarInsufficientBackground(int width, int height, int targetStage, int requiredStars, int currentStars) {
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 背景オーバーレイ（半透明の黒）
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
    
    // タイトル
    auto titleConfig = uiConfig.getStarInsufficientTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("YOU CAN'T UNLOCK STAGE " + std::to_string(targetStage) + " !", titlePos, titleConfig.color, titleConfig.scale);
    
    // 必要な星数
    auto requiredStarsConfig = uiConfig.getStarInsufficientRequiredStarsConfig();
    glm::vec2 requiredStarsPos = uiConfig.calculatePosition(requiredStarsConfig.position, width, height);
    renderText("YOU NEED " + std::to_string(requiredStars) + " STARS !", requiredStarsPos, requiredStarsConfig.color, requiredStarsConfig.scale);
    
    // アドバイス
    auto collectStarsConfig = uiConfig.getStarInsufficientCollectStarsConfig();
    glm::vec2 collectStarsPos = uiConfig.calculatePosition(collectStarsConfig.position, width, height);
    renderText("COLLECT STARS ON OTHER STAGES !", collectStarsPos, collectStarsConfig.color, collectStarsConfig.scale);
    
    // 閉じるボタン
    auto okButtonConfig = uiConfig.getStarInsufficientOkButtonConfig();
    glm::vec2 okButtonPos = uiConfig.calculatePosition(okButtonConfig.position, width, height);
    renderText("OK: SPACE", okButtonPos, okButtonConfig.color, okButtonConfig.scale);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderReadyScreen(int width, int height, int speedLevel, bool isFirstPersonMode) {
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 背景オーバーレイ（半透明の黒）
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
    
    // 座標系を1280x720に正規化
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    // "Ready?"メッセージ
    auto readyTextConfig = uiConfig.getReadyTextConfig();
    glm::vec2 readyTextPos = uiConfig.calculatePosition(readyTextConfig.position, width, height);
    renderText("READY", glm::vec2(readyTextPos.x * scaleX, readyTextPos.y * scaleY), readyTextConfig.color, readyTextConfig.scale);
    
    // "PLAY SPEED"ラベル
    auto playSpeedLabelConfig = uiConfig.getPlaySpeedLabelConfig();
    glm::vec2 playSpeedLabelPos = uiConfig.calculatePosition(playSpeedLabelConfig.position, width, height);
    renderText("PLAY SPEED", glm::vec2(playSpeedLabelPos.x * scaleX, playSpeedLabelPos.y * scaleY), playSpeedLabelConfig.color, playSpeedLabelConfig.scale);
    
    // 速度選択（1x, 2x, 3x）
    auto speedOptionsConfig = uiConfig.getSpeedOptionsConfig();
    glm::vec2 speedOptionsBasePos = uiConfig.calculatePosition(speedOptionsConfig.position, width, height);
    std::vector<std::string> speedTexts = {"1x", "2x", "3x"};
    for (int i = 0; i < 3; i++) {
        glm::vec3 color = (i == speedLevel) ? speedOptionsConfig.selectedColor : speedOptionsConfig.unselectedColor;
        float xPos = (speedOptionsBasePos.x + i * speedOptionsConfig.spacing) * scaleX;
        float yPos = speedOptionsBasePos.y * scaleY;
        renderText(speedTexts[i], glm::vec2(xPos, yPos), color, speedOptionsConfig.scale);
    }
    
    // "PRESS T"
    auto readyPressTConfig = uiConfig.getReadyPressTConfig();
    glm::vec2 readyPressTPos = uiConfig.calculatePosition(readyPressTConfig.position, width, height);
    renderText("PRESS T", glm::vec2(readyPressTPos.x * scaleX, readyPressTPos.y * scaleY), readyPressTConfig.color, readyPressTConfig.scale);
    
    // "TPS FPS"ラベル
    auto tpsFpsLabelConfig = uiConfig.getTpsFpsLabelConfig();
    glm::vec2 tpsFpsLabelPos = uiConfig.calculatePosition(tpsFpsLabelConfig.position, width, height);
    renderText("TPS FPS", glm::vec2(tpsFpsLabelPos.x * scaleX, tpsFpsLabelPos.y * scaleY), tpsFpsLabelConfig.color, tpsFpsLabelConfig.scale);
    
    // モード選択（TPS/FPS）
    auto tpsOptionConfig = uiConfig.getTpsOptionConfig();
    auto fpsOptionConfig = uiConfig.getFpsOptionConfig();
    glm::vec2 tpsPos = uiConfig.calculatePosition(tpsOptionConfig.position, width, height);
    glm::vec2 fpsPos = uiConfig.calculatePosition(fpsOptionConfig.position, width, height);
    renderText("TPS", glm::vec2(tpsPos.x * scaleX, tpsPos.y * scaleY), 
               isFirstPersonMode ? tpsOptionConfig.unselectedColor : tpsOptionConfig.selectedColor, tpsOptionConfig.scale);
    renderText("FPS", glm::vec2(fpsPos.x * scaleX, fpsPos.y * scaleY), 
               isFirstPersonMode ? fpsOptionConfig.selectedColor : fpsOptionConfig.unselectedColor, fpsOptionConfig.scale);
    
    // "PRESS F"
    auto readyPressFConfig = uiConfig.getReadyPressFConfig();
    glm::vec2 readyPressFPos = uiConfig.calculatePosition(readyPressFConfig.position, width, height);
    renderText("PRESS F", glm::vec2(readyPressFPos.x * scaleX, readyPressFPos.y * scaleY), readyPressFConfig.color, readyPressFConfig.scale);
    
    // "CONFIRM: ENTER"メッセージ
    auto confirmConfig = uiConfig.getConfirmConfig();
    glm::vec2 confirmPos = uiConfig.calculatePosition(confirmConfig.position, width, height);
    renderText("CONFIRM: ENTER", glm::vec2(confirmPos.x * scaleX, confirmPos.y * scaleY), confirmConfig.color, confirmConfig.scale);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderCountdown(int width, int height, int count) {
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 背景オーバーレイ（半透明の黒）
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
    
    // カウントダウン数字
    std::string countText = std::to_string(count);
    renderText(countText, countdownPos, countdownConfig.color, countdownConfig.scale);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderStage0Tutorial(int width, int height) {
    // フォントの初期化を確実に行う
    font.initialize();
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    // 半透明の背景
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 背景の四角形（画面全体）
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    
    // テキストの描画
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
    
    glDisable(GL_BLEND);
}

void GameStateUIRenderer::renderStageSelectionAssist(int width, int height, int targetStage, bool isVisible, bool isUnlocked) {
    if (!isVisible) {
        return; // 非表示の場合は何も描画しない
    }
    
    // フォントの初期化を確実に行う
    font.initialize();
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    auto assistConfig = uiConfig.getStageSelectionAssistTextConfig();
    glm::vec2 assistPos = uiConfig.calculatePosition(assistConfig.position, width, height);
    
    // 半透明の背景
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    // テキストの描画（画面中央少し下）
    std::string assistText;
    if (targetStage == 6 || isUnlocked) {
        // ステージ1は常に解放済み、または解放済みのステージ
        assistText = "ENTER STAGE " + std::to_string(targetStage) + " : ENTER";
    } else { 
        // 未解放のステージ
        assistText = "UNLOCK STAGE " + std::to_string(targetStage) + " : ENTER";
    }
    
    renderText(assistText, assistPos, assistConfig.color, assistConfig.scale);
    
    glDisable(GL_BLEND);
}

void GameStateUIRenderer::renderStaffRoll(int width, int height, float timer) {
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 黒い背景で暗転
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    
    // スタッフロールの表示（画面中央から開始して上にスクロール）
    float scrollOffset = timer * uiConfig.getStaffRollSpacing(); // 設定ファイルから間隔を取得
    float spacing = uiConfig.getStaffRollSpacing();
    
    auto skipConfig = uiConfig.getStaffRollSkipConfig();
    glm::vec2 skipPos = uiConfig.calculatePosition(skipConfig.position, width, height);
    renderText("SKIP : ENTER", skipPos, skipConfig.color, skipConfig.scale);
    
    // タイトル（画面中央から開始）
    auto titleConfig = uiConfig.getStaffRollTitleConfig();
    glm::vec2 titleBasePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("STAFF ROLL", glm::vec2(titleBasePos.x, titleBasePos.y - scrollOffset), titleConfig.color, titleConfig.scale);
    
    // 各スタッフ項目（画面中央から開始）
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
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::begin2DMode() {
    // 2D描画モードに切り替え
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
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale) {
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1280, 720, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 色を設定
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
        
        // ビットマップフォントで文字を描画
        renderBitmapChar(c, glm::vec2(currentX, position.y), color, scale);
        currentX += charWidth + GameConstants::RenderConstants::CHAR_SPACING * scale;  // 文字間隔を増加
    }
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale) {
    if (!font.hasCharacter(c)) {
        // 文字が見つからない場合は空白として扱う
        return;
    }
    
    const std::vector<bool>& charData = font.getCharacter(c);
    
    glColor3f(color.r, color.g, color.b);
    
    // 8x12のビットマップを描画
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
    // 1280x720基準のスケーリングを適用
    float scaleX = static_cast<float>(width) / 1280.0f;
    float scaleY = static_cast<float>(height) / 720.0f;
    float scaledScale = scale * std::min(scaleX, scaleY);
    
    glColor3f(color.r, color.g, color.b);
    
    // 星の中心点（positionは既にスケーリング済みなのでそのまま使用）
    float centerX = position.x;
    float centerY = position.y;
    
    // 星の5つの角を描画（塗りつぶし）
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; i++) {
        float angle1 = i * 72.0f * 3.14159f / 180.0f;
        float angle2 = (i + 2) * 72.0f * 3.14159f / 180.0f;
        
        // 外側の点（scaledScaleを使用）
        float x1 = centerX + cos(angle1) * 12.0f * scaledScale;
        float y1 = centerY + sin(angle1) * 12.0f * scaledScale;
        
        // 内側の点（scaledScaleを使用）
        float x2 = centerX + cos(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scaledScale;
        float y2 = centerY + sin(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scaledScale;
        
        // 次の外側の点（scaledScaleを使用）
        float x3 = centerX + cos(angle2) * 12.0f * scaledScale;
        float y3 = centerY + sin(angle2) * 12.0f * scaledScale;
        
        // 中心から各点への三角形を描画（塗りつぶし）
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
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 背景オーバーレイ（半透明の黒）
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
    
    // タイトル
    auto titleConfig = uiConfig.getEasyModeExplanationTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("DIFFICULTY SELECTION", titlePos, titleConfig.color, titleConfig.scale);
    
    // 説明文
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
    
    // 続行ボタン
    auto okButtonConfig = uiConfig.getEasyModeExplanationOkButtonConfig();
    glm::vec2 okButtonPos = uiConfig.calculatePosition(okButtonConfig.position, width, height);
    renderText("OK : ENTER", okButtonPos, okButtonConfig.color, okButtonConfig.scale);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderEasyModeSelectionUI(int width, int height, bool isEasyMode) {
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 背景オーバーレイ（半透明の黒）
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
    
    // タイトル
    auto titleConfig = uiConfig.getModeSelectionTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("SELECT MODE", titlePos, titleConfig.color, titleConfig.scale);
    
    // NORMAL/EASYの表示（選択された方がオレンジ色、選択されていない方がグレー）
    auto normalConfig = uiConfig.getModeSelectionNormalTextConfig();
    glm::vec2 normalPos = uiConfig.calculatePosition(normalConfig.position, width, height);
    glm::vec3 normalColor = isEasyMode ? normalConfig.unselectedColor : normalConfig.selectedColor;
    renderText("NORMAL", normalPos, normalColor, normalConfig.scale);
    
    auto easyConfig = uiConfig.getModeSelectionEasyTextConfig();
    glm::vec2 easyPos = uiConfig.calculatePosition(easyConfig.position, width, height);
    glm::vec3 easyColor = isEasyMode ? easyConfig.selectedColor : easyConfig.unselectedColor;
    renderText("EASY", easyPos, easyColor, easyConfig.scale);
    
    // PRESS T表示
    auto pressTConfig = uiConfig.getModeSelectionPressTConfig();
    glm::vec2 pressTPos = uiConfig.calculatePosition(pressTConfig.position, width, height);
    renderText("PRESS T", pressTPos, pressTConfig.color, pressTConfig.scale);
    
    auto confirmConfig = uiConfig.getModeSelectionConfirmConfig();
    glm::vec2 confirmPos = uiConfig.calculatePosition(confirmConfig.position, width, height);
    renderText("CONFIRM: ENTER", confirmPos, confirmConfig.color, confirmConfig.scale);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderTimeAttackSelectionUI(int width, int height, bool isTimeAttackMode) {
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 背景オーバーレイ（半透明の黒）
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
    
    // タイトル
    auto titleConfig = uiConfig.getModeSelectionTitleConfig();
    glm::vec2 titlePos = uiConfig.calculatePosition(titleConfig.position, width, height);
    renderText("SELECT MODE", titlePos, titleConfig.color, titleConfig.scale);
    
    // NORMAL/TIME ATTACKの表示（選択された方がオレンジ色、選択されていない方がグレー）
    auto normalConfig = uiConfig.getModeSelectionNormalTextConfig();
    glm::vec2 normalPos = uiConfig.calculatePosition(normalConfig.position, width, height);
    glm::vec3 normalColor = isTimeAttackMode ? normalConfig.unselectedColor : normalConfig.selectedColor;
    renderText("NORMAL", normalPos, normalColor, normalConfig.scale);
    
    auto timeAttackConfig = uiConfig.getModeSelectionTimeAttackTextConfig();
    glm::vec2 timeAttackPos = uiConfig.calculatePosition(timeAttackConfig.position, width, height);
    glm::vec3 timeAttackColor = isTimeAttackMode ? timeAttackConfig.selectedColor : timeAttackConfig.unselectedColor;
    renderText("TIME ATTACK", timeAttackPos, timeAttackColor, timeAttackConfig.scale);
    
    // PRESS T表示
    auto pressTConfig = uiConfig.getModeSelectionPressTConfig();
    glm::vec2 pressTPos = uiConfig.calculatePosition(pressTConfig.position, width, height);
    renderText("PRESS T", pressTPos, pressTConfig.color, pressTConfig.scale);
    
    auto confirmConfig = uiConfig.getModeSelectionConfirmConfig();
    glm::vec2 confirmPos = uiConfig.calculatePosition(confirmConfig.position, width, height);
    renderText("CONFIRM: ENTER", confirmPos, confirmConfig.color, confirmConfig.scale);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

} // namespace gfx
