/**
 * @file replay_state.h
 * @brief リプレイシステムの状態を管理する構造体
 * @details リプレイの記録、再生、一時停止、速度制御などの状態を保持します。
 */
#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

/**
 * @brief リプレイフレーム
 * @details リプレイの1フレーム分のデータを保持します。
 */
struct ReplayFrame {
    float timestamp;
    glm::vec3 playerPosition;
    glm::vec3 playerVelocity;
    std::vector<bool> itemCollectedStates;
};

struct ReplayData {
    int stageNumber;
    float clearTime;
    std::vector<ReplayFrame> frames;
    std::string recordedDate;
    float frameRate;
};

struct ReplayState {
    bool isRecordingReplay = false;
    std::vector<ReplayFrame> replayBuffer;
    float replayRecordTimer = 0.0f;
    const float REPLAY_RECORD_INTERVAL = 0.1f;
    
    bool isReplayMode = false;
    ReplayData currentReplay;
    float replayPlaybackTime = 0.0f;
    bool isReplayPaused = false;
    float replayPlaybackSpeed = 1.0f;
    
    bool pendingReplayLoad = false;  /**< @brief リプレイ読み込み待ちフラグ */
    int pendingReplayStage = 0;  /**< @brief 読み込み待ちのリプレイステージ番号 */
};

