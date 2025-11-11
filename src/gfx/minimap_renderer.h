#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "../game/game_state.h"
#include "../game/platform_system.h"
#include "texture_manager.h"

namespace gfx {

/**
 * ミニマップ描画クラス
 * Single Responsibility: ミニマップの描画のみを担当
 */
class MinimapRenderer {
public:
    MinimapRenderer();
    ~MinimapRenderer() = default;
    
    /**
     * ミニマップを描画
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @param windowWidth ウィンドウ幅
     * @param windowHeight ウィンドウ高さ
     * @param stageNumber ステージ番号（背景テクスチャ選択用）
     * @param viewRange 表示範囲（プレイヤー中心から±viewRange単位）
     */
    void render(const GameState& gameState, 
                const PlatformSystem& platformSystem,
                int windowWidth, 
                int windowHeight,
                int stageNumber,
                float viewRange = 30.0f);
    
    /**
     * ミニマップの表示/非表示を設定
     */
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }
    
    /**
     * 表示範囲を設定
     */
    void setViewRange(float range) { m_viewRange = range; }
    float getViewRange() const { return m_viewRange; }

private:
    /**
     * 3D座標を2Dマップ座標に変換
     * @param worldPos 3Dワールド座標
     * @param playerPos プレイヤー位置（中心）
     * @param mapCenter マップの中心座標（画面座標）
     * @param scale スケール
     * @return 2Dマップ座標
     */
    glm::vec2 worldToMapPosition(const glm::vec3& worldPos, 
                                  const glm::vec3& playerPos,
                                  const glm::vec2& mapCenter,
                                  float scale) const;
    
    /**
     * マップの背景を描画（ステージ背景テクスチャを使用）
     */
    void renderBackground(const glm::vec2& position, float size, int stageNumber) const;
    
    /**
     * ステージ番号に応じた背景テクスチャを取得
     */
    GLuint getStageBackgroundTexture(int stageNumber) const;
    
    /**
     * 足場を描画
     */
    void renderPlatforms(const PlatformSystem& platformSystem,
                        const glm::vec3& playerPos,
                        const glm::vec2& mapCenter,
                        float scale,
                        float viewRange,
                        float mapSize) const;
    
    /**
     * アイテムを描画
     */
    void renderItems(const GameState& gameState,
                    const glm::vec3& playerPos,
                    const glm::vec2& mapCenter,
                    float scale,
                    float viewRange,
                    float mapSize) const;
    
    /**
     * ゴールを描画
     */
    void renderGoal(const GameState& gameState,
                   const glm::vec3& playerPos,
                   const glm::vec2& mapCenter,
                   float scale,
                   float viewRange,
                   float mapSize) const;
    
    /**
     * プレイヤーを描画
     */
    void renderPlayer(const glm::vec2& mapCenter) const;
    
    /**
     * 境界線を描画
     */
    void renderBorder(const glm::vec2& position, float size) const;
    
    /**
     * テクスチャ付きの2D四角形を描画（ヘルパー関数）
     */
    void renderTexturedQuad(const glm::vec2& position, float size, GLuint textureID) const;
    
    /**
     * テクスチャを読み込む（初回のみ、キャッシュされる）
     */
    GLuint loadTexture(const std::string& filename) const;
    
    /**
     * プラットフォームタイプに応じたテクスチャを取得
     */
    GLuint getPlatformTexture(const std::string& platformType) const;
    
    /**
     * アイテムIDに応じたテクスチャを取得
     */
    GLuint getItemTexture(int itemId) const;
    
    bool m_visible = true;
    float m_viewRange = 30.0f;
    float m_mapSize = 200.0f;  // 基準サイズ（1280x720時）
    float m_margin = 20.0f;     // 画面端からのマージン
};

} // namespace gfx

