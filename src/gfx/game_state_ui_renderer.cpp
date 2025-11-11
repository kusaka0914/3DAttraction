#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state_ui_renderer.h"
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
    
    // 座標系を1280x720に正規化
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    // 画面中央より少し下に表示
    float centerX = 640.0f;  // 1280/2
    float centerY = 500.0f;  // 画面中央より少し下
    
    // ステップ表示
    std::string stepText = "STEP " + std::to_string(currentStep + 1) + "/11";
    renderText(stepText, glm::vec2(centerX - 70, centerY - 50), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);

    std::string explainText = "";
    std::string explainText2 = "";
    std::string explainText3 = "";
    std::string explainText4 = "";
    if(currentStep == 5){
        std::string explainText = "YOUR SPEED AND THE SPEED OF THE GIMMICK WILL CHANGE !";
        renderText(explainText, glm::vec2(centerX - 50, centerY - 400), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    }else if(currentStep == 9){
        std::string explainText = "THERE ARE THREE ITEMS ON THE STAGE !";
        std::string explainText2 = "RED, BLUE, AND GREEN !";
        std::string explainText3 = "COLLECT THEM ALL TO REACH THE GOAL !";
        std::string explainText4 = "LET'S GO AND COLLECT THEM !";
        renderText(explainText, glm::vec2(centerX - 200, centerY + 50), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
        renderText(explainText2, glm::vec2(centerX - 120, centerY + 90), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
        renderText(explainText3, glm::vec2(centerX - 200, centerY + 130), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
        renderText(explainText4, glm::vec2(centerX - 150, centerY + 170), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    }else if(currentStep == 10){
        std::string explainText = "THE YELLOW BLOCK IS THE GOAL !";
        std::string explainText2 = "TOUCH THE GOAL WITH THREE ITEMS TO CLEAR THE STAGE !";
        std::string explainText3 = "LET'S TRY IT!";
        renderText(explainText, glm::vec2(centerX - 190, centerY + 50), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
        renderText(explainText2, glm::vec2(centerX - 300, centerY + 90), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
        renderText(explainText3, glm::vec2(centerX - 90, centerY + 130), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    }
    
    // メッセージ表示
    glm::vec3 messageColor = stepCompleted ? glm::vec3(0.2f, 0.8f, 0.2f) : glm::vec3(1.0f, 1.0f, 1.0f);
    if(currentStep < 6 ){
        renderText(message, glm::vec2(centerX - 200, centerY), messageColor, 1.5f);
    }else if(currentStep == 6){
        renderText(message, glm::vec2(centerX-50, centerY), messageColor, 1.5f);
    }else if(currentStep == 7){
        renderText(message, glm::vec2(centerX-90, centerY), messageColor, 1.5f);
    }else if(currentStep == 8){
        renderText(message, glm::vec2(centerX-50, centerY), messageColor, 1.5f);
    }else if(currentStep == 9){
        renderText(message, glm::vec2(centerX-50, centerY), messageColor, 1.5f);
    }else if(currentStep == 10){
        renderText(message, glm::vec2(centerX-50, centerY), messageColor, 1.5f);
    }
    
    // ステップ完了時のメッセージ
    if (stepCompleted && currentStep != 9) {
        renderText("PRESS ENTER TO CONTINUE", glm::vec2(centerX - 150, centerY + 50), glm::vec3(0.2f, 0.8f, 0.2f), 1.2f);
    }else if(stepCompleted && currentStep == 9){
        renderText("PRESS ENTER TO CONTINUE", glm::vec2(centerX + 50, centerY +5), glm::vec3(0.2f, 0.8f, 0.2f), 1.2f);
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
    
    // ステージクリアボックス（中央に配置）
    float boxWidth = 800.0f;
    float boxHeight = 500.0f;
    float boxX = (width - boxWidth) / 2.0f;
    float boxY = (height - boxHeight) / 2.0f;
    
    // 座標系を1280x720に正規化
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    // ステージクリアメッセージ
    if (earnedStars == 3) {
        renderText("STAGE COMPLETED!", glm::vec2((boxX + boxWidth/2 - 300) * scaleX, (boxY - 80) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 2.5f);
    } else {
        renderText("STAGE CLEAR!", glm::vec2((boxX + boxWidth/2 - 200) * scaleX, (boxY - 80) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 2.5f);
    }
    
    // クリアタイム
    renderText("CLEAR TIME: " + std::to_string((int)clearTime) + "s", 
               glm::vec2((boxX + 200) * scaleX, (boxY + 350) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.8f);
    
    // 星の表示（実際の星を3つ表示）
    for (int i = 0; i < 3; i++) {
        glm::vec2 starPos = glm::vec2((boxX + 750 + i * 600) * scaleX, (boxY + 720) * scaleY);
        glm::vec3 starColor;
        
        if (i < earnedStars) {
            // 獲得した星（黄色で点灯）
            starColor = glm::vec3(1.0f, 1.0f, 0.0f);
        } else {
            // まだ獲得していない星（灰色）
            starColor = glm::vec3(0.5f, 0.5f, 0.5f);
        }
        
        renderStar(starPos, starColor, 10.0f);
    }
    
    // ステージ選択フィールドに戻るボタン
    renderText("RETURN FIELD: ENTER", 
               glm::vec2((boxX + 130) * scaleX, (boxY + 500) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 1.0f);
    
    // リトライボタン
    renderText("RETRY: R", 
               glm::vec2((boxX + 550) * scaleX, (boxY + 500) * scaleY), glm::vec3(0.8f, 0.8f, 0.8f), 1.0f);
    
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
    
    // 座標系を1280x720に正規化
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    // タイトル
    renderText("STAGE " + std::to_string(targetStage) + " HAS BEEN UNLOCKED.", 
               glm::vec2((width/2 - 350) * scaleX, (height/2 - 200) * scaleY), glm::vec3(1.0f, 1.0f, 0.0f), 2.0f);
    
    // ワープ機能の説明
    renderText("YOU CAN WARP TO THE STAGE", 
               glm::vec2((width/2 - 300) * scaleX, (height/2 - 70) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    
    renderText("BY PRESSING KEY " + std::to_string(targetStage) + ".", 
               glm::vec2((width/2 - 200) * scaleX, (height/2 + 20) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    
    // ステージに入るボタン
    renderText("ENTER STAGE: ENTER", 
               glm::vec2((width/2 - 200) * scaleX, (height/2 + 150) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 1.5f);
    
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
    
    // 座標系を1280x720に正規化
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    // ゲームオーバーメッセージ
    renderText("GAME OVER!", glm::vec2((width/2 - 300) * scaleX, (height/2 - 100) * scaleY), glm::vec3(1.0f, 0.2f, 0.2f), 3.0f);
    
    // ステージ選択フィールドに戻るボタン
    renderText("RETURN FIELD: ENTER", 
               glm::vec2((width/2 - 300) * scaleX, (height/2 + 50) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 1.5f);
    
    // リトライボタン
    renderText("RETRY: R", 
               glm::vec2((width/2 - 150) * scaleX, (height/2 + 150) * scaleY), glm::vec3(0.8f, 0.8f, 0.8f), 1.5f);
    
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
    
    // エンディングメッセージの表示
    glm::vec3 textColor = glm::vec3(1.0f, 1.0f, 0.0f); // 黄色
    
    // メッセージ1
    renderText("THANK YOU FOR PLAYING!", glm::vec2(width/2 - 700, height/2 - 400), textColor, 2.0f);
    
    // メッセージ2
    renderText("AND CONGRATULATIONS ON CONQUERING WORLD 1!", glm::vec2(width/2 - 850, height/2 - 300), textColor, 1.8f);
    
    // メッセージ3
    renderText("SEE YOU AGAIN SOMEWHERE!", glm::vec2(width/2 - 700, height/2 - 200), textColor, 2.0f);
    
    // スキッププロンプトを右上に表示
    glm::vec3 skipColor = glm::vec3(0.7f, 0.7f, 0.7f); // 薄いグレー
    renderText("SKIP : ENTER", glm::vec2(width - 400, 30), skipColor, 1.2f);
    
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
    
    // 座標系を1280x720に正規化
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    // タイトル
    renderText("UNLOCK STAGE " + std::to_string(targetStage) + " ?", 
               glm::vec2((width/2 - 250) * scaleX, (height/2 - 200) * scaleY), glm::vec3(1.0f, 1.0f, 0.0f), 2.0f);
    
    // 必要な星数
    renderText("YOU MUST USE " + std::to_string(requiredStars) + " STARS !", 
               glm::vec2((width/2 - 250) * scaleX, (height/2 - 50) * scaleY), glm::vec3(1.0f, 0.8f, 0.2f), 1.5f);
    
    // 確認ボタン
    renderText("UNLOCK: ENTER", 
               glm::vec2((width/2 - 300) * scaleX, (height/2+100) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 1.5f);
    
    // キャンセルボタン
    renderText("CANCEL: SPACE", 
               glm::vec2((width/2 +100) * scaleX, (height/2+100) * scaleY), glm::vec3(0.8f, 0.2f, 0.2f), 1.5f);
    
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
    
    // 座標系を1280x720に正規化
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    // タイトル
    renderText("YOU CAN'T UNLOCK STAGE " + std::to_string(targetStage) + " !", 
               glm::vec2((width/2 - 400) * scaleX, (height/2 - 200) * scaleY), glm::vec3(1.0f, 0.2f, 0.2f), 2.0f);
    
    // 必要な星数
    renderText("YOU NEED " + std::to_string(requiredStars) + " STARS !", 
               glm::vec2((width/2 - 200) * scaleX, (height/2 - 70) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    
    // アドバイス
    renderText("COLLECT STARS ON OTHER STAGES !", 
               glm::vec2((width/2 - 400) * scaleX, (height/2 + 50) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    
    // 閉じるボタン
    renderText("OK: SPACE", 
               glm::vec2((width/2 - 100) * scaleX, (height/2 + 200) * scaleY), glm::vec3(0.2f, 1.0f, 0.2f), 1.5f);
    
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
    
    // "Ready?"メッセージ
    renderText("READY", glm::vec2((width/2 - 230) * scaleX, (height/2 - 300) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 5.0f);
    renderText("PLAY SPEED", glm::vec2((width/2 - 400) * scaleX, (height/2 - 70) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    
    // 速度選択（1x, 2x, 3x）
    std::vector<std::string> speedTexts = {"1x", "2x", "3x"};
    for (int i = 0; i < 3; i++) {
        glm::vec3 color;
        if (i == speedLevel) {
            color = glm::vec3(1.0f, 0.8f, 0.2f); // 金色（選択中）
        } else {
            color = glm::vec3(0.5f, 0.5f, 0.5f); // 灰色（未選択）
        }
        
        float xPos = (width/2 - 450 + i * 150) * scaleX;
        float yPos = (height/2) * scaleY;
        renderText(speedTexts[i], glm::vec2(xPos, yPos), color, 2.0f);
    }
    renderText("PRESS T", glm::vec2((width/2 - 350) * scaleX, (height/2 + 100) * scaleY), glm::vec3(1.0f, 0.8f, 0.2f), 1.2f);
    renderText("TPS FPS", glm::vec2((width/2 + 190) * scaleX, (height/2 - 70) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    // モード選択
    renderText("TPS", glm::vec2((width/2 + 130) * scaleX, (height/2) * scaleY), 
               isFirstPersonMode ? glm::vec3(0.5f, 0.5f, 0.5f) : glm::vec3(1.0f, 0.8f, 0.2f), 2.0f);
    renderText("FPS", glm::vec2((width/2 + 320) * scaleX, (height/2) * scaleY), 
               isFirstPersonMode ? glm::vec3(1.0f, 0.8f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), 2.0f);
    
    renderText("PRESS F", glm::vec2((width/2 + 210) * scaleX, (height/2 + 100) * scaleY), glm::vec3(1.0f, 0.8f, 0.2f), 1.2f);
    
    // "Enter"メッセージ
    renderText("CONFIRM: ENTER", glm::vec2((width/2- 150) * scaleX, (height/2 + 250) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 1.2f);
    
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
    
    // 座標系を1280x720に正規化
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    // カウントダウン数字
    std::string countText = std::to_string(count);
    renderText(countText, glm::vec2((width/2 - 100) * scaleX, (height/2 - 100) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 5.0f);
    
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
    
    // 座標系を1280x720に正規化（他のUIと同じ方式）
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
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
    renderText("THIS IS THE WORLD.", 
               glm::vec2((width/2 - 250) * scaleX, (height/2 - 250) * scaleY), 
               glm::vec3(1.0f, 1.0f, 1.0f), 2.0f);
    
    renderText("THERE ARE ENTRANCES TO EACH STAGE.", 
               glm::vec2((width/2 - 380) * scaleX, (height/2 - 120) * scaleY), 
               glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    
    renderText("EACH STAGE IS LOCKED AT FIRST", 
               glm::vec2((width/2 - 320) * scaleX, (height/2 - 20) * scaleY), 
               glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    
    renderText("SO YOU NEED TO UNLOCK IT USING STARS YOU'VE EARNED.", 
                glm::vec2((width/2 - 580) * scaleX, (height/2 + 80) * scaleY), 
                glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    
    renderText("COLLECT LOTS OF STARS TO PROGRESS!", 
               glm::vec2((width/2 - 400) * scaleX, (height/2 + 180) * scaleY), 
               glm::vec3(1.0f, 0.8f, 0.2f), 1.5f); // 金色で強調
    
    renderText("OK : ENTER", 
               glm::vec2((width/2 - 80) * scaleX, (height/2 + 280) * scaleY), 
               glm::vec3(0.2f, 0.8f, 0.2f), 1.5f); // 緑色でENTERボタン
    
    glDisable(GL_BLEND);
}

void GameStateUIRenderer::renderStageSelectionAssist(int width, int height, int targetStage, bool isVisible, bool isUnlocked) {
    if (!isVisible) {
        return; // 非表示の場合は何も描画しない
    }
    
    // フォントの初期化を確実に行う
    font.initialize();
    
    // 座標系を1280x720に正規化（他のUIと同じ方式）
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
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
    
    renderText(assistText, 
               glm::vec2((width/2 - 200) * scaleX, (height/2 + 100) * scaleY), 
               glm::vec3(1.0f, 1.0f, 1.0f), 1.3f); // 白色で表示
    
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
    
    // スタッフロールの表示（画面中央から開始して上にスクロール）
    float scrollOffset = timer * 100.0f; // 1秒で100ピクセル上にスクロール
    glm::vec3 textColor = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::vec3 skipColor = glm::vec3(1.0f, 1.0f, 1.0f); // 薄いグレー
    renderText("SKIP : ENTER", glm::vec2(width/2 - 70, height/2 -650), skipColor, 1.5f);
    
    // タイトル（画面中央から開始）
    renderText("STAFF ROLL", glm::vec2(width/2 - 600, height/2 - scrollOffset), textColor, 2.5f);
    
    // 各スタッフ項目（画面中央から開始）
    renderText("DIRECTOR", glm::vec2(width/2 - 750, height/2 - scrollOffset + 100), textColor, 2.0f);
    renderText("TAKUMI KUSAKA", glm::vec2(width/2 - 500, height/2 - scrollOffset + 100), textColor, 2.0f);
    
    renderText("PROGRAMMER", glm::vec2(width/2 - 750, height/2 - scrollOffset + 200), textColor, 2.0f);
    renderText("TAKUMI KUSAKA", glm::vec2(width/2 - 500, height/2 - scrollOffset + 200), textColor, 2.0f);
    
    renderText("GRAPHICS", glm::vec2(width/2 - 750, height/2 - scrollOffset + 300), textColor, 2.0f);
    renderText("TAKUMI KUSAKA", glm::vec2(width/2 - 500, height/2 - scrollOffset + 300), textColor, 2.0f);
    
    renderText("SOUND", glm::vec2(width/2 - 750, height/2 - scrollOffset + 400), textColor, 2.0f);
    renderText("TAKUMI KUSAKA", glm::vec2(width/2 - 500, height/2 - scrollOffset + 400), textColor, 2.0f);
    
    renderText("GAMEDESIGN", glm::vec2(width/2 - 750, height/2 - scrollOffset + 500), textColor, 2.0f);
    renderText("TAKUMI KUSAKA", glm::vec2(width/2 - 500, height/2 - scrollOffset + 500), textColor, 2.0f);
    
    renderText("DEBUG", glm::vec2(width/2 - 750, height/2 - scrollOffset + 600), textColor, 2.0f);
    renderText("TAKUMI KUSAKA", glm::vec2(width/2 - 500, height/2 - scrollOffset + 600), textColor, 2.0f);
    
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

void GameStateUIRenderer::renderStar(const glm::vec2& position, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    // 星の中心点
    float centerX = position.x;
    float centerY = position.y;
    
    // 星の5つの角を描画（塗りつぶし）
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; i++) {
        float angle1 = i * 72.0f * 3.14159f / 180.0f;
        float angle2 = (i + 2) * 72.0f * 3.14159f / 180.0f;
        
        // 外側の点
        float x1 = centerX + cos(angle1) * 12.0f * scale;
        float y1 = centerY + sin(angle1) * 12.0f * scale;
        
        // 内側の点
        float x2 = centerX + cos(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scale;
        float y2 = centerY + sin(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scale;
        
        // 次の外側の点
        float x3 = centerX + cos(angle2) * 12.0f * scale;
        float y3 = centerY + sin(angle2) * 12.0f * scale;
        
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
    
    // 座標系を1280x720に正規化
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    // タイトル
    renderText("DIFFICULTY SELECTION", 
               glm::vec2((width/2 - 350) * scaleX, (height/2 - 300) * scaleY), glm::vec3(1.0f, 1.0f, 0.0f), 2.0f);
    
    // 説明文
    renderText("YOU CAN CHOOSE BETWEEN EASY AND NORMAL MODES.", 
               glm::vec2((width/2 - 550) * scaleX, (height/2 - 200) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    
    renderText("IN EASY MODE,", 
               glm::vec2((width/2 - 150) * scaleX, (height/2 - 80) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    
    renderText("1. NO LIVES ARE LOST.", 
               glm::vec2((width/2 - 400) * scaleX, (height/2) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    
    renderText("2. THE TIME LIMIT IS INCREASED BY 20 SECONDS.", 
               glm::vec2((width/2 - 400) * scaleX, (height/2 + 60) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    
    renderText("3. PRESS SPACE TWICE TO JUMP AGAIN IN THE AIR.", 
               glm::vec2((width/2 - 400) * scaleX, (height/2 + 120) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    
    renderText("4. IF YOU FALL, YOU CAN RESUME FROM THE PLATFORM", 
               glm::vec2((width/2 - 400) * scaleX, (height/2 + 180) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    
    renderText("   YOU WERE LAST STANDING ON.", 
               glm::vec2((width/2 - 380) * scaleX, (height/2 + 240) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    
    // 続行ボタン
    renderText("OK : ENTER", 
               glm::vec2((width/2 - 150) * scaleX, (height/2 + 320) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 1.5f);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GameStateUIRenderer::renderModeSelectionUI(int width, int height, bool isEasyMode) {
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
    
    // タイトル
    renderText("SELECT DIFFICULTY", 
               glm::vec2((width/2 - 300) * scaleX, (height/2 - 200) * scaleY), glm::vec3(1.0f, 1.0f, 0.0f), 2.0f);
    
    // EASY/NORMALの表示（選択された方がオレンジ色、選択されていない方がグレー）
    renderText("EASY", glm::vec2((width/2 - 200) * scaleX, (height/2-50) * scaleY), 
               isEasyMode ? glm::vec3(1.0f, 0.8f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), 2.0f);
    renderText("NORMAL", glm::vec2((width/2 + 20) * scaleX, (height/2-50) * scaleY), 
               isEasyMode ? glm::vec3(0.5f, 0.5f, 0.5f) : glm::vec3(1.0f, 0.8f, 0.2f), 2.0f);
    
    // PRESS E表示
    renderText("PRESS E", glm::vec2((width/2 - 80) * scaleX, (height/2 + 50) * scaleY), glm::vec3(1.0f, 0.8f, 0.2f), 1.2f);
    
    renderText("CONFIRM: ENTER", 
               glm::vec2((width/2 - 200) * scaleX, (height/2 + 150) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 1.5f);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

} // namespace gfx
