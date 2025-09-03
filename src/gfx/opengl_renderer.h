#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "bitmap_font.h"
#include "background_renderer.h"
#include "ui_renderer.h"
#include "game_state_ui_renderer.h"
#include <vector>
#include <string>
#include <map>

namespace gfx {



class OpenGLRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();
    
    bool initialize(GLFWwindow* window);
    void cleanup();
    
    void beginFrame();
    void beginFrameWithBackground(int stageNumber);
    void endFrame();
    
    void renderCube(const glm::vec3& position, const glm::vec3& color, float size = 1.0f);
    void renderBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size);
    void renderRotatedBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                         const glm::vec3& rotationAxis, float rotationAngle);
    void renderBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha);
    void renderRotatedBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                                  const glm::vec3& rotationAxis, float rotationAngle, float alpha);
    void renderRealisticBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha = 1.0f);
    // 背景描画関数はBackgroundRendererクラスに移動済み
    // UI表示関数はUIRendererクラスに移動済み
    // ゲーム状態UI関数はGameStateUIRendererクラスに移動済み
    
    // 3D描画関数（残存）
    void renderStar3D(const glm::vec3& position, const glm::vec3& color, float scale = 1.0f);
    void renderLock3D(const glm::vec3& position, const glm::vec3& color, float scale = 1.0f);
    void renderNumber3D(const glm::vec3& position, int number, const glm::vec3& color, float scale = 1.0f);
    void renderXMark3D(const glm::vec3& position, const glm::vec3& color, float scale = 1.0f);
    
    void setCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));
    void setProjection(float fov, float aspect, float near, float far);

private:
    void drawCube(const glm::mat4& model, const glm::vec3& color);
    void renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale);
    
    GLFWwindow* window;
    
    // Matrices
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    // 分離された描画クラス
    BitmapFont font;
    BackgroundRenderer backgroundRenderer;
    UIRenderer uiRenderer;
    GameStateUIRenderer gameStateUIRenderer;
};

} // namespace gfx
