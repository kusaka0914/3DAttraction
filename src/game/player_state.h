/**
 * @file player_state.h
 * @brief プレイヤーの状態を管理する構造体
 * @details プレイヤーの位置、速度、チェックポイント、プラットフォーム追従などの状態を保持します。
 */
#pragma once

#include <glm/glm.hpp>

/**
 * @brief プレイヤーの状態を管理する構造体
 * @details プレイヤーの位置、速度、チェックポイント、プラットフォーム追従などの状態を保持します。
 */
struct PlayerState {
    glm::vec3 position = glm::vec3(0, 30.0f, 0);
    glm::vec3 velocity = glm::vec3(0, 0, 0);  // 速度（単位: ワールド座標/秒）
    glm::vec3 color = glm::vec3(1, 0.8f, 0.9f);
    bool isMovingBackward = false;
    bool isShowingFrontTexture = false;
    
    int currentPlatformIndex = -1;  // -1の場合は足場の上にいない
    
    glm::vec3 lastCheckpoint = glm::vec3(0, 30.0f, 0);
    int lastCheckpointItemId = -1;  // -1の場合はチェックポイント未通過
    
    bool canDoubleJump = true;
    glm::vec3 lastPlatformPosition = glm::vec3(0, 0, 0);
    int lastPlatformIndex = -1;  // -1の場合は前回の足場なし
    bool isTrackingPlatform = false;
};

