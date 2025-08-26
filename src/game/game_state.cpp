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
}

