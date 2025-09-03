#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <functional>
#include "../game/game_constants.h"

namespace gfx {

class BackgroundRenderer {
public:
    BackgroundRenderer();
    ~BackgroundRenderer();
    
    // 背景描画関数
    void renderGrassland();
    void renderClouds();
    void renderSunset();
    void renderStars();
    void renderThunderClouds();
    void renderNebula();
    void renderStageBackground(int stageNumber);
    
    // 2D描画モードの設定
    void begin2DMode();
    void end2DMode();
    
    // 2Dモード管理の統一関数
    void renderWith2DMode(std::function<void()> renderFunc);
    
private:
    // 共通描画関数
    void renderCircle(float x, float y, float radius, int segments, const glm::vec3& color, float aspectRatio = 1.0f);
    void renderTriangleFan(float x, float y, float radius, int segments, const glm::vec3& color, float aspectRatio = 1.0f);
    void renderSquare(float x, float y, float size, const glm::vec3& color);
    void renderTriangle(float x, float y, float width, float height, const glm::vec3& color);
    // 背景描画のヘルパー関数
    void renderFlowers();
    void renderTrees();
    void renderHills();
};

} // namespace gfx
