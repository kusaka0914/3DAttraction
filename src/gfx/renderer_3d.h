#pragma once

#include <glm/glm.hpp>
#include <string>

// OpenGL型の前方宣言
typedef unsigned int GLuint;

namespace gfx {

class Renderer3D {
public:
    Renderer3D();
    ~Renderer3D();

    // 3Dキューブ描画
    void renderCube(const glm::vec3& position, const glm::vec3& color, float size);
    void renderRotatedBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                          const glm::vec3& rotationAxis, float rotationAngle);
    void renderBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha);
    void renderRotatedBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                                  const glm::vec3& rotationAxis, float rotationAngle, float alpha);
    void renderRealisticBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha);
    
    // テクスチャ対応の描画
    void renderTexturedBox(const glm::vec3& position, const glm::vec3& size, GLuint textureID);
    void renderTexturedBox(const glm::vec3& position, const glm::vec3& size, GLuint frontTextureID, GLuint otherTextureID);
    void renderTexturedBoxWithAlpha(const glm::vec3& position, const glm::vec3& size, GLuint textureID, float alpha);
    void renderTexturedRotatedBox(const glm::vec3& position, const glm::vec3& size, GLuint textureID, 
                                 const glm::vec3& rotationAxis, float rotationAngle);
    
    // 3D図形描画
    void renderStar3D(const glm::vec3& position, const glm::vec3& color, float scale);
    void renderNumber3D(const glm::vec3& position, int number, const glm::vec3& color, float scale);
    void renderXMark3D(const glm::vec3& position, const glm::vec3& color, float scale);

private:
    void drawCube(const glm::mat4& model, const glm::vec3& color);
    void drawTexturedCube(const glm::mat4& model, GLuint textureID, float alpha = 1.0f);
    void drawTexturedCubeWithFrontFace(const glm::mat4& model, GLuint frontTextureID, GLuint otherTextureID);
};

} // namespace gfx
