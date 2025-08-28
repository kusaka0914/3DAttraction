#include "opengl_renderer.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace gfx {

OpenGLRenderer::OpenGLRenderer() : window(nullptr), fontInitialized(false) {
}

OpenGLRenderer::~OpenGLRenderer() {
    cleanup();
}

bool OpenGLRenderer::initialize(GLFWwindow* window) {
    this->window = window;
    
    // OpenGLコンテキストを作成
    glfwMakeContextCurrent(window);
    
    // ビューポート設定
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    // 深度テスト有効化
    glEnable(GL_DEPTH_TEST);
    
    // ビットマップフォント初期化
    initializeBitmapFont();
    
    // カメラとプロジェクションの初期設定は削除（simple_main.cppで設定される）
    // setCamera(glm::vec3(0, 5, 10), glm::vec3(0, 0, 0));
    // setProjection(45.0f, (float)width / (float)height, 0.1f, 100.0f);
    
    return true;
}

void OpenGLRenderer::cleanup() {
    // OpenGL 2.1では特別なクリーンアップは不要
}

void OpenGLRenderer::beginFrame() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // プロジェクション行列を設定
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projectionMatrix));
    
    // ビュー行列を設定
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(viewMatrix));
}

void OpenGLRenderer::endFrame() {
    glfwSwapBuffers(window);
}

void OpenGLRenderer::renderCube(const glm::vec3& position, const glm::vec3& color, float size) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(size));
    drawCube(model, color);
}

void OpenGLRenderer::renderBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, size);
    drawCube(model, color);
}

void OpenGLRenderer::renderRotatedBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                                     const glm::vec3& rotationAxis, float rotationAngle) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
    model = glm::scale(model, size);
    drawCube(model, color);
}

void OpenGLRenderer::renderBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, size);
    
    // 透明度を適用した色
    glm::vec3 alphaColor = color * alpha;
    drawCube(model, alphaColor);
}

void OpenGLRenderer::renderRotatedBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                                              const glm::vec3& rotationAxis, float rotationAngle, float alpha) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
    model = glm::scale(model, size);
    
    // 透明度を適用した色
    glm::vec3 alphaColor = color * alpha;
    drawCube(model, alphaColor);
}



void OpenGLRenderer::renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale) {
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
    float charWidth = 8.0f * scale;
    float charHeight = 12.0f * scale;
    float spaceWidth = 6.0f * scale;  // スペース幅を増加
    
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        
        if (c == ' ') {
            currentX += spaceWidth;
            continue;
        }
        
        // ビットマップフォントで文字を描画
        renderBitmapChar(c, glm::vec2(currentX, position.y), color, scale);
        currentX += charWidth + 2.0f * scale;  // 文字間隔を増加
    }
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}



void OpenGLRenderer::setCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
    viewMatrix = glm::lookAt(position, target, up);
}

void OpenGLRenderer::setProjection(float fov, float aspect, float near, float far) {
    projectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
}

void OpenGLRenderer::drawCube(const glm::mat4& model, const glm::vec3& color) {
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(model));
    
    glColor3f(color.r, color.g, color.b);
    
    // キューブの各面を描画
    glBegin(GL_QUADS);
    
    // 前面
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    
    // 背面
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    
    // 上面
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    
    // 下面
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    
    // 右面
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    
    // 左面
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    
    glEnd();
    
    glPopMatrix();
}



// 制限時間UI表示
void OpenGLRenderer::renderTimeUI(float remainingTime, float timeLimit, int earnedStars, int existingStars) {
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
    
    // 残り時間表示（右上、より見やすい位置に調整）
    std::string timeText = std::to_string(static_cast<int>(remainingTime)) + "s";
    glm::vec3 timeColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 時間が少なくなったら赤色で警告
    if (remainingTime <= 5.0f) {
        timeColor = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    
    renderText(timeText, glm::vec2(1170, 30), timeColor, 3.0f);
    std::string goalText = "GOAL";
    glm::vec3 goalColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    renderText(goalText, glm::vec2(962, 65), goalColor, 1.0f);
    if(timeLimit<=20){
    std::string goalText2 = "5s";
        glm::vec3 goalColor2 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText2, glm::vec2(1040, 65), goalColor2, 1.0f);
        std::string goalText3 = "10s";
        glm::vec3 goalColor3 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText3, glm::vec2(1110, 65), goalColor3, 1.0f);
    }

    if(timeLimit>20){
        std::string goalText2 = "10s";
        glm::vec3 goalColor2 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText2, glm::vec2(1040, 65), goalColor2, 1.0f);
        std::string goalText3 = "20s";
        glm::vec3 goalColor3 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText3, glm::vec2(1110, 65), goalColor3, 1.0f);
    }
    

    if(existingStars==0){
        for (int i = 0; i < 3; i++) {
            glm::vec2 starPos = glm::vec2(980 + i * 70, 40);
            glm::vec3 starColor = glm::vec3(0.5f, 0.5f, 0.5f);
            renderStar(starPos, starColor, 1.5f);
        }
    }else{
    // 星の表示（右上、時間の下）
        for (int i = 0; i < existingStars; i++) {
            glm::vec2 starPos = glm::vec2(980 + i * 70, 40);
            glm::vec3 starColor;
            
            if (i < existingStars) {
                // 既に取得している星（黄色で点灯）
                starColor = glm::vec3(1.0f, 1.0f, 0.0f);
            } else if (i < existingStars + earnedStars) {
                // 今回新しく獲得した星（明るい黄色で点灯）
                // starColor = glm::vec3(1.0f, 1.0f, 0.0f);
            } else {
                // まだ取得していない星（灰色）
                starColor = glm::vec3(0.5f, 0.5f, 0.5f);
            }
            
            renderStar(starPos, starColor, 1.5f);
        }
        for (int i = existingStars; i < existingStars+1 && existingStars+1<4; i++) {
            glm::vec2 starPos = glm::vec2(900 + i * 70, 100);
            glm::vec3 starColor = glm::vec3(0.5f, 0.5f, 0.5f);
            renderStar(starPos, starColor, 1.5f);
        }
    }
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// 星の描画（塗りつぶし版）
void OpenGLRenderer::renderStar(const glm::vec2& position, const glm::vec3& color, float scale) {
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

// チュートリアルUI表示
void OpenGLRenderer::renderTutorialUI(int width, int height) {
    // フォントの初期化を確実に行う
    initializeBitmapFont();
    
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
    
    // チュートリアルボックス（中央に配置）
    float boxWidth = 800.0f;
    float boxHeight = 600.0f;
    float boxX = (width - boxWidth) / 2.0f;
    float boxY = (height - boxHeight) / 2.0f;
    
    // ボックス背景（白）
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(boxX, boxY);
    glVertex2f(boxX + boxWidth, boxY);
    glVertex2f(boxX + boxWidth, boxY + boxHeight);
    glVertex2f(boxX, boxY + boxHeight);
    glEnd();
    
    // ボックス枠線（青）
    glColor3f(0.2f, 0.4f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(boxX, boxY);
    glVertex2f(boxX + boxWidth, boxY);
    glVertex2f(boxX + boxWidth, boxY + boxHeight);
    glVertex2f(boxX, boxY + boxHeight);
    glEnd();
    glLineWidth(1.0f);
    
    // 座標系を1280x720に正規化
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    // タイトル
    renderText("TUTORIAL", glm::vec2((boxX + boxWidth/2 - 130) * scaleX, (boxY + 40) * scaleY), glm::vec3(0.2f, 0.4f, 1.0f), 2.0f);
    
    // 操作説明
    renderText("MOVE", glm::vec2((boxX + 60) * scaleX, (boxY + 120) * scaleY), glm::vec3(0.1f, 0.1f, 0.1f), 1.5f);
    renderText("FORWARD: W", glm::vec2((boxX + 100) * scaleX, (boxY + 170) * scaleY), glm::vec3(0.1f, 0.1f, 0.1f), 1.2f);
    renderText("LEFT: A", glm::vec2((boxX + 100) * scaleX, (boxY + 210) * scaleY), glm::vec3(0.1f, 0.1f, 0.1f), 1.2f);
    renderText("BACKWARD: S", glm::vec2((boxX + 100) * scaleX, (boxY + 250) * scaleY), glm::vec3(0.1f, 0.1f, 0.1f), 1.2f);
    renderText("RIGHT: D", glm::vec2((boxX + 100) * scaleX, (boxY + 290) * scaleY), glm::vec3(0.1f, 0.1f, 0.1f), 1.2f);
    
    renderText("ACTION", glm::vec2((boxX + 60) * scaleX, (boxY + 350) * scaleY), glm::vec3(0.1f, 0.1f, 0.1f), 1.5f);
    renderText("JUMP: SPACE", glm::vec2((boxX + 100) * scaleX, (boxY + 400) * scaleY), glm::vec3(0.1f, 0.1f, 0.1f), 1.2f);
    renderText("SPEED UP(1x 2x 3x): T", glm::vec2((boxX + 100) * scaleX, (boxY + 440) * scaleY), glm::vec3(0.1f, 0.1f, 0.1f), 1.2f);
    
    // 注意事項
    renderText("ENTER", glm::vec2((boxX + boxWidth/2 +200) * scaleX, (boxY + 500) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 1.3f);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ステージクリアUI用の背景描画
void OpenGLRenderer::renderStageClearBackground(int width, int height, float clearTime, int earnedStars) {
    // フォントの初期化を確実に行う
    initializeBitmapFont();
    
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
    
    // // ステージクリアボックス（中央に配置）
    float boxWidth = 800.0f;
    float boxHeight = 500.0f;
    float boxX = (width - boxWidth) / 2.0f;
    float boxY = (height - boxHeight) / 2.0f;
    
    // // ボックス背景（白）
    // glColor3f(1.0f, 1.0f, 1.0f);
    // glBegin(GL_QUADS);
    // glVertex2f(boxX, boxY);
    // glVertex2f(boxX + boxWidth, boxY);
    // glVertex2f(boxX + boxWidth, boxY + boxHeight);
    // glVertex2f(boxX, boxY + boxHeight);
    // glEnd();
    
    // // ボックス枠線（緑）
    // glColor3f(0.2f, 0.8f, 0.2f);
    // glLineWidth(3.0f);
    // glBegin(GL_LINE_LOOP);
    // glVertex2f(boxX, boxY);
    // glVertex2f(boxX + boxWidth, boxY);
    // glVertex2f(boxX + boxWidth, boxY + boxHeight);
    // glVertex2f(boxX, boxY + boxHeight);
    // glEnd();
    // glLineWidth(1.0f);
    
    // 座標系を1280x720に正規化
    float scaleX = 1280.0f / width;
    float scaleY = 720.0f / height;
    
    // ステージクリアメッセージ
    if (earnedStars == 3) {
        renderText("STAGE COMPLETED!", glm::vec2((boxX + boxWidth/2 - 300) * scaleX, (boxY - 80) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 2.5f);
    } else {
        renderText("STAGE CLEAR!", glm::vec2((boxX + boxWidth/2 - 300) * scaleX, (boxY - 80) * scaleY), glm::vec3(0.8f, 0.2f, 0.2f), 2.5f);
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

void OpenGLRenderer::initializeBitmapFont() {
    if (fontInitialized) return;
    
    // 簡単なビットマップフォントの定義（8x12ピクセル）
    // 各文字は8x12のビットマップで表現
    
    // 数字の定義
    // 0
    bitmapFont['0'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1
    };
    
    // 1
    bitmapFont['1'] = {
        0,0,0,0,1,1,0,0,
        0,0,0,0,1,1,0,0,
        0,0,0,0,1,1,0,0,
        0,0,0,0,1,1,0,0,
        0,0,0,0,1,1,0,0,
        0,0,0,0,1,1,0,0,
        0,0,0,0,1,1,0,0,
        0,0,0,0,1,1,0,0,
        0,0,0,0,1,1,0,0,
        0,0,0,0,1,1,0,0,
        0,0,0,0,1,1,0,0,
        0,0,0,0,1,1,0,0
    };
    
    // 2
    bitmapFont['2'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1
    };
    
    // 3
    bitmapFont['3'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1
    };
    
    // 4
    bitmapFont['4'] = {
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1
    };
    
    // 5
    bitmapFont['5'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1
    };
    
    // 6
    bitmapFont['6'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1
    };
    
    // 7
    bitmapFont['7'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1
    };
    
    // 8
    bitmapFont['8'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1
    };
    
    // 9
    bitmapFont['9'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1
    };
    
    // 文字の定義
    bitmapFont['R'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,1,1,0,0,
        1,1,0,0,0,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1
    };
    
    bitmapFont['e'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,0,0,
        0,1,1,1,1,1,1,0,
        0,0,1,1,1,1,0,0
    };
    
    bitmapFont['m'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        1,1,0,0,1,1,0,0,
        1,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1
    };
    
    bitmapFont['a'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1
    };
    
    bitmapFont['i'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0
    };
    
    bitmapFont['n'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,0,0,
        1,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1
    };

    bitmapFont['x'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        1,1,0,0,0,0,1,1,
        0,1,1,0,0,1,1,0,
        0,0,1,1,1,1,0,0,
        0,0,0,1,1,0,0,0,
        0,0,1,1,1,1,0,0,
        0,1,1,0,0,1,1,0,
        1,1,0,0,0,0,1,1,
        1,0,0,0,0,0,0,1,
    };
    
    bitmapFont['g'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        0,1,1,1,1,1,1,0,
        0,0,1,1,1,1,0,0,
        1,1,1,1,1,1,1,1
    };
    
    bitmapFont[':'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0
    };

    bitmapFont['-'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,1,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0
    };

    // bitmapFont['↑'] = {
    //     0,0,0,1,1,0,0,0,
    //     0,0,1,1,1,1,0,0,
    //     0,1,1,1,1,1,1,0,
    //     1,1,0,1,1,0,1,1,
    //     0,0,0,1,1,0,0,1,
    //     0,0,0,1,1,0,0,0,
    //     0,0,0,1,1,0,0,0,
    //     0,0,0,1,1,0,0,0,
    //     0,0,0,1,1,0,0,0,
    //     0,0,0,1,1,0,0,0,
    //     0,0,0,1,1,0,0,0,
    //     0,0,0,1,1,0,0,0
    // };
    
    bitmapFont['s'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,1,1,1,1,1,1,0,
        1,1,0,0,0,0,0,0,
        0,1,1,1,1,1,1,0,
        0,0,0,0,0,0,1,1,
        0,1,1,1,1,1,1,0,
        0,0,1,1,1,1,0,0
    };
    
    // 大文字のアルファベットを追加
    bitmapFont['T'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0
    };
    
    bitmapFont['U'] = {
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1
    };
    
    bitmapFont['O'] = {
        0,0,1,1,1,1,0,0,
        0,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        0,1,1,1,1,1,1,0,
        0,0,1,1,1,1,0,0
    };
    
    bitmapFont['P'] = {
        1,1,1,1,1,1,0,0,
        1,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0
    };
    
    bitmapFont['I'] = {
        0,0,1,1,1,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,1,1,1,1,0,0,
        0,0,1,1,1,1,0,0
    };
    
    bitmapFont['A'] = {
        0,0,0,1,1,0,0,0,
        0,0,1,1,1,1,0,0,
        0,1,1,0,0,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1
    };
    
    bitmapFont['L'] = {
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1
    };
    
    bitmapFont['M'] = {
        1,1,0,0,0,0,1,1,
        1,1,1,0,0,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,1,1,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1
    };
    
    bitmapFont['V'] = {
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        0,1,1,0,0,1,1,0,
        0,1,1,0,0,1,1,0,
        0,0,1,1,1,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0
    };
    
    bitmapFont['E'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,1,1,1,1,0,0,
        1,1,1,1,1,1,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1
    };
    
    bitmapFont['N'] = {
        1,1,0,0,0,0,1,1,
        1,1,1,0,0,0,1,1,
        1,1,1,1,0,0,1,1,
        1,1,0,1,1,0,1,1,
        1,1,0,0,1,1,1,1,
        1,1,0,0,0,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1
    };
    
    bitmapFont['S'] = {
        0,0,1,1,1,1,0,0,
        0,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,0,0,
        0,1,1,1,1,0,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,1,1,1,0,
        0,0,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        0,1,1,1,1,1,1,0,
        0,0,1,1,1,1,0,0
    };
    
    bitmapFont['P'] = {
        1,1,1,1,1,1,0,0,
        1,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0
    };
    
    bitmapFont['C'] = {
        0,0,1,1,1,1,0,0,
        0,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,1,1,
        0,1,1,1,1,1,1,0,
        0,0,1,1,1,1,0,0
    };
    
    bitmapFont['J'] = {
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        0,1,1,1,1,1,1,0,
        0,0,1,1,1,1,0,0
    };
    
    bitmapFont['B'] = {
        1,1,1,1,1,1,0,0,
        1,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,0,0
    };
    
    bitmapFont['D'] = {
        1,1,1,1,1,1,0,0,
        1,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,0,0
    };
    
    bitmapFont['F'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,1,1,1,1,0,0,
        1,1,1,1,1,1,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0
    };
    
    bitmapFont['G'] = {
        0,0,1,1,1,1,0,0,
        0,1,1,1,1,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,1,1,1,1,1,
        1,1,0,1,1,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        0,1,1,1,1,1,1,0,
        0,0,1,1,1,1,0,0
    };
    
    bitmapFont['H'] = {
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1
    };
    
    bitmapFont['K'] = {
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,1,1,0,
        1,1,0,0,1,1,0,0,
        1,1,0,1,1,0,0,0,
        1,1,1,1,0,0,0,0,
        1,1,1,1,0,0,0,0,
        1,1,1,1,0,0,0,0,
        1,1,0,1,1,0,0,0,
        1,1,0,0,1,1,0,0,
        1,1,0,0,0,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1
    };
    
    bitmapFont['W'] = {
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,1,1,0,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,0,0,1,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
    };
    
    bitmapFont['X'] = {
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        0,1,1,0,0,1,1,0,
        0,1,1,0,0,1,1,0,
        0,0,1,1,1,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,1,1,1,1,0,0,
        0,1,1,0,0,1,1,0,
        0,1,1,0,0,1,1,0,
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1
    };
    
    bitmapFont['Y'] = {
        1,1,0,0,0,0,1,1,
        1,1,0,0,0,0,1,1,
        0,1,1,0,0,1,1,0,
        0,1,1,0,0,1,1,0,
        0,0,1,1,1,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0
    };
    
    bitmapFont['Z'] = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,1,1,
        0,0,0,0,0,1,1,0,
        0,0,0,0,1,1,0,0,
        0,0,0,1,1,0,0,0,
        0,0,1,1,0,0,0,0,
        0,1,1,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1
    };
    
    fontInitialized = true;
}

void OpenGLRenderer::renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale) {
    auto it = bitmapFont.find(c);
    if (it == bitmapFont.end()) {
        // 文字が見つからない場合は空白として扱う
        return;
    }
    
    const std::vector<bool>& charData = it->second;
    
    glColor3f(color.r, color.g, color.b);
    
    // 8x12のビットマップを描画
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 8; x++) {
            int index = y * 8 + x;
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

// ゲームオーバーUI用の背景描画
void OpenGLRenderer::renderGameOverBackground(int width, int height) {
    // フォントの初期化を確実に行う
    initializeBitmapFont();
    
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



} // namespace gfx
