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

void OpenGLRenderer::beginFrameWithBackground(int stageNumber) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // ステージ別背景を描画
    renderStageBackground(stageNumber);
    
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

void OpenGLRenderer::renderRealisticBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, size);
    
    // 透明度を適用した色
    glm::vec3 alphaColor = color * alpha;
    
    // ライティングを有効化
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    
    // 光源の位置を設定
    GLfloat lightPosition[] = {10.0f, 20.0f, 10.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    
    // 環境光を設定
    GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    
    // 拡散光を設定
    GLfloat diffuseLight[] = {0.7f, 0.7f, 0.7f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(model));
    
    // 基本色を設定（透明度適用）
    glColor3f(alphaColor.r, alphaColor.g, alphaColor.b);
    
    // 法線を計算して各面を描画
    glBegin(GL_QUADS);
    
    // 前面（Z軸正方向）
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    
    // 背面（Z軸負方向）
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    
    // 上面（Y軸正方向）- より明るく
    glNormal3f(0.0f, 1.0f, 0.0f);
    glColor3f(alphaColor.r * 1.2f, alphaColor.g * 1.2f, alphaColor.b * 1.2f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    
    // 下面（Y軸負方向）- より暗く
    glNormal3f(0.0f, -1.0f, 0.0f);
    glColor3f(alphaColor.r * 0.6f, alphaColor.g * 0.6f, alphaColor.b * 0.6f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    
    // 右面（X軸正方向）- 中間の明るさ
    glNormal3f(1.0f, 0.0f, 0.0f);
    glColor3f(alphaColor.r * 0.9f, alphaColor.g * 0.9f, alphaColor.b * 0.9f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    
    // 左面（X軸負方向）- 中間の明るさ
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glColor3f(alphaColor.r * 0.9f, alphaColor.g * 0.9f, alphaColor.b * 0.9f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    
    glEnd();
    
    // エッジの強調（ワイヤーフレーム風）
    glDisable(GL_LIGHTING);
    glColor3f(alphaColor.r * 0.3f, alphaColor.g * 0.3f, alphaColor.b * 0.3f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    
    // 前面のエッジ
    glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    
    // 背面のエッジ
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, -0.5f);
    
    // 縦のエッジ
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    
    glEnd();
    
    glPopMatrix();
    
    // ライティングを無効化
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
}

void OpenGLRenderer::renderStageBackground(int stageNumber) {
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1280, 720, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    // ステージ別の背景色とグラデーション
    glm::vec3 topColor, bottomColor;
    
    switch (stageNumber) {
        case 0: // ステージ選択画面
            topColor = glm::vec3(0.5f, 0.7f, 1.0f);    // 青空
            bottomColor = glm::vec3(0.2f, 0.5f, 0.1f); // 深い緑（草原）
            break;
        case 1: // ステージ1 - 青空、白い雲（初心者向けの明るい雰囲気）
            topColor = glm::vec3(0.5f, 0.7f, 1.0f);    // 青空
            bottomColor = glm::vec3(0.8f, 0.9f, 1.0f); // 明るい青
            break;
        case 2: // ステージ2 - 夕日、オレンジ色の空（中級者向けの暖かい雰囲気）
            topColor = glm::vec3(1.0f, 0.6f, 0.3f);    // オレンジ
            bottomColor = glm::vec3(1.0f, 0.8f, 0.5f); // 明るいオレンジ
            break;
        case 3: // ステージ3 - 夜、星空（上級者向けの神秘的雰囲気）
            topColor = glm::vec3(0.1f, 0.1f, 0.3f);    // 深い青
            bottomColor = glm::vec3(0.3f, 0.2f, 0.5f); // 紫
            break;
        case 4: // ステージ4 - 雷雲、稲妻（難易度の高い雰囲気）
            topColor = glm::vec3(0.2f, 0.2f, 0.3f);    // 暗い青
            bottomColor = glm::vec3(0.4f, 0.4f, 0.5f); // グレー
            break;
        case 5: // ステージ5 - 宇宙、星雲（最終ステージの壮大な雰囲気）
            topColor = glm::vec3(0.0f, 0.0f, 0.1f);    // 深い黒
            bottomColor = glm::vec3(0.2f, 0.1f, 0.4f); // 深い紫
            break;
        default:
            topColor = glm::vec3(0.2f, 0.3f, 0.3f);    // デフォルト
            bottomColor = glm::vec3(0.4f, 0.5f, 0.5f);
            break;
    }
    
    // 背景を描画（ステージ0は空と草原を分離）
    if (stageNumber == 0) {
        // 空の部分（上半分）
        glBegin(GL_QUADS);
        glColor3f(topColor.r, topColor.g, topColor.b);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glVertex2f(1280, 360);
        glVertex2f(0, 360);
        glEnd();
        
        // 草原の部分（下半分）
        glBegin(GL_QUADS);
        glColor3f(bottomColor.r, bottomColor.g, bottomColor.b);
        glVertex2f(0, 360);
        glVertex2f(1280, 360);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();
    } else {
        // 他のステージはグラデーション
        glBegin(GL_QUADS);
        glColor3f(topColor.r, topColor.g, topColor.b);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glColor3f(bottomColor.r, bottomColor.g, bottomColor.b);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();
    }
    
    // ステージ別の装飾要素を追加
    switch (stageNumber) {
        case 0: // ステージ選択画面 - 草原
            renderGrassland();
            break;
        case 1: // ステージ1 - 雲
            renderClouds();
            break;
        case 2: // ステージ2 - 夕日
            renderSunset();
            break;
        case 3: // ステージ3 - 星
            renderStars();
            break;
        case 4: // ステージ4 - 雷雲
            renderThunderClouds();
            break;
        case 5: // ステージ5 - 星雲
            renderNebula();
            break;
    }
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void OpenGLRenderer::renderGrassland() {
    // 雲を描画（空の部分）
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 4; i++) {
        float x = fmod(i * 350.0f + (i * 30.0f), 1280.0f);
        float y = 80.0f + (i * 25.0f);
        float size = 70.0f + (i * 15.0f);
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int j = 0; j <= 12; j++) {
            float angle = 2.0f * 3.14159f * j / 12;
            float cloudX = x + cos(angle) * size;
            float cloudY = y + sin(angle) * (size * 0.6f);
            glVertex2f(cloudX, cloudY);
        }
        glEnd();
    }
    
    // 草を描画
    glColor3f(0.1f, 0.6f, 0.1f); // 濃い緑
    for (int i = 0; i < 100; i++) {
        float x = fmod(i * 12.0f + (i * 7.0f), 1280.0f);
        float y = 720.0f - fmod(i * 8.0f + (i * 13.0f), 200.0f); // 画面下部から
        float height = 15.0f + (i % 5) * 3.0f;
        float width = 2.0f + (i % 3);
        
        // 草の葉を描画
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y);
        glVertex2f(x - width, y - height);
        glVertex2f(x + width, y - height);
        glEnd();
    }
    
    // 花を描画
    for (int i = 0; i < 20; i++) {
        float x = fmod(i * 60.0f + (i * 23.0f), 1280.0f);
        float y = 720.0f - fmod(i * 15.0f + (i * 37.0f), 150.0f);
        float size = 8.0f + (i % 3) * 2.0f;
        
        // 花の色（ランダム）
        glm::vec3 flowerColors[] = {
            glm::vec3(1.0f, 0.3f, 0.3f), // 赤
            glm::vec3(1.0f, 0.8f, 0.2f), // 黄色
            glm::vec3(0.8f, 0.2f, 0.8f), // ピンク
            glm::vec3(1.0f, 1.0f, 1.0f)  // 白
        };
        glm::vec3 flowerColor = flowerColors[i % 4];
        
        glColor3f(flowerColor.r, flowerColor.g, flowerColor.b);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int j = 0; j <= 8; j++) {
            float angle = 2.0f * 3.14159f * j / 8;
            float flowerX = x + cos(angle) * size;
            float flowerY = y + sin(angle) * size;
            glVertex2f(flowerX, flowerY);
        }
        glEnd();
    }
    
    // 木を描画
    for (int i = 0; i < 8; i++) {
        float x = 100.0f + i * 150.0f;
        float y = 720.0f - 80.0f;
        
        // 木の幹
        glColor3f(0.4f, 0.2f, 0.1f); // 茶色
        glBegin(GL_QUADS);
        glVertex2f(x - 8, y);
        glVertex2f(x + 8, y);
        glVertex2f(x + 8, y - 60);
        glVertex2f(x - 8, y - 60);
        glEnd();
        
        // 木の葉
        glColor3f(0.2f, 0.5f, 0.1f); // 濃い緑
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y - 60);
        for (int j = 0; j <= 12; j++) {
            float angle = 2.0f * 3.14159f * j / 12;
            float leafX = x + cos(angle) * 40;
            float leafY = y - 60 + sin(angle) * 40;
            glVertex2f(leafX, leafY);
        }
        glEnd();
    }
    
    // 小さな丘を描画
    glColor3f(0.3f, 0.6f, 0.2f); // 明るい緑
    for (int i = 0; i < 3; i++) {
        float x = 200.0f + i * 400.0f;
        float y = 720.0f - 50.0f;
        float width = 200.0f;
        float height = 80.0f;
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y - height);
        for (int j = 0; j <= 16; j++) {
            float angle = 3.14159f * j / 16;
            float hillX = x + cos(angle) * width;
            float hillY = y - height + sin(angle) * height;
            glVertex2f(hillX, hillY);
        }
        glEnd();
    }
}

void OpenGLRenderer::renderClouds() {
    // 雲を描画
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 5; i++) {
        float x = fmod(i * 300.0f + (i * 50.0f), 1280.0f);
        float y = 100.0f + (i * 20.0f);
        float size = 80.0f + (i * 10.0f);
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int j = 0; j <= 12; j++) {
            float angle = 2.0f * 3.14159f * j / 12;
            float cloudX = x + cos(angle) * size;
            float cloudY = y + sin(angle) * (size * 0.6f);
            glVertex2f(cloudX, cloudY);
        }
        glEnd();
    }
}

void OpenGLRenderer::renderSunset() {
    // 夕日を描画
    glColor3f(1.0f, 0.8f, 0.3f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(1000, 150);
    for (int i = 0; i <= 20; i++) {
        float angle = 3.14159f * i / 20;
        float x = 1000 + cos(angle) * 80;
        float y = 150 + sin(angle) * 80;
        glVertex2f(x, y);
    }
    glEnd();
}

void OpenGLRenderer::renderStars() {
    // 星を描画
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 50; i++) {
        float x = fmod(i * 25.0f + (i * 17.0f), 1280.0f);
        float y = fmod(i * 15.0f + (i * 23.0f), 300.0f);
        float size = 2.0f + (i % 3);
        
        glBegin(GL_QUADS);
        glVertex2f(x - size, y - size);
        glVertex2f(x + size, y - size);
        glVertex2f(x + size, y + size);
        glVertex2f(x - size, y + size);
        glEnd();
    }
}

void OpenGLRenderer::renderThunderClouds() {
    // 雷雲を描画
    glColor3f(0.3f, 0.3f, 0.4f);
    for (int i = 0; i < 3; i++) {
        float x = 200.0f + i * 400.0f;
        float y = 80.0f + i * 30.0f;
        float size = 120.0f + i * 20.0f;
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int j = 0; j <= 12; j++) {
            float angle = 2.0f * 3.14159f * j / 12;
            float cloudX = x + cos(angle) * size;
            float cloudY = y + sin(angle) * (size * 0.8f);
            glVertex2f(cloudX, cloudY);
        }
        glEnd();
    }
}

void OpenGLRenderer::renderNebula() {
    // 星雲を描画
    for (int i = 0; i < 3; i++) {
        float x = 300.0f + i * 300.0f;
        float y = 200.0f + i * 100.0f;
        float size = 150.0f + i * 50.0f;
        
        // 星雲の色（紫、青、ピンク）
        glm::vec3 colors[] = {
            glm::vec3(0.8f, 0.2f, 0.8f), // 紫
            glm::vec3(0.2f, 0.4f, 0.8f), // 青
            glm::vec3(0.8f, 0.3f, 0.6f)  // ピンク
        };
        
        glColor3f(colors[i].r, colors[i].g, colors[i].b);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int j = 0; j <= 16; j++) {
            float angle = 2.0f * 3.14159f * j / 16;
            float nebulaX = x + cos(angle) * size;
            float nebulaY = y + sin(angle) * (size * 0.7f);
            glVertex2f(nebulaX, nebulaY);
        }
        glEnd();
    }
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
void OpenGLRenderer::renderTimeUI(float remainingTime, float timeLimit, int earnedStars, int existingStars, int lives) {
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
        for (int i = existingStars; i<3; i++) {
            glm::vec2 starPos = glm::vec2(980 + i * 70, 40);
            glm::vec3 starColor = glm::vec3(0.5f, 0.5f, 0.5f);
            renderStar(starPos, starColor, 1.5f);
        }
    }
    
    // ハートを6個表示（右から消えていく）
    for (int i = 0; i < 6; i++) {
        glm::vec3 heartColor;
        if (i < lives) {
            heartColor = glm::vec3(1.0f, 0.3f, 0.3f); // 赤色（残っているライフ）
        } else {
            heartColor = glm::vec3(0.3f, 0.3f, 0.3f); // 灰色（失ったライフ）
        }
        
        // STAGEテキストの右側にハートを配置
        float heartX = 200.0f + i * 40.0f; // STAGEテキストの右側から40px間隔
        float heartY = 45.0f; // STAGEテキストと同じY座標
        
        renderHeart(glm::vec2(heartX, heartY), heartColor, 1.0f);
    }
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// 時間停止スキルUI表示
void OpenGLRenderer::renderTimeStopUI(bool hasSkill, bool isTimeStopped, float timeStopTimer, int remainingUses, int maxUses) {
    // スキルを取得していない場合は表示しない
    if (!hasSkill) {
        return;
    }
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
    
    // 時間停止スキルの表示（左下）
    std::string skillText = "時間よ止まれ";
    glm::vec3 skillColor = glm::vec3(0.8f, 0.8f, 1.0f); // 薄い青色
    
    // 時間停止中は明るい青色
    if (isTimeStopped) {
        skillColor = glm::vec3(0.5f, 0.5f, 1.0f);
    }
    
    renderText(skillText, glm::vec2(30, 650), skillColor, 1.5f);
    
    // 使用回数表示
    std::string usesText = "Q: " + std::to_string(remainingUses) + "/" + std::to_string(maxUses);
    glm::vec3 usesColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 使用回数が0の場合は灰色
    if (remainingUses <= 0) {
        usesColor = glm::vec3(0.5f, 0.5f, 0.5f);
    }
    
    renderText(usesText, glm::vec2(30, 680), usesColor, 1.2f);
    
    // 時間停止中の残り時間表示
    if (isTimeStopped) {
        std::string timerText = "残り: " + std::to_string(static_cast<int>(timeStopTimer)) + "s";
        renderText(timerText, glm::vec2(30, 620), glm::vec3(1.0f, 0.5f, 0.5f), 1.2f);
    }
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// 二段ジャンプスキルUI表示
void OpenGLRenderer::renderDoubleJumpUI(bool hasSkill, bool isEasyMode, int remainingUses, int maxUses) {
    // お助けモードの場合は表示しない（無制限のため）
    if (isEasyMode) {
        return;
    }
    
    // スキルを取得していない場合は表示しない
    if (!hasSkill) {
        return;
    }
    
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
    
    // 二段ジャンプスキルの表示（左下、時間停止スキルの上）
    std::string skillText = "二段ジャンプ";
    glm::vec3 skillColor = glm::vec3(0.8f, 1.0f, 0.8f); // 薄い緑色
    
    renderText(skillText, glm::vec2(30, 590), skillColor, 1.5f);
    
    // 使用回数表示
    std::string usesText = "SPACE: " + std::to_string(remainingUses) + "/" + std::to_string(maxUses);
    glm::vec3 usesColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 使用回数が0の場合は灰色
    if (remainingUses <= 0) {
        usesColor = glm::vec3(0.5f, 0.5f, 0.5f);
    }
    
    renderText(usesText, glm::vec2(30, 620), usesColor, 1.2f);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ハートフエールスキルUI表示
void OpenGLRenderer::renderHeartFeelUI(bool hasSkill, int remainingUses, int maxUses, int currentLives) {
    // スキルを取得していない場合は表示しない
    if (!hasSkill) {
        return;
    }
    
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
    
    // ハートフエールスキルの表示（左下、二段ジャンプスキルの上）
    std::string skillText = "ハートフエール";
    glm::vec3 skillColor = glm::vec3(1.0f, 0.8f, 0.8f); // 薄い赤色
    
    renderText(skillText, glm::vec2(30, 530), skillColor, 1.5f);
    
    // 使用回数表示
    std::string usesText = "H: " + std::to_string(remainingUses) + "/" + std::to_string(maxUses);
    glm::vec3 usesColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 使用回数が0の場合は灰色
    if (remainingUses <= 0) {
        usesColor = glm::vec3(0.5f, 0.5f, 0.5f);
    }
    
    renderText(usesText, glm::vec2(30, 560), usesColor, 1.2f);
    
    // 現在の残機数表示
    std::string livesText = "残機: " + std::to_string(currentLives) + "/6";
    renderText(livesText, glm::vec2(30, 500), glm::vec3(1.0f, 0.3f, 0.3f), 1.2f);
    
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

// 3D星の描画（塗りつぶし版）
void OpenGLRenderer::renderStar3D(const glm::vec3& position, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    // 星の中心点
    float centerX = position.x;
    float centerY = position.y;
    float centerZ = position.z;
    
    // 星の5つの角を描画（塗りつぶし）
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; i++) {
        float angle1 = i * 72.0f * 3.14159f / 180.0f;
        float angle2 = (i + 2) * 72.0f * 3.14159f / 180.0f;
        
        // 外側の点
        float x1 = centerX + cos(angle1) * 0.3f * scale;
        float y1 = centerY + sin(angle1) * 0.3f * scale;
        
        // 内側の点
        float x2 = centerX + cos(angle1 + 36.0f * 3.14159f / 180.0f) * 0.12f * scale;
        float y2 = centerY + sin(angle1 + 36.0f * 3.14159f / 180.0f) * 0.12f * scale;
        
        // 次の外側の点
        float x3 = centerX + cos(angle2) * 0.3f * scale;
        float y3 = centerY + sin(angle2) * 0.3f * scale;
        
        // 中心から各点への三角形を描画（塗りつぶし）
        glVertex3f(centerX, centerY, centerZ);  // 中心点
        glVertex3f(x1, y1, centerZ);            // 外側の点1
        glVertex3f(x2, y2, centerZ);            // 内側の点
        
        glVertex3f(centerX, centerY, centerZ);  // 中心点
        glVertex3f(x2, y2, centerZ);            // 内側の点
        glVertex3f(x3, y3, centerZ);            // 外側の点2
    }
    glEnd();
}

// 3D鍵穴マークの描画
void OpenGLRenderer::renderLock3D(const glm::vec3& position, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    float centerX = position.x;
    float centerY = position.y;
    float centerZ = position.z;
    
    // 鍵穴の本体（四角形）
    float halfWidth = 0.3f * scale;
    float halfHeight = 0.4f * scale;
    
    glBegin(GL_QUADS);
    // 前面
    glVertex3f(centerX - halfWidth, centerY - halfHeight, centerZ);
    glVertex3f(centerX + halfWidth, centerY - halfHeight, centerZ);
    glVertex3f(centerX + halfWidth, centerY + halfHeight, centerZ);
    glVertex3f(centerX - halfWidth, centerY + halfHeight, centerZ);
    glEnd();
    
    // 鍵穴の穴（円形）
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(centerX, centerY, centerZ); // 中心
    int segments = 12;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159f * i / segments;
        float x = centerX + cos(angle) * 0.15f * scale;
        float y = centerY + sin(angle) * 0.15f * scale;
        glVertex3f(x, y, centerZ);
    }
    glEnd();
    
    // 鍵穴の下の穴（小さな四角形）
    float smallHalfWidth = 0.08f * scale;
    float smallHalfHeight = 0.15f * scale;
    
    glBegin(GL_QUADS);
    glVertex3f(centerX - smallHalfWidth, centerY - halfHeight - smallHalfHeight, centerZ);
    glVertex3f(centerX + smallHalfWidth, centerY - halfHeight - smallHalfHeight, centerZ);
    glVertex3f(centerX + smallHalfWidth, centerY - halfHeight + smallHalfHeight, centerZ);
    glVertex3f(centerX - smallHalfWidth, centerY - halfHeight + smallHalfHeight, centerZ);
    glEnd();
}

// 3D数字の描画
void OpenGLRenderer::renderNumber3D(const glm::vec3& position, int number, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    float centerX = position.x;
    float centerY = position.y;
    float centerZ = position.z;
    
    // 数字を文字列に変換
    std::string numStr = std::to_string(number);
    
    // 各文字を描画
    for (size_t i = 0; i < numStr.length(); i++) {
        char digit = numStr[i];
        float charX = centerX + i * 0.6f * scale;
        
        // 数字の形状を定義（実際の数字の形状）
        float width = 0.4f * scale;
        float height = 0.6f * scale;
        float thickness = 0.1f * scale;
        
        // 数字の形状を定義（各数字のセグメント）
        std::vector<std::vector<glm::vec2>> segments;
        
        if (digit == '0') {
            // 0の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, height/2}, {width/2, height/2}},    // 上
                {{width/2, height/2}, {width/2, -height/2}},    // 右
                {{width/2, -height/2}, {-width/2, -height/2}},  // 下
                {{-width/2, -height/2}, {-width/2, height/2}}   // 左
            };
        } else if (digit == '1') {
            // 1の形状（現在の向きに合わせて修正）
            segments = {
                {{0, height/2}, {0, -height/2}}  // 中央縦線
            };
        } else if (digit == '2') {
            // 2の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, 0}},            // 右下
                {{width/2, 0}, {-width/2, 0}},                  // 中央
                {{-width/2, 0}, {-width/2, height/2}},          // 左上
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '3') {
            // 3の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}},    // 右
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '4') {
            // 4の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {-width/2, 0}},         // 左下
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{width/2, -height/2}, {width/2, height/2}}     // 右
            };
        } else if (digit == '5') {
            // 5の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{-width/2, -height/2}, {-width/2, 0}},         // 左下
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{width/2, 0}, {width/2, height/2}},            // 右上
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '6') {
            // 6の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{-width/2, -height/2}, {-width/2, height/2}},  // 左
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{width/2, 0}, {width/2, height/2}},            // 右上
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '7') {
            // 7の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}}     // 右
            };
        } else if (digit == '8') {
            // 8の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}},    // 右
                {{-width/2, -height/2}, {-width/2, height/2}},  // 左
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '9') {
            // 9の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}},    // 右
                {{-width/2, -height/2}, {-width/2, 0}},         // 左下
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        }
        
        // 各セグメントを描画
        for (const auto& segment : segments) {
            glBegin(GL_QUADS);
            // セグメントを太い線として描画（線対称で正しい向きに）
            glm::vec2 start = segment[0];
            glm::vec2 end = segment[1];
            // 線対称（180度回転：xとyを両方反転）
            start = glm::vec2(-start.x, -start.y);
            end = glm::vec2(-end.x, -end.y);
            glm::vec2 dir = glm::normalize(end - start);
            glm::vec2 perp = glm::vec2(-dir.y, dir.x) * thickness / 2.0f;
            
            glVertex3f(charX + start.x - perp.x, centerY + start.y - perp.y, centerZ);
            glVertex3f(charX + start.x + perp.x, centerY + start.y + perp.y, centerZ);
            glVertex3f(charX + end.x + perp.x, centerY + end.y + perp.y, centerZ);
            glVertex3f(charX + end.x - perp.x, centerY + end.y - perp.y, centerZ);
            glEnd();
        }
    }
}

// 3D×記号の描画
void OpenGLRenderer::renderXMark3D(const glm::vec3& position, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    float centerX = position.x;
    float centerY = position.y;
    float centerZ = position.z;
    
    // ×記号を2つの交差する線で表現
    float halfSize = 0.3f * scale;
    
    glBegin(GL_QUADS);
    // 左上から右下への線
    glVertex3f(centerX - halfSize, centerY + halfSize, centerZ);
    glVertex3f(centerX - halfSize + 0.1f * scale, centerY + halfSize, centerZ);
    glVertex3f(centerX + halfSize, centerY - halfSize, centerZ);
    glVertex3f(centerX + halfSize - 0.1f * scale, centerY - halfSize, centerZ);
    
    // 右上から左下への線
    glVertex3f(centerX + halfSize, centerY + halfSize, centerZ);
    glVertex3f(centerX + halfSize - 0.1f * scale, centerY + halfSize, centerZ);
    glVertex3f(centerX - halfSize, centerY - halfSize, centerZ);
    glVertex3f(centerX - halfSize + 0.1f * scale, centerY - halfSize, centerZ);
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

// Ready画面の描画
void OpenGLRenderer::renderReadyScreen(int width, int height, int speedLevel, bool isFirstPersonMode) {
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
    
    // "Ready?"メッセージ
    renderText("READY", glm::vec2((width/2 - 230) * scaleX, (height/2 - 300) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 5.0f);
    renderText("PLAY SPEED", glm::vec2((width/2 - 150) * scaleX, (height/2 - 70) * scaleY), glm::vec3(1.0f, 1.0f, 1.0f), 1.5f);
    
    // 速度選択（1x, 2x, 3x）
    std::vector<std::string> speedTexts = {"1x", "2x", "3x"};
    for (int i = 0; i < 3; i++) {
        glm::vec3 color;
        if (i == speedLevel) {
            color = glm::vec3(1.0f, 0.8f, 0.2f); // 金色（選択中）
        } else {
            color = glm::vec3(0.5f, 0.5f, 0.5f); // 灰色（未選択）
        }
        
        float xPos = (width/2 - 200 + i * 150) * scaleX;
        float yPos = (height/2) * scaleY;
        renderText(speedTexts[i], glm::vec2(xPos, yPos), color, 2.0f);
    }
    renderText("PRESS T", glm::vec2((width/2 - 100) * scaleX, (height/2 + 100) * scaleY), glm::vec3(1.0f, 0.8f, 0.2f), 1.2f);
    
    // モード選択
    renderText("3RD PERSON", glm::vec2((width/2 - 200) * scaleX, (height/2 + 150) * scaleY), 
               isFirstPersonMode ? glm::vec3(0.5f, 0.5f, 0.5f) : glm::vec3(1.0f, 0.8f, 0.2f), 1.2f);
    renderText("1ST PERSON", glm::vec2((width/2 + 50) * scaleX, (height/2 + 150) * scaleY), 
               isFirstPersonMode ? glm::vec3(1.0f, 0.8f, 0.2f) : glm::vec3(0.5f, 0.5f, 0.5f), 1.2f);
    
    renderText("PRESS F", glm::vec2((width/2 - 100) * scaleX, (height/2 + 200) * scaleY), glm::vec3(1.0f, 0.8f, 0.2f), 1.2f);
    
    // "Enter"メッセージ
    renderText("ENTER", glm::vec2((width/2 + 60) * scaleX, (height/2 + 300) * scaleY), glm::vec3(0.2f, 0.8f, 0.2f), 1.2f);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// カウントダウンの描画
void OpenGLRenderer::renderCountdown(int width, int height, int count) {
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

// ハートの描画
void OpenGLRenderer::renderHeart(const glm::vec2& position, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    // ハートの中心点
    float centerX = position.x;
    float centerY = position.y;
    
    // ハートの形状を描画（塗りつぶし）
    glBegin(GL_TRIANGLES);
    
    // 左側の円形部分
    for (int i = 0; i < 8; i++) {
        float angle1 = i * 45.0f * 3.14159f / 180.0f;
        float angle2 = (i + 1) * 45.0f * 3.14159f / 180.0f;
        
        float x1 = centerX - 8.0f * scale + cos(angle1) * 8.0f * scale;
        float y1 = centerY - 4.0f * scale + sin(angle1) * 8.0f * scale;
        float x2 = centerX - 8.0f * scale + cos(angle2) * 8.0f * scale;
        float y2 = centerY - 4.0f * scale + sin(angle2) * 8.0f * scale;
        
        glVertex2f(centerX - 8.0f * scale, centerY - 4.0f * scale); // 中心
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    }
    
    // 右側の円形部分
    for (int i = 0; i < 8; i++) {
        float angle1 = i * 45.0f * 3.14159f / 180.0f;
        float angle2 = (i + 1) * 45.0f * 3.14159f / 180.0f;
        
        float x1 = centerX + 8.0f * scale + cos(angle1) * 8.0f * scale;
        float y1 = centerY - 4.0f * scale + sin(angle1) * 8.0f * scale;
        float x2 = centerX + 8.0f * scale + cos(angle2) * 8.0f * scale;
        float y2 = centerY - 4.0f * scale + sin(angle2) * 8.0f * scale;
        
        glVertex2f(centerX + 8.0f * scale, centerY - 4.0f * scale); // 中心
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    }
    
    // 下部の三角形部分
    glVertex2f(centerX, centerY + 12.0f * scale); // 下部の尖った部分
    glVertex2f(centerX - 16.0f * scale, centerY - 4.0f * scale); // 左側
    glVertex2f(centerX + 16.0f * scale, centerY - 4.0f * scale); // 右側
    
    glEnd();
}



} // namespace gfx
