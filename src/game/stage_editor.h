#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state.h"
#include "platform_system.h"
#include "stage_manager.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <memory>

// 前方宣言
namespace gfx {
    class UIRenderer;
}

// エディタモード
enum class EditorMode {
    SELECT,    // 選択モード
    PLACE,     // 配置モード
    MOVE,      // 移動モード
    DELETE     // 削除モード
};

// エディタ状態
struct EditorState {
    bool isActive = false;
    EditorMode currentMode = EditorMode::SELECT;
    
    // 選択中のオブジェクト
    int selectedPlatformIndex = -1;
    PlatformVariant* selectedPlatform = nullptr;
    
    // 配置するオブジェクトタイプ
    std::string placingObjectType = "staticPlatform";
    
    // グリッドスナップ
    bool snapToGrid = true;
    float gridSize = 1.0f;
    
    // エディタカメラ位置（フリーカメラを利用）
    bool wasFreeCameraActive = false;
    
    // マウス位置
    double mouseX = 0.0;
    double mouseY = 0.0;
    
    // 配置用の一時オブジェクト位置
    glm::vec3 previewPosition = glm::vec3(0.0f);
    bool showPreview = false;
    
    // ドラッグ移動用
    bool isDragging = false;
    glm::vec3 dragStartPosition = glm::vec3(0.0f);
    glm::vec3 dragStartObjectPosition = glm::vec3(0.0f);
    
    // Gizmo（軸移動）用
    enum class GizmoAxis {
        NONE,
        X,
        Y,
        Z
    };
    GizmoAxis selectedAxis = GizmoAxis::NONE;
    float gizmoSize = 3.0f;  // Gizmoのサイズ
};

// ステージエディタシステム
class StageEditor {
public:
    // エディタモードの切り替え
    static void toggleEditorMode(GLFWwindow* window, GameState& gameState, EditorState& editorState);
    
    // エディタ入力処理
    static void processEditorInput(GLFWwindow* window, GameState& gameState, 
                                   EditorState& editorState, PlatformSystem& platformSystem,
                                   StageManager& stageManager, float deltaTime);
    
    // レイキャスト（マウス位置から3D空間への変換）
    static bool screenToWorldRay(GLFWwindow* window, const glm::vec3& cameraPos, 
                                const glm::vec3& cameraTarget, float fov,
                                double mouseX, double mouseY, 
                                glm::vec3& rayOrigin, glm::vec3& rayDirection);
    
    // レイと平面の交差判定（Y=0の平面）
    static bool rayPlaneIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                    float planeY, glm::vec3& intersection);
    
    // レイとプラットフォームの上面との交差判定（配置用）
    static bool rayPlatformTopIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                          const PlatformSystem& platformSystem, glm::vec3& intersection);
    
    // オブジェクト選択（レイキャスト）
    static int selectObjectByRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                const PlatformSystem& platformSystem);
    
    // オブジェクト配置
    static void placeObject(EditorState& editorState, PlatformSystem& platformSystem,
                           const glm::vec3& position);
    
    // オブジェクト削除
    static void deleteSelectedObject(EditorState& editorState, PlatformSystem& platformSystem);
    
    // オブジェクト移動
    static void moveSelectedObject(EditorState& editorState, PlatformSystem& platformSystem,
                                  const glm::vec3& delta);
    
    // エディタUI描画
    static void renderEditorUI(GLFWwindow* window, const EditorState& editorState,
                              const GameState& gameState,
                              std::unique_ptr<gfx::UIRenderer>& uiRenderer);
    
    // 選択中のオブジェクトをハイライト表示
    static void renderSelectionHighlight(const EditorState& editorState,
                                        const PlatformSystem& platformSystem,
                                        const GameState& gameState);
    
    // Gizmo（軸移動用の矢印）を描画
    static void renderGizmo(const EditorState& editorState, const glm::vec3& cameraPos,
                           const glm::vec3& cameraTarget);
    
    // マウス位置からGizmoのどの軸が選択されているかを判定
    static EditorState::GizmoAxis selectGizmoAxis(const EditorState& editorState,
                                                  const glm::vec3& cameraPos,
                                                  const glm::vec3& cameraTarget,
                                                  double mouseX, double mouseY,
                                                  GLFWwindow* window);
    
    // グリッド表示
    static void renderGrid(const glm::vec3& cameraPos, float gridSize, int gridCount);
};

