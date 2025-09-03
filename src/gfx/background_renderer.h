#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "../app/game_constants.h"

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
    
private:
    // 背景描画のヘルパー関数
    void renderFlowers();
    void renderTrees();
    void renderHills();
};

} // namespace gfx
