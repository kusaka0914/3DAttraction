/**
 * @file json_stage_loader.h
 * @brief JSONステージローダー
 * @details JSONファイルからステージデータを読み込み、保存する機能を提供します。
 */
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state.h"
#include "platform_system.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

/**
 * @brief JSONステージローダー
 * @details JSONファイルからステージデータを読み込み、保存する機能を提供します。
 */
class JsonStageLoader {
public:
    /**
     * @brief JSONファイルからステージを読み込む
     * @details JSONファイルからステージデータを読み込み、ゲーム状態とプラットフォームシステムに適用します。
     * 
     * @param filename ファイル名
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @return 読み込み成功時true
     */
    static bool loadStageFromJSON(const std::string& filename, GameState& gameState, PlatformSystem& platformSystem);
    
    /**
     * @brief ステージ情報のみを読み込む
     * @details プレイヤー位置、ゴール位置、制限時間などのステージ情報のみを読み込みます。
     * 
     * @param filename ファイル名
     * @param gameState ゲーム状態
     * @return 読み込み成功時true
     */
    static bool loadStageInfoFromJSON(const std::string& filename, GameState& gameState);
    
    /**
     * @brief ステージをJSONファイルに保存する
     * @details エディタ用にステージデータをJSONファイルに保存します。
     * 
     * @param filename ファイル名
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @param stageNumber ステージ番号
     * @return 保存成功時true
     */
    static bool saveStageToJSON(const std::string& filename, const GameState& gameState, 
                                const PlatformSystem& platformSystem, int stageNumber);
    
private:
    /**
     * @brief ファイルが存在するか確認する
     * @param filename ファイル名
     * @return 存在する場合true
     */
    static bool fileExists(const std::string& filename);
    
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
