#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
    void renderStageBackground(int stageNumber);
    void renderGrassland();
    void renderClouds();
    void renderSunset();
    void renderStars();
    void renderThunderClouds();
    void renderNebula();
    void renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale = 1.0f);
    
    // UI表示関数
    void renderTimeUI(float remainingTime, float timeLimit, int earnedStars, int existingStars, int lives);
    void renderStar(const glm::vec2& position, const glm::vec3& color, float scale = 1.0f);
    void renderStar3D(const glm::vec3& position, const glm::vec3& color, float scale = 1.0f);
    void renderLock3D(const glm::vec3& position, const glm::vec3& color, float scale = 1.0f);
    void renderNumber3D(const glm::vec3& position, int number, const glm::vec3& color, float scale = 1.0f);
    void renderXMark3D(const glm::vec3& position, const glm::vec3& color, float scale = 1.0f);
    void renderTutorialUI(int width, int height);
    void renderStageClearBackground(int width, int height, float clearTime, int earnedStars);
    void renderGameOverBackground(int width, int height);
    void renderReadyScreen(int width, int height, int speedLevel);
    void renderCountdown(int width, int height, int count);
    void renderHeart(const glm::vec2& position, const glm::vec3& color, float scale = 1.0f);
    
    void setCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));
    void setProjection(float fov, float aspect, float near, float far);

private:
    void drawCube(const glm::mat4& model, const glm::vec3& color);
    void initializeBitmapFont();
    void renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale);
    
    GLFWwindow* window;
    
    // Matrices
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    
    // ビットマップフォント用のデータ
    std::map<char, std::vector<bool>> bitmapFont;
    bool fontInitialized;
    

};

} // namespace gfx
