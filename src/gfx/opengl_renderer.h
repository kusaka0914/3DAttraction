#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "bitmap_font.h"
#include "background_renderer.h"
#include "ui_renderer.h"
#include "game_state_ui_renderer.h"
#include "renderer_3d.h"
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
    
    // 3D描画関数はRenderer3Dクラスに移動
    
    void setCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));
    void setProjection(float fov, float aspect, float near, float far);

private:
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
public:
    Renderer3D renderer3D;
private:
};

} // namespace gfx
