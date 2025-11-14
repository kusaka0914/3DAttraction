#pragma once

#include "game_state.h"

/**
 * @brief 大砲システム
 * @details 大砲の更新と衝突判定を管理します。
 */
class CannonSystem {
public:
    /**
     * @brief 大砲を更新する
     * @details 全大砲の状態を更新します。
     * 
     * @param gameState ゲーム状態
     * @param deltaTime デルタタイム
     */
    static void updateCannons(GameState& gameState, float deltaTime);
    
    /**
     * @brief 大砲との衝突判定
     * @details プレイヤーと大砲の衝突を判定します。
     * 
     * @param gameState ゲーム状態
     * @param playerPosition プレイヤー位置
     * @param playerSize プレイヤーサイズ
     */
    static void checkCannonCollision(GameState& gameState, const glm::vec3& playerPosition, const glm::vec3& playerSize);
};
