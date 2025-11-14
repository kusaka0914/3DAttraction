#pragma once

/**
 * @brief スキルシステムの状態を管理する構造体
 */
struct SkillState {
    bool hasDoubleJumpSkill = true;
    int doubleJumpMaxUses = 3;
    int doubleJumpRemainingUses = 3;
    
    bool hasHeartFeelSkill = false;
    int heartFeelMaxUses = 3;
    int heartFeelRemainingUses = 3;
    
    bool hasFreeCameraSkill = false;
    bool isFreeCameraActive = false;
    float freeCameraDuration = 20.0f;
    float freeCameraTimer = 0.0f;
    int freeCameraMaxUses = 3;
    int freeCameraRemainingUses = 3;
    float freeCameraYaw = 90.0f;
    float freeCameraPitch = 0.0f;
    
    bool hasBurstJumpSkill = true;
    bool isBurstJumpActive = false;
    bool hasUsedBurstJump = false;
    bool isInBurstJumpAir = false;
    float burstJumpDelayTimer = 0.0f;
    int burstJumpMaxUses = 3;
    int burstJumpRemainingUses = 3;
    
    bool hasTimeStopSkill = false;
    bool isTimeStopped = false;
    float timeStopDuration = 5.0f;
    float timeStopTimer = 0.0f;
    int timeStopMaxUses = 3;
    int timeStopRemainingUses = 3;
};

