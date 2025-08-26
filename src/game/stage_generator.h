#pragma once

#include "game_state.h"
#include "platform_system.h"

// ステージ生成システム
class StageGenerator {
public:
    // 全ステージ要素の生成（新しい設計）
    static void generateTerrain(GameState& gameState, PlatformSystem& platformSystem);
    
    // 個別の要素生成
    static void generatePlatforms(GameState& gameState, PlatformSystem& platformSystem);
    static void generateGravityZones(GameState& gameState);
    static void generateSwitches(GameState& gameState);
    static void generateCannons(GameState& gameState);
    
    // 敵の初期化
    static void initializeEnemies(GameState& gameState);
    
    // 古い設計との互換性
    static void generateTerrain(GameState& gameState);
};
