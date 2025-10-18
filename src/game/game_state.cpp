#include "game_state.h"

// ゲーム状態の初期化関数
void initializeGameState(GameState& gameState) {    
    gameState.platforms.clear();
    gameState.gravityZones.clear();
    gameState.switches.clear();
    gameState.cannons.clear(); 
    gameState.isGoalReached = false;
    gameState.clearTime = 0.0f;

    // Ready画面システムの初期化
    gameState.showReadyScreen = false;
    gameState.readyScreenShown = false;
    gameState.readyScreenSpeedLevel = 0;
    gameState.isCountdownActive = false;
    gameState.countdownTimer = 3.0f;
    
    // ステージ解放状態の初期化
    gameState.unlockedStages[0] = true; 
    gameState.unlockedStages[1] = false;
    gameState.unlockedStages[2] = false;
    gameState.unlockedStages[3] = false;
    gameState.unlockedStages[4] = false;
    gameState.unlockedStages[5] = false;
    
    // ステージ解放確認UI状態の初期化
    gameState.showUnlockConfirmUI = false;
    gameState.unlockTargetStage = 0;
    gameState.unlockRequiredStars = 0;
    
    // 星不足警告UI状態の初期化
    gameState.showStarInsufficientUI = false;
    gameState.insufficientTargetStage = 0;
    gameState.insufficientRequiredStars = 0;
    
    // 操作アシストUI状態の初期化
    gameState.showStageSelectionAssist = false;
    gameState.assistTargetStage = 0;
    
    // 初回ステージ0入場チュートリアルUI状態の初期化
    gameState.showStage0Tutorial = true;
    
    // プレイヤー移動状態の初期化
    gameState.isMovingBackward = false;
}

