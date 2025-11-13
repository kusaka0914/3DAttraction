#define _USE_MATH_DEFINES
#include "background_renderer.h"
#include "../core/utils/resource_path.h"
#include <iostream>
#include <cmath>

namespace gfx {

BackgroundRenderer::BackgroundRenderer() {
}

BackgroundRenderer::~BackgroundRenderer() {
}

void BackgroundRenderer::begin2DMode() {
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1280, 720, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
}

void BackgroundRenderer::end2DMode() {
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void BackgroundRenderer::renderStageBackground(int stageNumber) {
    begin2DMode();
    
    // ステージ0（選択フィールド）、1、2、3、4、5、6（チュートリアル）は画像ベースの背景を使用
    if (stageNumber == 0) {
        renderStageSelectionWithTexture();
        end2DMode();
        return;
    }
    
    if (stageNumber == 1) {
        renderStage1WithTexture();
        end2DMode();
        return;
    }
    
    if (stageNumber == 2) {
        renderStage2WithTexture();
        end2DMode();
        return;
    }
    
    if (stageNumber == 3) {
        renderStage3WithTexture();
        end2DMode();
        return;
    }
    
    if (stageNumber == 4) {
        renderStage4WithTexture();
        end2DMode();
        return;
    }
    
    if (stageNumber == 5) {
        renderStage5WithTexture();
        end2DMode();
        return;
    }
    
    if (stageNumber == 6) {
        renderTutorialWithTexture();
        end2DMode();
        return;
    }
    
    // ステージ別の背景色とグラデーション
    glm::vec3 topColor, bottomColor;
    
    switch (stageNumber) {
        case 0: // ステージ選択画面
            topColor = GameConstants::RenderConstants::STAGE_0_TOP_COLOR;
            bottomColor = GameConstants::RenderConstants::STAGE_0_BOTTOM_COLOR;
            break;
        case 2: // ステージ2 - 夕日、オレンジ色の空（中級者向けの暖かい雰囲気）
            topColor = GameConstants::RenderConstants::STAGE_2_TOP_COLOR;
            bottomColor = GameConstants::RenderConstants::STAGE_2_BOTTOM_COLOR;
            break;
        case 3: // ステージ3 - 夜、星空（上級者向けの神秘的雰囲気）
            topColor = GameConstants::RenderConstants::STAGE_3_TOP_COLOR;
            bottomColor = GameConstants::RenderConstants::STAGE_3_BOTTOM_COLOR;
            break;
        case 4: // ステージ4 - 雷雲、稲妻（難易度の高い雰囲気）
            topColor = GameConstants::RenderConstants::STAGE_4_TOP_COLOR;
            bottomColor = GameConstants::RenderConstants::STAGE_4_BOTTOM_COLOR;
            break;
        case 5: // ステージ5 - 宇宙、星雲（最終ステージの壮大な雰囲気）
            topColor = GameConstants::RenderConstants::STAGE_5_TOP_COLOR;
            bottomColor = GameConstants::RenderConstants::STAGE_5_BOTTOM_COLOR;
            break;
        default:
            topColor = GameConstants::RenderConstants::DEFAULT_BACKGROUND_COLOR;
            bottomColor = glm::vec3(0.4f, 0.5f, 0.5f);
            break;
    }
    
    // 背景を描画（ステージ0は空と草原を分離）
    if (stageNumber == 0) {
        // 空の部分（上半分）
        glBegin(GL_QUADS);
        glColor3f(topColor.r, topColor.g, topColor.b);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glVertex2f(1280, 360);
        glVertex2f(0, 360);
        glEnd();
        
        // 草原の部分（下半分）
        glBegin(GL_QUADS);
        glColor3f(bottomColor.r, bottomColor.g, bottomColor.b);
        glVertex2f(0, 360);
        glVertex2f(1280, 360);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();
    } else {
        // 他のステージはグラデーション
        glBegin(GL_QUADS);
        glColor3f(topColor.r, topColor.g, topColor.b);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glColor3f(bottomColor.r, bottomColor.g, bottomColor.b);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();
    }
    
    // ステージ別の装飾要素を追加
    switch (stageNumber) {
        case 0: // ステージ選択画面 - 草原
            renderGrassland();
            break;
        case 1: // ステージ1 - 雲
            renderClouds();
            break;
        case 2: // ステージ2 - 夕日
            renderSunset();
            break;
        case 3: // ステージ3 - 星
            renderStars();
            break;
        case 4: // ステージ4 - 雷雲
            renderThunderClouds();
            break;
        case 5: // ステージ5 - 星雲
            renderNebula();
            break;
    }
    
    end2DMode();
}

// 共通描画関数
void BackgroundRenderer::renderCircle(float x, float y, float radius, int segments, 
                                     const glm::vec3& color, float aspectRatio) {
    glColor3f(color.r, color.g, color.b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int j = 0; j <= segments; j++) {
        float angle = 2.0f * M_PI * j / segments;
        float drawX = x + cos(angle) * radius;
        float drawY = y + sin(angle) * (radius * aspectRatio);
        glVertex2f(drawX, drawY);
    }
    glEnd();
}

void BackgroundRenderer::renderTriangleFan(float x, float y, float radius, int segments, 
                                         const glm::vec3& color, float aspectRatio) {
    renderCircle(x, y, radius, segments, color, aspectRatio);
}

void BackgroundRenderer::renderSquare(float x, float y, float size, const glm::vec3& color) {
    glColor3f(color.r, color.g, color.b);
    glBegin(GL_QUADS);
    glVertex2f(x - size, y - size);
    glVertex2f(x + size, y - size);
    glVertex2f(x + size, y + size);
    glVertex2f(x - size, y + size);
    glEnd();
}

void BackgroundRenderer::renderTriangle(float x, float y, float width, float height, const glm::vec3& color) {
    glColor3f(color.r, color.g, color.b);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y);
    glVertex2f(x - width, y - height);
    glVertex2f(x + width, y - height);
    glEnd();
}

// 2Dモード管理の統一関数
void BackgroundRenderer::renderWith2DMode(std::function<void()> renderFunc) {
    begin2DMode();
    renderFunc();
    end2DMode();
}

void BackgroundRenderer::renderGrassland() {
    // 雲を描画（空の部分）
    for (int i = 0; i < GameConstants::RenderConstants::BackgroundLayout::CLOUD_COUNT_STAGE_0; i++) {
        float x = fmod(i * GameConstants::RenderConstants::BackgroundLayout::CLOUD_BASE_SPACING + 
                       (i * GameConstants::RenderConstants::BackgroundLayout::CLOUD_SPACING_OFFSET), 
                       GameConstants::RenderConstants::BackgroundLayout::SCREEN_WIDTH);
        float y = GameConstants::RenderConstants::BackgroundLayout::CLOUD_BASE_Y + 
                  (i * GameConstants::RenderConstants::BackgroundLayout::CLOUD_Y_OFFSET);
        float size = GameConstants::RenderConstants::BackgroundLayout::CLOUD_BASE_SIZE + 
                     (i * GameConstants::RenderConstants::BackgroundLayout::CLOUD_SIZE_OFFSET);
        
        renderCircle(x, y, size, GameConstants::RenderConstants::BackgroundLayout::CLOUD_SEGMENTS, 
                    glm::vec3(1.0f, 1.0f, 1.0f), 
                    GameConstants::RenderConstants::BackgroundLayout::CLOUD_ASPECT_RATIO);
    }
    
    // 草を描画
    for (int i = 0; i < GameConstants::RenderConstants::BackgroundLayout::GRASS_COUNT; i++) {
        float x = fmod(i * GameConstants::RenderConstants::BackgroundLayout::GRASS_SPACING + 
                       (i * GameConstants::RenderConstants::BackgroundLayout::GRASS_SPACING_OFFSET), 
                       GameConstants::RenderConstants::BackgroundLayout::SCREEN_WIDTH);
        float y = GameConstants::RenderConstants::BackgroundLayout::SCREEN_HEIGHT - 
                  fmod(i * GameConstants::RenderConstants::BackgroundLayout::GRASS_BASE_Y_OFFSET + 
                       (i * GameConstants::RenderConstants::BackgroundLayout::GRASS_Y_OFFSET), 200.0f);
        float height = GameConstants::RenderConstants::BackgroundLayout::GRASS_HEIGHT_BASE + 
                       (i % 5) * GameConstants::RenderConstants::BackgroundLayout::GRASS_HEIGHT_VARIATION;
        float width = GameConstants::RenderConstants::BackgroundLayout::GRASS_WIDTH_BASE + 
                      (i % GameConstants::RenderConstants::BackgroundLayout::GRASS_WIDTH_VARIATION);
        
        renderTriangle(x, y, width, height, glm::vec3(0.1f, 0.6f, 0.1f));
    }
    
    // 花を描画
    renderFlowers();
    
    // 木を描画
    renderTrees();
}

void BackgroundRenderer::renderFlowers() {
    for (int i = 0; i < GameConstants::RenderConstants::BackgroundLayout::FLOWER_COUNT; i++) {
        float x = fmod(i * GameConstants::RenderConstants::BackgroundLayout::FLOWER_SPACING + 
                       (i * GameConstants::RenderConstants::BackgroundLayout::FLOWER_SPACING_OFFSET), 
                       GameConstants::RenderConstants::BackgroundLayout::SCREEN_WIDTH);
        float y = GameConstants::RenderConstants::BackgroundLayout::SCREEN_HEIGHT - 
                  fmod(i * GameConstants::RenderConstants::BackgroundLayout::FLOWER_BASE_Y_OFFSET + 
                       (i * GameConstants::RenderConstants::BackgroundLayout::FLOWER_Y_OFFSET), 150.0f);
        float size = GameConstants::RenderConstants::BackgroundLayout::FLOWER_SIZE_BASE + 
                     (i % 3) * GameConstants::RenderConstants::BackgroundLayout::FLOWER_SIZE_VARIATION;
        
        // 花の色（ランダム）
        glm::vec3 flowerColors[] = {
            GameConstants::Colors::FLOWER_RED,     // 赤
            GameConstants::Colors::FLOWER_YELLOW,  // 黄色
            GameConstants::Colors::FLOWER_PINK,    // ピンク
            GameConstants::Colors::FLOWER_WHITE    // 白
        };
        glm::vec3 flowerColor = flowerColors[i % 4];
        
        renderCircle(x, y, size, GameConstants::RenderConstants::BackgroundLayout::FLOWER_SEGMENTS, flowerColor, 1.0f);
    }
}

void BackgroundRenderer::renderTrees() {
    for (int i = 0; i < GameConstants::RenderConstants::BackgroundLayout::TREE_COUNT; i++) {
        float x = GameConstants::RenderConstants::BackgroundLayout::TREE_START_X + 
                  i * GameConstants::RenderConstants::BackgroundLayout::TREE_SPACING;
        float y = GameConstants::RenderConstants::BackgroundLayout::SCREEN_HEIGHT - 
                  GameConstants::RenderConstants::BackgroundLayout::TREE_BASE_Y;
        
        // 木の幹
        glColor3f(GameConstants::Colors::TREE_TRUNK.r, 
                  GameConstants::Colors::TREE_TRUNK.g, 
                  GameConstants::Colors::TREE_TRUNK.b);
        glBegin(GL_QUADS);
        glVertex2f(x - GameConstants::RenderConstants::BackgroundLayout::TREE_TRUNK_WIDTH, y);
        glVertex2f(x + GameConstants::RenderConstants::BackgroundLayout::TREE_TRUNK_WIDTH, y);
        glVertex2f(x + GameConstants::RenderConstants::BackgroundLayout::TREE_TRUNK_WIDTH, y - GameConstants::RenderConstants::BackgroundLayout::TREE_TRUNK_HEIGHT);
        glVertex2f(x - GameConstants::RenderConstants::BackgroundLayout::TREE_TRUNK_WIDTH, y - GameConstants::RenderConstants::BackgroundLayout::TREE_TRUNK_HEIGHT);
        glEnd();
        
        // 木の葉
        renderCircle(x, y - GameConstants::RenderConstants::BackgroundLayout::TREE_TRUNK_HEIGHT, 
                    GameConstants::RenderConstants::BackgroundLayout::TREE_LEAVES_RADIUS, 
                    GameConstants::RenderConstants::BackgroundLayout::TREE_LEAVES_SEGMENTS, 
                    GameConstants::Colors::TREE_LEAVES, 1.0f);
    }
}

void BackgroundRenderer::renderClouds() {
    // 雲を描画
    for (int i = 0; i < GameConstants::RenderConstants::BackgroundLayout::CLOUD_COUNT_STAGE_1; i++) {
        float x = fmod(i * GameConstants::RenderConstants::BackgroundLayout::CLOUD_BASE_SPACING + 
                       (i * GameConstants::RenderConstants::BackgroundLayout::CLOUD_SPACING_OFFSET), 
                       GameConstants::RenderConstants::BackgroundLayout::SCREEN_WIDTH);
        float y = GameConstants::RenderConstants::BackgroundLayout::CLOUD_BASE_Y + 
                  (i * GameConstants::RenderConstants::BackgroundLayout::CLOUD_Y_OFFSET);
        float size = GameConstants::RenderConstants::BackgroundLayout::CLOUD_BASE_SIZE + 
                     (i * GameConstants::RenderConstants::BackgroundLayout::CLOUD_SIZE_OFFSET);
        
                renderCircle(x, y, size, GameConstants::RenderConstants::BackgroundLayout::CLOUD_SEGMENTS, 
                    GameConstants::Colors::CLOUD_COLOR, 
                    GameConstants::RenderConstants::BackgroundLayout::CLOUD_ASPECT_RATIO);
    }
}

void BackgroundRenderer::renderSunset() {
    // 夕日を描画
    renderCircle(GameConstants::RenderConstants::BackgroundLayout::SUNSET_X, 
                GameConstants::RenderConstants::BackgroundLayout::SUNSET_Y, 
                GameConstants::RenderConstants::BackgroundLayout::SUNSET_RADIUS, 
                GameConstants::RenderConstants::BackgroundLayout::SUNSET_SEGMENTS, 
                GameConstants::Colors::SUNSET_COLOR, 1.0f);
}

void BackgroundRenderer::renderStars() {
    // 星を描画
    for (int i = 0; i < GameConstants::RenderConstants::BackgroundLayout::STAR_COUNT; i++) {
        float x = fmod(i * GameConstants::RenderConstants::BackgroundLayout::STAR_SPACING_X + 
                       (i * GameConstants::RenderConstants::BackgroundLayout::STAR_SPACING_X_OFFSET), 
                       GameConstants::RenderConstants::BackgroundLayout::SCREEN_WIDTH);
        float y = fmod(i * GameConstants::RenderConstants::BackgroundLayout::STAR_SPACING_Y + 
                       (i * GameConstants::RenderConstants::BackgroundLayout::STAR_SPACING_Y_OFFSET), 
                       GameConstants::RenderConstants::BackgroundLayout::STAR_MAX_Y);
        float size = GameConstants::RenderConstants::BackgroundLayout::STAR_SIZE_BASE + 
                     (i % GameConstants::RenderConstants::BackgroundLayout::STAR_SIZE_VARIATION);
        
        renderSquare(x, y, size, glm::vec3(1.0f, 1.0f, 1.0f));
    }
}

void BackgroundRenderer::renderThunderClouds() {
    // 雷雲を描画
    for (int i = 0; i < GameConstants::RenderConstants::BackgroundLayout::THUNDER_CLOUD_COUNT; i++) {
        float x = GameConstants::RenderConstants::BackgroundLayout::THUNDER_CLOUD_START_X + 
                  i * GameConstants::RenderConstants::BackgroundLayout::THUNDER_CLOUD_SPACING;
        float y = GameConstants::RenderConstants::BackgroundLayout::THUNDER_CLOUD_BASE_Y + 
                  i * GameConstants::RenderConstants::BackgroundLayout::THUNDER_CLOUD_Y_OFFSET;
        float size = GameConstants::RenderConstants::BackgroundLayout::THUNDER_CLOUD_BASE_SIZE + 
                     i * GameConstants::RenderConstants::BackgroundLayout::THUNDER_CLOUD_SIZE_OFFSET;
        
        renderCircle(x, y, size, GameConstants::RenderConstants::BackgroundLayout::THUNDER_CLOUD_SEGMENTS, 
                    glm::vec3(0.3f, 0.3f, 0.4f), 
                    GameConstants::RenderConstants::BackgroundLayout::THUNDER_CLOUD_ASPECT_RATIO);
    }
}

void BackgroundRenderer::renderNebula() {
    // 星雲を描画
    for (int i = 0; i < GameConstants::RenderConstants::BackgroundLayout::NEBULA_COUNT; i++) {
        float x = GameConstants::RenderConstants::BackgroundLayout::NEBULA_START_X + 
                  i * GameConstants::RenderConstants::BackgroundLayout::NEBULA_SPACING;
        float y = GameConstants::RenderConstants::BackgroundLayout::NEBULA_BASE_Y + 
                  i * GameConstants::RenderConstants::BackgroundLayout::NEBULA_Y_OFFSET;
        float size = GameConstants::RenderConstants::BackgroundLayout::NEBULA_BASE_SIZE + 
                     i * GameConstants::RenderConstants::BackgroundLayout::NEBULA_SIZE_OFFSET;
        
        // 星雲の色（紫、青、ピンク）
        glm::vec3 colors[] = {
            GameConstants::RenderConstants::BackgroundLayout::NEBULA_PURPLE,  // 紫
            GameConstants::RenderConstants::BackgroundLayout::NEBULA_BLUE,    // 青
            GameConstants::RenderConstants::BackgroundLayout::NEBULA_PINK     // ピンク
        };
        
        renderCircle(x, y, size, GameConstants::RenderConstants::BackgroundLayout::NEBULA_SEGMENTS, 
                    colors[i], 
                    GameConstants::RenderConstants::BackgroundLayout::NEBULA_ASPECT_RATIO);
    }
}

void BackgroundRenderer::renderStage1WithTexture() {
    // ステージ1の背景画像を読み込んで表示
    GLuint backgroundTexture = TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/stage1_bg.png"));
    
    if (backgroundTexture == 0) {
        // 画像の読み込みに失敗した場合は従来の色ベース背景にフォールバック
        std::cerr << "WARNING: Failed to load stage1 background texture, using fallback" << std::endl;
        
        // 従来の色ベース背景を描画
        glm::vec3 topColor = GameConstants::RenderConstants::STAGE_1_TOP_COLOR;
        glm::vec3 bottomColor = GameConstants::RenderConstants::STAGE_1_BOTTOM_COLOR;
        
        glBegin(GL_QUADS);
        glColor3f(topColor.r, topColor.g, topColor.b);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glColor3f(bottomColor.r, bottomColor.g, bottomColor.b);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();
        
        // 雲を描画
        renderClouds();
        return;
    }
    
    // テクスチャを有効化
    glEnable(GL_TEXTURE_2D);
    TextureManager::bindTexture(backgroundTexture);
    
    // 背景画像を画面全体に描画
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1280, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1280, 720);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 720);
    glEnd();
    
    // テクスチャを無効化
    glDisable(GL_TEXTURE_2D);
}

void BackgroundRenderer::renderStage2WithTexture() {
    // ステージ2の背景画像を読み込んで表示
    GLuint backgroundTexture = TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/stage2_bg.png"));
    
    if (backgroundTexture == 0) {
        // 画像の読み込みに失敗した場合は従来の色ベース背景にフォールバック
        std::cerr << "WARNING: Failed to load stage2 background texture, using fallback" << std::endl;
        
        // 従来の色ベース背景を描画
        glm::vec3 topColor = GameConstants::RenderConstants::STAGE_2_TOP_COLOR;
        glm::vec3 bottomColor = GameConstants::RenderConstants::STAGE_2_BOTTOM_COLOR;
        
        glBegin(GL_QUADS);
        glColor3f(topColor.r, topColor.g, topColor.b);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glColor3f(bottomColor.r, bottomColor.g, bottomColor.b);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();
        
        // 夕日の雲を描画
        renderSunset();
        return;
    }
    
    // テクスチャを有効化
    glEnable(GL_TEXTURE_2D);
    TextureManager::bindTexture(backgroundTexture);
    
    // 背景画像を画面全体に描画
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1280, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1280, 720);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 720);
    glEnd();
    
    // テクスチャを無効化
    glDisable(GL_TEXTURE_2D);
}

void BackgroundRenderer::renderStage3WithTexture() {
    // ステージ3の背景画像を読み込んで表示
    GLuint backgroundTexture = TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/stage3_bg.png"));
    
    if (backgroundTexture == 0) {
        // 画像の読み込みに失敗した場合は従来の色ベース背景にフォールバック
        std::cerr << "WARNING: Failed to load stage3 background texture, using fallback" << std::endl;
        
        // 従来の色ベース背景を描画
        glm::vec3 topColor = GameConstants::RenderConstants::STAGE_3_TOP_COLOR;
        glm::vec3 bottomColor = GameConstants::RenderConstants::STAGE_3_BOTTOM_COLOR;
        
        glBegin(GL_QUADS);
        glColor3f(topColor.r, topColor.g, topColor.b);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glColor3f(bottomColor.r, bottomColor.g, bottomColor.b);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();
        
        // 星空を描画
        renderStars();
        return;
    }
    
    // テクスチャを有効化
    glEnable(GL_TEXTURE_2D);
    TextureManager::bindTexture(backgroundTexture);
    
    // 背景画像を画面全体に描画
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1280, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1280, 720);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 720);
    glEnd();
    
    // テクスチャを無効化
    glDisable(GL_TEXTURE_2D);
}

void BackgroundRenderer::renderStage4WithTexture() {
    // ステージ4の背景画像を読み込んで表示
    GLuint backgroundTexture = TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/stage4_bg.png"));
    
    if (backgroundTexture == 0) {
        // 画像の読み込みに失敗した場合は従来の色ベース背景にフォールバック
        std::cerr << "WARNING: Failed to load stage4 background texture, using fallback" << std::endl;
        
        // 従来の色ベース背景を描画
        glm::vec3 topColor = GameConstants::RenderConstants::STAGE_4_TOP_COLOR;
        glm::vec3 bottomColor = GameConstants::RenderConstants::STAGE_4_BOTTOM_COLOR;
        
        glBegin(GL_QUADS);
        glColor3f(topColor.r, topColor.g, topColor.b);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glColor3f(bottomColor.r, bottomColor.g, bottomColor.b);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();
        
        // 雷雲を描画
        renderThunderClouds();
        return;
    }
    
    // テクスチャを有効化
    glEnable(GL_TEXTURE_2D);
    TextureManager::bindTexture(backgroundTexture);
    
    // 背景画像を画面全体に描画
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1280, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1280, 720);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 720);
    glEnd();
    
    // テクスチャを無効化
    glDisable(GL_TEXTURE_2D);
}

void BackgroundRenderer::renderStage5WithTexture() {
    // ステージ5の背景画像を読み込んで表示
    GLuint backgroundTexture = TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/stage5_bg.png"));
    
    if (backgroundTexture == 0) {
        // 画像の読み込みに失敗した場合は従来の色ベース背景にフォールバック
        std::cerr << "WARNING: Failed to load stage5 background texture, using fallback" << std::endl;
        
        // 従来の色ベース背景を描画
        glm::vec3 topColor = GameConstants::RenderConstants::STAGE_5_TOP_COLOR;
        glm::vec3 bottomColor = GameConstants::RenderConstants::STAGE_5_BOTTOM_COLOR;
        
        glBegin(GL_QUADS);
        glColor3f(topColor.r, topColor.g, topColor.b);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glColor3f(bottomColor.r, bottomColor.g, bottomColor.b);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();
        
        // 星雲を描画
        renderNebula();
        return;
    }
    
    // テクスチャを有効化
    glEnable(GL_TEXTURE_2D);
    TextureManager::bindTexture(backgroundTexture);
    
    // 背景画像を画面全体に描画
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1280, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1280, 720);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 720);
    glEnd();
    
    // テクスチャを無効化
    glDisable(GL_TEXTURE_2D);
}

void BackgroundRenderer::renderStageSelectionWithTexture() {
    // ステージ選択フィールドの背景画像を読み込んで表示
    GLuint backgroundTexture = TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/stage_selection_bg.png"));
    
    if (backgroundTexture == 0) {
        // 画像の読み込みに失敗した場合は従来の色ベース背景にフォールバック
        std::cerr << "WARNING: Failed to load stage selection background texture, using fallback" << std::endl;
        
        // 従来の色ベース背景を描画
        glm::vec3 topColor = GameConstants::RenderConstants::STAGE_0_TOP_COLOR;
        glm::vec3 bottomColor = GameConstants::RenderConstants::STAGE_0_BOTTOM_COLOR;
        
        glBegin(GL_QUADS);
        glColor3f(topColor.r, topColor.g, topColor.b);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glColor3f(bottomColor.r, bottomColor.g, bottomColor.b);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();
        
        // 草原と雲を描画
        renderGrassland();
        renderClouds();
        return;
    }
    
    // テクスチャを有効化
    glEnable(GL_TEXTURE_2D);
    TextureManager::bindTexture(backgroundTexture);
    
    // 背景画像を画面全体に描画
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1280, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1280, 720);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 720);
    glEnd();
    
    // テクスチャを無効化
    glDisable(GL_TEXTURE_2D);
}

void BackgroundRenderer::renderTutorialWithTexture() {
    // チュートリアルステージの背景画像を読み込んで表示
    GLuint backgroundTexture = TextureManager::loadTexture(ResourcePath::getResourcePath("assets/textures/tutorial_bg.png"));
    
    if (backgroundTexture == 0) {
        // 画像の読み込みに失敗した場合は従来の色ベース背景にフォールバック
        std::cerr << "WARNING: Failed to load tutorial background texture, using fallback" << std::endl;
        
        // 従来の色ベース背景を描画（チュートリアル用のデフォルト色）
        glm::vec3 topColor = glm::vec3(0.2f, 0.1f, 0.3f);  // 暗い紫
        glm::vec3 bottomColor = glm::vec3(0.1f, 0.05f, 0.2f);  // より暗い紫
        
        glBegin(GL_QUADS);
        glColor3f(topColor.r, topColor.g, topColor.b);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glColor3f(bottomColor.r, bottomColor.g, bottomColor.b);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();
        return;
    }
    
    // テクスチャを有効化
    glEnable(GL_TEXTURE_2D);
    TextureManager::bindTexture(backgroundTexture);
    
    // 背景画像を画面全体に描画
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1280, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1280, 720);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 720);
    glEnd();
    
    // テクスチャを無効化
    glDisable(GL_TEXTURE_2D);
}

} // namespace gfx
