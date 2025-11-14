#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

/**
 * @brief リプレイシステムの状態を管理する構造体
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
};

