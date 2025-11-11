#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "switch_system.h"
#include "../core/constants/game_constants.h"
#include <iostream>

// スイッチの更新
void SwitchSystem::updateSwitches(GameState& gameState, float deltaTime) {
    // スイッチの更新
    for (auto& switch_obj : gameState.switches) {
        // クールダウンタイマーの更新
        if (switch_obj.cooldownTimer > 0.0f) {
            switch_obj.cooldownTimer -= deltaTime;
        }
        
        // 複数スイッチの処理
        if (switch_obj.isMultiSwitch) {
            // 同じグループのスイッチが全て押されているかチェック
            bool allPressed = true;
            for (const auto& otherSwitch : gameState.switches) {
                if (otherSwitch.isMultiSwitch && otherSwitch.multiSwitchGroup == switch_obj.multiSwitchGroup) {
                    if (!otherSwitch.isPressed) {
                        allPressed = false;
                        break;
                    }
                }
            }
            
            // 全て押されている場合、特別な足場を出現させる
            if (allPressed) {
                // 複数スイッチ用の特別な足場を出現（例：ゴールへの近道）
                // ここでは例として、新しい足場を動的に追加
            }
        }
    }
}

// スイッチとの衝突判定
bool SwitchSystem::checkSwitchCollision(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (auto& switch_obj : gameState.switches) {
        if (switch_obj.cooldownTimer > 0.0f) continue; // クールダウン中は無視
        
        glm::vec3 switchMin = switch_obj.position - switch_obj.size * GameConstants::PhysicsCalculationConstants::PLATFORM_HALF_SIZE_MULTIPLIER;
        glm::vec3 switchMax = switch_obj.position + switch_obj.size * GameConstants::PhysicsCalculationConstants::PLATFORM_HALF_SIZE_MULTIPLIER;
        glm::vec3 playerMin = playerPos - playerSize * GameConstants::PhysicsCalculationConstants::PLAYER_HALF_SIZE_MULTIPLIER;
        glm::vec3 playerMax = playerPos + playerSize * GameConstants::PhysicsCalculationConstants::PLAYER_HALF_SIZE_MULTIPLIER;
        
        // 衝突判定
        if (playerMax.x >= switchMin.x && playerMin.x <= switchMax.x &&
            playerMax.y >= switchMin.y && playerMin.y <= switchMax.y &&
            playerMax.z >= switchMin.z && playerMin.z <= switchMax.z) {
            
            // スイッチを押す
            if (!switch_obj.isPressed) {
                switch_obj.isPressed = true;
                switch_obj.pressTimer = 0.0f;
                switch_obj.cooldownTimer = GameConstants::StageConstants::SWITCH_COOLDOWN_TIME; // クールダウン
                
                // 単体スイッチの処理
                if (!switch_obj.isMultiSwitch) {
                    for (size_t i = 0; i < switch_obj.targetPlatformIndices.size(); i++) {
                        int platformIndex = switch_obj.targetPlatformIndices[i];
                        if (platformIndex >= 0 && platformIndex < static_cast<int>(gameState.platforms.size())) {
                            bool targetState = switch_obj.targetStates[i];
                            if (switch_obj.isToggle) {
                                // トグルスイッチ：現在の状態を反転
                                if (targetState) {
                                    // 足場を出現させる
                                    gameState.platforms[platformIndex].size = gameState.platforms[platformIndex].originalSize;
                                } else {
                                    // 足場を消失させる
                                    gameState.platforms[platformIndex].size = glm::vec3(0, 0, 0);
                                }
                                // 目標状態を反転
                                switch_obj.targetStates[i] = !targetState;
                            } else {
                                // 通常スイッチ：指定された状態に設定
                                if (targetState) {
                                    gameState.platforms[platformIndex].size = gameState.platforms[platformIndex].originalSize;
                                } else {
                                    gameState.platforms[platformIndex].size = glm::vec3(0, 0, 0);
                                }
                            }
                        }
                    }
                }
            }
            return true;
        }
    }
    return false;
}

