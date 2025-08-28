#include "opengl_renderer.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace gfx {

OpenGLRenderer::OpenGLRenderer() : window(nullptr), fontInitialized(false), textureFontInitialized(false), fontTexture(0) {
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
    
    // テクスチャフォント初期化
    initializeTextureFont();
    
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

void OpenGLRenderer::renderTriangle(const glm::vec3& position, const glm::vec3& color, float size) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(size));
    drawTriangle(model, color);
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
    float spaceWidth = 4.0f * scale;
    
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        
        if (c == ' ') {
            currentX += spaceWidth;
            continue;
        }
        
        // ビットマップフォントで文字を描画
        renderBitmapChar(c, glm::vec2(currentX, position.y), color, scale);
        currentX += charWidth + 1.0f * scale;
    }
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void OpenGLRenderer::renderGround(float size) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(size, 0.1f, size));
    model = glm::translate(model, glm::vec3(0, -0.5f, 0));
    drawCube(model, glm::vec3(0.5f, 0.5f, 0.5f));
}

void OpenGLRenderer::renderTerrain(const std::vector<std::vector<float>>& heightmap, float scale) {
    if (heightmap.empty() || heightmap[0].empty()) return;
    
    int width = heightmap.size();
    int height = heightmap[0].size();
    
    glPushMatrix();
    
    // 草原の緑色
    glColor3f(0.2f, 0.8f, 0.3f);
    
    // 三角形メッシュでハイトマップを描画
    for (int x = 0; x < width - 1; x++) {
        for (int z = 0; z < height - 1; z++) {
            float x1 = (x - width/2.0f) * scale;
            float x2 = (x + 1 - width/2.0f) * scale;
            float z1 = (z - height/2.0f) * scale;
            float z2 = (z + 1 - height/2.0f) * scale;
            
            float h1 = heightmap[x][z];
            float h2 = heightmap[x+1][z];
            float h3 = heightmap[x][z+1];
            float h4 = heightmap[x+1][z+1];
            
            // 高度によって色を変える（草原効果）
            float avgHeight = (h1 + h2 + h3 + h4) / 4.0f;
            if (avgHeight > 2.0f) {
                glColor3f(0.1f, 0.6f, 0.2f); // 高い場所は濃い緑
            } else if (avgHeight > 1.0f) {
                glColor3f(0.2f, 0.8f, 0.3f); // 中間は明るい緑
            } else {
                glColor3f(0.3f, 0.9f, 0.4f); // 低い場所は薄い緑
            }
            
            // 四角形を2つの三角形に分割
            glBegin(GL_TRIANGLES);
            
            // 最初の三角形
            glVertex3f(x1, h1, z1);
            glVertex3f(x2, h2, z1);
            glVertex3f(x1, h3, z2);
            
            // 二番目の三角形
            glVertex3f(x2, h2, z1);
            glVertex3f(x2, h4, z2);
            glVertex3f(x1, h3, z2);
            
            glEnd();
        }
    }
    
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

void OpenGLRenderer::drawTriangle(const glm::mat4& model, const glm::vec3& color) {
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(model));
    
    glColor3f(color.r, color.g, color.b);
    
    glBegin(GL_TRIANGLES);
    glVertex3f( 0.0f,  0.5f, 0.0f);
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glVertex3f( 0.5f, -0.5f, 0.0f);
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
    std::string timeText = "Remaining: " + std::to_string(static_cast<int>(remainingTime)) + "s";
    glm::vec3 timeColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 時間が少なくなったら赤色で警告
    if (remainingTime <= 5.0f) {
        timeColor = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    
    renderText(timeText, glm::vec2(900, 50), timeColor, 2.0f);
    if(existingStars==0){
        for (int i = 0; i < 3; i++) {
            glm::vec2 starPos = glm::vec2(900 + i * 70, 100);
            glm::vec3 starColor = glm::vec3(0.5f, 0.5f, 0.5f);
            renderStar(starPos, starColor, 1.5f);
        }
    }else{
    // 星の表示（右上、時間の下）
        for (int i = 0; i < existingStars; i++) {
            glm::vec2 starPos = glm::vec2(900 + i * 70, 100);
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

// 星の描画
void OpenGLRenderer::renderStar(const glm::vec2& position, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    // 星の5つの角を描画
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; i++) {
        float angle1 = i * 72.0f * 3.14159f / 180.0f;
        float angle2 = (i + 2) * 72.0f * 3.14159f / 180.0f;
        
        // 外側の点
        float x1 = position.x + cos(angle1) * 12.0f * scale;
        float y1 = position.y + sin(angle1) * 12.0f * scale;
        
        // 内側の点
        float x2 = position.x + cos(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scale;
        float y2 = position.y + sin(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scale;
        
        // 次の外側の点
        float x3 = position.x + cos(angle2) * 12.0f * scale;
        float y3 = position.y + sin(angle2) * 12.0f * scale;
        
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glVertex2f(x3, y3);
    }
    glEnd();
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

void OpenGLRenderer::initializeTextureFont() {
    if (textureFontInitialized) return;
    
    // 簡単なテクスチャフォントの初期化
    // 実際のプロジェクトでは、事前に生成したフォントテクスチャを使用
    
    // テクスチャ生成
    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    // 簡単なフォントテクスチャを生成（8x12ピクセルの文字を並べたもの）
    const int atlasWidth = 256;
    const int atlasHeight = 256;
    const int charWidth = 8;
    const int charHeight = 12;
    const int charsPerRow = atlasWidth / charWidth;
    
    std::vector<unsigned char> atlasData(atlasWidth * atlasHeight, 0);
    
    // ビットマップフォントデータからテクスチャを生成
    for (auto& [c, charData] : bitmapFont) {
        int charIndex = static_cast<int>(c);
        int row = charIndex / charsPerRow;
        int col = charIndex % charsPerRow;
        
        int startX = col * charWidth;
        int startY = row * charHeight;
        
        // 文字データをテクスチャに配置
        for (int y = 0; y < charHeight; y++) {
            for (int x = 0; x < charWidth; x++) {
                int atlasIndex = (startY + y) * atlasWidth + (startX + x);
                int charIndex = y * charWidth + x;
                
                if (charIndex < charData.size() && charData[charIndex]) {
                    atlasData[atlasIndex] = 255; // 白
                } else {
                    atlasData[atlasIndex] = 0;   // 透明
                }
            }
        }
        
        // グリフ情報を設定
        GlyphInfo& glyph = glyphs[c];
        glyph.x = static_cast<float>(startX) / atlasWidth;
        glyph.y = static_cast<float>(startY) / atlasHeight;
        glyph.width = static_cast<float>(charWidth) / atlasWidth;
        glyph.height = static_cast<float>(charHeight) / atlasHeight;
        glyph.advanceX = charWidth + 1;
        glyph.bearingX = 0;
        glyph.bearingY = charHeight;
    }
    
    // テクスチャデータをアップロード
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, atlasWidth, atlasHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, atlasData.data());
    
    // テクスチャパラメータ設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    fontAtlasWidth = static_cast<float>(atlasWidth);
    fontAtlasHeight = static_cast<float>(atlasHeight);
    textureFontInitialized = true;
}

void OpenGLRenderer::renderTextureText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale) {
    if (!textureFontInitialized) return;
    
    // テクスチャを有効化
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    // ブレンディングを有効化
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glColor3f(color.r, color.g, color.b);
    
    float currentX = position.x;
    
    for (char c : text) {
        if (c == ' ') {
            currentX += 8.0f * scale;
            continue;
        }
        
        auto it = glyphs.find(c);
        if (it == glyphs.end()) continue;
        
        const GlyphInfo& glyph = it->second;
        
        float charWidth = glyph.width * fontAtlasWidth * scale;
        float charHeight = glyph.height * fontAtlasHeight * scale;
        
        // 文字の四角形を描画
        glBegin(GL_QUADS);
        glTexCoord2f(glyph.x, glyph.y + glyph.height);
        glVertex2f(currentX, position.y);
        
        glTexCoord2f(glyph.x + glyph.width, glyph.y + glyph.height);
        glVertex2f(currentX + charWidth, position.y);
        
        glTexCoord2f(glyph.x + glyph.width, glyph.y);
        glVertex2f(currentX + charWidth, position.y + charHeight);
        
        glTexCoord2f(glyph.x, glyph.y);
        glVertex2f(currentX, position.y + charHeight);
        glEnd();
        
        currentX += glyph.advanceX * scale;
    }
    
    // テクスチャを無効化
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

} // namespace gfx
