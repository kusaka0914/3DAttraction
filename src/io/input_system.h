#pragma once

#include <GLFW/glfw3.h>
#include "../game/game_state.h"
#include "../game/platform_system.h"

namespace io {
    class AudioManager;
}

/**
 * @brief 入力システム
 * @details キーボード、マウス、ゲームパッドの入力処理を統合的に管理します。
 */
class InputSystem {
public:
    /**
     * @brief マウスコールバック
     * @details マウスの移動を処理します。
     * 
     * @param window GLFWウィンドウ
     * @param xpos マウスX座標
     * @param ypos マウスY座標
     */
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    
    /**
     * @brief スクロールコールバック
     * @details マウスホイールのスクロールを処理します。
     * 
     * @param window GLFWウィンドウ
     * @param xoffset スクロールXオフセット（未使用）
     * @param yoffset スクロールYオフセット
     */
    static void scroll_callback(GLFWwindow* window, double, double yoffset);
    
    /**
     * @brief 入力処理
     * @details キーボード入力の処理を実行します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param deltaTime デルタタイム
     */
    static void processInput(GLFWwindow* window, GameState& gameState, float deltaTime);
    
    /**
     * @brief ジャンプと浮遊の処理
     * @details ジャンプと浮遊の入力処理を実行します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param deltaTime デルタタイム
     * @param gravityDirection 重力方向
     * @param platformSystem プラットフォームシステム
     * @param audioManager オーディオマネージャー
     */
    static void processJumpAndFloat(GLFWwindow* window, GameState& gameState, float deltaTime, const glm::vec3& gravityDirection, PlatformSystem& platformSystem, io::AudioManager& audioManager);
    
    /**
     * @brief ゲームパッドを初期化する
     * @details ゲームパッドの接続を確認し、初期化します。
     */
    static void initializeGamepad();
    
    /**
     * @brief ゲームパッドが接続されているか確認する
     * @return 接続されている場合true
     */
    static bool isGamepadConnected();
    
    /**
     * @brief ゲームパッドの左スティックの値を取得する
     * @return 左スティックのX/Y値のベクター
     */
    static glm::vec2 getGamepadLeftStick();
    
    /**
     * @brief ゲームパッドボタンが押されているか確認する
     * @param button ボタン番号
     * @return 押されている場合true
     */
    static bool isGamepadButtonPressed(int button);
    
    /**
     * @brief ゲームパッドボタンが今フレームで押されたか確認する
     * @param button ボタン番号
     * @return 今フレームで押された場合true
     */
    static bool isGamepadButtonJustPressed(int button);
};
