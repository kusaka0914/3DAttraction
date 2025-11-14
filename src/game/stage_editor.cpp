#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "stage_editor.h"
#include "json_stage_loader.h"
#include "../gfx/camera_system.h"
#include "../gfx/ui_renderer.h"
#include "../core/constants/game_constants.h"
#include "../core/utils/resource_path.h"
#include "../core/error_handler.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <fstream>
#include <sstream>
#include <vector>

static float distanceToLineSegment2D(const glm::vec2& point, const glm::vec2& lineStart, const glm::vec2& lineEnd);

void StageEditor::toggleEditorMode(GLFWwindow* window, GameState& gameState, EditorState& editorState) {
    editorState.isActive = !editorState.isActive;
    
    if (editorState.isActive) {
        editorState.wasFreeCameraActive = gameState.skills.isFreeCameraActive;
        gameState.skills.isFreeCameraActive = true;
        gameState.skills.freeCameraYaw = 90.0f;
        gameState.skills.freeCameraPitch = -30.0f;
        gameState.skills.freeCameraTimer = 999999.0f;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        printf("EDITOR: Editor mode ON\n");
    } else {
        gameState.skills.isFreeCameraActive = editorState.wasFreeCameraActive;
        editorState.selectedPlatformIndex = -1;
        editorState.selectedPlatform = nullptr;
        if (gameState.skills.isFreeCameraActive) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        printf("EDITOR: Editor mode OFF\n");
    }
}

bool StageEditor::screenToWorldRay(GLFWwindow* window, const glm::vec3& cameraPos, 
                                   const glm::vec3& cameraTarget, float fov,
                                   double mouseX, double mouseY, 
                                   glm::vec3& rayOrigin, glm::vec3& rayDirection) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    float x = (2.0f * static_cast<float>(mouseX)) / static_cast<float>(width) - 1.0f;
    float y = 1.0f - (2.0f * static_cast<float>(mouseY)) / static_cast<float>(height);
    
    glm::vec3 cameraForward = glm::normalize(cameraTarget - cameraPos);
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraForward, glm::vec3(0, 1, 0)));
    glm::vec3 cameraUp = glm::cross(cameraRight, cameraForward);
    
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    
    float tanFov = tan(glm::radians(fov / 2.0f));
    
    glm::vec3 rayDir = cameraForward + 
                       (cameraRight * x * tanFov * aspect) + 
                       (cameraUp * y * tanFov);
    rayDir = glm::normalize(rayDir);
    
    rayOrigin = cameraPos;
    rayDirection = rayDir;
    
    return true;
}

bool StageEditor::rayPlaneIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                       float planeY, glm::vec3& intersection) {
    if (std::abs(rayDirection.y) < 0.0001f) {
        return false;
    }
    
    float t = (planeY - rayOrigin.y) / rayDirection.y;
    
    if (t < 0.0f) {
        return false;  // レイの後ろ側
    }
    
    intersection = rayOrigin + rayDirection * t;
    return true;
}

bool StageEditor::rayPlatformTopIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                            const PlatformSystem& platformSystem, glm::vec3& intersection) {
    float closestDistance = std::numeric_limits<float>::max();
    bool found = false;
    glm::vec3 closestIntersection;
    
    const auto& platforms = platformSystem.getPlatforms();
    for (const auto& platform : platforms) {
        std::visit([&](const auto& p) {
            glm::vec3 halfSize = p.size * 0.5f;
            float topY = p.position.y + halfSize.y;
            
            if (std::abs(rayDirection.y) < 0.0001f) {
                return;  // レイが水平方向の場合はスキップ
            }
            
            float t = (topY - rayOrigin.y) / rayDirection.y;
            if (t < 0.0f) {
                return;  // レイの後ろ側
            }
            
            glm::vec3 hitPoint = rayOrigin + rayDirection * t;
            
            if (hitPoint.x >= p.position.x - halfSize.x && hitPoint.x <= p.position.x + halfSize.x &&
                hitPoint.z >= p.position.z - halfSize.z && hitPoint.z <= p.position.z + halfSize.z) {
                if (t < closestDistance) {
                    closestDistance = t;
                    closestIntersection = hitPoint;
                    found = true;
                }
            }
        }, platform);
    }
    
    if (found) {
        intersection = closestIntersection;
        return true;
    }
    
    return false;
}

int StageEditor::selectObjectByRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                   const PlatformSystem& platformSystem) {
    int closestIndex = -1;
    float closestDistance = std::numeric_limits<float>::max();
    
    const auto& platforms = platformSystem.getPlatforms();
    for (size_t i = 0; i < platforms.size(); i++) {
        std::visit([&](const auto& platform) {
            glm::vec3 halfSize = platform.size * 0.5f;
            glm::vec3 min = platform.position - halfSize;
            glm::vec3 max = platform.position + halfSize;
            
            float tmin = (min.x - rayOrigin.x) / rayDirection.x;
            float tmax = (max.x - rayOrigin.x) / rayDirection.x;
            if (tmin > tmax) std::swap(tmin, tmax);
            
            float tymin = (min.y - rayOrigin.y) / rayDirection.y;
            float tymax = (max.y - rayOrigin.y) / rayDirection.y;
            if (tymin > tymax) std::swap(tymin, tymax);
            
            if ((tmin > tymax) || (tymin > tmax)) return;
            
            if (tymin > tmin) tmin = tymin;
            if (tymax < tmax) tmax = tymax;
            
            float tzmin = (min.z - rayOrigin.z) / rayDirection.z;
            float tzmax = (max.z - rayOrigin.z) / rayDirection.z;
            if (tzmin > tzmax) std::swap(tzmin, tzmax);
            
            if ((tmin > tzmax) || (tzmin > tmax)) return;
            
            if (tzmin > tmin) tmin = tzmin;
            if (tzmax < tmax) tmax = tzmax;
            
            if (tmin > 0.0f && tmin < closestDistance) {
                closestDistance = tmin;
                closestIndex = static_cast<int>(i);
            }
        }, platforms[i]);
    }
    
    return closestIndex;
}

void StageEditor::placeObject(EditorState& editorState, PlatformSystem& platformSystem,
                              const glm::vec3& position) {
    glm::vec3 snappedPos = position;
    if (editorState.snapToGrid) {
        snappedPos.x = std::round(position.x / editorState.gridSize) * editorState.gridSize;
        snappedPos.y = std::round(position.y / editorState.gridSize) * editorState.gridSize;
        snappedPos.z = std::round(position.z / editorState.gridSize) * editorState.gridSize;
    }
    
    if (editorState.placingObjectType == "staticPlatform") {
        StaticPlatform newPlatform(snappedPos, glm::vec3(4, 1, 4), glm::vec3(0.0f, 1.0f, 0.0f));
        platformSystem.addPlatform(newPlatform);
        printf("EDITOR: Placed static platform at (%.2f, %.2f, %.2f)\n", 
               snappedPos.x, snappedPos.y, snappedPos.z);
    }
}

void StageEditor::deleteSelectedObject(EditorState& editorState, PlatformSystem& platformSystem) {
    if (editorState.selectedPlatformIndex < 0) {
        return;
    }
    
    int indexToDelete = editorState.selectedPlatformIndex;
    if (platformSystem.removePlatform(indexToDelete)) {
        printf("EDITOR: Deleted platform %d\n", indexToDelete);
        editorState.selectedPlatformIndex = -1;
        editorState.selectedPlatform = nullptr;
    } else {
        printf("EDITOR: Failed to delete platform %d\n", indexToDelete);
    }
}

void StageEditor::moveSelectedObject(EditorState& editorState, PlatformSystem& platformSystem,
                                    const glm::vec3& delta) {
    if (editorState.selectedPlatformIndex < 0 || !editorState.selectedPlatform) {
        return;
    }
    
    std::visit([&](auto& platform) {
        platform.position += delta;
        if (editorState.snapToGrid) {
            platform.position.x = std::round(platform.position.x / editorState.gridSize) * editorState.gridSize;
            platform.position.y = std::round(platform.position.y / editorState.gridSize) * editorState.gridSize;
            platform.position.z = std::round(platform.position.z / editorState.gridSize) * editorState.gridSize;
        }
    }, *editorState.selectedPlatform);
}

void StageEditor::renderEditorUI(GLFWwindow* window, const EditorState& editorState,
                                 const GameState& gameState,
                                 std::unique_ptr<gfx::UIRenderer>& uiRenderer) {
    if (!uiRenderer) {
        return;
    }
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    std::string modeName;
    switch (editorState.currentMode) {
        case EditorMode::SELECT: modeName = "SELECT"; break;
        case EditorMode::PLACE: modeName = "PLACE"; break;
        case EditorMode::MOVE: modeName = "MOVE"; break;
        case EditorMode::DELETE: modeName = "DELETE"; break;
    }
    
    std::vector<std::string> lines;
    lines.push_back("=== STAGE EDITOR ===");
    lines.push_back("Mode: " + modeName);
    lines.push_back("Selected: " + (editorState.selectedPlatformIndex >= 0 ? 
                                    std::to_string(editorState.selectedPlatformIndex) : "None"));
    lines.push_back("Object Type: " + editorState.placingObjectType);
    lines.push_back("Grid Snap: " + std::string(editorState.snapToGrid ? "ON" : "OFF"));
    lines.push_back("Grid Size: " + std::to_string(static_cast<int>(editorState.gridSize)));
    lines.push_back("");
    lines.push_back("--- Controls ---");
    lines.push_back("P: Toggle Editor");
    lines.push_back("1: Select Mode");
    lines.push_back("2: Place Mode");
    lines.push_back("3: Move Mode (Drag)");
    lines.push_back("4: Delete Mode");
    lines.push_back("G: Toggle Grid Snap");
    lines.push_back("O: Save Stage");
    lines.push_back("WASD: Move Camera");
    lines.push_back("Scroll: Rotate Camera");
    lines.push_back("Left Click: Select/Place/Delete");
    lines.push_back("Move Mode: Drag to Move");
    lines.push_back("Arrow Keys: Move (when selected)");
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float boxWidth = 420.0f;
    float boxHeight = static_cast<float>(lines.size() * 22 + 20);
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(10, 10);
    glVertex2f(10 + boxWidth, 10);
    glVertex2f(10 + boxWidth, 10 + boxHeight);
    glVertex2f(10, 10 + boxHeight);
    glEnd();
    
    float yPos = 30.0f;
    for (const auto& line : lines) {
        glm::vec2 pos(20.0f, yPos);
        glm::vec3 color(1.0f, 1.0f, 1.0f);
        uiRenderer->renderText(line, pos, color, 0.5f);
        yPos += 22.0f;
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void StageEditor::renderSelectionHighlight(const EditorState& editorState,
                                          const PlatformSystem& platformSystem,
                                          const GameState& gameState) {
    if (editorState.selectedPlatformIndex < 0 || !editorState.selectedPlatform) {
        return;
    }
    
    if (editorState.isDragging) {
        glLineWidth(5.0f);
    } else {
        glLineWidth(3.0f);
    }
    
    std::visit([&](const auto& platform) {
        glPushMatrix();
        glTranslatef(platform.position.x, platform.position.y, platform.position.z);
        
        glDisable(GL_TEXTURE_2D);
        glColor3f(1.0f, 1.0f, 0.0f);  // 黄色
        glLineWidth(3.0f);
        
        glBegin(GL_LINE_LOOP);
        glm::vec3 halfSize = platform.size * 0.5f;
        glVertex3f(-halfSize.x, -halfSize.y, -halfSize.z);
        glVertex3f( halfSize.x, -halfSize.y, -halfSize.z);
        glVertex3f( halfSize.x, -halfSize.y,  halfSize.z);
        glVertex3f(-halfSize.x, -halfSize.y,  halfSize.z);
        glEnd();
        
        glBegin(GL_LINE_LOOP);
        glVertex3f(-halfSize.x,  halfSize.y, -halfSize.z);
        glVertex3f( halfSize.x,  halfSize.y, -halfSize.z);
        glVertex3f( halfSize.x,  halfSize.y,  halfSize.z);
        glVertex3f(-halfSize.x,  halfSize.y,  halfSize.z);
        glEnd();
        
        glBegin(GL_LINES);
        glVertex3f(-halfSize.x, -halfSize.y, -halfSize.z);
        glVertex3f(-halfSize.x,  halfSize.y, -halfSize.z);
        glVertex3f( halfSize.x, -halfSize.y, -halfSize.z);
        glVertex3f( halfSize.x,  halfSize.y, -halfSize.z);
        glVertex3f( halfSize.x, -halfSize.y,  halfSize.z);
        glVertex3f( halfSize.x,  halfSize.y,  halfSize.z);
        glVertex3f(-halfSize.x, -halfSize.y,  halfSize.z);
        glVertex3f(-halfSize.x,  halfSize.y,  halfSize.z);
        glEnd();
        
        glLineWidth(1.0f);
        glPopMatrix();
    }, *editorState.selectedPlatform);
    
    if (!editorState.isDragging) {
        glLineWidth(1.0f);
    }
    
    if (editorState.currentMode == EditorMode::MOVE && editorState.selectedPlatform) {
        CameraConfig cameraConfig;
        cameraConfig.fov = GameConstants::CAMERA_FOV;
        float yaw = glm::radians(gameState.skills.freeCameraYaw);
        float pitch = glm::radians(gameState.skills.freeCameraPitch);
        float distance = 15.0f;
        cameraConfig.position.x = gameState.player.position.x + distance * cos(yaw) * cos(pitch);
        cameraConfig.position.y = gameState.player.position.y + distance * sin(pitch);
        cameraConfig.position.z = gameState.player.position.z + distance * sin(yaw) * cos(pitch);
        cameraConfig.target = gameState.player.position;
        
        std::visit([&](const auto& platform) {
            renderGizmo(editorState, cameraConfig.position, platform.position);
        }, *editorState.selectedPlatform);
    }
}

void StageEditor::renderGizmo(const EditorState& editorState, const glm::vec3& cameraPos,
                              const glm::vec3& objectPos) {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(3.0f);
    
    float arrowLength = editorState.gizmoSize;
    float arrowHeadSize = 0.5f;
    
    glColor3f(1.0f, 0.0f, 0.0f);
    if (editorState.selectedAxis == EditorState::GizmoAxis::X) {
        glLineWidth(5.0f);
    }
    glBegin(GL_LINES);
    glVertex3f(objectPos.x, objectPos.y, objectPos.z);
    glVertex3f(objectPos.x + arrowLength, objectPos.y, objectPos.z);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3f(objectPos.x + arrowLength, objectPos.y, objectPos.z);
    glVertex3f(objectPos.x + arrowLength - arrowHeadSize, objectPos.y + arrowHeadSize * 0.3f, objectPos.z);
    glVertex3f(objectPos.x + arrowLength - arrowHeadSize, objectPos.y - arrowHeadSize * 0.3f, objectPos.z);
    glVertex3f(objectPos.x + arrowLength, objectPos.y, objectPos.z);
    glVertex3f(objectPos.x + arrowLength - arrowHeadSize, objectPos.y, objectPos.z + arrowHeadSize * 0.3f);
    glVertex3f(objectPos.x + arrowLength - arrowHeadSize, objectPos.y, objectPos.z - arrowHeadSize * 0.3f);
    glEnd();
    
    glColor3f(0.0f, 1.0f, 0.0f);
    if (editorState.selectedAxis == EditorState::GizmoAxis::Y) {
        glLineWidth(5.0f);
    } else {
        glLineWidth(3.0f);
    }
    glBegin(GL_LINES);
    glVertex3f(objectPos.x, objectPos.y, objectPos.z);
    glVertex3f(objectPos.x, objectPos.y + arrowLength, objectPos.z);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3f(objectPos.x, objectPos.y + arrowLength, objectPos.z);
    glVertex3f(objectPos.x + arrowHeadSize * 0.3f, objectPos.y + arrowLength - arrowHeadSize, objectPos.z);
    glVertex3f(objectPos.x - arrowHeadSize * 0.3f, objectPos.y + arrowLength - arrowHeadSize, objectPos.z);
    glVertex3f(objectPos.x, objectPos.y + arrowLength, objectPos.z);
    glVertex3f(objectPos.x, objectPos.y + arrowLength - arrowHeadSize, objectPos.z + arrowHeadSize * 0.3f);
    glVertex3f(objectPos.x, objectPos.y + arrowLength - arrowHeadSize, objectPos.z - arrowHeadSize * 0.3f);
    glEnd();
    
    glColor3f(0.0f, 0.0f, 1.0f);
    if (editorState.selectedAxis == EditorState::GizmoAxis::Z) {
        glLineWidth(5.0f);
    } else {
        glLineWidth(3.0f);
    }
    glBegin(GL_LINES);
    glVertex3f(objectPos.x, objectPos.y, objectPos.z);
    glVertex3f(objectPos.x, objectPos.y, objectPos.z + arrowLength);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3f(objectPos.x, objectPos.y, objectPos.z + arrowLength);
    glVertex3f(objectPos.x + arrowHeadSize * 0.3f, objectPos.y, objectPos.z + arrowLength - arrowHeadSize);
    glVertex3f(objectPos.x - arrowHeadSize * 0.3f, objectPos.y, objectPos.z + arrowLength - arrowHeadSize);
    glVertex3f(objectPos.x, objectPos.y, objectPos.z + arrowLength);
    glVertex3f(objectPos.x, objectPos.y + arrowHeadSize * 0.3f, objectPos.z + arrowLength - arrowHeadSize);
    glVertex3f(objectPos.x, objectPos.y - arrowHeadSize * 0.3f, objectPos.z + arrowLength - arrowHeadSize);
    glEnd();
    
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

static glm::vec2 worldToScreen(const glm::vec3& worldPos, const glm::vec3& cameraPos,
                               const glm::vec3& cameraTarget, float fov,
                               int screenWidth, int screenHeight) {
    glm::vec3 forward = glm::normalize(cameraTarget - cameraPos);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    glm::vec3 up = glm::cross(right, forward);
    
    glm::vec3 toObject = worldPos - cameraPos;
    
    float z = glm::dot(toObject, forward);
    if (z <= 0.0f) {
        return glm::vec2(-1, -1);  // カメラの後ろ
    }
    
    float x = glm::dot(toObject, right);
    float y = glm::dot(toObject, up);
    
    float aspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    float tanHalfFov = tan(glm::radians(fov * 0.5f));
    
    float screenX = (x / (z * tanHalfFov * aspect) + 1.0f) * 0.5f * screenWidth;
    float screenY = (1.0f - (y / (z * tanHalfFov) + 1.0f) * 0.5f) * screenHeight;
    
    return glm::vec2(screenX, screenY);
}

EditorState::GizmoAxis StageEditor::selectGizmoAxis(const EditorState& editorState,
                                                    const glm::vec3& cameraPos,
                                                    const glm::vec3& cameraTarget,
                                                    double mouseX, double mouseY,
                                                    GLFWwindow* window) {
    if (!editorState.selectedPlatform) {
        return EditorState::GizmoAxis::NONE;
    }
    
    glm::vec3 objectPos;
    std::visit([&](const auto& platform) {
        objectPos = platform.position;
    }, *editorState.selectedPlatform);
    
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    
    float arrowLength = editorState.gizmoSize;
    float selectionRadius = 30.0f;  // スクリーン空間での選択範囲（ピクセル単位、大きく設定）
    float closestDistance = std::numeric_limits<float>::max();
    EditorState::GizmoAxis closestAxis = EditorState::GizmoAxis::NONE;
    
    glm::vec2 mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));
    
    glm::vec2 objScreen = worldToScreen(objectPos, cameraPos, cameraTarget, GameConstants::CAMERA_FOV,
                                        screenWidth, screenHeight);
    
    glm::vec3 xAxisEnd = objectPos + glm::vec3(arrowLength, 0, 0);
    glm::vec2 xAxisEndScreen = worldToScreen(xAxisEnd, cameraPos, cameraTarget, GameConstants::CAMERA_FOV,
                                             screenWidth, screenHeight);
    if (xAxisEndScreen.x >= 0 && objScreen.x >= 0) {
        float dist = distanceToLineSegment2D(mousePos, objScreen, xAxisEndScreen);
        if (dist < selectionRadius && dist < closestDistance) {
            closestDistance = dist;
            closestAxis = EditorState::GizmoAxis::X;
        }
    }
    
    glm::vec3 yAxisEnd = objectPos + glm::vec3(0, arrowLength, 0);
    glm::vec2 yAxisEndScreen = worldToScreen(yAxisEnd, cameraPos, cameraTarget, GameConstants::CAMERA_FOV,
                                             screenWidth, screenHeight);
    if (yAxisEndScreen.x >= 0 && objScreen.x >= 0) {
        float dist = distanceToLineSegment2D(mousePos, objScreen, yAxisEndScreen);
        if (dist < selectionRadius && dist < closestDistance) {
            closestDistance = dist;
            closestAxis = EditorState::GizmoAxis::Y;
        }
    }
    
    glm::vec3 zAxisEnd = objectPos + glm::vec3(0, 0, arrowLength);
    glm::vec2 zAxisEndScreen = worldToScreen(zAxisEnd, cameraPos, cameraTarget, GameConstants::CAMERA_FOV,
                                             screenWidth, screenHeight);
    if (zAxisEndScreen.x >= 0 && objScreen.x >= 0) {
        float dist = distanceToLineSegment2D(mousePos, objScreen, zAxisEndScreen);
        if (dist < selectionRadius && dist < closestDistance) {
            closestDistance = dist;
            closestAxis = EditorState::GizmoAxis::Z;
        }
    }
    
    return closestAxis;
}

static float distanceToLineSegment2D(const glm::vec2& point, const glm::vec2& lineStart, const glm::vec2& lineEnd) {
    glm::vec2 lineDir = lineEnd - lineStart;
    float lineLengthSq = glm::dot(lineDir, lineDir);
    
    if (lineLengthSq < 0.0001f) {
        return glm::length(point - lineStart);
    }
    
    float t = glm::dot(point - lineStart, lineDir) / lineLengthSq;
    t = std::max(0.0f, std::min(1.0f, t));  // 線分の範囲内に制限
    
    glm::vec2 closestPoint = lineStart + lineDir * t;
    return glm::length(point - closestPoint);
}

void StageEditor::renderGrid(const glm::vec3& cameraPos, float gridSize, int gridCount) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(1.0f);
    
    float startX = std::floor(cameraPos.x / gridSize) * gridSize - gridCount * gridSize;
    float endX = startX + gridCount * 2 * gridSize;
    float startZ = std::floor(cameraPos.z / gridSize) * gridSize - gridCount * gridSize;
    float endZ = startZ + gridCount * 2 * gridSize;
    float gridY = 0.0f;
    
    glBegin(GL_LINES);
    for (float x = startX; x <= endX; x += gridSize) {
        glVertex3f(x, gridY, startZ);
        glVertex3f(x, gridY, endZ);
    }
    for (float z = startZ; z <= endZ; z += gridSize) {
        glVertex3f(startX, gridY, z);
        glVertex3f(endX, gridY, z);
    }
    glEnd();
}

void StageEditor::processEditorInput(GLFWwindow* window, GameState& gameState, 
                                     EditorState& editorState, PlatformSystem& platformSystem,
                                     StageManager& stageManager, float deltaTime) {
    static bool pKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pKeyPressed) {
        pKeyPressed = true;
        toggleEditorMode(window, gameState, editorState);
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        pKeyPressed = false;
    }
    
    if (!editorState.isActive) {
        return;
    }
    
    glfwGetCursorPos(window, &editorState.mouseX, &editorState.mouseY);
    
    CameraConfig cameraConfig;
    cameraConfig.fov = GameConstants::CAMERA_FOV;
    cameraConfig.nearPlane = GameConstants::CAMERA_NEAR;
    cameraConfig.farPlane = GameConstants::CAMERA_FAR;
    
    float yaw = glm::radians(gameState.skills.freeCameraYaw);
    float pitch = glm::radians(gameState.skills.freeCameraPitch);
    float distance = 15.0f;
    cameraConfig.position.x = gameState.player.position.x + distance * cos(yaw) * cos(pitch);
    cameraConfig.position.y = gameState.player.position.y + distance * sin(pitch);
    cameraConfig.position.z = gameState.player.position.z + distance * sin(yaw) * cos(pitch);
    cameraConfig.target = gameState.player.position;
    
    glm::vec3 rayOrigin, rayDirection;
    screenToWorldRay(window, cameraConfig.position, cameraConfig.target, cameraConfig.fov,
                     editorState.mouseX, editorState.mouseY, rayOrigin, rayDirection);
    
    glm::vec3 platformIntersection;
    bool foundPlatform = rayPlatformTopIntersection(rayOrigin, rayDirection, platformSystem, platformIntersection);
    
    glm::vec3 groundIntersection;
    bool foundGround = rayPlaneIntersection(rayOrigin, rayDirection, 0.0f, groundIntersection);
    
    if (foundPlatform) {
        editorState.previewPosition = platformIntersection;
        editorState.showPreview = true;
    } else if (foundGround) {
        editorState.previewPosition = groundIntersection;
        editorState.showPreview = true;
    } else {
        float defaultDistance = 20.0f;
        editorState.previewPosition = rayOrigin + rayDirection * defaultDistance;
        editorState.showPreview = true;
    }
    
    if (editorState.snapToGrid) {
        editorState.previewPosition.x = std::round(editorState.previewPosition.x / editorState.gridSize) * editorState.gridSize;
        editorState.previewPosition.y = std::round(editorState.previewPosition.y / editorState.gridSize) * editorState.gridSize;
        editorState.previewPosition.z = std::round(editorState.previewPosition.z / editorState.gridSize) * editorState.gridSize;
    }
    
    static bool leftMousePressed = false;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !leftMousePressed) {
        leftMousePressed = true;
        
        if (editorState.currentMode == EditorMode::PLACE && editorState.showPreview) {
            placeObject(editorState, platformSystem, editorState.previewPosition);
        } else if (editorState.currentMode == EditorMode::DELETE) {
            int selectedIndex = selectObjectByRay(rayOrigin, rayDirection, platformSystem);
            if (selectedIndex >= 0) {
                editorState.selectedPlatformIndex = selectedIndex;
                deleteSelectedObject(editorState, platformSystem);
            }
        } else if (editorState.currentMode == EditorMode::SELECT) {
            int selectedIndex = selectObjectByRay(rayOrigin, rayDirection, platformSystem);
            if (selectedIndex >= 0) {
                editorState.selectedPlatformIndex = selectedIndex;
                auto& platforms = platformSystem.getPlatforms();
                if (selectedIndex < static_cast<int>(platforms.size())) {
                    editorState.selectedPlatform = &platforms[selectedIndex];
                }
                printf("EDITOR: Selected platform %d\n", selectedIndex);
            } else {
                editorState.selectedPlatformIndex = -1;
                editorState.selectedPlatform = nullptr;
                printf("EDITOR: Deselected\n");
            }
        } else if (editorState.currentMode == EditorMode::MOVE) {
            if (editorState.selectedPlatformIndex >= 0 && editorState.selectedPlatform) {
                std::visit([&](const auto& platform) {
                    CameraConfig cameraConfig;
                    cameraConfig.fov = GameConstants::CAMERA_FOV;
                    float yaw = glm::radians(gameState.skills.freeCameraYaw);
                    float pitch = glm::radians(gameState.skills.freeCameraPitch);
                    float distance = 15.0f;
                    cameraConfig.position.x = gameState.player.position.x + distance * cos(yaw) * cos(pitch);
                    cameraConfig.position.y = gameState.player.position.y + distance * sin(pitch);
                    cameraConfig.position.z = gameState.player.position.z + distance * sin(yaw) * cos(pitch);
                    cameraConfig.target = gameState.player.position;
                    
                    EditorState::GizmoAxis axis = selectGizmoAxis(editorState, cameraConfig.position,
                                                                  cameraConfig.target,
                                                                  editorState.mouseX, editorState.mouseY, window);
                    if (axis != EditorState::GizmoAxis::NONE) {
                        editorState.selectedAxis = axis;
                        editorState.isDragging = true;
                        editorState.dragStartObjectPosition = platform.position;
                        glm::vec3 platformIntersection;
                        bool foundPlatform = rayPlatformTopIntersection(rayOrigin, rayDirection, platformSystem, platformIntersection);
                        if (!foundPlatform) {
                            rayPlaneIntersection(rayOrigin, rayDirection, 0.0f, platformIntersection);
                        }
                        editorState.dragStartPosition = platformIntersection;
                        printf("EDITOR: Started dragging axis %d\n", static_cast<int>(axis));
                    } else {
                        editorState.selectedPlatformIndex = -1;
                        editorState.selectedPlatform = nullptr;
                        editorState.selectedAxis = EditorState::GizmoAxis::NONE;
                    }
                }, *editorState.selectedPlatform);
            } else {
                int selectedIndex = selectObjectByRay(rayOrigin, rayDirection, platformSystem);
                if (selectedIndex >= 0) {
                    editorState.selectedPlatformIndex = selectedIndex;
                    auto& platforms = platformSystem.getPlatforms();
                    if (selectedIndex < static_cast<int>(platforms.size())) {
                        editorState.selectedPlatform = &platforms[selectedIndex];
                        editorState.selectedAxis = EditorState::GizmoAxis::NONE;
                        printf("EDITOR: Selected platform %d\n", selectedIndex);
                    }
                }
            }
        }
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        leftMousePressed = false;
        if (editorState.isDragging) {
            editorState.isDragging = false;
            editorState.selectedAxis = EditorState::GizmoAxis::NONE;
            printf("EDITOR: Stopped dragging\n");
        }
    }
    
    if (editorState.currentMode == EditorMode::MOVE && editorState.isDragging && 
        editorState.selectedPlatform && editorState.selectedAxis != EditorState::GizmoAxis::NONE) {
        glm::vec3 currentIntersection;
        bool foundPlatform = rayPlatformTopIntersection(rayOrigin, rayDirection, platformSystem, currentIntersection);
        if (!foundPlatform) {
            rayPlaneIntersection(rayOrigin, rayDirection, 0.0f, currentIntersection);
        }
        
        glm::vec3 delta = currentIntersection - editorState.dragStartPosition;
        
        glm::vec3 axisDelta(0.0f);
        if (editorState.selectedAxis == EditorState::GizmoAxis::X) {
            axisDelta.x = delta.x;
        } else if (editorState.selectedAxis == EditorState::GizmoAxis::Y) {
            axisDelta.y = delta.y;
        } else if (editorState.selectedAxis == EditorState::GizmoAxis::Z) {
            axisDelta.z = delta.z;
        }
        
        std::visit([&](auto& platform) {
            glm::vec3 newPosition = editorState.dragStartObjectPosition + axisDelta;
            
            if (editorState.snapToGrid) {
                newPosition.x = std::round(newPosition.x / editorState.gridSize) * editorState.gridSize;
                newPosition.y = std::round(newPosition.y / editorState.gridSize) * editorState.gridSize;
                newPosition.z = std::round(newPosition.z / editorState.gridSize) * editorState.gridSize;
            }
            
            platform.position = newPosition;
        }, *editorState.selectedPlatform);
    }
    
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        editorState.currentMode = EditorMode::SELECT;
        editorState.placingObjectType = "staticPlatform";
    } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        editorState.currentMode = EditorMode::PLACE;
        editorState.placingObjectType = "staticPlatform";
    } else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        editorState.currentMode = EditorMode::MOVE;
    } else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        editorState.currentMode = EditorMode::DELETE;
    }
    
    if (editorState.currentMode == EditorMode::MOVE && editorState.selectedPlatformIndex >= 0 && 
        editorState.selectedPlatform && !editorState.isDragging) {
        glm::vec3 moveDelta(0.0f);
        float moveSpeed = 5.0f * deltaTime;
        
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            moveDelta.z -= moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDelta.z += moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            moveDelta.x -= moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            moveDelta.x += moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
            moveDelta.y += moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
            moveDelta.y -= moveSpeed;
        }
        
        if (glm::length(moveDelta) > 0.0f) {
            moveSelectedObject(editorState, platformSystem, moveDelta);
        }
    }
    
    static bool oKeyPressed = false;
    if (editorState.isActive && glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !oKeyPressed) {
        oKeyPressed = true;
        int currentStage = stageManager.getCurrentStage();
        printf("EDITOR: Attempting to save stage %d\n", currentStage);
        
        std::string stagePath = stageManager.getCurrentStageFilePath();
        
        if (stagePath.empty()) {
            std::string relativePath;
            switch (currentStage) {
                case 0: relativePath = "assets/stages/stage_selection.json"; break;
                case 1: relativePath = "assets/stages/stage1.json"; break;
                case 2: relativePath = "assets/stages/stage2.json"; break;
                case 3: relativePath = "assets/stages/stage3.json"; break;
                case 4: relativePath = "assets/stages/stage4.json"; break;
                case 5: relativePath = "assets/stages/stage5.json"; break;
                case 6: relativePath = "assets/stages/tutorial.json"; break;
                default:
                    ErrorHandler::logErrorFormat("EDITOR: Unknown stage number %d", currentStage);
                    break;
            }
            if (!relativePath.empty()) {
                stagePath = ResourcePath::getResourcePath(relativePath);
            }
        }
        
        printf("EDITOR: Saving to path: %s\n", stagePath.c_str());
        
        if (!stagePath.empty()) {
            if (JsonStageLoader::saveStageToJSON(stagePath, gameState, platformSystem, currentStage)) {
                printf("EDITOR: Stage %d saved successfully to %s\n", currentStage, stagePath.c_str());
            } else {
                ErrorHandler::logErrorFormat("EDITOR: Failed to save stage %d to %s", currentStage, stagePath.c_str());
            }
        } else {
            ErrorHandler::logErrorFormat("EDITOR: No valid path found for stage %d", currentStage);
        }
    } else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
        oKeyPressed = false;
    }
    
    static bool gKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gKeyPressed) {
        gKeyPressed = true;
        editorState.snapToGrid = !editorState.snapToGrid;
        printf("EDITOR: Grid snap %s\n", editorState.snapToGrid ? "ON" : "OFF");
    } else if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) {
        gKeyPressed = false;
    }
}

