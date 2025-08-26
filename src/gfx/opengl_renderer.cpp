#include "opengl_renderer.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace gfx {

OpenGLRenderer::OpenGLRenderer() : window(nullptr) {
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
    
    std::cout << "OpenGL initialized with viewport: " << width << "x" << height << std::endl;
    
    // 深度テスト有効化
    glEnable(GL_DEPTH_TEST);
    
    // カメラとプロジェクションの初期設定は削除（simple_main.cppで設定される）
    // setCamera(glm::vec3(0, 5, 10), glm::vec3(0, 0, 0));
    // setProjection(45.0f, (float)width / (float)height, 0.1f, 100.0f);
    
    std::cout << "OpenGL renderer initialized successfully" << std::endl;
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
    // 簡易的なテキスト表示（実際のテキストレンダリングは複雑なので、ここではスキップ）
    // コンソールに出力してデバッグ用に表示
    std::cout << "Text: " << text << " at (" << position.x << ", " << position.y << ") scale: " << scale << std::endl;
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

} // namespace gfx
