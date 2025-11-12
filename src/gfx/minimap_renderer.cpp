#include "minimap_renderer.h"
#include "../core/constants/game_constants.h"
#include "texture_manager.h"
#include <algorithm>
#include <cmath>

namespace gfx {

MinimapRenderer::MinimapRenderer() {
}

void MinimapRenderer::render(const GameState& gameState, 
                            const PlatformSystem& platformSystem,
                            int windowWidth, 
                            int windowHeight,
                            int stageNumber,
                            float viewRange) {
    if (!m_visible) {
        return;
    }
    
    // 実際の表示範囲を使用（引数が指定されていない場合はメンバ変数を使用）
    float actualViewRange = (viewRange > 0.0f) ? viewRange : m_viewRange;
    
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    // ミニマップのサイズと位置を計算（画面右下）
    float scaleX = static_cast<float>(windowWidth) / 1280.0f;
    float scaleY = static_cast<float>(windowHeight) / 720.0f;
    float scale = std::min(scaleX, scaleY);
    
    float mapSize = m_mapSize * scale;
    float margin = m_margin * scale;
    glm::vec2 mapPosition(windowWidth - mapSize - margin, 
                         windowHeight - mapSize - margin);
    glm::vec2 mapCenter(mapPosition.x + mapSize / 2.0f,
                       mapPosition.y + mapSize / 2.0f);
    
    // スケール：viewRangeをmapSizeにマッピング
    float mapScale = mapSize / (actualViewRange * 2.0f);
    
    // 背景を描画（ステージ背景テクスチャを使用）
    renderBackground(mapPosition, mapSize, stageNumber);
    
    // 各要素を描画
    renderPlatforms(platformSystem, gameState.playerPosition, mapCenter, mapScale, actualViewRange, mapSize);
    renderItems(gameState, gameState.playerPosition, mapCenter, mapScale, actualViewRange, mapSize);
    renderGoal(gameState, gameState.playerPosition, mapCenter, mapScale, actualViewRange, mapSize);
    renderPlayer(mapCenter);
    renderBorder(mapPosition, mapSize);
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

glm::vec2 MinimapRenderer::worldToMapPosition(const glm::vec3& worldPos, 
                                              const glm::vec3& playerPos,
                                              const glm::vec2& mapCenter,
                                              float scale) const {
    // プレイヤーからの相対位置
    glm::vec3 relativePos = worldPos - playerPos;
    
    // X-Z平面を上から見たビューとして変換（X軸とZ軸の符号を調整）
    return glm::vec2(
        mapCenter.x - relativePos.x * scale,  // X軸を反転
        mapCenter.y - relativePos.z * scale   // Z軸を反転
    );
}

void MinimapRenderer::renderBackground(const glm::vec2& position, float size, int stageNumber) const {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // ステージ背景テクスチャを取得
    GLuint stageTexture = getStageBackgroundTexture(stageNumber);
    
    glm::vec2 center(position.x + size / 2.0f, position.y + size / 2.0f);
    float radius = size / 2.0f;
    
    if (stageTexture != 0) {
        // テクスチャを有効化
        glEnable(GL_TEXTURE_2D);
        TextureManager::bindTexture(stageTexture);
        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);  // 少し透明にして見やすく
        
        // ステンシルバッファを使って円形のマスクを作成
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);
        
        // 円形のマスクを描画（ステンシルバッファに書き込む）
        int segments = 32;
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);  // 色は描画しない
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(center.x, center.y);
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * 3.14159265359f * i / segments;
            glVertex2f(center.x + radius * std::cos(angle), 
                       center.y + radius * std::sin(angle));
        }
        glEnd();
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);  // 色の描画を再有効化
        
        // ステンシルバッファのマスク内にのみテクスチャを描画
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilMask(0x00);
        
        // テクスチャを四角形で描画（円形のマスクが適用される）
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(position.x, position.y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(position.x + size, position.y);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(position.x + size, position.y + size);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(position.x, position.y + size);
        glEnd();
        
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_TEXTURE_2D);
    } else {
        // テクスチャがない場合は黒い背景を描画
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        int segments = 32;
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(center.x, center.y);
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * 3.14159265359f * i / segments;
            glVertex2f(center.x + radius * std::cos(angle), 
                       center.y + radius * std::sin(angle));
        }
        glEnd();
    }
    
    glDisable(GL_BLEND);
}

void MinimapRenderer::renderPlatforms(const PlatformSystem& platformSystem,
                                     const glm::vec3& playerPos,
                                     const glm::vec2& mapCenter,
                                     float scale,
                                     float viewRange,
                                     float mapSize) const {
    auto positions = platformSystem.getPositions();
    auto sizes = platformSystem.getSizes();
    auto visibility = platformSystem.getVisibility();
    
    for (size_t i = 0; i < positions.size(); i++) {
        if (!visibility[i]) {
            continue;
        }
        
        // プレイヤーからの相対位置
        glm::vec3 relativePos = positions[i] - playerPos;
        
        // 表示範囲内かチェック
        if (std::abs(relativePos.x) > viewRange || std::abs(relativePos.z) > viewRange) {
            continue;
        }
        
        // マップ座標に変換
        glm::vec2 mapPos = worldToMapPosition(positions[i], playerPos, mapCenter, scale);
        float platformSize = sizes[i].x * scale * 1.7f;  // サイズを1.5倍に（元に戻す）
        
        // 円形の範囲内かチェック
        float distanceFromCenter = std::sqrt(
            (mapPos.x - mapCenter.x) * (mapPos.x - mapCenter.x) +
            (mapPos.y - mapCenter.y) * (mapPos.y - mapCenter.y)
        );
        float mapRadius = mapSize / 2.0f;
        if (distanceFromCenter + platformSize / 2.0f > mapRadius) {
            continue;  // 円の外側にある場合はスキップ
        }
        
        // 足場を描画（テクスチャを使用）
        auto platformTypes = platformSystem.getPlatformTypes();
        std::string platformType = (i < platformTypes.size()) ? platformTypes[i] : "static";
        
        // 足場のテクスチャを取得
        GLuint platformTexture = getPlatformTexture(platformType);
        if (platformTexture != 0) {
            renderTexturedQuad(mapPos, platformSize, platformTexture);
        } else {
            // テクスチャがない場合は色で描画
            glColor3f(0.5f, 0.5f, 0.5f);
            glBegin(GL_QUADS);
            glVertex2f(mapPos.x - platformSize / 2.0f, mapPos.y - platformSize / 2.0f);
            glVertex2f(mapPos.x + platformSize / 2.0f, mapPos.y - platformSize / 2.0f);
            glVertex2f(mapPos.x + platformSize / 2.0f, mapPos.y + platformSize / 2.0f);
            glVertex2f(mapPos.x - platformSize / 2.0f, mapPos.y + platformSize / 2.0f);
            glEnd();
        }
    }
}

void MinimapRenderer::renderItems(const GameState& gameState,
                                 const glm::vec3& playerPos,
                                 const glm::vec2& mapCenter,
                                 float scale,
                                 float viewRange,
                                 float mapSize) const {
    for (const auto& item : gameState.items) {
        if (item.isCollected) {
            continue;
        }
        
        glm::vec3 relativePos = item.position - playerPos;
        if (std::abs(relativePos.x) > viewRange || std::abs(relativePos.z) > viewRange) {
            continue;
        }
        
        glm::vec2 mapPos = worldToMapPosition(item.position, playerPos, mapCenter, scale);
        
        // 円形の範囲内かチェック
        float distanceFromCenter = std::sqrt(
            (mapPos.x - mapCenter.x) * (mapPos.x - mapCenter.x) +
            (mapPos.y - mapCenter.y) * (mapPos.y - mapCenter.y)
        );
        float mapRadius = mapSize / 2.0f;
        float itemSize = 16.0f;  // アイテムのサイズを元に戻す（12.0f → 6.0f）
        if (distanceFromCenter + itemSize / 2.0f > mapRadius) {
            continue;  // 円の外側にある場合はスキップ
        }
        
        // アイテムを描画（テクスチャを使用）
        // 色に基づいてテクスチャを選択（game_loop.cppと同じロジック）
        bool isRedItem = (item.color.r > 0.9f && item.color.g < 0.1f && item.color.b < 0.1f);
        bool isGreenItem = (item.color.r < 0.1f && item.color.g > 0.9f && item.color.b < 0.1f);
        bool isBlueItem = (item.color.r < 0.1f && item.color.g < 0.1f && item.color.b > 0.9f);
        
        GLuint itemTexture = 0;
        if (isRedItem) {
            itemTexture = loadTexture("assets/textures/item_first.png");
            if (itemTexture == 0) {
                itemTexture = loadTexture("../assets/textures/item_first.png");
            }
        } else if (isGreenItem) {
            itemTexture = loadTexture("assets/textures/item_second.png");
            if (itemTexture == 0) {
                itemTexture = loadTexture("../assets/textures/item_second.png");
            }
        } else if (isBlueItem) {
            itemTexture = loadTexture("assets/textures/item_third.png");
            if (itemTexture == 0) {
                itemTexture = loadTexture("../assets/textures/item_third.png");
            }
        }
        
        if (itemTexture != 0) {
            renderTexturedQuad(mapPos, itemSize, itemTexture);
        } else {
            // テクスチャがない場合は色で描画
            glColor3f(item.color.r, item.color.g, item.color.b);
            glBegin(GL_QUADS);
            glVertex2f(mapPos.x - itemSize, mapPos.y - itemSize);
            glVertex2f(mapPos.x + itemSize, mapPos.y - itemSize);
            glVertex2f(mapPos.x + itemSize, mapPos.y + itemSize);
            glVertex2f(mapPos.x - itemSize, mapPos.y + itemSize);
            glEnd();
        }
    }
}

void MinimapRenderer::renderGoal(const GameState& gameState,
                                const glm::vec3& playerPos,
                                const glm::vec2& mapCenter,
                                float scale,
                                float viewRange,
                                float mapSize) const {
    glm::vec3 relativeGoalPos = gameState.goalPosition - playerPos;
    if (std::abs(relativeGoalPos.x) > viewRange || std::abs(relativeGoalPos.z) > viewRange) {
        return;
    }
    
    glm::vec2 mapPos = worldToMapPosition(gameState.goalPosition, playerPos, mapCenter, scale);
    
    // 円形の範囲内かチェック
    float distanceFromCenter = std::sqrt(
        (mapPos.x - mapCenter.x) * (mapPos.x - mapCenter.x) +
        (mapPos.y - mapCenter.y) * (mapPos.y - mapCenter.y)
    );
    float mapRadius = mapSize / 2.0f;
    float goalSize = 8.0f;  // ゴールのサイズを元に戻す（16.0f → 8.0f）
    if (distanceFromCenter + goalSize / 2.0f > mapRadius) {
        return;  // 円の外側にある場合はスキップ
    }
    
    // ゴールを描画（テクスチャを使用）
    GLuint goalTexture = loadTexture("assets/textures/goal_platform.png");
    if (goalTexture == 0) {
        goalTexture = loadTexture("../assets/textures/goal_platform.png");
    }
    if (goalTexture != 0) {
        renderTexturedQuad(mapPos, goalSize, goalTexture);
    } else {
        // テクスチャがない場合は色で描画
        glColor3f(1.0f, 1.0f, 0.0f);  // 黄色
        glBegin(GL_QUADS);
        glVertex2f(mapPos.x - goalSize, mapPos.y - goalSize);
        glVertex2f(mapPos.x + goalSize, mapPos.y - goalSize);
        glVertex2f(mapPos.x + goalSize, mapPos.y + goalSize);
        glVertex2f(mapPos.x - goalSize, mapPos.y + goalSize);
        glEnd();
    }
}

void MinimapRenderer::renderPlayer(const glm::vec2& mapCenter) const {
    // プレイヤーを描画（テクスチャを使用）
    GLuint playerTexture = loadTexture("assets/textures/player_front.png");
    if (playerTexture == 0) {
        playerTexture = loadTexture("../assets/textures/player_front.png");
    }
    
    float playerSize = 20.0f;  // プレイヤーのサイズを元に戻す（24.0f → 12.0f）
    if (playerTexture != 0) {
        renderTexturedQuad(mapCenter, playerSize, playerTexture);
    } else {
        // テクスチャがない場合は矢印で描画
        glColor3f(1.0f, 0.0f, 1.0f);  // マゼンタ
        glBegin(GL_TRIANGLES);
        float arrowSize = 12.0f;  // 矢印のサイズも元に戻す
        glVertex2f(mapCenter.x, mapCenter.y - arrowSize);
        glVertex2f(mapCenter.x - arrowSize * 0.6f, mapCenter.y + arrowSize * 0.6f);
        glVertex2f(mapCenter.x + arrowSize * 0.6f, mapCenter.y + arrowSize * 0.6f);
        glEnd();
    }
}

void MinimapRenderer::renderBorder(const glm::vec2& position, float size) const {
    // 円形の境界線を描画
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    
    glm::vec2 center(position.x + size / 2.0f, position.y + size / 2.0f);
    float radius = size / 2.0f;
    int segments = 32;  // 円の分割数
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * 3.14159265359f * i / segments;
        glVertex2f(center.x + radius * std::cos(angle), 
                   center.y + radius * std::sin(angle));
    }
    glEnd();
    
    glLineWidth(1.0f);
}

GLuint MinimapRenderer::loadTexture(const std::string& filename) const {
    return TextureManager::loadTexture(filename);
}

void MinimapRenderer::renderTexturedQuad(const glm::vec2& position, float size, GLuint textureID) const {
    if (textureID == 0) {
        return;
    }
    
    glEnable(GL_TEXTURE_2D);
    TextureManager::bindTexture(textureID);
    glColor3f(1.0f, 1.0f, 1.0f);  // テクスチャの色をそのまま使用
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(position.x - size / 2.0f, position.y - size / 2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(position.x + size / 2.0f, position.y - size / 2.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(position.x + size / 2.0f, position.y + size / 2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(position.x - size / 2.0f, position.y + size / 2.0f);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
}

GLuint MinimapRenderer::getPlatformTexture(const std::string& platformType) const {
    // プラットフォームタイプに応じたテクスチャを返す
    std::string textureName;
    
    if (platformType == "static") {
        textureName = "static_platform.png";
    } else if (platformType == "moving") {
        textureName = "moving_platform.png";
    } else if (platformType == "rotating") {
        textureName = "rotating_platform.png";
    } else if (platformType == "patrolling") {
        textureName = "moving_platform.png";  // 巡回足場は移動足場と同じテクスチャ
    } else if (platformType == "teleport") {
        textureName = "teleport_platform.png";
    } else if (platformType == "jumppad") {
        textureName = "jumppad_platform.png";
    } else if (platformType == "cycle_disappearing") {
        textureName = "cyclingdisappearing_platform.png";
    } else if (platformType == "disappearing") {
        textureName = "disappearing_platform.png";
    } else if (platformType == "flying") {
        textureName = "flying_platform.png";
    } else {
        textureName = "static_platform.png";  // デフォルト
    }
    
    // テクスチャを読み込む（複数のパスを試す）
    GLuint tex = loadTexture("assets/textures/" + textureName);
    if (tex == 0) {
        tex = loadTexture("../assets/textures/" + textureName);
    }
    return tex;
}

GLuint MinimapRenderer::getItemTexture(int itemId) const {
    // アイテムIDに応じたテクスチャを返す
    std::string filename;
    if (itemId == 0) {
        filename = "assets/textures/item_first.png";
    } else if (itemId == 1) {
        filename = "assets/textures/item_second.png";
    } else if (itemId == 2) {
        filename = "assets/textures/item_third.png";
    } else {
        filename = "assets/textures/item_first.png";  // デフォルト
    }
    
    GLuint tex = loadTexture(filename);
    if (tex == 0) {
        // 代替パスを試す
        if (filename.find("assets/") == 0) {
            filename = "../" + filename;
            tex = loadTexture(filename);
        }
    }
    return tex;
}

GLuint MinimapRenderer::getStageBackgroundTexture(int stageNumber) const {
    // ステージ番号に応じた背景テクスチャを返す
    std::string textureName;
    
    switch (stageNumber) {
        case 0:
            textureName = "stage_selection_bg.png";
            break;
        case 1:
            textureName = "stage1_bg.png";
            break;
        case 2:
            textureName = "stage2_bg.png";
            break;
        case 3:
            textureName = "stage3_bg.png";
            break;
        case 4:
            textureName = "stage4_bg.png";
            break;
        case 5:
            textureName = "stage5_bg.png";
            break;
        case 6:
            textureName = "tutorial_bg.png";
            break;
        default:
            textureName = "stage1_bg.png";  // デフォルト
            break;
    }
    
    // テクスチャを読み込む（複数のパスを試す）
    GLuint tex = loadTexture("assets/textures/" + textureName);
    if (tex == 0) {
        tex = loadTexture("../assets/textures/" + textureName);
    }
    return tex;
}

} // namespace gfx

