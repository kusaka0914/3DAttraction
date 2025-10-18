#pragma once

#include "game_state.h"
#include "platform_system.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class JsonStageLoader {
public:
    // JSONファイルからステージを読み込み
    static bool loadStageFromJSON(const std::string& filename, GameState& gameState, PlatformSystem& platformSystem);
    
    // ステージ情報のみを読み込み（プレイヤー位置、ゴール位置、制限時間など）
    static bool loadStageInfoFromJSON(const std::string& filename, GameState& gameState);
    
private:
    // JSONファイルの存在確認
    static bool fileExists(const std::string& filename);
    
    // 各セクションの解析
    static bool parseStageInfo(const nlohmann::json& root, GameState& gameState);
    static bool parseItems(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseStaticPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parsePatrolPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseMovingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseRotatingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseDisappearingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseConsecutiveCyclingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseFlyingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseTeleportPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseJumpPads(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseStageSelectionAreas(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseConditionalCyclingDisappearingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseConditionalPatrolPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
    static bool parseConditionalFlyingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem);
};
