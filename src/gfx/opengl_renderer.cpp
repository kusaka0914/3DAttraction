#include "opengl_renderer.h"
#include "../core/constants/game_constants.h"
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
    
    // 深度テスト有効化
    glEnable(GL_DEPTH_TEST);
    
    // テクスチャ機能を有効化
    glEnable(GL_TEXTURE_2D);
    
    // ビットマップフォント初期化
    font.initialize();
    
    return true;
}

void OpenGLRenderer::cleanup() {
    // テクスチャをクリーンアップ
    TextureManager::cleanup();
}

void OpenGLRenderer::beginFrame() {
    glClearColor(GameConstants::RenderConstants::DEFAULT_BACKGROUND_COLOR.r, 
                 GameConstants::RenderConstants::DEFAULT_BACKGROUND_COLOR.g, 
                 GameConstants::RenderConstants::DEFAULT_BACKGROUND_COLOR.b, 1.0f);
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
    
    // ステージ6（チュートリアル）の場合は黒背景を設定
    if (stageNumber == 6) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // ステージ別背景を描画（黒背景の上に）
        backgroundRenderer.renderStageBackground(stageNumber);
    } else {
        // ステージ別背景を描画
        backgroundRenderer.renderStageBackground(stageNumber);
    }
    
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

void OpenGLRenderer::setCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
    viewMatrix = glm::lookAt(position, target, up);
}

void OpenGLRenderer::setProjection(float fov, float aspect, float near, float far) {
    projectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
}
} // namespace gfx
