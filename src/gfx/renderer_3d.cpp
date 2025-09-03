#include "renderer_3d.h"
#include "../app/game_constants.h"
#include <iostream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gfx {

Renderer3D::Renderer3D() {
}

Renderer3D::~Renderer3D() {
}

void Renderer3D::renderCube(const glm::vec3& position, const glm::vec3& color, float size) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(size));
    drawCube(model, color);
}

void Renderer3D::renderRotatedBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                                 const glm::vec3& rotationAxis, float rotationAngle) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
    model = glm::scale(model, size);
    drawCube(model, color);
}

void Renderer3D::renderBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, size);
    
    // 透明度を適用した色
    glm::vec3 alphaColor = color * alpha;
    drawCube(model, alphaColor);
}

void Renderer3D::renderRotatedBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                                          const glm::vec3& rotationAxis, float rotationAngle, float alpha) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
    model = glm::scale(model, size);
    
    // 透明度を適用した色
    glm::vec3 alphaColor = color * alpha;
    drawCube(model, alphaColor);
}

void Renderer3D::renderRealisticBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, size);
    
    // 透明度を適用した色
    glm::vec3 alphaColor = color * alpha;
    
    // ライティングを有効化
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    
    // 光源の位置を設定
    GLfloat lightPosition[] = {GameConstants::RenderConstants::LIGHT_POSITION.x, 
                              GameConstants::RenderConstants::LIGHT_POSITION.y, 
                              GameConstants::RenderConstants::LIGHT_POSITION.z, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    
    // 環境光を設定
    GLfloat ambientLight[] = {GameConstants::RenderConstants::AMBIENT_LIGHT.r, 
                             GameConstants::RenderConstants::AMBIENT_LIGHT.g, 
                             GameConstants::RenderConstants::AMBIENT_LIGHT.b, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    
    // 拡散光を設定
    GLfloat diffuseLight[] = {GameConstants::RenderConstants::DIFFUSE_LIGHT.r, 
                             GameConstants::RenderConstants::DIFFUSE_LIGHT.g, 
                             GameConstants::RenderConstants::DIFFUSE_LIGHT.b, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(model));
    
    // 基本色を設定（透明度適用）
    glColor3f(alphaColor.r, alphaColor.g, alphaColor.b);
    
    // 法線を計算して各面を描画
    glBegin(GL_QUADS);
    
    // 前面（Z軸正方向）
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    // 背面（Z軸負方向）
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    // 上面（Y軸正方向）- より明るく
    glNormal3f(0.0f, 1.0f, 0.0f);
    glColor3f(alphaColor.r * GameConstants::RenderConstants::LIGHTING_BRIGHTNESS_MULTIPLIER, 
              alphaColor.g * GameConstants::RenderConstants::LIGHTING_BRIGHTNESS_MULTIPLIER, 
              alphaColor.b * GameConstants::RenderConstants::LIGHTING_BRIGHTNESS_MULTIPLIER);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    // 下面（Y軸負方向）- より暗く
    glNormal3f(0.0f, -1.0f, 0.0f);
    glColor3f(alphaColor.r * GameConstants::RenderConstants::LIGHTING_DARKNESS_MULTIPLIER, 
              alphaColor.g * GameConstants::RenderConstants::LIGHTING_DARKNESS_MULTIPLIER, 
              alphaColor.b * GameConstants::RenderConstants::LIGHTING_DARKNESS_MULTIPLIER);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    // 右面（X軸正方向）- 中間の明るさ
    glNormal3f(1.0f, 0.0f, 0.0f);
    glColor3f(alphaColor.r * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER, 
              alphaColor.g * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER, 
              alphaColor.b * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    // 左面（X軸負方向）- 中間の明るさ
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glColor3f(alphaColor.r * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER, 
              alphaColor.g * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER, 
              alphaColor.b * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glEnd();
    
    // エッジの強調（ワイヤーフレーム風）
    glDisable(GL_LIGHTING);
    glColor3f(alphaColor.r * GameConstants::RenderConstants::EDGE_ALPHA_MULTIPLIER, 
              alphaColor.g * GameConstants::RenderConstants::EDGE_ALPHA_MULTIPLIER, 
              alphaColor.b * GameConstants::RenderConstants::EDGE_ALPHA_MULTIPLIER);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    
    // 前面のエッジ
    glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    
    // 背面のエッジ
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, -0.5f);
    
    // 縦のエッジ
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    
    glEnd();
    
    glPopMatrix();
    
    // ライティングを無効化
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
}

void Renderer3D::drawCube(const glm::mat4& model, const glm::vec3& color) {
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(model));
    
    glColor3f(color.r, color.g, color.b);
    
    // キューブの各面を描画
    glBegin(GL_QUADS);
    
    // 前面
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    // 背面
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    // 上面
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    // 下面
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    // 右面
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    // 左面
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glEnd();
    
    glPopMatrix();
}

// 3D星の描画（塗りつぶし版）
void Renderer3D::renderStar3D(const glm::vec3& position, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    // 星の中心点
    float centerX = position.x;
    float centerY = position.y;
    float centerZ = position.z;
    
    // 星の5つの角を描画（塗りつぶし）
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; i++) {
        float angle1 = i * 72.0f * 3.14159f / 180.0f;
        float angle2 = (i + 2) * 72.0f * 3.14159f / 180.0f;
        
        // 外側の点
        float x1 = centerX + cos(angle1) * 0.3f * scale;
        float y1 = centerY + sin(angle1) * 0.3f * scale;
        
        // 内側の点
        float x2 = centerX + cos(angle1 + 36.0f * 3.14159f / 180.0f) * 0.12f * scale;
        float y2 = centerY + sin(angle1 + 36.0f * 3.14159f / 180.0f) * 0.12f * scale;
        
        // 次の外側の点
        float x3 = centerX + cos(angle2) * 0.3f * scale;
        float y3 = centerY + sin(angle2) * 0.3f * scale;
        
        // 中心から各点への三角形を描画（塗りつぶし）
        glVertex3f(centerX, centerY, centerZ);  // 中心点
        glVertex3f(x1, y1, centerZ);            // 外側の点1
        glVertex3f(x2, y2, centerZ);            // 内側の点
        
        glVertex3f(centerX, centerY, centerZ);  // 中心点
        glVertex3f(x2, y2, centerZ);            // 内側の点
        glVertex3f(x3, y3, centerZ);            // 外側の点2
    }
    glEnd();
}

// 3D数字の描画
void Renderer3D::renderNumber3D(const glm::vec3& position, int number, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    float centerX = position.x;
    float centerY = position.y;
    float centerZ = position.z;
    
    // 数字を文字列に変換
    std::string numStr = std::to_string(number);
    
    // 各文字を描画
    for (size_t i = 0; i < numStr.length(); i++) {
        char digit = numStr[i];
        float charX = centerX + i * 0.6f * scale;
        
        // 数字の形状を定義（実際の数字の形状）
        float width = 0.4f * scale;
        float height = 0.6f * scale;
        float thickness = 0.1f * scale;
        
        // 数字の形状を定義（各数字のセグメント）
        std::vector<std::vector<glm::vec2>> segments;
        
        if (digit == '0') {
            // 0の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, height/2}, {width/2, height/2}},    // 上
                {{width/2, height/2}, {width/2, -height/2}},    // 右
                {{width/2, -height/2}, {-width/2, -height/2}},  // 下
                {{-width/2, -height/2}, {-width/2, height/2}}   // 左
            };
        } else if (digit == '1') {
            // 1の形状（現在の向きに合わせて修正）
            segments = {
                {{0, height/2}, {0, -height/2}}  // 中央縦線
            };
        } else if (digit == '2') {
            // 2の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, 0}},            // 右下
                {{width/2, 0}, {-width/2, 0}},                  // 中央
                {{-width/2, 0}, {-width/2, height/2}},          // 左上
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '3') {
            // 3の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}},    // 右
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '4') {
            // 4の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {-width/2, 0}},         // 左下
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{width/2, -height/2}, {width/2, height/2}}     // 右
            };
        } else if (digit == '5') {
            // 5の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{-width/2, -height/2}, {-width/2, 0}},         // 左下
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{width/2, 0}, {width/2, height/2}},            // 右上
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '6') {
            // 6の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{-width/2, -height/2}, {-width/2, height/2}},  // 左
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{width/2, 0}, {width/2, height/2}},            // 右上
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '7') {
            // 7の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}}     // 右
            };
        } else if (digit == '8') {
            // 8の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}},    // 右
                {{-width/2, -height/2}, {-width/2, height/2}},  // 左
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '9') {
            // 9の形状（現在の向きに合わせて修正）
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}},    // 右
                {{-width/2, -height/2}, {-width/2, 0}},         // 左下
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        }
        
        // 各セグメントを描画
        for (const auto& segment : segments) {
            glBegin(GL_QUADS);
            // セグメントを太い線として描画（線対称で正しい向きに）
            glm::vec2 start = segment[0];
            glm::vec2 end = segment[1];
            // 線対称（180度回転：xとyを両方反転）
            start = glm::vec2(-start.x, -start.y);
            end = glm::vec2(-end.x, -end.y);
            glm::vec2 dir = glm::normalize(end - start);
            glm::vec2 perp = glm::vec2(-dir.y, dir.x) * thickness / 2.0f;
            
            glVertex3f(charX + start.x - perp.x, centerY + start.y - perp.y, centerZ);
            glVertex3f(charX + start.x + perp.x, centerY + start.y + perp.y, centerZ);
            glVertex3f(charX + end.x + perp.x, centerY + end.y + perp.y, centerZ);
            glVertex3f(charX + end.x - perp.x, centerY + end.y - perp.y, centerZ);
            glEnd();
        }
    }
}

// 3D×記号の描画
void Renderer3D::renderXMark3D(const glm::vec3& position, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    float centerX = position.x;
    float centerY = position.y;
    float centerZ = position.z;
    
    // ×記号を2つの交差する線で表現
    float halfSize = 0.3f * scale;
    
    glBegin(GL_QUADS);
    // 左上から右下への線
    glVertex3f(centerX - halfSize, centerY + halfSize, centerZ);
    glVertex3f(centerX - halfSize + 0.1f * scale, centerY + halfSize, centerZ);
    glVertex3f(centerX + halfSize, centerY - halfSize, centerZ);
    glVertex3f(centerX + halfSize - 0.1f * scale, centerY - halfSize, centerZ);
    
    // 右上から左下への線
    glVertex3f(centerX + halfSize, centerY + halfSize, centerZ);
    glVertex3f(centerX + halfSize - 0.1f * scale, centerY + halfSize, centerZ);
    glVertex3f(centerX - halfSize, centerY - halfSize, centerZ);
    glVertex3f(centerX - halfSize + 0.1f * scale, centerY - halfSize, centerZ);
    glEnd();
}

} // namespace gfx
