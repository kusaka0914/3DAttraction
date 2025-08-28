#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <map>

namespace gfx {

struct GlyphInfo {
    float x, y, width, height;  // テクスチャ座標
    float advanceX;             // 次の文字までの距離
    float bearingX, bearingY;   // 文字の基準位置
};

class OpenGLRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();
    
    bool initialize(GLFWwindow* window);
    void cleanup();
    
    void beginFrame();
    void endFrame();
    
    void renderCube(const glm::vec3& position, const glm::vec3& color, float size = 1.0f);
    void renderBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size);
    void renderRotatedBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                         const glm::vec3& rotationAxis, float rotationAngle);
    void renderBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha);
    void renderRotatedBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                                  const glm::vec3& rotationAxis, float rotationAngle, float alpha);
    void renderTriangle(const glm::vec3& position, const glm::vec3& color, float size = 1.0f);
    void renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale = 1.0f);
    void renderGround(float size = 10.0f);
    void renderTerrain(const std::vector<std::vector<float>>& heightmap, float scale = 1.0f);
    
    // UI表示関数
    void renderTimeUI(float remainingTime, float timeLimit, int earnedStars, int existingStars = 0);
    void renderStar(const glm::vec2& position, const glm::vec3& color, float scale = 1.0f);
    
    void setCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));
    void setProjection(float fov, float aspect, float near, float far);

private:
    void drawCube(const glm::mat4& model, const glm::vec3& color);
    void drawTriangle(const glm::mat4& model, const glm::vec3& color);
    void initializeBitmapFont();
    void renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale);
    void initializeTextureFont();
    void renderTextureText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale);
    
    GLFWwindow* window;
    
    // Matrices
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    
    // ビットマップフォント用のデータ
    std::map<char, std::vector<bool>> bitmapFont;
    bool fontInitialized;
    
    // テクスチャフォント用のデータ
    GLuint fontTexture;
    std::map<char, GlyphInfo> glyphs;
    bool textureFontInitialized;
    float fontAtlasWidth, fontAtlasHeight;
};

} // namespace gfx
