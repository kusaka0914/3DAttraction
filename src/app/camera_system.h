#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "../gfx/opengl_renderer.h"
#include "../game/game_state.h"
#include "../game/stage_manager.h"
#include "game_constants.h"

struct CameraConfig {
    glm::vec3 position;
    glm::vec3 target;
    float fov;
    float nearPlane;
    float farPlane;
};

class CameraSystem {
public:
    static CameraConfig calculateCameraConfig(const GameState& gameState, 
                                            const StageManager& stageManager, 
                                            GLFWwindow* window) {
        CameraConfig config;
        config.fov = GameConstants::CAMERA_FOV;
        config.nearPlane = GameConstants::CAMERA_NEAR;
        config.farPlane = GameConstants::CAMERA_FAR;
        
        if (gameState.isFreeCameraActive) {
            // フリーカメラ：プレイヤーを中心にカメラが回転
            float yaw = glm::radians(gameState.freeCameraYaw);
            float pitch = glm::radians(gameState.freeCameraPitch);
            
            // カメラの位置を計算（プレイヤーからの相対位置）
            float distance = (stageManager.getCurrentStage() == 0) ? 15.0f : 8.0f;
            
            // プレイヤーを中心とした球面座標でカメラ位置を計算
            config.position.x = gameState.playerPosition.x + distance * cos(yaw) * cos(pitch);
            config.position.y = gameState.playerPosition.y + distance * sin(pitch);
            config.position.z = gameState.playerPosition.z + distance * sin(yaw) * cos(pitch);
            
            // カメラは常にプレイヤーを見る
            config.target = gameState.playerPosition;
        } else if (gameState.isFirstPersonView) {
            // 1人称視点：プレイヤーの目の位置
            config.position = gameState.playerPosition + glm::vec3(0, 2.0f, 0); // 目の高さ
            
            // マウス入力でカメラの向きを制御
            float yaw = glm::radians(gameState.cameraYaw);
            float pitch = glm::radians(gameState.cameraPitch);
            
            // カメラの向きを計算
            glm::vec3 direction;
            direction.x = cos(yaw) * cos(pitch);
            direction.y = sin(pitch);
            direction.z = sin(yaw) * cos(pitch);
            
            config.target = config.position + direction;
        } else {
            // 3人称視点
            if (stageManager.getCurrentStage() == 0) {
                // ステージ選択フィールドでは上からのアングル
                config.position = gameState.playerPosition + GameConstants::STAGE_SELECT_CAMERA_OFFSET;
            } else {
                // 通常のステージでは従来のアングル
                config.position = gameState.playerPosition + GameConstants::NORMAL_STAGE_CAMERA_OFFSET;
            }
            config.target = gameState.playerPosition;
        }
        
        return config;
    }
    
    static void applyCameraConfig(gfx::OpenGLRenderer* renderer, 
                                const CameraConfig& config, 
                                GLFWwindow* window) {
        renderer->setCamera(config.position, config.target);
        
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        renderer->setProjection(config.fov, (float)width / (float)height, 
                               config.nearPlane, config.farPlane);
    }
    
    static std::pair<int, int> getWindowSize(GLFWwindow* window) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return {width, height};
    }
};
