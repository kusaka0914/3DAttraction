#include "game_state.h"

// ゲーム状態の初期化関数
void initializeGameState(GameState& gameState) {
    // デフォルトの初期化は構造体のデフォルト値を使用
    // 必要に応じて追加の初期化処理をここに記述
    
    // 例：敵リストのクリア
    gameState.enemies.clear();
    
    // 例：足場リストのクリア
    gameState.platforms.clear();
    
    // 例：重力ゾーンのクリア
    gameState.gravityZones.clear();
    
    // 例：スイッチのクリア
    gameState.switches.clear();
    
    // 例：大砲のクリア
    gameState.cannons.clear();
    
    // ゴール後の移動制限の初期化
    gameState.isGoalReached = false;
    
    // クリアタイムの初期化
    gameState.clearTime = 0.0f;
    
    // Ready画面システムの初期化
    gameState.showReadyScreen = false;
    gameState.readyScreenShown = false;
    gameState.readyScreenSpeedLevel = 0;
    gameState.isCountdownActive = false;
    gameState.countdownTimer = 3.0f;
    
    // ステージ解放状態の初期化
    gameState.unlockedStages[0] = true;  // ステージ0（ステージ選択フィールド）は常に解放済み
    gameState.unlockedStages[1] = false; // ステージ1はロック済み（1スター必要）
    gameState.unlockedStages[2] = false; // ステージ2-5は未解放
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
    gameState.showStage0Tutorial = true; // 初回は表示
}

