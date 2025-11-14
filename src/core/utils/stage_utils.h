/**
 * @file stage_utils.h
 * @brief ステージユーティリティ
 * @details ステージ生成用の設定構造体とユーティリティ関数を提供します。
 */
#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <initializer_list>
#include <tuple>

struct GameState;
class PlatformSystem;

/**
 * @brief アイテム設定の構造体
 * @details アイテムの位置、色、説明を保持します。
 */
struct ItemConfig {
    glm::vec3 position;
    glm::vec3 color;
    std::string description;
};

/**
 * @brief 静的プラットフォーム設定の構造体
 * @details 静的プラットフォームの位置、サイズ、色、説明を保持します。
 */
struct StaticConfig {
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    std::string description;
};

using StaticPlatformConfig = StaticConfig;

/**
 * @brief 巡回プラットフォーム設定の構造体
 * @details 巡回プラットフォームの巡回ポイントと説明を保持します。
 */
struct PatrolConfig {
    std::vector<glm::vec3> points;
    std::string description;
};

using PatrolPlatformConfig = PatrolConfig;

/**
 * @brief 移動プラットフォーム設定の構造体
 * @details 移動プラットフォームの開始位置、終了位置、サイズ、色、速度、説明を保持します。
 */
struct MovingConfig {
    glm::vec3 startPosition;
    glm::vec3 endPosition;
    glm::vec3 size;
    glm::vec3 color;
    float speed;
    std::string description;
};

/**
 * @brief サイクル消失プラットフォーム設定の構造体
 * @details サイクル消失プラットフォームの位置、サイズ、色、表示時間、非表示時間、初期タイマー、説明を保持します。
 */
struct CyclingDisappearingConfig {
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    float visibleTime;
    float invisibleTime;
    float initialTimer;
    std::string description;
};

/**
 * @brief ステージユーティリティ
 * @details ステージ生成用のユーティリティ関数を提供します。
 */
namespace StageUtils {

/**
 * @brief アイテムを初期化する
 * @param gameState ゲーム状態
 */
void initializeItems(GameState& gameState);

/**
 * @brief 設定からアイテムを作成する
 * @param gameState ゲーム状態
 * @param itemConfigs アイテム設定のベクター
 */
void createItemsFromConfig(GameState& gameState, const std::vector<ItemConfig>& itemConfigs);

/**
 * @brief 設定でアイテムを初期化する
 * @param gameState ゲーム状態
 * @param itemConfigs アイテム設定のベクター
 */
void initializeItemsWithConfig(GameState& gameState, const std::vector<ItemConfig>& itemConfigs);

/**
 * @brief アイテムプラットフォームを作成する
 * @param platformSystem プラットフォームシステム
 * @param itemConfigs アイテム設定のベクター
 */
void createItemPlatforms(PlatformSystem& platformSystem, const std::vector<ItemConfig>& itemConfigs);

/**
 * @brief 可変長のアイテムを生成する
 * @param gameState ゲーム状態
 * @param platformSystem プラットフォームシステム
 * @param items アイテムの初期化リスト
 */
void createItems(GameState& gameState, PlatformSystem& platformSystem,
                std::initializer_list<std::tuple<std::tuple<float, float, float>, glm::vec3, std::string>> items);

/**
 * @brief 静的プラットフォームを生成する
 * @param gameState ゲーム状態
 * @param platformSystem プラットフォームシステム
 * @param platforms プラットフォームの初期化リスト
 */
void createStaticPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, glm::vec3, std::string>> platforms);

/**
 * @brief 設定から静的プラットフォームを生成する
 * @param gameState ゲーム状態
 * @param platformSystem プラットフォームシステム
 * @param configs プラットフォーム設定のベクター
 */
void createStaticPlatformsFromConfig(GameState& gameState, PlatformSystem& platformSystem, const std::vector<StaticPlatformConfig>& configs);

/**
 * @brief 設定から巡回プラットフォームを生成する
 * @param gameState ゲーム状態
 * @param platformSystem プラットフォームシステム
 * @param configs 巡回プラットフォーム設定のベクター
 */
void createPatrolPlatformsFromConfig(GameState& gameState, PlatformSystem& platformSystem, const std::vector<PatrolPlatformConfig>& configs);

/**
 * @brief 巡回プラットフォームを生成する
 * @param platformSystem プラットフォームシステム
 * @param patrolConfigs 巡回設定のベクター
 */
void createPatrolPlatforms(PlatformSystem& platformSystem, const std::vector<PatrolConfig>& patrolConfigs);

/**
 * @brief 巡回プラットフォームを生成する
 * @param gameState ゲーム状態
 * @param platformSystem プラットフォームシステム
 * @param paths 巡回パスのベクター
 * @param baseDescription 基本説明
 */
void createPatrolPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          const std::vector<std::vector<glm::vec3>>& paths,
                          const std::string& baseDescription = "巡回足場");

/**
 * @brief 巡回プラットフォームを生成する
 * @param gameState ゲーム状態
 * @param platformSystem プラットフォームシステム
 * @param patrols 巡回の初期化リスト
 */
void createPatrolPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::vector<std::tuple<float, float, float>>, std::string>> patrols);

/**
 * @brief 移動プラットフォームを生成する
 * @param gameState ゲーム状態
 * @param platformSystem プラットフォームシステム
 * @param platforms プラットフォームの初期化リスト
 */
void createMovingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, glm::vec3, float, std::string>> platforms);

/**
 * @brief 飛来プラットフォームを生成する
 * @param gameState ゲーム状態
 * @param platformSystem プラットフォームシステム
 * @param platforms プラットフォームのベクター
 */
void createFlyingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          const std::vector<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, float, float, std::string>>& platforms);

/**
 * @brief サイクル消失プラットフォームを生成する
 * @param gameState ゲーム状態
 * @param platformSystem プラットフォームシステム
 * @param configs サイクル消失プラットフォーム設定のベクター
 */
void createCyclingDisappearingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                       const std::vector<CyclingDisappearingConfig>& configs);

/**
 * @brief 連続サイクル消失プラットフォームを生成する
 * @param gameState ゲーム状態
 * @param platformSystem プラットフォームシステム
 * @param platforms プラットフォームの初期化リスト
 */
void createConsecutiveCyclingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                      std::initializer_list<std::tuple<std::tuple<float, float, float>, int, float, std::tuple<float, float, float>, glm::vec3, float, float, float, float, std::tuple<float, float, float>, bool>> platforms);

/**
 * @brief 連続サイクル消失プラットフォームを生成する
 * @param gameState ゲーム状態
 * @param platformSystem プラットフォームシステム
 * @param platforms プラットフォームのベクター
 */
void createConsecutiveCyclingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                      const std::vector<std::tuple<std::tuple<float, float, float>, int, float, std::tuple<float, float, float>, glm::vec3, float, float, float, float, std::tuple<float, float, float>, bool>>& platforms);

} // namespace StageUtils
