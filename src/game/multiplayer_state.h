/**
 * @file multiplayer_state.h
 * @brief マルチプレイの状態を管理する構造体
 * @details ネットワーク通信、プレイヤー状態、レース進行などのマルチプレイ関連の状態を保持します。
 */
#pragma once

#include <glm/glm.hpp>
#include "player_state.h"

/**
 * @brief マルチプレイの状態を管理する構造体
 * @details ネットワーク通信、プレイヤー状態、レース進行などのマルチプレイ関連の状態を保持します。
 */
struct MultiplayerState {
    bool isMultiplayerMode = false;  /**< @brief マルチプレイモードかどうか */
    bool isHost = false;  /**< @brief ホストかどうか */
    bool isConnected = false;  /**< @brief 接続中かどうか */
    
    // プレイヤー状態（2人分）
    PlayerState localPlayer;  /**< @brief ローカルプレイヤーの状態 */
    PlayerState remotePlayer;  /**< @brief リモートプレイヤーの状態 */
    
    // レース状態
    bool isRaceStarted = false;  /**< @brief レース開始済みかどうか */
    bool isRaceFinished = false;  /**< @brief レース終了済みかどうか */
    int winnerPlayerId = -1;  /**< @brief 勝者プレイヤーID（-1=未決定、0=ローカル、1=リモート） */
    float winnerTime = 0.0f;  /**< @brief 勝者のタイム（秒） */
    float loserTime = 0.0f;  /**< @brief 敗者のタイム（秒） */
    
    // リモートプレイヤーの補間用
    glm::vec3 remotePlayerTargetPosition;  /**< @brief リモートプレイヤーの目標位置（補間用） */
    glm::vec3 remotePlayerTargetVelocity;  /**< @brief リモートプレイヤーの目標速度（補間用） */
    float remotePlayerInterpolationTimer = 0.0f;  /**< @brief リモートプレイヤーの補間タイマー（秒） */
    const float REMOTE_PLAYER_INTERPOLATION_TIME = 0.1f;  /**< @brief リモートプレイヤーの補間時間（秒） */
    
    // ステージ選択通知用
    int pendingStageSelection = -1;  /**< @brief 送信待ちのステージ番号（-1=なし、1-5=ステージ番号） */
};

