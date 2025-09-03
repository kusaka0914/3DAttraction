#pragma once

#include <glm/glm.hpp>
#include <string>

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
    
    // 3D図形描画
    void renderStar3D(const glm::vec3& position, const glm::vec3& color, float scale);
    void renderNumber3D(const glm::vec3& position, int number, const glm::vec3& color, float scale);
    void renderXMark3D(const glm::vec3& position, const glm::vec3& color, float scale);

private:
    void drawCube(const glm::mat4& model, const glm::vec3& color);
};

} // namespace gfx
