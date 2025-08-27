#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../gfx/opengl_renderer.h"
#include "../game/game_state.h"
#include "../game/stage_generator.h"
#include "../game/stage_manager.h"
#include "../game/platform_system.h"
#include "../game/cannon_system.h"
#include "../game/switch_system.h"
#include "../game/gravity_system.h"
#include "../physics/physics_system.h"
#include "../io/input_system.h"

// ======================================================
//                        main
// ======================================================
int main(int argc, char* argv[]) {
    
    // GLFW初期化
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // OpenGL 2.1設定
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    
    // ウィンドウ作成
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Vulkan3D - Portal Physics Puzzle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // OpenGLレンダラー初期化
    auto renderer = std::make_unique<gfx::OpenGLRenderer>();
    if (!renderer->initialize(window)) {
        std::cerr << "Failed to initialize OpenGL renderer" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // ゲーム状態
    GameState gameState;
    
    // コマンドライン引数で初期ステージを指定
    if (argc > 1) {
        int requestedStage = std::atoi(argv[1]);
        if (requestedStage >= 1 && requestedStage <= 5) {
            gameState.currentStage = requestedStage;
            printf("Initial stage set to %d via command line argument\n", requestedStage);
        }
    }
    
    glfwSetWindowUserPointer(window, &gameState);
    
    // 新しい設計のプラットフォームシステム
    PlatformSystem platformSystem;
    
    // ステージ管理システム
    StageManager stageManager;
    
    // currentStageの値に基づいて初期ステージを設定
    int initialStage = gameState.currentStage;
    if (initialStage < 1) initialStage = 1;
    if (initialStage > stageManager.getTotalStages()) initialStage = stageManager.getTotalStages();
    
    printf("Starting from stage %d\n", initialStage);
    stageManager.loadStage(initialStage, gameState, platformSystem);

    // ゲームパッド初期化
    InputSystem::initializeGamepad();

    // ゲーム開始準備完了
    bool gameRunning = true;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastFrameTime = startTime;
    
    // --------------------------
    //         ゲームループ
    // --------------------------
    while (!glfwWindowShouldClose(window) && gameRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
        
        // deltaTimeの異常値を制限（フレームレート低下時の問題を防ぐ）
        deltaTime = std::min(deltaTime, 0.1f); // 最大100ms（10FPS相当）
        
        lastFrameTime = currentTime;
        
        gameState.gameTime = std::chrono::duration<float>(currentTime - startTime).count();
        
        // システム更新
        platformSystem.update(deltaTime, gameState.playerPosition);
        GravitySystem::updateGravityZones(gameState, deltaTime);
        SwitchSystem::updateSwitches(gameState, deltaTime);
        CannonSystem::updateCannons(gameState, deltaTime);
        
        // 入力処理
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            gameRunning = false;
        }
        
        // ステージ切り替えキー
        static bool key1Pressed = false, key2Pressed = false, key3Pressed = false, key4Pressed = false, key5Pressed = false;
        static bool nextStagePressed = false, prevStagePressed = false;
        
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed) {
            stageManager.goToStage(1, gameState, platformSystem);
            key1Pressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) {
            key1Pressed = false;
        }
        
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed) {
            stageManager.goToStage(2, gameState, platformSystem);
            key2Pressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) {
            key2Pressed = false;
        }
        
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !key3Pressed) {
            stageManager.goToStage(3, gameState, platformSystem);
            key3Pressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) {
            key3Pressed = false;
        }
        
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !key4Pressed) {
            stageManager.goToStage(4, gameState, platformSystem);
            key4Pressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) {
            key4Pressed = false;
        }
        
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && !key5Pressed) {
            stageManager.goToStage(5, gameState, platformSystem);
            key5Pressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE) {
            key5Pressed = false;
        }
        
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && !nextStagePressed) {
            stageManager.goToNextStage(gameState, platformSystem);
            nextStagePressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE) {
            nextStagePressed = false;
        }
        
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && !prevStagePressed) {
            stageManager.goToPreviousStage(gameState, platformSystem);
            prevStagePressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE) {
            prevStagePressed = false;
        }
        
        // ステージクリアUIでのキー入力処理
        if (gameState.showStageClearUI) {
            static bool enterPressed = false, rPressed = false;
            
            // ENTERキーで次のステージに進む
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterPressed) {
                enterPressed = true;
                if (stageManager.getCurrentStage() < stageManager.getTotalStages()) {
                    // ステージをクリアして次のステージに進む
                    stageManager.completeStage(stageManager.getCurrentStage());
                    if (stageManager.goToNextStage(gameState, platformSystem)) {
                        printf("Moving to next stage: %d\n", stageManager.getCurrentStage());
                        gameState.showStageClearUI = false;
                        gameState.gameWon = false;
                    }
                }
            } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                enterPressed = false;
            }
            
            // Rキーでリトライ
            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rPressed) {
                rPressed = true;
                // 現在のステージを再読み込み
                stageManager.loadStage(stageManager.getCurrentStage(), gameState, platformSystem);
                gameState.showStageClearUI = false;
                gameState.gameWon = false;
            } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
                rPressed = false;
            }
        }
        
        // アイテム更新
        for (auto& item : gameState.items) {
            if (!item.isCollected) {
                // アイテムの回転
                item.rotationAngle += deltaTime * 90.0f; // 1秒で90度回転
                if (item.rotationAngle >= 360.0f) {
                    item.rotationAngle -= 360.0f;
                }
                
                // アイテムの上下の揺れ
                item.bobTimer += deltaTime;
                item.bobHeight = sin(item.bobTimer * 2.0f) * 0.2f;
                
                // プレイヤーとの距離チェック（アイテム収集）
                float distance = glm::length(gameState.playerPosition - item.position);
                if (distance < 1.5f) { // 収集範囲
                    item.isCollected = true;
                    gameState.collectedItems++;
                    
                    // チェックポイントを更新
                    gameState.lastCheckpoint = item.position;
                    gameState.lastCheckpointItemId = item.itemId;
                    
                    printf("Item %d collected! (%d/%d) - Checkpoint set at (%.1f, %.1f, %.1f)\n", 
                           item.itemId, gameState.collectedItems, gameState.requiredItems,
                           gameState.lastCheckpoint.x, gameState.lastCheckpoint.y, gameState.lastCheckpoint.z);
                }
            }
        }
        
        // 重力反転エリアのチェック
        glm::vec3 gravityDirection = glm::vec3(0, -1, 0); // デフォルトは下向き
        bool inGravityZone = PhysicsSystem::isPlayerInGravityZone(gameState, gameState.playerPosition, gravityDirection);
        
        // 物理演算
        float gravityStrength = 12.0f * deltaTime;  // 重力を強くして、より速く落下するように
        if (gravityDirection.y > 0.5f) {
            gravityStrength *= 0.7f; // 重力反転時は70%の強度
        }
        glm::vec3 gravityForce = gravityDirection * gravityStrength;

        gameState.playerVelocity += gravityForce;
        gameState.playerVelocity *= 0.98f; // 空気抵抗
        
        // 入力処理
        InputSystem::processInput(window, gameState, deltaTime);
        InputSystem::processJumpAndFloat(window, gameState, deltaTime, gravityDirection, platformSystem);

        // 垂直位置更新
        gameState.playerPosition.y += gameState.playerVelocity.y * deltaTime;

        // 新しい分離型設計での衝突判定
        glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
        
        // 衝突判定
        SwitchSystem::checkSwitchCollision(gameState, gameState.playerPosition, playerSize);
        CannonSystem::checkCannonCollision(gameState, gameState.playerPosition, playerSize);
        
        // 新しい設計でのプラットフォーム衝突判定
        GameState::PlatformVariant* currentPlatform = platformSystem.checkCollision(gameState.playerPosition, playerSize);
        
        // 新しい設計での足場衝突判定
        if (currentPlatform != nullptr) {
            std::visit(overloaded{
                [&](const GameState::StaticPlatform& platform) {
                    // 静的足場の処理
                    if (gravityDirection.y > 0.5f) {
                        gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                    }
                    gameState.playerVelocity.y = 0.0f;
                    
                    // ゴール判定（黄色い足場の場合）
                    if (platform.color.r > 0.9f && platform.color.g > 0.9f && platform.color.b < 0.1f) {
                        // 黄色い足場（ゴール足場）に触れた場合
                        if (!gameState.gameWon && gameState.collectedItems >= gameState.requiredItems) {
                            gameState.gameWon = true;
                            gameState.showStageClearUI = true;
                            gameState.stageClearTimer = 0.0f;
                            printf("Stage %d completed! All items collected (%d/%d)\n", 
                                   stageManager.getCurrentStage(), gameState.collectedItems, gameState.requiredItems);
                        } else if (!gameState.gameWon && gameState.collectedItems < gameState.requiredItems) {
                            printf("Need to collect all items first! (%d/%d)\n", gameState.collectedItems, gameState.requiredItems);
                        }
                    }
                },
                [&](const GameState::MovingPlatform& platform) {
                    // 移動足場の処理
                    if (gravityDirection.y > 0.5f) {
                        gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                    }
                    gameState.playerVelocity.y = 0.0f;
                    const_cast<GameState::MovingPlatform&>(platform).hasPlayerOnBoard = true;
                },
                [&](const GameState::RotatingPlatform& platform) {
                    // 回転足場の処理
                    if (gravityDirection.y > 0.5f) {
                        gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                    }
                    gameState.playerVelocity.y = 0.0f;
                    gameState.isFloating = false;
                    gameState.floatCount = 0;
                },
                [&](const GameState::PatrollingPlatform& platform) {
                    // 巡回足場の処理
                    if (gravityDirection.y > 0.5f) {
                        gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                    }
                    gameState.playerVelocity.y = 0.0f;
                    gameState.isFloating = false;
                    gameState.floatCount = 0;
                },
                [&](const GameState::TeleportPlatform& platform) {
                    // テレポート足場の処理
                    if (gravityDirection.y > 0.5f) {
                        gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                    }
                    gameState.playerVelocity.y = 0.0f;
                    
                    if (!platform.hasTeleported && platform.cooldownTimer <= 0.0f) {
                        gameState.playerPosition = platform.teleportDestination;
                        const_cast<GameState::TeleportPlatform&>(platform).hasTeleported = true;
                        const_cast<GameState::TeleportPlatform&>(platform).cooldownTimer = 2.0f;
                    }
                },
                [&](const GameState::JumpPad& platform) {
                    // ジャンプパッドの処理
                    if (gravityDirection.y > 0.5f) {
                        gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                    }
                    gameState.playerVelocity.y = platform.jumpPower;
                },
                [&](const GameState::CycleDisappearingPlatform& platform) {
                    // 周期的に消える足場の処理
                    if (platform.isCurrentlyVisible) {
                        if (gravityDirection.y > 0.5f) {
                            gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                        } else {
                            gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                        }
                        gameState.playerVelocity.y = 0.0f;
                    }
                },
                [&](const GameState::DisappearingPlatform& platform) {
                    // 消える足場の処理
                    if (gravityDirection.y > 0.5f) {
                        gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                    }
                    gameState.playerVelocity.y = 0.0f;
                },
                [&](const GameState::FlyingPlatform& platform) {
                    // 飛んでくる足場の処理
                    if (gravityDirection.y > 0.5f) {
                        gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                    }
                    gameState.playerVelocity.y = 0.0f;
                }
            }, *currentPlatform);
        }
        
        // 新しい設計でのプレイヤー-足場同期処理
        if (currentPlatform != nullptr) {
            std::visit(overloaded{
                [&](const GameState::StaticPlatform& platform) {
                    // 静的足場は何もしない
                },
                [&](GameState::MovingPlatform& platform) {
                    if (platform.hasPlayerOnBoard) {
                        // プレイヤーが足場の上に正しく配置されるようにする
                        if (gravityDirection.y > 0.5f) {
                            gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                        } else {
                            gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                        }
                        
                        // プレイヤーのX座標とZ座標を足場の移動に合わせて更新
                        // 足場の移動量を計算してプレイヤーに適用
                        glm::vec3 platformMovement = platform.position - platform.previousPosition;
                        gameState.playerPosition.x += platformMovement.x;
                        gameState.playerPosition.z += platformMovement.z;
                    }
                },
                [&](GameState::RotatingPlatform& platform) {
                    // 回転足場の処理
                    // プレイヤーが足場の上にいる場合、回転に合わせて移動させる
                    glm::vec3 halfSize = platform.size * 0.5f;
                    glm::vec3 platformMin = platform.position - halfSize;
                    glm::vec3 platformMax = platform.position + halfSize;
                    
                    // プレイヤーが足場の範囲内にいるかチェック
                    bool onPlatform = (gameState.playerPosition.x >= platformMin.x && gameState.playerPosition.x <= platformMax.x &&
                                      gameState.playerPosition.z >= platformMin.z && gameState.playerPosition.z <= platformMax.z);
                    
                    if (onPlatform) {
                        // プレイヤーの位置を足場のローカル座標系に変換
                        glm::vec3 localPlayerPos = gameState.playerPosition - platform.position;
                        
                        // 回転軸に応じて回転を適用
                        if (glm::length(platform.rotationAxis - glm::vec3(0, 1, 0)) < 0.1f) {
                            // Y軸回転の場合、XZ平面での回転
                            float angle = glm::radians(platform.rotationSpeed * gameState.gameTime);
                            float cosAngle = cos(angle);
                            float sinAngle = sin(angle);
                            
                            float newX = localPlayerPos.x * cosAngle - localPlayerPos.z * sinAngle;
                            float newZ = localPlayerPos.x * sinAngle + localPlayerPos.z * cosAngle;
                            
                            gameState.playerPosition = platform.position + glm::vec3(newX, localPlayerPos.y, newZ);
                        } else if (glm::length(platform.rotationAxis - glm::vec3(1, 0, 0)) < 0.1f) {
                            // X軸回転（縦回転）の場合、YZ平面での回転
                            float angle = glm::radians(platform.rotationSpeed * gameState.gameTime);
                            float cosAngle = cos(angle);
                            float sinAngle = sin(angle);
                            
                            float newY = localPlayerPos.y * cosAngle - localPlayerPos.z * sinAngle;
                            float newZ = localPlayerPos.y * sinAngle + localPlayerPos.z * cosAngle;
                            
                            gameState.playerPosition = platform.position + glm::vec3(localPlayerPos.x, newY, newZ);
                        }
                        
                        // Y座標を足場の上に固定
                        if (gravityDirection.y > 0.5f) {
                            gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                        } else {
                            gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                        }
                    }
                },
                [&](GameState::PatrollingPlatform& platform) {
                    // 巡回足場の場合も同様の処理
                    // プレイヤーが足場の範囲内にいるかチェック（より緩い判定）
                    glm::vec3 halfSize = platform.size * 0.5f;
                    glm::vec3 platformMin = platform.position - halfSize;
                    glm::vec3 platformMax = platform.position + halfSize;
                    
                    // より緩い判定範囲（足場のサイズの1.5倍）
                    glm::vec3 extendedHalfSize = halfSize * 1.5f;
                    glm::vec3 extendedMin = platform.position - extendedHalfSize;
                    glm::vec3 extendedMax = platform.position + extendedHalfSize;
                    
                    bool onPlatform = (gameState.playerPosition.x >= platformMin.x && gameState.playerPosition.x <= platformMax.x &&
                                      gameState.playerPosition.z >= platformMin.z && gameState.playerPosition.z <= platformMax.z);
                    
                    bool inExtendedRange = (gameState.playerPosition.x >= extendedMin.x && gameState.playerPosition.x <= extendedMax.x &&
                                           gameState.playerPosition.z >= extendedMin.z && gameState.playerPosition.z <= extendedMax.z);
                    
                    if (onPlatform || inExtendedRange) {
                        // Y座標を足場の上に固定
                        if (gravityDirection.y > 0.5f) {
                            gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                        } else {
                            gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                        }
                        
                        // プレイヤーのX座標とZ座標を足場の移動に合わせて更新
                        // 足場の移動量を計算してプレイヤーに適用
                        glm::vec3 platformMovement = platform.position - platform.previousPosition;
                        gameState.playerPosition.x += platformMovement.x;
                        gameState.playerPosition.z += platformMovement.z;
                        
                        // プレイヤーが足場の中心に近づくように調整
                        if (!onPlatform && inExtendedRange) {
                            glm::vec3 directionToCenter = platform.position - gameState.playerPosition;
                            directionToCenter.y = 0; // Y軸は無視
                            float distanceToCenter = glm::length(directionToCenter);
                            if (distanceToCenter > 0.1f) {
                                glm::vec3 normalizedDirection = glm::normalize(directionToCenter);
                                gameState.playerPosition += normalizedDirection * 0.5f * deltaTime;
                            }
                        }
                    }
                },
                [&](const GameState::TeleportPlatform& platform) {
                    // テレポート足場は何もしない
                },
                [&](const GameState::JumpPad& platform) {
                    // ジャンプパッドは何もしない
                },
                [&](const GameState::CycleDisappearingPlatform& platform) {
                    // 周期的に消える足場は何もしない
                },
                [&](const GameState::DisappearingPlatform& platform) {
                    // 消える足場は何もしない
                },
                [&](const GameState::FlyingPlatform& platform) {
                    // 飛んでくる足場は何もしない
                }
            }, *currentPlatform);
        }
        
        // プレイヤーが足場から離れた時の処理
        if (currentPlatform == nullptr) {
            // 全てのMovingPlatformのhasPlayerOnBoardフラグをリセット
            platformSystem.resetMovingPlatformFlags();
        }
        
        // 下限チェック（奈落に落ちた場合）
        if (gameState.playerPosition.y < 0) {
            // チェックポイントがある場合はそこに復活、ない場合はスタート地点に復活
            if (gameState.lastCheckpointItemId != -1) {
                gameState.playerPosition = gameState.lastCheckpoint;
                printf("Respawned at checkpoint (Item %d) at (%.1f, %.1f, %.1f)\n", 
                       gameState.lastCheckpointItemId,
                       gameState.lastCheckpoint.x, gameState.lastCheckpoint.y, gameState.lastCheckpoint.z);
            } else {
                gameState.playerPosition = glm::vec3(0, 6.0f, -25.0f); // スタート地点にリセット
                printf("Respawned at start position (no checkpoint)\n");
            }
            gameState.playerVelocity = glm::vec3(0, 0, 0);
        }
        
        // ステージクリアUIのタイマー更新
        if (gameState.showStageClearUI) {
            gameState.stageClearTimer += deltaTime;
        }

        // --------------------------
        //            描画
        // --------------------------
        renderer->beginFrame();
        
        // カメラ設定
        glm::vec3 cameraPos = gameState.playerPosition + glm::vec3(0, 2, -8);
        glm::vec3 cameraTarget = gameState.playerPosition;
        renderer->setCamera(cameraPos, cameraTarget);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        renderer->setProjection(45.0f, (float)width / (float)height, 0.1f, 100.0f);
        
        // 新しい設計での足場の描画
        auto positions = platformSystem.getPositions();
        auto sizes = platformSystem.getSizes();
        auto colors = platformSystem.getColors();
        auto visibility = platformSystem.getVisibility();
        auto isRotating = platformSystem.getIsRotating();
        auto rotationAngles = platformSystem.getRotationAngles();
        auto rotationAxes = platformSystem.getRotationAxes();
        auto blinkAlphas = platformSystem.getBlinkAlphas();
        
        for (size_t i = 0; i < positions.size(); i++) {
            if (!visibility[i] || sizes[i].x <= 0 || sizes[i].y <= 0 || sizes[i].z <= 0) continue;
            
            if (isRotating[i]) {
                renderer->renderRotatedBox(positions[i], colors[i], sizes[i], rotationAxes[i], rotationAngles[i]);
            } else {
                renderer->renderBoxWithAlpha(positions[i], colors[i], sizes[i], blinkAlphas[i]);
            }
        }
        
        // 重力反転エリアの描画
        for (const auto& zone : gameState.gravityZones) {
            if (zone.isActive) {
                glm::vec3 zoneColor = glm::vec3(0.2f, 0.6f, 1.0f);
                renderer->renderBoxWithAlpha(zone.position, zoneColor, zone.size, 0.3f);
            }
        }
        
        // スイッチの描画
        for (const auto& switch_obj : gameState.switches) {
            glm::vec3 switchColor = switch_obj.color;
            if (switch_obj.isPressed) {
                switchColor *= 0.7f;
            }
            renderer->renderBox(switch_obj.position, switchColor, switch_obj.size);
        }
        
        // 大砲の描画
        for (const auto& cannon : gameState.cannons) {
            if (cannon.isActive) {
                glm::vec3 color = cannon.color;
                if (cannon.cooldownTimer > 0.0f) {
                    color *= 0.5f;
                }
                renderer->renderBox(cannon.position, color, cannon.size);
            }
        }
        
        // アイテムの描画
        for (const auto& item : gameState.items) {
            if (!item.isCollected) {
                // アイテムの位置に揺れを適用
                glm::vec3 itemPos = item.position + glm::vec3(0, item.bobHeight, 0);
                renderer->renderRotatedBox(itemPos, item.color, item.size, glm::vec3(0, 1, 0), item.rotationAngle);
            }
        }
        
        // プレイヤーの描画
        renderer->renderCube(gameState.playerPosition, gameState.playerColor, 0.5f);

        // UI
        renderer->renderText("Score: " + std::to_string(gameState.score), glm::vec2(10, 10), glm::vec3(1, 1, 1));
        renderer->renderText("Time: " + std::to_string((int)gameState.gameTime) + "s", glm::vec2(10, 30), glm::vec3(1, 1, 1));
        renderer->renderText("Float Count: " + std::to_string(gameState.floatCount) + "/2", glm::vec2(10, 50), glm::vec3(1, 1, 1));
        
        // ステージ情報
        const StageData* currentStageData = stageManager.getStageData(stageManager.getCurrentStage());
        if (currentStageData) {
            renderer->renderText("Stage " + std::to_string(stageManager.getCurrentStage()) + ": " + currentStageData->stageName, 
                               glm::vec2(10, 70), glm::vec3(1, 1, 0));
        }
        
        renderer->renderText("Platforms: " + std::to_string(platformSystem.getPlatforms().size()), glm::vec2(10, 90), glm::vec3(1, 1, 1));
        
        // アイテム収集状況の表示
        renderer->renderText("Items: " + std::to_string(gameState.collectedItems) + "/" + std::to_string(gameState.requiredItems), 
                           glm::vec2(10, 130), glm::vec3(1, 1, 0));
        
        // チェックポイント情報の表示
        if (gameState.lastCheckpointItemId != -1) {
            renderer->renderText("Checkpoint: Item " + std::to_string(gameState.lastCheckpointItemId), 
                               glm::vec2(10, 170), glm::vec3(0, 1, 1));
        } else {
            renderer->renderText("Checkpoint: None", glm::vec2(10, 170), glm::vec3(0.5f, 0.5f, 0.5f));
        }
        
        // 重力状態の表示
        if (inGravityZone) {
            renderer->renderText("GRAVITY INVERTED!", glm::vec2(10, 190), glm::vec3(0.2f, 0.6f, 1.0f));
        } else {
            renderer->renderText("Normal Gravity", glm::vec2(10, 190), glm::vec3(1.0f, 1.0f, 1.0f));
        }
        
        // システム情報の表示
        renderer->renderText("Switches: " + std::to_string(gameState.switches.size()), glm::vec2(10, 130), glm::vec3(1, 1, 1));
        renderer->renderText("Cannons: " + std::to_string(gameState.cannons.size()), glm::vec2(10, 150), glm::vec3(1, 1, 1));
        
        // ステージクリアUI
        if (gameState.showStageClearUI) {
            // 背景オーバーレイ
            renderer->renderText("", glm::vec2(0, 0), glm::vec3(0, 0, 0), 0.7f);
            
            // ステージクリアメッセージ
            renderer->renderText("STAGE COMPLETE!", glm::vec2(width/2 - 150, height/2 - 100), glm::vec3(1, 1, 0), 2.0f);
            
            // ステージ情報
            const StageData* currentStageData = stageManager.getStageData(stageManager.getCurrentStage());
            if (currentStageData) {
                renderer->renderText("Stage " + std::to_string(stageManager.getCurrentStage()) + ": " + currentStageData->stageName, 
                                   glm::vec2(width/2 - 120, height/2 - 50), glm::vec3(1, 1, 1), 1.2f);
            }
            
            // スコアとタイム
            renderer->renderText("Score: " + std::to_string(gameState.score), 
                               glm::vec2(width/2 - 80, height/2), glm::vec3(1, 1, 1), 1.0f);
            renderer->renderText("Time: " + std::to_string((int)gameState.gameTime) + "s", 
                               glm::vec2(width/2 - 80, height/2 + 30), glm::vec3(1, 1, 1), 1.0f);
            renderer->renderText("Items Collected: " + std::to_string(gameState.collectedItems) + "/" + std::to_string(gameState.requiredItems), 
                               glm::vec2(width/2 - 100, height/2 + 60), glm::vec3(1, 1, 0), 1.0f);
            
            // 次のステージボタン
            if (stageManager.getCurrentStage() < stageManager.getTotalStages()) {
                renderer->renderText("Press ENTER to continue to next stage", 
                                   glm::vec2(width/2 - 180, height/2 + 100), glm::vec3(0.2f, 1.0f, 0.2f), 1.0f);
            } else {
                renderer->renderText("All stages completed! Congratulations!", 
                                   glm::vec2(width/2 - 200, height/2 + 100), glm::vec3(1.0f, 0.8f, 0.2f), 1.0f);
            }
            
            // リトライボタン
            renderer->renderText("Press R to retry this stage", 
                               glm::vec2(width/2 - 120, height/2 + 140), glm::vec3(0.8f, 0.8f, 0.8f), 0.8f);
        }
        
        // 操作説明
        renderer->renderText("Controls: WASD=Move, SPACE=Jump, 1-5=Stage Select, LEFT/RIGHT=Next/Prev Stage", 
                           glm::vec2(10, height - 30), glm::vec3(0.8f, 0.8f, 0.8f));
        
        renderer->endFrame();
        
        // フレームレート制限
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        
        glfwPollEvents();
    }
    
    // クリーンアップ
    renderer->cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
