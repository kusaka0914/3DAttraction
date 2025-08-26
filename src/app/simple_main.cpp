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
#include "../game/platform_system.h"
#include "../game/cannon_system.h"
#include "../game/switch_system.h"
#include "../game/gravity_system.h"
#include "../physics/physics_system.h"
#include "../io/input_system.h"

// ======================================================
//                        main
// ======================================================
int main() {
    
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
    glfwSetWindowUserPointer(window, &gameState);
    
    // 新しい設計のプラットフォームシステム
    PlatformSystem platformSystem;
    
    // ステージ生成（新しい設計）
    StageGenerator::generateTerrain(gameState, platformSystem);
    
    // プレイヤーとゴール位置設定
    gameState.playerPosition = glm::vec3(0, 6.0f, -25.0f);  // スタート足場の上
    gameState.goalPosition   = glm::vec3(0, 16.0f, 25.0f);  // ゴール足場の上

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
        
        // 重力反転エリアのチェック
        glm::vec3 gravityDirection = glm::vec3(0, -1, 0); // デフォルトは下向き
        bool inGravityZone = PhysicsSystem::isPlayerInGravityZone(gameState, gameState.playerPosition, gravityDirection);
        
        // 物理演算
        float gravityStrength = 8.0f * deltaTime;
        if (gravityDirection.y > 0.5f) {
            gravityStrength *= 0.7f; // 重力反転時は70%の強度
        }
        glm::vec3 gravityForce = gravityDirection * gravityStrength;

        if (!gameState.isFloating) {
            gameState.playerVelocity += gravityForce;
            gameState.playerVelocity *= 0.98f; // 空気抵抗
        }
        
        // 入力処理
        InputSystem::processInput(window, gameState, deltaTime);
        InputSystem::processJumpAndFloat(window, gameState, deltaTime, gravityDirection);

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
                    gameState.isFloating = false;
                    gameState.floatCount = 0;
                },
                [&](const GameState::MovingPlatform& platform) {
                    // 移動足場の処理
                    if (gravityDirection.y > 0.5f) {
                        gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                    }
                    gameState.playerVelocity.y = 0.0f;
                    gameState.isFloating = false;
                    gameState.floatCount = 0;
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
                    gameState.isFloating = false;
                    gameState.floatCount = 0;
                    
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
                    gameState.isFloating = false;
                    gameState.floatCount = 0;
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
                        gameState.isFloating = false;
                        gameState.floatCount = 0;
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
                    gameState.isFloating = false;
                    gameState.floatCount = 0;
                },
                [&](const GameState::FlyingPlatform& platform) {
                    // 飛んでくる足場の処理
                    if (gravityDirection.y > 0.5f) {
                        gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                    }
                    gameState.playerVelocity.y = 0.0f;
                    gameState.isFloating = false;
                    gameState.floatCount = 0;
                }
            }, *currentPlatform);
        }
        
        // 新しい設計でのプレイヤー-足場同期処理
        if (currentPlatform != nullptr) {
            std::visit(overloaded{
                [&](const GameState::StaticPlatform& platform) {
                    // 静的足場は何もしない
                },
                [&](const GameState::MovingPlatform& platform) {
                    if (platform.hasPlayerOnBoard) {
                        // プレイヤーが足場の上に正しく配置されるようにする
                        if (gravityDirection.y > 0.5f) {
                            gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                        } else {
                            gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
                        }
                    }
                },
                [&](const GameState::RotatingPlatform& platform) {
                    // 回転足場は何もしない
                },
                [&](const GameState::PatrollingPlatform& platform) {
                    // 巡回足場の場合も同様の処理
                    if (gravityDirection.y > 0.5f) {
                        gameState.playerPosition.y = platform.position.y - platform.size.y * 0.5f - playerSize.y * 0.5f;
                    } else {
                        gameState.playerPosition.y = platform.position.y + platform.size.y * 0.5f + playerSize.y * 0.5f;
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
        
        // 下限チェック（奈落に落ちた場合）
        if (gameState.playerPosition.y < -10.0f) {
            gameState.playerPosition = glm::vec3(0, 6.0f, -25.0f); // スタート地点にリセット
            gameState.playerVelocity = glm::vec3(0, 0, 0);
        }
        
        // ゴール判定
            float goalDistance = glm::length(gameState.playerPosition - gameState.goalPosition);
            if (goalDistance < 2.0f) {
                gameState.gameWon = true;
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
        
        for (size_t i = 0; i < positions.size(); i++) {
            if (!visibility[i] || sizes[i].x <= 0 || sizes[i].y <= 0 || sizes[i].z <= 0) continue;
            
            if (isRotating[i]) {
                renderer->renderRotatedBox(positions[i], colors[i], sizes[i], rotationAxes[i], rotationAngles[i]);
            } else {
                renderer->renderBox(positions[i], colors[i], sizes[i]);
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
        
        // プレイヤーの描画
        renderer->renderCube(gameState.playerPosition, gameState.playerColor, 0.5f);
        
        // ゴールの描画
        renderer->renderCube(gameState.goalPosition, gameState.goalColor, 1.5f);

                // UI
        renderer->renderText("Score: " + std::to_string(gameState.score), glm::vec2(10, 10), glm::vec3(1, 1, 1));
        renderer->renderText("Time: " + std::to_string((int)gameState.gameTime) + "s", glm::vec2(10, 30), glm::vec3(1, 1, 1));
        renderer->renderText("Float Count: " + std::to_string(gameState.floatCount) + "/2", glm::vec2(10, 50), glm::vec3(1, 1, 1));
        renderer->renderText("Platforms: " + std::to_string(platformSystem.getPlatforms().size()), glm::vec2(10, 90), glm::vec3(1, 1, 1));
        
        // 重力状態の表示
        if (inGravityZone) {
            renderer->renderText("GRAVITY INVERTED!", glm::vec2(10, 70), glm::vec3(0.2f, 0.6f, 1.0f));
        } else {
            renderer->renderText("Normal Gravity", glm::vec2(10, 70), glm::vec3(1.0f, 1.0f, 1.0f));
        }
        
        // システム情報の表示
        renderer->renderText("Platforms: " + std::to_string(gameState.platforms.size()), glm::vec2(10, 90), glm::vec3(1, 1, 1));
        renderer->renderText("Switches: " + std::to_string(gameState.switches.size()), glm::vec2(10, 110), glm::vec3(1, 1, 1));
        renderer->renderText("Cannons: " + std::to_string(gameState.cannons.size()), glm::vec2(10, 130), glm::vec3(1, 1, 1));
        
        if (gameState.gameWon) {
            renderer->renderText("STAGE COMPLETE!", glm::vec2(640, 360), glm::vec3(1, 1, 0));
        }
        
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
