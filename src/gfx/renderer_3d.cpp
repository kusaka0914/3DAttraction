#include "renderer_3d.h"
#include "../core/constants/game_constants.h"
#include "texture_manager.h"
#include <iostream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    
    glm::vec3 alphaColor = color * alpha;
    drawCube(model, alphaColor);
}

void Renderer3D::renderRotatedBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                                          const glm::vec3& rotationAxis, float rotationAngle, float alpha) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
    model = glm::scale(model, size);
    
    glm::vec3 alphaColor = color * alpha;
    drawCube(model, alphaColor);
}

void Renderer3D::renderRealisticBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, size);
    
    glm::vec3 alphaColor = color * alpha;
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    
    GLfloat lightPosition[] = {GameConstants::RenderConstants::LIGHT_POSITION.x, 
                              GameConstants::RenderConstants::LIGHT_POSITION.y, 
                              GameConstants::RenderConstants::LIGHT_POSITION.z, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    
    GLfloat ambientLight[] = {GameConstants::RenderConstants::AMBIENT_LIGHT.r, 
                             GameConstants::RenderConstants::AMBIENT_LIGHT.g, 
                             GameConstants::RenderConstants::AMBIENT_LIGHT.b, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    
    GLfloat diffuseLight[] = {GameConstants::RenderConstants::DIFFUSE_LIGHT.r, 
                             GameConstants::RenderConstants::DIFFUSE_LIGHT.g, 
                             GameConstants::RenderConstants::DIFFUSE_LIGHT.b, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(model));
    
    glColor3f(alphaColor.r, alphaColor.g, alphaColor.b);
    
    glBegin(GL_QUADS);
    
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glNormal3f(0.0f, 1.0f, 0.0f);
    glColor3f(alphaColor.r * GameConstants::RenderConstants::LIGHTING_BRIGHTNESS_MULTIPLIER, 
              alphaColor.g * GameConstants::RenderConstants::LIGHTING_BRIGHTNESS_MULTIPLIER, 
              alphaColor.b * GameConstants::RenderConstants::LIGHTING_BRIGHTNESS_MULTIPLIER);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glNormal3f(0.0f, -1.0f, 0.0f);
    glColor3f(alphaColor.r * GameConstants::RenderConstants::LIGHTING_DARKNESS_MULTIPLIER, 
              alphaColor.g * GameConstants::RenderConstants::LIGHTING_DARKNESS_MULTIPLIER, 
              alphaColor.b * GameConstants::RenderConstants::LIGHTING_DARKNESS_MULTIPLIER);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glNormal3f(1.0f, 0.0f, 0.0f);
    glColor3f(alphaColor.r * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER, 
              alphaColor.g * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER, 
              alphaColor.b * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glColor3f(alphaColor.r * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER, 
              alphaColor.g * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER, 
              alphaColor.b * GameConstants::RenderConstants::LIGHTING_MEDIUM_MULTIPLIER);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glEnd();
    
    glDisable(GL_LIGHTING);
    glColor3f(alphaColor.r * GameConstants::RenderConstants::EDGE_ALPHA_MULTIPLIER, 
              alphaColor.g * GameConstants::RenderConstants::EDGE_ALPHA_MULTIPLIER, 
              alphaColor.b * GameConstants::RenderConstants::EDGE_ALPHA_MULTIPLIER);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    
    glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, -0.5f);
    
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    
    glEnd();
    
    glPopMatrix();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
}

void Renderer3D::renderTexturedBox(const glm::vec3& position, const glm::vec3& size, GLuint textureID) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, size);
    drawTexturedCube(model, textureID, 1.0f);
}

void Renderer3D::renderTexturedBoxWithAlpha(const glm::vec3& position, const glm::vec3& size, GLuint textureID, float alpha) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, size);
    
    drawTexturedCube(model, textureID, alpha);
}

void Renderer3D::renderTexturedBox(const glm::vec3& position, const glm::vec3& size, GLuint frontTextureID, GLuint otherTextureID) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, size);
    
    drawTexturedCubeWithFrontFace(model, frontTextureID, otherTextureID);
}

void Renderer3D::renderTexturedRotatedBox(const glm::vec3& position, const glm::vec3& size, GLuint textureID, 
                                         const glm::vec3& rotationAxis, float rotationAngle) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
    model = glm::scale(model, size);
    
    drawTexturedCube(model, textureID);
}

void Renderer3D::drawCube(const glm::mat4& model, const glm::vec3& color) {
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(model));
    
    glColor3f(color.r, color.g, color.b);
    
    glBegin(GL_QUADS);
    
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glEnd();
    
    glPopMatrix();
}

void Renderer3D::renderStar3D(const glm::vec3& position, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    float centerX = position.x;
    float centerY = position.y;
    float centerZ = position.z;
    
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; i++) {
        float angle1 = i * GameConstants::STAR_ANGLE_STEP * M_PI / 180.0f;
        float angle2 = (i + 2) * GameConstants::STAR_ANGLE_STEP * M_PI / 180.0f;
        
        float x1 = centerX + cos(angle1) * GameConstants::STAR_OUTER_RADIUS * scale;
        float y1 = centerY + sin(angle1) * GameConstants::STAR_OUTER_RADIUS * scale;
        
        float x2 = centerX + cos(angle1 + GameConstants::STAR_INNER_ANGLE_OFFSET * M_PI / 180.0f) * GameConstants::STAR_INNER_RADIUS * scale;
        float y2 = centerY + sin(angle1 + GameConstants::STAR_INNER_ANGLE_OFFSET * M_PI / 180.0f) * GameConstants::STAR_INNER_RADIUS * scale;
        
        float x3 = centerX + cos(angle2) * GameConstants::STAR_OUTER_RADIUS * scale;
        float y3 = centerY + sin(angle2) * GameConstants::STAR_OUTER_RADIUS * scale;
        
        glVertex3f(centerX, centerY, centerZ);  // 中心点
        glVertex3f(x1, y1, centerZ);            // 外側の点1
        glVertex3f(x2, y2, centerZ);            // 内側の点
        
        glVertex3f(centerX, centerY, centerZ);  // 中心点
        glVertex3f(x2, y2, centerZ);            // 内側の点
        glVertex3f(x3, y3, centerZ);            // 外側の点2
    }
    glEnd();
}

void Renderer3D::renderNumber3D(const glm::vec3& position, int number, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    float centerX = position.x;
    float centerY = position.y;
    float centerZ = position.z;
    
    std::string numStr = std::to_string(number);
    
    for (size_t i = 0; i < numStr.length(); i++) {
        char digit = numStr[i];
        float charX = centerX + i * GameConstants::NUMBER_CHAR_SPACING * scale;
        
        float width = GameConstants::NUMBER_WIDTH * scale;
        float height = GameConstants::NUMBER_HEIGHT * scale;
        float thickness = GameConstants::NUMBER_THICKNESS * scale;
        
        std::vector<std::vector<glm::vec2>> segments;
        
        if (digit == '0') {
            segments = {
                {{-width/2, height/2}, {width/2, height/2}},    // 上
                {{width/2, height/2}, {width/2, -height/2}},    // 右
                {{width/2, -height/2}, {-width/2, -height/2}},  // 下
                {{-width/2, -height/2}, {-width/2, height/2}}   // 左
            };
        } else if (digit == '1') {
            segments = {
                {{0, height/2}, {0, -height/2}}  // 中央縦線
            };
        } else if (digit == '2') {
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, 0}},            // 右下
                {{width/2, 0}, {-width/2, 0}},                  // 中央
                {{-width/2, 0}, {-width/2, height/2}},          // 左上
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '3') {
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}},    // 右
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '4') {
            segments = {
                {{-width/2, -height/2}, {-width/2, 0}},         // 左下
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{width/2, -height/2}, {width/2, height/2}}     // 右
            };
        } else if (digit == '5') {
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{-width/2, -height/2}, {-width/2, 0}},         // 左下
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{width/2, 0}, {width/2, height/2}},            // 右上
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '6') {
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{-width/2, -height/2}, {-width/2, height/2}},  // 左
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{width/2, 0}, {width/2, height/2}},            // 右上
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '7') {
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}}     // 右
            };
        } else if (digit == '8') {
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}},    // 右
                {{-width/2, -height/2}, {-width/2, height/2}},  // 左
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        } else if (digit == '9') {
            segments = {
                {{-width/2, -height/2}, {width/2, -height/2}},  // 下
                {{width/2, -height/2}, {width/2, height/2}},    // 右
                {{-width/2, -height/2}, {-width/2, 0}},         // 左下
                {{-width/2, 0}, {width/2, 0}},                  // 中央
                {{-width/2, height/2}, {width/2, height/2}}     // 上
            };
        }
        
        for (const auto& segment : segments) {
            glBegin(GL_QUADS);
            glm::vec2 start = segment[0];
            glm::vec2 end = segment[1];
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

void Renderer3D::renderXMark3D(const glm::vec3& position, const glm::vec3& color, float scale) {
    glColor3f(color.r, color.g, color.b);
    
    float centerX = position.x;
    float centerY = position.y;
    float centerZ = position.z;
    
    float halfSize = GameConstants::X_MARK_SIZE * scale;
    
    glBegin(GL_QUADS);
    glVertex3f(centerX - halfSize, centerY + halfSize, centerZ);
    glVertex3f(centerX - halfSize + GameConstants::X_MARK_THICKNESS * scale, centerY + halfSize, centerZ);
    glVertex3f(centerX + halfSize, centerY - halfSize, centerZ);
    glVertex3f(centerX + halfSize - GameConstants::X_MARK_THICKNESS * scale, centerY - halfSize, centerZ);
    
    glVertex3f(centerX + halfSize, centerY + halfSize, centerZ);
    glVertex3f(centerX + halfSize - GameConstants::X_MARK_THICKNESS * scale, centerY + halfSize, centerZ);
    glVertex3f(centerX - halfSize, centerY - halfSize, centerZ);
    glVertex3f(centerX - halfSize + GameConstants::X_MARK_THICKNESS * scale, centerY - halfSize, centerZ);
    glEnd();
}

void Renderer3D::drawTexturedCube(const glm::mat4& model, GLuint textureID, float alpha) {
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(model));
    
    glEnable(GL_TEXTURE_2D);
    TextureManager::bindTexture(textureID);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glTexCoord2f(0.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glTexCoord2f(0.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    
    glPopMatrix();
}

void Renderer3D::drawTexturedCubeWithFrontFace(const glm::mat4& model, GLuint frontTextureID, GLuint otherTextureID) {
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(model));
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    glEnable(GL_TEXTURE_2D);
    
    TextureManager::bindTexture(frontTextureID);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glEnd();
    
    TextureManager::bindTexture(otherTextureID);
    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glTexCoord2f(0.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE, -GameConstants::RenderConstants::CUBE_HALF_SIZE);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE,  GameConstants::RenderConstants::CUBE_HALF_SIZE);
    
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    
    glPopMatrix();
}

} // namespace gfx
