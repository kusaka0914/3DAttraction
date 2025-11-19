#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state.h"

void initializeGameState(GameState& gameState) {
    // マルチプレイ状態の初期化
    gameState.multiplayer.isMultiplayerMode = false;
    gameState.multiplayer.isHost = false;
    gameState.multiplayer.isConnected = false;
    gameState.multiplayer.isRaceStarted = false;
    gameState.multiplayer.isRaceFinished = false;
    gameState.multiplayer.winnerPlayerId = -1;
    gameState.multiplayer.winnerTime = 0.0f;
    gameState.multiplayer.loserTime = 0.0f;
    gameState.multiplayer.localPlayer = PlayerState();
    gameState.multiplayer.remotePlayer = PlayerState();
    gameState.multiplayer.remotePlayer.color = glm::vec3(0.2f, 0.8f, 1.0f); // リモートプレイヤーは青系の色    
    gameState.platforms.clear();
    gameState.gravityZones.clear();
    gameState.switches.clear();
    gameState.cannons.clear(); 
    gameState.progress.isGoalReached = false;
    gameState.progress.clearTime = 0.0f;

    gameState.ui.showReadyScreen = false;
    gameState.ui.readyScreenShown = false;
    gameState.ui.readyScreenSpeedLevel = 0;
    gameState.ui.isCountdownActive = false;
    gameState.ui.countdownTimer = 3.0f;
    
    gameState.progress.unlockedStages[0] = true; 
    gameState.progress.unlockedStages[1] = false;
    gameState.progress.unlockedStages[2] = false;
    gameState.progress.unlockedStages[3] = false;
    gameState.progress.unlockedStages[4] = false;
    gameState.progress.unlockedStages[5] = false;
    
    gameState.ui.showUnlockConfirmUI = false;
    gameState.ui.unlockTargetStage = 0;
    gameState.ui.unlockRequiredStars = 0;
    
    gameState.ui.showStarInsufficientUI = false;
    gameState.ui.insufficientTargetStage = 0;
    gameState.ui.insufficientRequiredStars = 0;
    
    gameState.ui.showStageSelectionAssist = false;
    gameState.ui.assistTargetStage = 0;
    
    gameState.ui.showLeaderboardAssist = false;
    gameState.ui.showLeaderboardUI = false;
    gameState.ui.leaderboardTargetStage = 0;
    gameState.ui.leaderboardEntries.clear();
    gameState.ui.isLoadingLeaderboard = false;
    gameState.ui.leaderboardRetryTimer = 0.0f;
    gameState.ui.leaderboardRetryCount = 0;
    
    gameState.ui.showStage0Tutorial = true;
    
    gameState.player.isMovingBackward = false;
}

