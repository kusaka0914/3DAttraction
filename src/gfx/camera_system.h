/**
 * @file camera_system.h
 * @brief カメラシステム
 * @details カメラの位置計算、スムージング、設定適用を管理します。
 */
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cmath>
#include "opengl_renderer.h"
#include "../game/game_state.h"
#include "../game/stage_manager.h"
#include "../core/constants/game_constants.h"

/**
 * @brief カメラ設定の構造体
 * @details カメラの位置、ターゲット、投影パラメータを保持します。
 */
struct CameraConfig {
    glm::vec3 position;
    glm::vec3 target;
    float fov;
    float nearPlane;
    float farPlane;
};

/**
 * @brief カメラシステム
 * @details カメラの位置計算、スムージング、設定適用を管理します。
 */
class CameraSystem {
private:
    // カメラスムージング用の静的変数（画面揺れ防止）
    inline static glm::vec3 smoothCameraPosition = glm::vec3(0, 30.0f, 0);
    inline static glm::vec3 smoothCameraTarget = glm::vec3(0, 30.0f, 0);
    inline static bool isInitialized = false;
    
public:
    /**
     * @brief カメラ設定を計算する
     * @details プレイヤーの位置に基づいてカメラの位置とターゲットを計算します。
     * 落下時の急激な位置変化を滑らかにするため、カメラスムージングを適用します。
     * 
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param window GLFWウィンドウ
     * @param deltaTime デルタタイム（スムージング用）
     * @return CameraConfig 計算されたカメラ設定
     */
    static CameraConfig calculateCameraConfig(const GameState& gameState, 
                                            const StageManager& stageManager, 
                                            GLFWwindow* /* window */, 
                                            float deltaTime) {
        CameraConfig config;
        config.fov = GameConstants::CAMERA_FOV;
        config.nearPlane = GameConstants::CAMERA_NEAR;
        config.farPlane = GameConstants::CAMERA_FAR;
        
        glm::vec3 targetPosition;
        glm::vec3 targetTarget;
        
        if (gameState.skills.isFreeCameraActive) {
            // フリーカメラ：プレイヤーを中心にカメラが回転
            float yaw = glm::radians(gameState.skills.freeCameraYaw);
            float pitch = glm::radians(gameState.skills.freeCameraPitch);
            
            // カメラの位置を計算（プレイヤーからの相対位置）
            float distance = (stageManager.getCurrentStage() == 0) ? 15.0f : 8.0f;
            
            // プレイヤーを中心とした球面座標でカメラ位置を計算
            targetPosition.x = gameState.player.position.x + distance * cos(yaw) * cos(pitch);
            targetPosition.y = gameState.player.position.y + distance * sin(pitch);
            targetPosition.z = gameState.player.position.z + distance * sin(yaw) * cos(pitch);
            
            // カメラは常にプレイヤーを見る
            targetTarget = gameState.player.position;
        } else if (gameState.camera.isFirstPersonView) {
            // 1人称視点：プレイヤーの目の位置
            targetPosition = gameState.player.position + glm::vec3(0, 2.0f, 0); // 目の高さ
            
            // マウス入力でカメラの向きを制御
            float yaw = glm::radians(gameState.camera.yaw);
            float pitch = glm::radians(gameState.camera.pitch);
            
            // カメラの向きを計算
            glm::vec3 direction;
            direction.x = cos(yaw) * cos(pitch);
            direction.y = sin(pitch);
            direction.z = sin(yaw) * cos(pitch);
            
            targetTarget = targetPosition + direction;
        } else {
            // 3人称視点
            if (stageManager.getCurrentStage() == 0) {
                // ステージ選択フィールドでは上からのアングル
                targetPosition = gameState.player.position + GameConstants::STAGE_SELECT_CAMERA_OFFSET;
            } else {
                // 通常のステージでは従来のアングル
                targetPosition = gameState.player.position + GameConstants::NORMAL_STAGE_CAMERA_OFFSET;
            }
            targetTarget = gameState.player.position;
        }
        
        // カメラスムージング（画面揺れ防止）
        // 落下時の急激な位置変化を滑らかにする
        if (!isInitialized) {
            // 初回は目標位置で初期化
            smoothCameraPosition = targetPosition;
            smoothCameraTarget = targetTarget;
            isInitialized = true;
        }
        
        const float SMOOTH_FACTOR = 20.0f; // スムージング係数（大きいほど速く追従）
        float lerpFactor = 1.0f - exp(-SMOOTH_FACTOR * deltaTime);
        
        smoothCameraPosition = glm::mix(smoothCameraPosition, targetPosition, lerpFactor);
        smoothCameraTarget = glm::mix(smoothCameraTarget, targetTarget, lerpFactor);
        
        config.position = smoothCameraPosition;
        config.target = smoothCameraTarget;
        
        return config;
    }
    
    /**
     * @brief カメラ設定を適用する
     * @details 計算されたカメラ設定をレンダラーに適用します。
     * 
     * @param renderer OpenGLレンダラー
     * @param config カメラ設定
     * @param window GLFWウィンドウ
     */
    static void applyCameraConfig(gfx::OpenGLRenderer* renderer, 
                                const CameraConfig& config, 
                                GLFWwindow* window) {
        renderer->setCamera(config.position, config.target);
        
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        renderer->setProjection(config.fov, (float)width / (float)height, 
                               config.nearPlane, config.farPlane);
    }
    
    /**
     * @brief ウィンドウサイズを取得する
     * @details GLFWウィンドウのフレームバッファサイズを取得します。
     * 
     * @param window GLFWウィンドウ
     * @return ウィンドウの幅と高さのペア
     */
    static std::pair<int, int> getWindowSize(GLFWwindow* window) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return {width, height};
    }
    
    /**
     * @brief カメラスムージングをリセットする
     * @details ステージ遷移時など、カメラ位置を即座に更新したい場合に使用します。
     */
    static void resetCameraSmoothing() {
        isInitialized = false;
    }
};
