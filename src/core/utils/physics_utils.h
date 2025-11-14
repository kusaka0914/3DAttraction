#pragma once

#include <glm/glm.hpp>
#include "../constants/game_constants.h"

struct GameState;

/**
 * @brief 物理ユーティリティ
 * @details 物理演算に関するユーティリティ関数を提供します。
 */
namespace PhysicsUtils {

/**
 * @brief 重力方向に応じてプレイヤー位置を調整する
 * @details プラットフォームとの衝突時に、重力方向に応じてプレイヤー位置を調整します。
 * 
 * @param gameState ゲーム状態
 * @param platformPosition プラットフォーム位置
 * @param platformSize プラットフォームサイズ
 * @param playerSize プレイヤーサイズ
 * @param gravityDirection 重力方向
 */
void adjustPlayerPositionForGravity(GameState& gameState, const glm::vec3& platformPosition, 
                                   const glm::vec3& platformSize, const glm::vec3& playerSize, 
                                   const glm::vec3& gravityDirection);

/**
 * @brief 速度倍率に応じた重力強度を計算する
 * @details タイムスケールを考慮した重力強度を計算します。
 * 
 * @param baseGravity 基本重力
 * @param deltaTime デルタタイム
 * @param timeScale タイムスケール
 * @param gravityDirection 重力方向
 * @param gameState ゲーム状態
 * @return 計算された重力強度
 */
float calculateGravityStrength(float baseGravity, float deltaTime, float timeScale, 
                              const glm::vec3& gravityDirection, GameState& gameState);

} // namespace PhysicsUtils
