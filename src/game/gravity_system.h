/**
 * @file gravity_system.h
 * @brief 重力システム
 * @details 重力反転エリアの更新を管理します。
 */
#pragma once

#include "game_state.h"

/**
 * @brief 重力システム
 * @details 重力反転エリアの更新を管理します。
 */
class GravitySystem {
public:
    /**
     * @brief 重力反転エリアを更新する
     * @details 全重力反転エリアの状態を更新します。
     * 
     * @param gameState ゲーム状態
     * @param deltaTime デルタタイム
     */
    static void updateGravityZones(GameState& gameState, float deltaTime);
};
