/**
 * @file switch_system.h
 * @brief スイッチシステム
 * @details スイッチの更新と衝突判定を管理します。
 */
#pragma once

#include "game_state.h"

/**
 * @brief スイッチシステム
 * @details スイッチの更新と衝突判定を管理します。
 */
class SwitchSystem {
public:
    /**
     * @brief スイッチを更新する
     * @details 全スイッチの状態を更新します。
     * 
     * @param gameState ゲーム状態
     * @param deltaTime デルタタイム
     */
    static void updateSwitches(GameState& gameState, float deltaTime);
    
    /**
     * @brief スイッチとの衝突判定
     * @details プレイヤーとスイッチの衝突を判定します。
     * 
     * @param gameState ゲーム状態
     * @param playerPos プレイヤー位置
     * @param playerSize プレイヤーサイズ
     * @return 衝突した場合true
     */
    static bool checkSwitchCollision(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize);
};
