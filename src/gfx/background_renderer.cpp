#include "background_renderer.h"
#include <iostream>

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
    
    // ステージ別の背景色とグラデーション
    glm::vec3 topColor, bottomColor;
    
    switch (stageNumber) {
        case 0: // ステージ選択画面
            topColor = GameConstants::RenderConstants::STAGE_0_TOP_COLOR;
            bottomColor = GameConstants::RenderConstants::STAGE_0_BOTTOM_COLOR;
            break;
        case 1: // ステージ1 - 青空、白い雲（初心者向けの明るい雰囲気）
            topColor = GameConstants::RenderConstants::STAGE_1_TOP_COLOR;
            bottomColor = GameConstants::RenderConstants::STAGE_1_BOTTOM_COLOR;
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

void BackgroundRenderer::renderGrassland() {
    // 雲を描画（空の部分）
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 4; i++) {
        float x = fmod(i * 350.0f + (i * 30.0f), 1280.0f);
        float y = 80.0f + (i * 25.0f);
        float size = 70.0f + (i * 15.0f);
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int j = 0; j <= 12; j++) {
            float angle = 2.0f * 3.14159f * j / 12;
            float cloudX = x + cos(angle) * size;
            float cloudY = y + sin(angle) * (size * 0.6f);
            glVertex2f(cloudX, cloudY);
        }
        glEnd();
    }
    
    // 草を描画
    glColor3f(0.1f, 0.6f, 0.1f); // 濃い緑
    for (int i = 0; i < 100; i++) {
        float x = fmod(i * 12.0f + (i * 7.0f), 1280.0f);
        float y = 720.0f - fmod(i * 8.0f + (i * 13.0f), 200.0f); // 画面下部から
        float height = 15.0f + (i % 5) * 3.0f;
        float width = 2.0f + (i % 3);
        
        // 草の葉を描画
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y);
        glVertex2f(x - width, y - height);
        glVertex2f(x + width, y - height);
        glEnd();
    }
    
    // 花を描画
    renderFlowers();
    
    // 木を描画
    renderTrees();
    
    // 小さな丘を描画
    renderHills();
}

void BackgroundRenderer::renderFlowers() {
    for (int i = 0; i < 20; i++) {
        float x = fmod(i * 60.0f + (i * 23.0f), 1280.0f);
        float y = 720.0f - fmod(i * 15.0f + (i * 37.0f), 150.0f);
        float size = 8.0f + (i % 3) * 2.0f;
        
        // 花の色（ランダム）
        glm::vec3 flowerColors[] = {
            GameConstants::FLOWER_RED,     // 赤
            GameConstants::FLOWER_YELLOW,  // 黄色
            GameConstants::FLOWER_PINK,    // ピンク
            GameConstants::FLOWER_WHITE    // 白
        };
        glm::vec3 flowerColor = flowerColors[i % 4];
        
        glColor3f(flowerColor.r, flowerColor.g, flowerColor.b);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int j = 0; j <= 8; j++) {
            float angle = 2.0f * 3.14159f * j / 8;
            float flowerX = x + cos(angle) * size;
            float flowerY = y + sin(angle) * size;
            glVertex2f(flowerX, flowerY);
        }
        glEnd();
    }
}

void BackgroundRenderer::renderTrees() {
    for (int i = 0; i < 8; i++) {
        float x = 100.0f + i * 150.0f;
        float y = 720.0f - 80.0f;
        
        // 木の幹
        glColor3f(GameConstants::TREE_TRUNK.r, 
                  GameConstants::TREE_TRUNK.g, 
                  GameConstants::TREE_TRUNK.b); // 茶色
        glBegin(GL_QUADS);
        glVertex2f(x - 8, y);
        glVertex2f(x + 8, y);
        glVertex2f(x + 8, y - 60);
        glVertex2f(x - 8, y - 60);
        glEnd();
        
        // 木の葉
        glColor3f(GameConstants::TREE_LEAVES.r, 
                  GameConstants::TREE_LEAVES.g, 
                  GameConstants::TREE_LEAVES.b); // 濃い緑
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y - 60);
        for (int j = 0; j <= 12; j++) {
            float angle = 2.0f * 3.14159f * j / 12;
            float leafX = x + cos(angle) * 40;
            float leafY = y - 60 + sin(angle) * 40;
            glVertex2f(leafX, leafY);
        }
        glEnd();
    }
}

void BackgroundRenderer::renderHills() {
    glColor3f(GameConstants::HILL_COLOR.r, 
               GameConstants::HILL_COLOR.g, 
               GameConstants::HILL_COLOR.b); // 明るい緑
    for (int i = 0; i < 3; i++) {
        float x = 200.0f + i * 400.0f;
        float y = 720.0f - 50.0f;
        float width = 200.0f;
        float height = 80.0f;
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y - height);
        for (int j = 0; j <= 16; j++) {
            float angle = 3.14159f * j / 16;
            float hillX = x + cos(angle) * width;
            float hillY = y - height + sin(angle) * height;
            glVertex2f(hillX, hillY);
        }
        glEnd();
    }
}

void BackgroundRenderer::renderClouds() {
    // 雲を描画
    glColor3f(GameConstants::CLOUD_COLOR.r, 
               GameConstants::CLOUD_COLOR.g, 
               GameConstants::CLOUD_COLOR.b);
    for (int i = 0; i < 5; i++) {
        float x = fmod(i * 300.0f + (i * 50.0f), 1280.0f);
        float y = 100.0f + (i * 20.0f);
        float size = 80.0f + (i * 10.0f);
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int j = 0; j <= 12; j++) {
            float angle = 2.0f * 3.14159f * j / 12;
            float cloudX = x + cos(angle) * size;
            float cloudY = y + sin(angle) * (size * 0.6f);
            glVertex2f(cloudX, cloudY);
        }
        glEnd();
    }
}

void BackgroundRenderer::renderSunset() {
    // 夕日を描画
    glColor3f(GameConstants::SUNSET_COLOR.r, 
               GameConstants::SUNSET_COLOR.g, 
               GameConstants::SUNSET_COLOR.b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(1000, 150);
    for (int i = 0; i <= 20; i++) {
        float angle = 3.14159f * i / 20;
        float x = 1000 + cos(angle) * 80;
        float y = 150 + sin(angle) * 80;
        glVertex2f(x, y);
    }
    glEnd();
}

void BackgroundRenderer::renderStars() {
    // 星を描画
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 50; i++) {
        float x = fmod(i * 25.0f + (i * 17.0f), 1280.0f);
        float y = fmod(i * 15.0f + (i * 23.0f), 300.0f);
        float size = 2.0f + (i % 3);
        
        glBegin(GL_QUADS);
        glVertex2f(x - size, y - size);
        glVertex2f(x + size, y - size);
        glVertex2f(x + size, y + size);
        glVertex2f(x - size, y + size);
        glEnd();
    }
}

void BackgroundRenderer::renderThunderClouds() {
    // 雷雲を描画
    glColor3f(0.3f, 0.3f, 0.4f);
    for (int i = 0; i < 3; i++) {
        float x = 200.0f + i * 400.0f;
        float y = 80.0f + i * 30.0f;
        float size = 120.0f + i * 20.0f;
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int j = 0; j <= 12; j++) {
            float angle = 2.0f * 3.14159f * j / 12;
            float cloudX = x + cos(angle) * size;
            float cloudY = y + sin(angle) * (size * 0.8f);
            glVertex2f(cloudX, cloudY);
        }
        glEnd();
    }
}

void BackgroundRenderer::renderNebula() {
    // 星雲を描画
    for (int i = 0; i < 3; i++) {
        float x = 300.0f + i * 300.0f;
        float y = 200.0f + i * 100.0f;
        float size = 150.0f + i * 50.0f;
        
        // 星雲の色（紫、青、ピンク）
        glm::vec3 colors[] = {
            glm::vec3(0.8f, 0.2f, 0.8f), // 紫
            glm::vec3(0.2f, 0.4f, 0.8f), // 青
            glm::vec3(0.8f, 0.3f, 0.6f)  // ピンク
        };
        
        glColor3f(colors[i].r, colors[i].g, colors[i].b);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int j = 0; j <= 16; j++) {
            float angle = 2.0f * 3.14159f * j / 16;
            float nebulaX = x + cos(angle) * size;
            float nebulaY = y + sin(angle) * (size * 0.7f);
            glVertex2f(nebulaX, nebulaY);
        }
        glEnd();
    }
}

} // namespace gfx
