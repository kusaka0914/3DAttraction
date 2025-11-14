#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "input_system.h"
#include "../physics/physics_system.h"
#include "../core/constants/game_constants.h"
#include "audio_manager.h"
#include "../game/stage_editor.h"
#include <variant>
#include <algorithm>

static bool gamepadConnected = false;
static int gamepadId = GameConstants::InputConstants::DEFAULT_GAMEPAD_ID;
static bool gamepadButtons[GameConstants::InputConstants::MAX_GAMEPAD_BUTTONS] = {false}; // ボタンの状態を保存
static bool gamepadButtonsLast[GameConstants::InputConstants::MAX_GAMEPAD_BUTTONS] = {false}; // 前フレームのボタン状態

void InputSystem::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    GameState* gameState = static_cast<GameState*>(glfwGetWindowUserPointer(window));
    
    if (gameState->editorState && gameState->editorState->isActive) {
        return;
    }
    
    if (gameState->firstMouse) {
        gameState->lastMouseX = xpos;
        gameState->lastMouseY = ypos;
        gameState->firstMouse = false;
    }
    float xoffset = float(xpos - gameState->lastMouseX);
    float yoffset = float(ypos - gameState->lastMouseY);
    gameState->lastMouseX = float(xpos);
    gameState->lastMouseY = float(ypos);
    
    xoffset *= GameConstants::InputConstants::MOUSE_SENSITIVITY;
    yoffset *= GameConstants::InputConstants::MOUSE_SENSITIVITY;
    
    if (gameState->isFreeCameraActive) {
        gameState->freeCameraYaw += xoffset;
        gameState->freeCameraPitch -= yoffset;
        gameState->freeCameraPitch = std::max(GameConstants::InputConstants::MIN_CAMERA_PITCH, 
                                             std::min(GameConstants::InputConstants::MAX_CAMERA_PITCH, gameState->freeCameraPitch));
    } else if (gameState->isFirstPersonView) {
        gameState->cameraYaw += xoffset;
        gameState->cameraPitch -= yoffset;
        gameState->cameraPitch = std::max(GameConstants::InputConstants::MIN_CAMERA_PITCH, 
                                        std::min(GameConstants::InputConstants::MAX_CAMERA_PITCH, gameState->cameraPitch));
    }
}

void InputSystem::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    GameState* gameState = static_cast<GameState*>(glfwGetWindowUserPointer(window));
    
    if (gameState->editorState && gameState->editorState->isActive) {
        float sensitivity = 2.0f;
        gameState->freeCameraYaw += float(xoffset) * sensitivity;
        gameState->freeCameraPitch -= float(yoffset) * sensitivity;
        gameState->freeCameraPitch = std::max(GameConstants::InputConstants::MIN_CAMERA_PITCH, 
                                             std::min(GameConstants::InputConstants::MAX_CAMERA_PITCH, gameState->freeCameraPitch));
        return;
    }
    
    gameState->cameraDistance -= float(yoffset);
    gameState->cameraDistance = std::max(GameConstants::InputConstants::MIN_CAMERA_DISTANCE, 
                                       std::min(GameConstants::InputConstants::MAX_CAMERA_DISTANCE, gameState->cameraDistance));
}

void InputSystem::processInput(GLFWwindow* window, GameState& gameState, float deltaTime) {
    float moveSpeed = GameConstants::InputConstants::MOVE_SPEED;
    
    glm::vec3 gravityDirection = glm::vec3(0, -1, 0); // デフォルトは下向き
    PhysicsSystem::isPlayerInGravityZone(gameState, gameState.player.position, gravityDirection);
    
    glm::vec3 moveDir(0.0f);
    
    gameState.player.isMovingBackward = false;
    
    if (gameState.progress.isGoalReached) {
        return; // ゴール後は移動入力を無視
    }
    
    if (gameState.progress.isGameOver) {
        return; // ゲームオーバー時は移動入力を無視
    }
    
    if (gameState.skills.isFreeCameraActive) {
        float yaw = glm::radians(gameState.skills.freeCameraYaw);
        float pitch = glm::radians(gameState.skills.freeCameraPitch);
        float cosYaw = cos(yaw);
        float sinYaw = sin(yaw);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            moveDir.x -= cosYaw * cos(pitch);
            moveDir.z -= sinYaw * cos(pitch);
            gameState.player.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.x += cosYaw * cos(pitch);
            moveDir.z += sinYaw * cos(pitch);
            gameState.player.isMovingBackward = true;
            gameState.player.isShowingFrontTexture = true;  // Sキーを押したらフロントテクスチャ表示を開始
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            moveDir.x -= sinYaw;
            moveDir.z += cosYaw;
            gameState.player.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            moveDir.x += sinYaw;
            moveDir.z -= cosYaw;
            gameState.player.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
    } else if (gameState.camera.isFirstPersonView) {
        float yaw = glm::radians(gameState.camera.yaw);
        float pitch = glm::radians(gameState.camera.pitch);
        float cosYaw = cos(yaw);
        float sinYaw = sin(yaw);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            moveDir.x += cosYaw * cos(pitch);
            moveDir.z += sinYaw * cos(pitch);
            gameState.player.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.x -= cosYaw * cos(pitch);
            moveDir.z -= sinYaw * cos(pitch);
            gameState.player.isMovingBackward = true;
            gameState.player.isShowingFrontTexture = true;  // Sキーを押したらフロントテクスチャ表示を開始
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            moveDir.x += sinYaw;
            moveDir.z -= cosYaw;
            gameState.player.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            moveDir.x -= sinYaw;
            moveDir.z += cosYaw;
            gameState.player.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
    } else {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            moveDir.z += 1.0f;
            gameState.player.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.z -= 1.0f;
            gameState.player.isMovingBackward = true;
            gameState.player.isShowingFrontTexture = true;  // Sキーを押したらフロントテクスチャ表示を開始
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            moveDir.x += 1.0f;
            gameState.player.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            moveDir.x -= 1.0f;
            gameState.player.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
    }
    
    if (isGamepadConnected()) {
        glm::vec2 gamepadStick = getGamepadLeftStick();
        moveDir.x += gamepadStick.x;
        moveDir.z -= gamepadStick.y; // Y軸を反転（ゲームパッドの上が前進）
        
        if (gamepadStick.y > 0.1f) { // 下方向の入力
            gameState.player.isMovingBackward = true;
        }
    }

    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);
        
        float moveDistance = moveSpeed * deltaTime;
        
        if (gameState.skills.isInBurstJumpAir) {
            moveDistance *= 2.0f;

        }
        
        glm::vec3 newPosition = gameState.player.position;
        newPosition.x += moveDir.x * moveDistance;
        newPosition.z += moveDir.z * moveDistance;
        
        glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
        if (!PhysicsSystem::checkPlatformCollisionHorizontal(gameState, newPosition, playerSize)) {
            gameState.player.position = newPosition;
        }
    }
}

void InputSystem::processJumpAndFloat(GLFWwindow* window, GameState& gameState, float deltaTime, const glm::vec3& gravityDirection, PlatformSystem& platformSystem, io::AudioManager& audioManager) {
    if (gameState.progress.isGoalReached) {
        return; // ゴール後はジャンプ入力を無視
    }
    
    if (gameState.progress.isGameOver) {
        return; // ゲームオーバー時はジャンプ入力を無視
    }
    
    static bool spacePressed = false;
    static bool gamepadJumpPressed = false;
    
    bool spaceCurrentlyPressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    bool gamepadJumpCurrentlyPressed = isGamepadButtonPressed(0); // Aボタン（通常は0番）
    
    bool shouldJump = (spaceCurrentlyPressed && !spacePressed) || 
                     (gamepadJumpCurrentlyPressed && !gamepadJumpPressed);

    if (shouldJump) {
        glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
        bool onPlatform = false;
        
        const auto& platforms = platformSystem.getPlatforms();
        
        for (const auto& platform : platforms) {
            std::visit([&](const auto& p) {
                if (p.size.x <= 0 || p.size.y <= 0 || p.size.z <= 0) return;
                
                if (gravityDirection.y > 0.5f) {
                    glm::vec3 platformMin = p.position - p.size * 0.5f;
                    glm::vec3 platformMax = p.position + p.size * 0.5f;
                    glm::vec3 playerMin = gameState.player.position - playerSize * 0.5f;
                    glm::vec3 playerMax = gameState.player.position + playerSize * 0.5f;
                    
                    bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                             playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
                    
                    if (horizontalOverlap && std::abs(playerMax.y - platformMin.y) < 0.5f) {
                        onPlatform = true;
                    }
                } else {
                    glm::vec3 platformMin = p.position - p.size * 0.5f;
                    glm::vec3 platformMax = p.position + p.size * 0.5f;
                    glm::vec3 playerMin = gameState.player.position - playerSize * 0.5f;
                    glm::vec3 playerMax = gameState.player.position + playerSize * 0.5f;
                    
                    bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                             playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
                    
                    if (horizontalOverlap && std::abs(playerMin.y - platformMax.y) < 0.5f) {
                        onPlatform = true;
                    }
                }
            }, platform);
            
            if (onPlatform) break;
        }
        
        if (onPlatform) {
            if (gameState.audioEnabled) {
                audioManager.playSFX("jump");
            }
            
            if (gameState.skills.isBurstJumpActive && !gameState.skills.hasUsedBurstJump) {
                if (gravityDirection.y > 0.5f) {
                    gameState.player.velocity.y = -20.0f; // 重力反転時は下向きにバーストジャンプ
                } else {
                    gameState.player.velocity.y = 20.0f; // 通常時は上向きにバーストジャンプ
                }
                gameState.skills.hasUsedBurstJump = true;
                gameState.skills.isBurstJumpActive = false; // バーストジャンプを使用したので非アクティブに
                gameState.skills.burstJumpDelayTimer = 0.01f; // 1秒後に空中フラグを設定
            } else {
                if (gravityDirection.y > 0.5f) {
                    gameState.player.velocity.y = -8.0f; // 重力反転時は下向きにジャンプ
                } else {
                    gameState.player.velocity.y = 8.0f; // 通常時は上向きにジャンプ
                }
            }
            gameState.player.canDoubleJump = true;
            if (gameState.skills.isInBurstJumpAir) {
                gameState.skills.isInBurstJumpAir = false; // バーストジャンプ空中フラグをリセット
            }
        } else if ((gameState.progress.isEasyMode && gameState.player.canDoubleJump) || 
                   (!gameState.progress.isEasyMode && gameState.skills.hasDoubleJumpSkill && gameState.skills.doubleJumpRemainingUses > 0 && gameState.player.canDoubleJump)) {
            if (gameState.progress.currentStage != 0) {
                if (gameState.audioEnabled) {
                    audioManager.playSFX("jump");
                }
                
                if (gravityDirection.y > 0.5f) {
                    gameState.player.velocity.y = -6.0f; // 重力反転時は下向きにジャンプ
                } else {
                    gameState.player.velocity.y = 6.0f; // 通常時は上向きにジャンプ
                }
                gameState.player.canDoubleJump = false; // 二段ジャンプを使用
                
                if (!gameState.progress.isEasyMode) {
                    gameState.skills.doubleJumpRemainingUses--;
                }
            }
        } else if (gameState.skills.isBurstJumpActive && !gameState.skills.hasUsedBurstJump && !gameState.skills.isInBurstJumpAir) {
            if (gameState.audioEnabled) {
                audioManager.playSFX("jump");
            }
            
            if (gravityDirection.y > 0.5f) {
                gameState.player.velocity.y = -20.0f; // 重力反転時は下向きにバーストジャンプ
            } else {
                gameState.player.velocity.y = 20.0f; // 通常時は上向きにバーストジャンプ
            }
            gameState.skills.hasUsedBurstJump = true;
            gameState.skills.isBurstJumpActive = false; // バーストジャンプを使用したので非アクティブに
            gameState.skills.isInBurstJumpAir = true; // バーストジャンプ空中フラグを設定
        }
    }
    spacePressed = spaceCurrentlyPressed;
    gamepadJumpPressed = gamepadJumpCurrentlyPressed;
}

void InputSystem::initializeGamepad() {
    if (glfwJoystickPresent(gamepadId)) {
        gamepadConnected = true;
    } else {
        gamepadConnected = false;
    }
}

bool InputSystem::isGamepadConnected() {
    gamepadConnected = glfwJoystickPresent(gamepadId);
    return gamepadConnected;
}

glm::vec2 InputSystem::getGamepadLeftStick() {
    if (!gamepadConnected) return glm::vec2(0.0f);
    
    int axesCount;
    const float* axes = glfwGetJoystickAxes(gamepadId, &axesCount);
    if (axesCount >= 2) {
        float deadzone = 0.1f;
        float x = std::abs(axes[0]) > deadzone ? axes[0] : 0.0f;
        float y = std::abs(axes[1]) > deadzone ? axes[1] : 0.0f;
        return glm::vec2(x, y);
    }
    return glm::vec2(0.0f);
}

bool InputSystem::isGamepadButtonPressed(int button) {
    if (!gamepadConnected) return false;
    
    int buttonCount;
    const unsigned char* buttons = glfwGetJoystickButtons(gamepadId, &buttonCount);
    if (button < buttonCount) {
        return buttons[button] == GLFW_PRESS;
    }
    return false;
}

bool InputSystem::isGamepadButtonJustPressed(int button) {
    if (!gamepadConnected) return false;
    
    bool currentState = isGamepadButtonPressed(button);
    bool justPressed = currentState && !gamepadButtonsLast[button];
    gamepadButtonsLast[button] = currentState;
    return justPressed;
}

