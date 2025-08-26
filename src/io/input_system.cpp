#include "input_system.h"
#include "../physics/physics_system.h"
#include <algorithm>

// マウスコールバック
void InputSystem::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    GameState* gameState = static_cast<GameState*>(glfwGetWindowUserPointer(window));
    if (gameState->firstMouse) {
        gameState->lastMouseX = xpos;
        gameState->lastMouseY = ypos;
        gameState->firstMouse = false;
    }
    float xoffset = float(xpos - gameState->lastMouseX);
    float yoffset = float(gameState->lastMouseY - ypos);
    gameState->lastMouseX = float(xpos);
    gameState->lastMouseY = float(ypos);
    
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    gameState->cameraYaw += xoffset;
    gameState->cameraPitch += yoffset;
    gameState->cameraPitch = std::max(-89.0f, std::min(89.0f, gameState->cameraPitch));
}

// スクロールコールバック
void InputSystem::scroll_callback(GLFWwindow* window, double, double yoffset) {
    GameState* gameState = static_cast<GameState*>(glfwGetWindowUserPointer(window));
    gameState->cameraDistance -= float(yoffset);
    gameState->cameraDistance = std::max(1.0f, std::min(50.0f, gameState->cameraDistance));
}

// 入力処理
void InputSystem::processInput(GLFWwindow* window, GameState& gameState, float deltaTime) {
    float moveSpeed = 6.0f;
    
    // 重力反転エリアのチェック
    glm::vec3 gravityDirection = glm::vec3(0, -1, 0); // デフォルトは下向き
    PhysicsSystem::isPlayerInGravityZone(gameState, gameState.playerPosition, gravityDirection);
    
    // 移動入力
    glm::vec3 moveDir(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir.z += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir.z -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir.x += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir.x -= 1.0f;

    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);
        
        // 移動量を計算
        float moveDistance = moveSpeed * deltaTime;
        glm::vec3 newPosition = gameState.playerPosition;
        newPosition.x += moveDir.x * moveDistance;
        newPosition.z += moveDir.z * moveDistance;
        
        // 重力反転時は水平移動の制限を緩和
        if (gravityDirection.y > 0.5f) {
            // 重力反転時：水平移動を自由に許可
            gameState.playerPosition = newPosition;
        } else {
            // 通常時：水平移動での足場衝突をチェック
            glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
            if (!PhysicsSystem::checkPlatformCollisionHorizontal(gameState, newPosition, playerSize)) {
                gameState.playerPosition = newPosition;
            }
        }
    }
}

// ジャンプと浮遊の処理
void InputSystem::processJumpAndFloat(GLFWwindow* window, GameState& gameState, float deltaTime, const glm::vec3& gravityDirection) {
    // ジャンプと浮遊
    static bool spacePressed = false;
    bool spaceCurrentlyPressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    if (spaceCurrentlyPressed && !spacePressed) {
        // 重力方向を考慮した足場判定
        glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
        bool onPlatform = false;
        
        // 重力方向に応じて足場判定を行う
        for (const auto& platform : gameState.platforms) {
            if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
            if (PhysicsSystem::isPlayerOnPlatformWithGravityForMovement(platform, gameState.playerPosition, playerSize, gravityDirection)) {
                onPlatform = true;
                break;
            }
        }
        
        if (onPlatform) {
            // 重力方向に応じたジャンプ
            if (gravityDirection.y > 0.5f) {
                gameState.playerVelocity.y = -8.0f; // 重力反転時は下向きにジャンプ
            } else {
                gameState.playerVelocity.y = 8.0f; // 通常時は上向きにジャンプ
            }
            gameState.isFloating = false;
        } else if (!gameState.isFloating && gameState.floatCount < 2) { // 最大2回まで
            gameState.isFloating = true;
            gameState.floatTimer = 0.0f;
            if (gravityDirection.y > 0.5f) {
                gameState.playerVelocity.y = -2.0f; // 重力反転時は下向きに浮遊
            } else {
                gameState.playerVelocity.y = 2.0f; // 通常時は上向きに浮遊
            }
            gameState.floatCount++; // 浮遊回数を増加
        }
    }
    spacePressed = spaceCurrentlyPressed;

    if (gameState.isFloating) {
        gameState.floatTimer += deltaTime;
        if (gameState.floatTimer < 5.0f) {
            if (gravityDirection.y > 0.5f) {
                // 重力反転時：下向きに浮遊
                gameState.playerVelocity.y = std::min(gameState.playerVelocity.y + 1.0f * deltaTime, 1.0f);
                if (spaceCurrentlyPressed) {
                    gameState.playerVelocity.y = std::max(gameState.playerVelocity.y - 3.0f * deltaTime, -2.0f);
                }
            } else {
                // 通常時：上向きに浮遊
                gameState.playerVelocity.y = std::max(gameState.playerVelocity.y - 1.0f * deltaTime, -1.0f);
                if (spaceCurrentlyPressed) {
                    gameState.playerVelocity.y = std::min(gameState.playerVelocity.y + 3.0f * deltaTime, 2.0f);
                }
            }
        } else {
            gameState.isFloating = false;
        }
    }
}

