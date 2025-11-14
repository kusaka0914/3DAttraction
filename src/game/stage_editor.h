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

namespace gfx {
    class UIRenderer;
}

/**
 * @brief エディタモード
 * @details ステージエディタの動作モードを定義します。
 */
enum class EditorMode {
    SELECT,
    PLACE,
    MOVE,
    DELETE
};

/**
 * @brief エディタ状態
 * @details ステージエディタの状態を保持します。
 */
struct EditorState {
    bool isActive = false;
    EditorMode currentMode = EditorMode::SELECT;
    
    int selectedPlatformIndex = -1;  // -1の場合は未選択
    PlatformVariant* selectedPlatform = nullptr;
    
    std::string placingObjectType = "staticPlatform";
    
    bool snapToGrid = true;
    float gridSize = 1.0f;  // グリッドのサイズ（単位: ワールド座標）
    
    bool wasFreeCameraActive = false;
    
    double mouseX = 0.0;
    double mouseY = 0.0;
    
    glm::vec3 previewPosition = glm::vec3(0.0f);
    bool showPreview = false;
    
    bool isDragging = false;
    glm::vec3 dragStartPosition = glm::vec3(0.0f);
    glm::vec3 dragStartObjectPosition = glm::vec3(0.0f);
    
    enum class GizmoAxis {
        NONE,
        X,
        Y,
        Z
    };
    GizmoAxis selectedAxis = GizmoAxis::NONE;
    float gizmoSize = 3.0f;  // Gizmoの表示サイズ（単位: ワールド座標）
};

/**
 * @brief ステージエディタシステム
 * @details ステージエディタの機能を統合的に管理します。
 */
class StageEditor {
public:
    /**
     * @brief エディタモードを切り替える
     * @details エディタモードのON/OFFを切り替えます。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param editorState エディタ状態
     */
    static void toggleEditorMode(GLFWwindow* window, GameState& gameState, EditorState& editorState);
    
    /**
     * @brief エディタ入力処理
     * @details エディタモードでの入力処理を実行します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param editorState エディタ状態
     * @param platformSystem プラットフォームシステム
     * @param stageManager ステージマネージャー
     * @param deltaTime デルタタイム
     */
    static void processEditorInput(GLFWwindow* window, GameState& gameState, 
                                   EditorState& editorState, PlatformSystem& platformSystem,
                                   StageManager& stageManager, float deltaTime);
    
    /**
     * @brief レイキャスト（マウス位置から3D空間への変換）
     * @details マウス位置から3D空間へのレイを計算します。
     * 
     * @param window GLFWウィンドウ
     * @param cameraPos カメラ位置
     * @param cameraTarget カメラターゲット
     * @param fov 視野角
     * @param mouseX マウスX座標
     * @param mouseY マウスY座標
     * @param rayOrigin 出力: レイの原点
     * @param rayDirection 出力: レイの方向
     * @return 計算成功時true
     */
    static bool screenToWorldRay(GLFWwindow* window, const glm::vec3& cameraPos, 
                                const glm::vec3& cameraTarget, float fov,
                                double mouseX, double mouseY, 
                                glm::vec3& rayOrigin, glm::vec3& rayDirection);
    
    /**
     * @brief レイと平面の交差判定
     * @details レイと指定されたY座標の平面との交差を判定します。
     * 
     * @param rayOrigin レイの原点
     * @param rayDirection レイの方向
     * @param planeY 平面のY座標
     * @param intersection 出力: 交差点
     * @return 交差する場合true
     */
    static bool rayPlaneIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                    float planeY, glm::vec3& intersection);
    
    /**
     * @brief レイとプラットフォームの上面との交差判定
     * @details 配置用にレイとプラットフォームの上面との交差を判定します。
     * 
     * @param rayOrigin レイの原点
     * @param rayDirection レイの方向
     * @param platformSystem プラットフォームシステム
     * @param intersection 出力: 交差点
     * @return 交差する場合true
     */
    static bool rayPlatformTopIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                          const PlatformSystem& platformSystem, glm::vec3& intersection);
    
    /**
     * @brief オブジェクト選択（レイキャスト）
     * @details レイキャストを使用してオブジェクトを選択します。
     * 
     * @param rayOrigin レイの原点
     * @param rayDirection レイの方向
     * @param platformSystem プラットフォームシステム
     * @return 選択されたプラットフォームのインデックス（選択されない場合は-1）
     */
    static int selectObjectByRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                const PlatformSystem& platformSystem);
    
    /**
     * @brief オブジェクトを配置する
     * @details 指定された位置にオブジェクトを配置します。
     * 
     * @param editorState エディタ状態
     * @param platformSystem プラットフォームシステム
     * @param position 配置位置
     */
    static void placeObject(EditorState& editorState, PlatformSystem& platformSystem,
                           const glm::vec3& position);
    
    /**
     * @brief 選択中のオブジェクトを削除する
     * @details 選択中のオブジェクトを削除します。
     * 
     * @param editorState エディタ状態
     * @param platformSystem プラットフォームシステム
     */
    static void deleteSelectedObject(EditorState& editorState, PlatformSystem& platformSystem);
    
    /**
     * @brief 選択中のオブジェクトを移動する
     * @details 選択中のオブジェクトを指定された量だけ移動します。
     * 
     * @param editorState エディタ状態
     * @param platformSystem プラットフォームシステム
     * @param delta 移動量
     */
    static void moveSelectedObject(EditorState& editorState, PlatformSystem& platformSystem,
                                  const glm::vec3& delta);
    
    /**
     * @brief エディタUIを描画する
     * @details エディタのUI要素を描画します。
     * 
     * @param window GLFWウィンドウ
     * @param editorState エディタ状態
     * @param gameState ゲーム状態
     * @param uiRenderer UIレンダラー
     */
    static void renderEditorUI(GLFWwindow* window, const EditorState& editorState,
                              const GameState& gameState,
                              std::unique_ptr<gfx::UIRenderer>& uiRenderer);
    
    /**
     * @brief 選択中のオブジェクトをハイライト表示する
     * @details 選択中のオブジェクトを視覚的に強調表示します。
     * 
     * @param editorState エディタ状態
     * @param platformSystem プラットフォームシステム
     * @param gameState ゲーム状態
     */
    static void renderSelectionHighlight(const EditorState& editorState,
                                        const PlatformSystem& platformSystem,
                                        const GameState& gameState);
    
    /**
     * @brief Gizmo（軸移動用の矢印）を描画する
     * @details オブジェクト移動用のGizmoを描画します。
     * 
     * @param editorState エディタ状態
     * @param cameraPos カメラ位置
     * @param cameraTarget カメラターゲット
     */
    static void renderGizmo(const EditorState& editorState, const glm::vec3& cameraPos,
                           const glm::vec3& cameraTarget);
    
    /**
     * @brief マウス位置からGizmoのどの軸が選択されているかを判定する
     * @details マウス位置に基づいてGizmoの軸選択を判定します。
     * 
     * @param editorState エディタ状態
     * @param cameraPos カメラ位置
     * @param cameraTarget カメラターゲット
     * @param mouseX マウスX座標
     * @param mouseY マウスY座標
     * @param window GLFWウィンドウ
     * @return 選択された軸
     */
    static EditorState::GizmoAxis selectGizmoAxis(const EditorState& editorState,
                                                  const glm::vec3& cameraPos,
                                                  const glm::vec3& cameraTarget,
                                                  double mouseX, double mouseY,
                                                  GLFWwindow* window);
    
    /**
     * @brief グリッドを表示する
     * @details エディタ用のグリッドを描画します。
     * 
     * @param cameraPos カメラ位置
     * @param gridSize グリッドサイズ
     * @param gridCount グリッド数
     */
    static void renderGrid(const glm::vec3& cameraPos, float gridSize, int gridCount);
};
