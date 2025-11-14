#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <functional>
#include "../core/constants/game_constants.h"
#include "texture_manager.h"

namespace gfx {

/**
 * @brief 背景レンダラー
 * @details ステージ背景の描画を管理します。
 */
class BackgroundRenderer {
public:
    BackgroundRenderer();
    ~BackgroundRenderer();
    
    /**
     * @brief 草原背景を描画する
     */
    void renderGrassland();
    
    /**
     * @brief 雲背景を描画する
     */
    void renderClouds();
    
    /**
     * @brief 夕焼け背景を描画する
     */
    void renderSunset();
    
    /**
     * @brief 星空背景を描画する
     */
    void renderStars();
    
    /**
     * @brief 雷雲背景を描画する
     */
    void renderThunderClouds();
    
    /**
     * @brief 星雲背景を描画する
     */
    void renderNebula();
    
    /**
     * @brief ステージ背景を描画する
     * @details ステージ番号に応じた背景を描画します。
     * 
     * @param stageNumber ステージ番号
     */
    void renderStageBackground(int stageNumber);
    
    /**
     * @brief ステージ1の背景をテクスチャで描画する
     */
    void renderStage1WithTexture();
    
    /**
     * @brief ステージ2の背景をテクスチャで描画する
     */
    void renderStage2WithTexture();
    
    /**
     * @brief ステージ3の背景をテクスチャで描画する
     */
    void renderStage3WithTexture();
    
    /**
     * @brief ステージ4の背景をテクスチャで描画する
     */
    void renderStage4WithTexture();
    
    /**
     * @brief ステージ5の背景をテクスチャで描画する
     */
    void renderStage5WithTexture();
    
    /**
     * @brief ステージ選択画面の背景をテクスチャで描画する
     */
    void renderStageSelectionWithTexture();
    
    /**
     * @brief チュートリアル画面の背景をテクスチャで描画する
     */
    void renderTutorialWithTexture();
    
    /**
     * @brief 2D描画モードを開始する
     */
    void begin2DMode();
    
    /**
     * @brief 2D描画モードを終了する
     */
    void end2DMode();
    
    /**
     * @brief 2Dモードで描画を実行する
     * @details 2D描画モードの開始・終了を自動的に管理します。
     * 
     * @param renderFunc 描画関数
     */
    void renderWith2DMode(std::function<void()> renderFunc);
    
private:
    void renderCircle(float x, float y, float radius, int segments, const glm::vec3& color, float aspectRatio = 1.0f);
    void renderTriangleFan(float x, float y, float radius, int segments, const glm::vec3& color, float aspectRatio = 1.0f);
    void renderSquare(float x, float y, float size, const glm::vec3& color);
    void renderTriangle(float x, float y, float width, float height, const glm::vec3& color);
    void renderFlowers();
    void renderTrees();
    void renderHills();
};

} // namespace gfx
