#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "input_system.h"
#include "../physics/physics_system.h"
#include "../core/constants/game_constants.h"
#include "audio_manager.h"
#include <variant>
#include <algorithm>

// ゲームパッド関連の静的変数
static bool gamepadConnected = false;
static int gamepadId = GameConstants::InputConstants::DEFAULT_GAMEPAD_ID;
static bool gamepadButtons[GameConstants::InputConstants::MAX_GAMEPAD_BUTTONS] = {false}; // ボタンの状態を保存
static bool gamepadButtonsLast[GameConstants::InputConstants::MAX_GAMEPAD_BUTTONS] = {false}; // 前フレームのボタン状態

// マウスコールバック
void InputSystem::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    GameState* gameState = static_cast<GameState*>(glfwGetWindowUserPointer(window));
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
        // フリーカメラ中のマウス入力
        gameState->freeCameraYaw += xoffset;
        gameState->freeCameraPitch -= yoffset;
        gameState->freeCameraPitch = std::max(GameConstants::InputConstants::MIN_CAMERA_PITCH, 
                                             std::min(GameConstants::InputConstants::MAX_CAMERA_PITCH, gameState->freeCameraPitch));
    } else if (gameState->isFirstPersonView) {
        // 1人称視点中のマウス入力
        gameState->cameraYaw += xoffset;
        gameState->cameraPitch -= yoffset;
        gameState->cameraPitch = std::max(GameConstants::InputConstants::MIN_CAMERA_PITCH, 
                                        std::min(GameConstants::InputConstants::MAX_CAMERA_PITCH, gameState->cameraPitch));
    }
}

// スクロールコールバック
void InputSystem::scroll_callback(GLFWwindow* window, double, double yoffset) {
    GameState* gameState = static_cast<GameState*>(glfwGetWindowUserPointer(window));
    gameState->cameraDistance -= float(yoffset);
    gameState->cameraDistance = std::max(GameConstants::InputConstants::MIN_CAMERA_DISTANCE, 
                                       std::min(GameConstants::InputConstants::MAX_CAMERA_DISTANCE, gameState->cameraDistance));
}

// 入力処理
void InputSystem::processInput(GLFWwindow* window, GameState& gameState, float deltaTime) {
    float moveSpeed = GameConstants::InputConstants::MOVE_SPEED;
    
    // 重力反転エリアのチェック
    glm::vec3 gravityDirection = glm::vec3(0, -1, 0); // デフォルトは下向き
    PhysicsSystem::isPlayerInGravityZone(gameState, gameState.playerPosition, gravityDirection);
    
    // 移動入力
    glm::vec3 moveDir(0.0f);
    
    // 後退フラグをリセット
    gameState.isMovingBackward = false;
    
    // ゴール後の移動制限チェック
    if (gameState.isGoalReached) {
        return; // ゴール後は移動入力を無視
    }
    
    // ゲームオーバー時の移動制限チェック
    if (gameState.isGameOver) {
        return; // ゲームオーバー時は移動入力を無視
    }
    
    // キーボード入力
    if (gameState.isFreeCameraActive) {
        // フリーカメラ：1人称視点と同じ移動（進行方向を逆に）
        float yaw = glm::radians(gameState.freeCameraYaw);
        float pitch = glm::radians(gameState.freeCameraPitch);
        float cosYaw = cos(yaw);
        float sinYaw = sin(yaw);
        
        // Wキー：常に前進（カメラの向いている方向）
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            // カメラの向きベクトルと同じ方向に移動（Y成分は無視）
            moveDir.x -= cosYaw * cos(pitch);
            moveDir.z -= sinYaw * cos(pitch);
            gameState.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        // Sキー：後退（カメラの向いている方向の逆）
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.x += cosYaw * cos(pitch);
            moveDir.z += sinYaw * cos(pitch);
            gameState.isMovingBackward = true;
            gameState.isShowingFrontTexture = true;  // Sキーを押したらフロントテクスチャ表示を開始
        }
        // Aキー：左移動（カメラの向いている方向に対して左）
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            // カメラの向きベクトルを90度左に回転
            moveDir.x -= sinYaw;
            moveDir.z += cosYaw;
            gameState.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        // Dキー：右移動（カメラの向いている方向に対して右）
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            // カメラの向きベクトルを90度右に回転
            moveDir.x += sinYaw;
            moveDir.z -= cosYaw;
            gameState.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
    } else if (gameState.isFirstPersonView) {
        // 1人称視点：カメラの向いている方向に応じて移動
        float yaw = glm::radians(gameState.cameraYaw);
        float pitch = glm::radians(gameState.cameraPitch);
        float cosYaw = cos(yaw);
        float sinYaw = sin(yaw);
        
        // Wキー：常に前進（カメラの向いている方向）
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            // カメラの向きベクトルと同じ方向に移動（Y成分は無視）
            moveDir.x += cosYaw * cos(pitch);
            moveDir.z += sinYaw * cos(pitch);
            gameState.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        // Sキー：後退（カメラの向いている方向の逆）
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.x -= cosYaw * cos(pitch);
            moveDir.z -= sinYaw * cos(pitch);
            gameState.isMovingBackward = true;
            gameState.isShowingFrontTexture = true;  // Sキーを押したらフロントテクスチャ表示を開始
        }
        // Aキー：左移動（カメラの向いている方向に対して左）
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            // カメラの向きベクトルを90度左に回転
            moveDir.x += sinYaw;
            moveDir.z -= cosYaw;
            gameState.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        // Dキー：右移動（カメラの向いている方向に対して右）
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            // カメラの向きベクトルを90度右に回転
            moveDir.x -= sinYaw;
            moveDir.z += cosYaw;
            gameState.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
    } else {
        // 3人称視点：従来の固定方向移動
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            moveDir.z += 1.0f;
            gameState.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.z -= 1.0f;
            gameState.isMovingBackward = true;
            gameState.isShowingFrontTexture = true;  // Sキーを押したらフロントテクスチャ表示を開始
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            moveDir.x += 1.0f;
            gameState.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            moveDir.x -= 1.0f;
            gameState.isShowingFrontTexture = false;  // 他の移動キーを押したらフロントテクスチャ表示を停止
        }
    }
    
    // ゲームパッド入力（左スティック）
    if (isGamepadConnected()) {
        glm::vec2 gamepadStick = getGamepadLeftStick();
        moveDir.x += gamepadStick.x;
        moveDir.z -= gamepadStick.y; // Y軸を反転（ゲームパッドの上が前進）
        
        // ゲームパッドで後退している場合
        if (gamepadStick.y > 0.1f) { // 下方向の入力
            gameState.isMovingBackward = true;
        }
    }

    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);
        
        // 移動量を計算
        float moveDistance = moveSpeed * deltaTime;
        
        // バーストジャンプ空中中は移動速度を2倍にする
        if (gameState.isInBurstJumpAir) {
            moveDistance *= 2.0f;

        }
        
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
void InputSystem::processJumpAndFloat(GLFWwindow* window, GameState& gameState, float deltaTime, const glm::vec3& gravityDirection, PlatformSystem& platformSystem, io::AudioManager& audioManager) {
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
                    
                    if (horizontalOverlap && std::abs(playerMin.y - platformMax.y) < 0.5f) {
                        onPlatform = true;
                    }
                }
            }, platform);
            
            if (onPlatform) break;
        }
        
        if (onPlatform) {
            // ジャンプSEを再生
            if (gameState.audioEnabled) {
                audioManager.playSFX("jump");
            }
            
            // バーストジャンプがアクティブで未使用の場合
            if (gameState.isBurstJumpActive && !gameState.hasUsedBurstJump) {
                // バーストジャンプ：めちゃくちゃ高いジャンプ力
                if (gravityDirection.y > 0.5f) {
                    gameState.playerVelocity.y = -20.0f; // 重力反転時は下向きにバーストジャンプ
                } else {
                    gameState.playerVelocity.y = 20.0f; // 通常時は上向きにバーストジャンプ
                }
                gameState.hasUsedBurstJump = true;
                gameState.isBurstJumpActive = false; // バーストジャンプを使用したので非アクティブに
                gameState.burstJumpDelayTimer = 0.01f; // 1秒後に空中フラグを設定
            } else {
                // 通常のジャンプ
                if (gravityDirection.y > 0.5f) {
                    gameState.playerVelocity.y = -8.0f; // 重力反転時は下向きにジャンプ
                } else {
                    gameState.playerVelocity.y = 8.0f; // 通常時は上向きにジャンプ
                }
            }
            // 足場に着地したら二段ジャンプとバーストジャンプ空中フラグをリセット
            gameState.canDoubleJump = true;
            if (gameState.isInBurstJumpAir) {
                gameState.isInBurstJumpAir = false; // バーストジャンプ空中フラグをリセット
            }
        } else if ((gameState.isEasyMode && gameState.canDoubleJump) || 
                   (!gameState.isEasyMode && gameState.hasDoubleJumpSkill && gameState.doubleJumpRemainingUses > 0 && gameState.canDoubleJump)) {
            // 二段ジャンプ（お助けモードまたは通常モードでスキル取得済み）
            // ステージ選択フィールド（ステージ0）ではダブルジャンプを無効化
            if (gameState.currentStage != 0) {
                // 二段ジャンプSEを再生
                if (gameState.audioEnabled) {
                    audioManager.playSFX("jump");
                }
                
                if (gravityDirection.y > 0.5f) {
                    gameState.playerVelocity.y = -6.0f; // 重力反転時は下向きにジャンプ
                } else {
                    gameState.playerVelocity.y = 6.0f; // 通常時は上向きにジャンプ
                }
                gameState.canDoubleJump = false; // 二段ジャンプを使用
                
                // 通常モードの場合は使用回数を減らす
                if (!gameState.isEasyMode) {
                    gameState.doubleJumpRemainingUses--;
                }
            }
        } else if (gameState.isBurstJumpActive && !gameState.hasUsedBurstJump && !gameState.isInBurstJumpAir) {
            // バーストジャンプ：空中でジャンプボタンを押した場合（バーストジャンプ中は無効）
            // バーストジャンプSEを再生
            if (gameState.audioEnabled) {
                audioManager.playSFX("jump");
            }
            
            if (gravityDirection.y > 0.5f) {
                gameState.playerVelocity.y = -20.0f; // 重力反転時は下向きにバーストジャンプ
            } else {
                gameState.playerVelocity.y = 20.0f; // 通常時は上向きにバーストジャンプ
            }
            gameState.hasUsedBurstJump = true;
            gameState.isBurstJumpActive = false; // バーストジャンプを使用したので非アクティブに
            gameState.isInBurstJumpAir = true; // バーストジャンプ空中フラグを設定
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

