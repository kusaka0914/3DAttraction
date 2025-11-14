#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "bitmap_font.h"
#include "background_renderer.h"
#include "ui_renderer.h"
#include "game_state_ui_renderer.h"
#include "renderer_3d.h"
#include "texture_manager.h"
#include <vector>
#include <string>
#include <map>

namespace gfx {

/**
 * @brief OpenGLレンダラー
 * @details OpenGLを使用した描画処理を統合的に管理します。
 */
class OpenGLRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();
    
    /**
     * @brief レンダラーを初期化する
     * @details GLFWウィンドウを使用してレンダラーを初期化します。
     * 
     * @param window GLFWウィンドウ
     * @return 初期化成功時true
     */
    bool initialize(GLFWwindow* window);
    
    /**
     * @brief レンダラーをクリーンアップする
     * @details リソースを解放します。
     */
    void cleanup();
    
    /**
     * @brief フレームの開始処理
     * @details フレーム描画の開始処理を実行します。
     */
    void beginFrame();
    
    /**
     * @brief 背景付きフレームの開始処理
     * @details ステージ番号に応じた背景を描画してからフレームを開始します。
     * 
     * @param stageNumber ステージ番号
     */
    void beginFrameWithBackground(int stageNumber);
    
    /**
     * @brief フレームの終了処理
     * @details フレーム描画の終了処理を実行します。
     */
    void endFrame();
    
    /**
     * @brief カメラを設定する
     * @details カメラの位置とターゲットを設定します。
     * 
     * @param position カメラ位置
     * @param target カメラターゲット
     * @param up 上方向ベクトル（デフォルト: (0, 1, 0)）
     */
    void setCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));
    
    /**
     * @brief 投影行列を設定する
     * @details 投影行列のパラメータを設定します。
     * 
     * @param fov 視野角
     * @param aspect アスペクト比
     * @param near ニア平面
     * @param far ファー平面
     */
    void setProjection(float fov, float aspect, float near, float far);

private:
    /**
     * @brief ビットマップ文字を描画する
     * @details 指定された位置にビットマップ文字を描画します。
     * 
     * @param c 描画する文字
     * @param position 描画位置
     * @param color 色
     * @param scale スケール
     */
    void renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale);
    
    GLFWwindow* window;
    
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    BitmapFont font;
    BackgroundRenderer backgroundRenderer;
    UIRenderer uiRenderer;
    GameStateUIRenderer gameStateUIRenderer;
public:
    Renderer3D renderer3D;
private:
};

} // namespace gfx
