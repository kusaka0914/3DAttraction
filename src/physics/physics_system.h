#pragma once

#include "../game/game_state.h"

/**
 * @brief 物理システム
 * @details 物理演算と衝突判定を統合的に管理します。
 */
class PhysicsSystem {
public:
    /**
     * @brief 重力反転エリアのチェック
     * @details プレイヤーが重力反転エリア内にいるか確認し、重力方向を更新します。
     * 
     * @param gameState ゲーム状態
     * @param playerPos プレイヤー位置
     * @param gravityDirection 出力: 重力方向
     * @return 重力反転エリア内の場合true
     */
    static bool isPlayerInGravityZone(const GameState& gameState, const glm::vec3& playerPos, glm::vec3& gravityDirection);
    
    /**
     * @brief 足場衝突判定（水平方向）
     * @details プレイヤーと足場の水平方向の衝突を判定します。
     * 
     * @param gameState ゲーム状態
     * @param playerPos プレイヤー位置
     * @param playerSize プレイヤーサイズ
     * @return 衝突している場合true
     */
    static bool checkPlatformCollisionHorizontal(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize);
    
    /**
     * @brief 足場衝突判定（重力方向考慮）
     * @details 重力方向を考慮した足場衝突判定を行います。
     * 
     * @param gameState ゲーム状態
     * @param playerPos プレイヤー位置
     * @param playerSize プレイヤーサイズ
     * @param gravityDirection 重力方向
     * @return 衝突したプラットフォームへのポインタ（衝突しない場合はnullptr）
     */
    static GameState::Platform* checkPlatformCollisionWithGravity(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection);
    
    /**
     * @brief 足場衝突判定（垂直方向）
     * @details プレイヤーと足場の垂直方向の衝突を判定します。
     * 
     * @param gameState ゲーム状態
     * @param playerPos プレイヤー位置
     * @param playerSize プレイヤーサイズ
     * @return 衝突したプラットフォームへのポインタ（衝突しない場合はnullptr）
     */
    static GameState::Platform* checkPlatformCollisionVertical(GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize);
    
    /**
     * @brief プレイヤーが足場の上にいるか判定（重力方向考慮）
     * @details 重力方向を考慮してプレイヤーが足場の上にいるか判定します。
     * 
     * @param platform プラットフォーム
     * @param playerPos プレイヤー位置
     * @param playerSize プレイヤーサイズ
     * @param gravityDirection 重力方向
     * @return 足場の上にいる場合true
     */
    static bool isPlayerOnPlatformWithGravity(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection);
    
    /**
     * @brief プレイヤーが足場の上にいるか判定（移動用、重力方向考慮）
     * @details 移動処理用に重力方向を考慮してプレイヤーが足場の上にいるか判定します。
     * 
     * @param platform プラットフォーム
     * @param playerPos プレイヤー位置
     * @param playerSize プレイヤーサイズ
     * @param gravityDirection 重力方向
     * @return 足場の上にいる場合true
     */
    static bool isPlayerOnPlatformWithGravityForMovement(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize, const glm::vec3& gravityDirection);
    
    /**
     * @brief プレイヤーが足場の上にいるか判定
     * @details プレイヤーが足場の上にいるか判定します。
     * 
     * @param gameState ゲーム状態
     * @param playerPos プレイヤー位置
     * @param playerSize プレイヤーサイズ
     * @return 足場の上にいる場合true
     */
    static bool isPlayerOnPlatform(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize);
    
    /**
     * @brief プレイヤーが回転足場の上にいるか判定
     * @details 回転している足場の上にプレイヤーがいるか判定します。
     * 
     * @param platform プラットフォーム
     * @param playerPos プレイヤー位置
     * @param playerSize プレイヤーサイズ
     * @return 回転足場の上にいる場合true
     */
    static bool isPlayerOnRotatedPlatform(const GameState::Platform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize);
    
    /**
     * @brief 点を軸の周りに回転させる
     * @details 指定された軸の周りに点を回転させます。
     * 
     * @param point 回転させる点
     * @param axis 回転軸
     * @param angle 回転角度
     * @param center 回転中心
     * @return 回転後の点
     */
    static glm::vec3 rotatePointAroundAxis(const glm::vec3& point, const glm::vec3& axis, float angle, const glm::vec3& center);
    
    /**
     * @brief 回転足場の角を取得する
     * @details 回転している足場の8つの角の座標を取得します。
     * 
     * @param platform プラットフォーム
     * @param corners 出力: 8つの角の座標
     */
    static void getRotatedPlatformCorners(const GameState::Platform& platform, glm::vec3 corners[8]);
    
    /**
     * @brief 回転足場の上面の角を取得する
     * @details 回転している足場の上面の4つの角の座標を取得します。
     * 
     * @param platform プラットフォーム
     * @param topCorners 出力: 4つの角の座標
     */
    static void getRotatedPlatformTopCorners(const GameState::Platform& platform, glm::vec3 topCorners[4]);
    
    /**
     * @brief 回転足場の下面の角を取得する
     * @details 回転している足場の下面の4つの角の座標を取得します。
     * 
     * @param platform プラットフォーム
     * @param bottomCorners 出力: 4つの角の座標
     */
    static void getRotatedPlatformBottomCorners(const GameState::Platform& platform, glm::vec3 bottomCorners[4]);
};
