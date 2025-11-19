#include "multiplayer_manager.h"
#include "stage_manager.h"
#include "../io/input_system.h"
#include "../core/constants/game_constants.h"
#include "../core/utils/input_utils.h"
#include <iostream>

MultiplayerManager::MultiplayerManager() {
    networkManager_.setConnectionCallback([this](bool connected) {
        // 接続状態変更時のコールバック
        std::cout << "Multiplayer: Connection " << (connected ? "established" : "lost") 
                  << " (isHost: " << (networkManager_.isHost() ? "true" : "false") << ")" << std::endl;
    });
}

MultiplayerManager::~MultiplayerManager() {
    disconnect();
}

bool MultiplayerManager::startHost(int port) {
    return networkManager_.startHost(port);
}

bool MultiplayerManager::connectToHost(const std::string& ipAddress, int port) {
    return networkManager_.connectToHost(ipAddress, port);
}

void MultiplayerManager::disconnect() {
    networkManager_.disconnect();
}

void MultiplayerManager::update(GLFWwindow* window, GameState& gameState, PlatformSystem& platformSystem, float deltaTime, io::AudioManager& audioManager) {
    if (!networkManager_.isConnected()) {
        return;
    }
    
    // 入力の送信
    sendPlayerInput(window, gameState, deltaTime);
    
    // リモートプレイヤーの入力を受信して適用
    applyRemotePlayerInput(gameState, platformSystem, deltaTime, audioManager);
    
    // ホストの場合、ゲーム状態を送信
    if (networkManager_.isHost()) {
        sendGameState(gameState);
        // プラットフォーム状態も送信
        sendPlatformStates(platformSystem);
    } else {
        // クライアントの場合、プラットフォーム状態を受信して適用
        applyPlatformStates(platformSystem);
    }
    
    // リモートプレイヤーの状態を更新
    updateRemotePlayer(gameState, deltaTime);
    
    // ゴール到達通知をチェック
    checkGoalReached(gameState);
}

void MultiplayerManager::sendPlayerInput(GLFWwindow* window, const GameState& gameState, float deltaTime) {
    if (!networkManager_.isConnected()) {
        return;
    }
    
    // 入力方向を計算
    glm::vec3 moveDir(0.0f);
    glm::vec3 gravityDirection = glm::vec3(0, -1, 0);
    
    if (gameState.skills.isFreeCameraActive) {
        float yaw = glm::radians(gameState.skills.freeCameraYaw);
        float pitch = glm::radians(gameState.skills.freeCameraPitch);
        float cosYaw = cos(yaw);
        float sinYaw = sin(yaw);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            moveDir.x -= cosYaw * cos(pitch);
            moveDir.z -= sinYaw * cos(pitch);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.x += cosYaw * cos(pitch);
            moveDir.z += sinYaw * cos(pitch);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            moveDir.x -= sinYaw;
            moveDir.z += cosYaw;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            moveDir.x += sinYaw;
            moveDir.z -= cosYaw;
        }
    } else if (gameState.camera.isFirstPersonView) {
        float yaw = glm::radians(gameState.camera.yaw);
        float pitch = glm::radians(gameState.camera.pitch);
        float cosYaw = cos(yaw);
        float sinYaw = sin(yaw);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            moveDir.x -= cosYaw * cos(pitch);
            moveDir.z -= sinYaw * cos(pitch);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.x += cosYaw * cos(pitch);
            moveDir.z += sinYaw * cos(pitch);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            moveDir.x -= sinYaw;
            moveDir.z += cosYaw;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            moveDir.x += sinYaw;
            moveDir.z -= cosYaw;
        }
    } else {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            moveDir.x -= 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.x += 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            moveDir.z += 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            moveDir.z -= 1.0f;
        }
    }
    
    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);
    }
    
    // ジャンプ入力
    bool jumpPressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
    bool jumpJustPressed = false;
    static bool lastJumpState = false;
    if (jumpPressed && !lastJumpState) {
        jumpJustPressed = true;
    }
    lastJumpState = jumpPressed;
    
    PlayerInputData input;
    input.moveDirection = moveDir;
    input.jumpPressed = jumpPressed;
    input.jumpJustPressed = jumpJustPressed;
    input.deltaTime = deltaTime;
    
    networkManager_.sendPlayerInput(input);
}

void MultiplayerManager::applyRemotePlayerInput(GameState& gameState, PlatformSystem& platformSystem, float deltaTime, io::AudioManager& audioManager) {
    if (!networkManager_.isConnected() || !networkManager_.isHost()) {
        return;
    }
    
    PlayerInputData remoteInput;
    if (networkManager_.getReceivedPlayerInput(remoteInput)) {
        // リモートプレイヤーの入力に基づいて移動を適用
        float moveSpeed = GameConstants::InputConstants::MOVE_SPEED;
        
        if (glm::length(remoteInput.moveDirection) > 0.0f) {
            glm::vec3 moveDir = glm::normalize(remoteInput.moveDirection);
            float moveDistance = moveSpeed * deltaTime;
            
            glm::vec3 newPosition = gameState.multiplayer.remotePlayer.position;
            newPosition.x += moveDir.x * moveDistance;
            newPosition.z += moveDir.z * moveDistance;
            
            glm::vec3 playerSize = GameConstants::PLAYER_SIZE;
            // 簡易的な衝突判定（完全な実装には物理演算システムが必要）
            gameState.multiplayer.remotePlayer.position = newPosition;
        }
        
        // ジャンプ処理
        if (remoteInput.jumpJustPressed) {
            // 簡易的なジャンプ実装
            // 完全な実装には物理演算システムとプラットフォーム判定が必要
            gameState.multiplayer.remotePlayer.velocity.y += 15.0f; // ジャンプ速度
        }
    }
}

void MultiplayerManager::sendGameState(const GameState& gameState) {
    if (!networkManager_.isConnected() || !networkManager_.isHost()) {
        return;
    }
    
    PlayerStateData localPlayerState = convertToPlayerStateData(gameState.player);
    PlayerStateData remotePlayerState = convertToPlayerStateData(gameState.multiplayer.remotePlayer);
    
    networkManager_.sendGameStateUpdate(localPlayerState, remotePlayerState);
}

void MultiplayerManager::updateRemotePlayer(GameState& gameState, float deltaTime) {
    if (!networkManager_.isConnected() || networkManager_.isHost()) {
        return;
    }
    
    PlayerStateData localPlayerState, remotePlayerState;
    if (networkManager_.getReceivedGameState(localPlayerState, remotePlayerState)) {
        // ホストから送られてきた状態を適用
        // ローカルプレイヤーの状態はホストが計算したもの
        applyPlayerStateData(gameState.player, localPlayerState);
        
        // リモートプレイヤーの状態を補間
        gameState.multiplayer.remotePlayerTargetPosition = remotePlayerState.position;
        gameState.multiplayer.remotePlayerTargetVelocity = remotePlayerState.velocity;
        gameState.multiplayer.remotePlayerInterpolationTimer = 0.0f;
        
        applyPlayerStateData(gameState.multiplayer.remotePlayer, remotePlayerState);
    }
    
    // リモートプレイヤーの位置を補間
    if (gameState.multiplayer.remotePlayerInterpolationTimer < gameState.multiplayer.REMOTE_PLAYER_INTERPOLATION_TIME) {
        gameState.multiplayer.remotePlayerInterpolationTimer += deltaTime;
        float t = gameState.multiplayer.remotePlayerInterpolationTimer / gameState.multiplayer.REMOTE_PLAYER_INTERPOLATION_TIME;
        t = std::min(t, 1.0f);
        
        gameState.multiplayer.remotePlayer.position = glm::mix(
            gameState.multiplayer.remotePlayer.position,
            gameState.multiplayer.remotePlayerTargetPosition,
            t
        );
    }
}

void MultiplayerManager::sendGoalReached(int playerId, float clearTime) {
    networkManager_.sendGoalReached(playerId, clearTime);
}

void MultiplayerManager::checkGoalReached(GameState& gameState) {
    int playerId;
    float clearTime;
    if (networkManager_.getGoalReached(playerId, clearTime)) {
        // ゴール到達通知を受信
        if (playerId == 0) {
            // ローカルプレイヤーがゴール
            gameState.multiplayer.winnerPlayerId = 0;
            gameState.multiplayer.winnerTime = clearTime;
        } else {
            // リモートプレイヤーがゴール
            gameState.multiplayer.winnerPlayerId = 1;
            gameState.multiplayer.winnerTime = clearTime;
        }
        gameState.multiplayer.isRaceFinished = true;
        gameState.ui.showRaceResultUI = true;
        gameState.ui.raceWinnerPlayerId = gameState.multiplayer.winnerPlayerId;
        gameState.ui.raceWinnerTime = gameState.multiplayer.winnerTime;
    }
}

void MultiplayerManager::sendStageSelection(int stageNumber) {
    if (!networkManager_.isConnected() || !networkManager_.isHost()) {
        return;
    }
    networkManager_.sendStageSelection(stageNumber);
}

bool MultiplayerManager::checkStageSelection(GameState& gameState, StageManager& stageManager, PlatformSystem& platformSystem, std::function<void()> resetStageStartTime, GLFWwindow* window) {
    if (!networkManager_.isConnected() || networkManager_.isHost()) {
        return false;
    }
    
    int stageNumber;
    if (networkManager_.getStageSelection(stageNumber)) {
        // ステージ選択通知を受信
        // クライアント側でも同じステージに遷移
        if (stageNumber >= 1 && stageNumber <= 5) {
            // ステージをロード
            resetStageStartTime();
            gameState.progress.lives = 6;
            stageManager.goToStage(stageNumber, gameState, platformSystem);
            gameState.ui.readyScreenShown = false;
            gameState.ui.showReadyScreen = true;
            gameState.ui.readyScreenSpeedLevel = 0;
            gameState.progress.timeScale = 1.0f;
            gameState.progress.timeScaleLevel = 0;
            gameState.progress.isTimeAttackMode = false;
            
            // リモートプレイヤーの位置をリセット
            gameState.multiplayer.remotePlayer.position = gameState.player.position;
            gameState.multiplayer.remotePlayer.velocity = glm::vec3(0, 0, 0);
            gameState.multiplayer.isRaceStarted = true;
            
        } else if (stageNumber == 0) {
            // ステージ選択画面に戻る
            resetStageStartTime();
            stageManager.goToStage(0, gameState, platformSystem);
            gameState.ui.showReadyScreen = false;
            gameState.ui.readyScreenShown = false;
            gameState.progress.timeScale = 1.0f;
            gameState.progress.timeScaleLevel = 0;
            
            gameState.camera.isFirstPersonMode = false;
            gameState.camera.isFirstPersonView = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            
            gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
            gameState.multiplayer.isRaceStarted = false;
        }
        return true;
    }
    return false;
}

PlayerStateData MultiplayerManager::convertToPlayerStateData(const PlayerState& playerState) {
    PlayerStateData data;
    data.position = playerState.position;
    data.velocity = playerState.velocity;
    data.isMovingBackward = playerState.isMovingBackward;
    data.isShowingFrontTexture = playerState.isShowingFrontTexture;
    return data;
}

void MultiplayerManager::applyPlayerStateData(PlayerState& playerState, const PlayerStateData& data) {
    playerState.position = data.position;
    playerState.velocity = data.velocity;
    playerState.isMovingBackward = data.isMovingBackward;
    playerState.isShowingFrontTexture = data.isShowingFrontTexture;
}

void MultiplayerManager::sendPlatformStates(const PlatformSystem& platformSystem) {
    if (!networkManager_.isConnected() || !networkManager_.isHost()) {
        return;
    }
    
    std::vector<PlatformStateData> platformStates;
    const auto& platforms = platformSystem.getPlatforms();
    
    for (size_t i = 0; i < platforms.size(); ++i) {
        PlatformStateData platformData;
        platformData.platformIndex = static_cast<int>(i);
        
        std::visit([&](const auto& platform) {
            using T = std::decay_t<decltype(platform)>;
            
            if constexpr (std::is_same_v<T, MovingPlatform>) {
                platformData.platformType = PlatformType::MOVING;
                platformData.position = platform.position;
                platformData.moveTimer = platform.moveTimer;
                platformData.hasPlayerOnBoard = platform.hasPlayerOnBoard;
            } else if constexpr (std::is_same_v<T, RotatingPlatform>) {
                platformData.platformType = PlatformType::ROTATING;
                platformData.position = platform.position;
                platformData.rotationAngle = platform.rotationAngle;
            } else if constexpr (std::is_same_v<T, PatrollingPlatform>) {
                platformData.platformType = PlatformType::PATROLLING;
                platformData.position = platform.position;
                platformData.currentPatrolIndex = platform.currentPatrolIndex;
                platformData.patrolTimer = platform.patrolTimer;
            } else if constexpr (std::is_same_v<T, CycleDisappearingPlatform>) {
                platformData.platformType = PlatformType::CYCLE_DISAPPEARING;
                platformData.position = platform.position;
                platformData.cycleTimer = platform.cycleTimer;
                platformData.isCurrentlyVisible = platform.isCurrentlyVisible;
                platformData.blinkAlpha = platform.blinkAlpha;
            } else {
                // その他のプラットフォームタイプは位置のみ同期
                platformData.platformType = PlatformType::STATIC;
                platformData.position = platform.position;
            }
        }, platforms[i]);
        
        platformStates.push_back(platformData);
    }
    
    networkManager_.sendPlatformStates(platformStates);
}

void MultiplayerManager::applyPlatformStates(PlatformSystem& platformSystem) {
    if (!networkManager_.isConnected() || networkManager_.isHost()) {
        return;
    }
    
    std::vector<PlatformStateData> platformStates;
    if (!networkManager_.getReceivedPlatformStates(platformStates)) {
        return;
    }
    
    auto& platforms = platformSystem.getPlatforms();
    
    for (const auto& platformData : platformStates) {
        if (platformData.platformIndex < 0 || platformData.platformIndex >= static_cast<int>(platforms.size())) {
            continue;
        }
        
        std::visit([&](auto& platform) {
            using T = std::decay_t<decltype(platform)>;
            
            if constexpr (std::is_same_v<T, MovingPlatform>) {
                if (platformData.platformType == PlatformType::MOVING) {
                    platform.position = platformData.position;
                    platform.moveTimer = platformData.moveTimer;
                    platform.hasPlayerOnBoard = platformData.hasPlayerOnBoard;
                }
            } else if constexpr (std::is_same_v<T, RotatingPlatform>) {
                if (platformData.platformType == PlatformType::ROTATING) {
                    platform.position = platformData.position;
                    platform.rotationAngle = platformData.rotationAngle;
                }
            } else if constexpr (std::is_same_v<T, PatrollingPlatform>) {
                if (platformData.platformType == PlatformType::PATROLLING) {
                    platform.position = platformData.position;
                    platform.currentPatrolIndex = platformData.currentPatrolIndex;
                    platform.patrolTimer = platformData.patrolTimer;
                }
            } else if constexpr (std::is_same_v<T, CycleDisappearingPlatform>) {
                if (platformData.platformType == PlatformType::CYCLE_DISAPPEARING) {
                    platform.position = platformData.position;
                    platform.cycleTimer = platformData.cycleTimer;
                    platform.isCurrentlyVisible = platformData.isCurrentlyVisible;
                    platform.blinkAlpha = platformData.blinkAlpha;
                }
            } else {
                // その他のプラットフォームタイプは位置のみ同期
                platform.position = platformData.position;
            }
        }, platforms[platformData.platformIndex]);
    }
}

