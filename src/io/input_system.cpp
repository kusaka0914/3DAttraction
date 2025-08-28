#include "input_system.h"
#include "../physics/physics_system.h"
#include <algorithm>

// ゲームパッド関連の静的変数
static bool gamepadConnected = false;
static int gamepadId = GLFW_JOYSTICK_1;
static bool gamepadButtons[15] = {false}; // ボタンの状態を保存
static bool gamepadButtonsLast[15] = {false}; // 前フレームのボタン状態

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
    
    // ゴール後の移動制限チェック
    if (gameState.isGoalReached) {
        return; // ゴール後は移動入力を無視
    }
    
    // ゲームオーバー時の移動制限チェック
    if (gameState.isGameOver) {
        return; // ゲームオーバー時は移動入力を無視
    }
    
    // キーボード入力
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir.z += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir.z -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir.x += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir.x -= 1.0f;
    
    // ゲームパッド入力（左スティック）
    if (isGamepadConnected()) {
        glm::vec2 gamepadStick = getGamepadLeftStick();
        moveDir.x += gamepadStick.x;
        moveDir.z -= gamepadStick.y; // Y軸を反転（ゲームパッドの上が前進）
    }

    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);
        
        // 移動量を計算
        float moveDistance = moveSpeed * deltaTime;
        glm::vec3 newPosition = gameState.playerPosition;
        newPosition.x += moveDir.x * moveDistance;
        newPosition.z += moveDir.z * moveDistance;
        
        // 水平移動での足場衝突をチェック（足場の上にいる時は移動を許可）
        glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
        if (!PhysicsSystem::checkPlatformCollisionHorizontal(gameState, newPosition, playerSize)) {
            gameState.playerPosition = newPosition;
        }
    }
}

// ジャンプの処理
void InputSystem::processJumpAndFloat(GLFWwindow* window, GameState& gameState, float deltaTime, const glm::vec3& gravityDirection, PlatformSystem& platformSystem) {
    // ゴール後の移動制限チェック
    if (gameState.isGoalReached) {
        return; // ゴール後はジャンプ入力を無視
    }
    
    // ゲームオーバー時の移動制限チェック
    if (gameState.isGameOver) {
        return; // ゲームオーバー時はジャンプ入力を無視
    }
    
    // シンプルなジャンプ処理
    static bool spacePressed = false;
    static bool gamepadJumpPressed = false;
    
    bool spaceCurrentlyPressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    bool gamepadJumpCurrentlyPressed = isGamepadButtonPressed(0); // Aボタン（通常は0番）
    
    bool shouldJump = (spaceCurrentlyPressed && !spacePressed) || 
                     (gamepadJumpCurrentlyPressed && !gamepadJumpPressed);

    if (shouldJump) {
        // 重力方向を考慮した足場判定
        glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
        bool onPlatform = false;
        
        // 新しいPlatformSystemから足場データを取得
        const auto& platforms = platformSystem.getPlatforms();
        
        // 重力方向に応じて足場判定を行う
        for (const auto& platform : platforms) {
            std::visit([&](const auto& p) {
                if (p.size.x <= 0 || p.size.y <= 0 || p.size.z <= 0) return;
                
                // 重力方向に応じた判定
                if (gravityDirection.y > 0.5f) {
                    // 重力反転時：足場の下面に衝突判定
                    glm::vec3 platformMin = p.position - p.size * 0.5f;
                    glm::vec3 platformMax = p.position + p.size * 0.5f;
                    glm::vec3 playerMin = gameState.playerPosition - playerSize * 0.5f;
                    glm::vec3 playerMax = gameState.playerPosition + playerSize * 0.5f;
                    
                    bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                             playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
                    
                    if (horizontalOverlap && std::abs(playerMax.y - platformMin.y) < 0.5f) {
                        onPlatform = true;
                    }
                } else {
                    // 通常の重力：足場の上面に衝突判定
                    glm::vec3 platformMin = p.position - p.size * 0.5f;
                    glm::vec3 platformMax = p.position + p.size * 0.5f;
                    glm::vec3 playerMin = gameState.playerPosition - playerSize * 0.5f;
                    glm::vec3 playerMax = gameState.playerPosition + playerSize * 0.5f;
                    
                    bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                             playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
                    
                    if (horizontalOverlap && std::abs(playerMin.y - platformMax.y) < 0.2f) {
                        onPlatform = true;
                    }
                }
            }, platform);
            
            if (onPlatform) break;
        }
        
        if (onPlatform) {
            // 重力方向に応じたジャンプ
            if (gravityDirection.y > 0.5f) {
                gameState.playerVelocity.y = -8.0f; // 重力反転時は下向きにジャンプ
            } else {
                gameState.playerVelocity.y = 8.0f; // 通常時は上向きにジャンプ
            }
        }
    }
    spacePressed = spaceCurrentlyPressed;
    gamepadJumpPressed = gamepadJumpCurrentlyPressed;
}

// ゲームパッド初期化
void InputSystem::initializeGamepad() {
    if (glfwJoystickPresent(gamepadId)) {
        gamepadConnected = true;
    } else {
        gamepadConnected = false;
    }
}

// ゲームパッド接続状態をチェック
bool InputSystem::isGamepadConnected() {
    gamepadConnected = glfwJoystickPresent(gamepadId);
    return gamepadConnected;
}

// 左スティックの入力を取得
glm::vec2 InputSystem::getGamepadLeftStick() {
    if (!gamepadConnected) return glm::vec2(0.0f);
    
    int axesCount;
    const float* axes = glfwGetJoystickAxes(gamepadId, &axesCount);
    if (axesCount >= 2) {
        // デッドゾーンを設定（小さな入力を無視）
        float deadzone = 0.1f;
        float x = std::abs(axes[0]) > deadzone ? axes[0] : 0.0f;
        float y = std::abs(axes[1]) > deadzone ? axes[1] : 0.0f;
        return glm::vec2(x, y);
    }
    return glm::vec2(0.0f);
}

// ゲームパッドボタンの状態を取得
bool InputSystem::isGamepadButtonPressed(int button) {
    if (!gamepadConnected) return false;
    
    int buttonCount;
    const unsigned char* buttons = glfwGetJoystickButtons(gamepadId, &buttonCount);
    if (button < buttonCount) {
        return buttons[button] == GLFW_PRESS;
    }
    return false;
}

// ゲームパッドボタンが今フレーム押されたかをチェック
bool InputSystem::isGamepadButtonJustPressed(int button) {
    if (!gamepadConnected) return false;
    
    bool currentState = isGamepadButtonPressed(button);
    bool justPressed = currentState && !gamepadButtonsLast[button];
    gamepadButtonsLast[button] = currentState;
    return justPressed;
}

