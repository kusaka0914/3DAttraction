#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <initializer_list>
#include <tuple>

// 前方宣言
struct GameState;
class PlatformSystem;

// 共通で使用する構造体を定義
struct ItemConfig {
    glm::vec3 position;
    glm::vec3 color;
    std::string description;
};

struct StaticConfig {
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    std::string description;
};

struct PatrolConfig {
    std::vector<glm::vec3> points;
    std::string description;
};

struct MovingConfig {
    glm::vec3 startPosition;
    glm::vec3 endPosition;
    glm::vec3 size;
    glm::vec3 color;
    float speed;
    std::string description;
};

struct CyclingDisappearingConfig {
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    float visibleTime;
    float invisibleTime;
    float initialTimer;
    std::string description;
};

namespace StageUtils {

// アイテム管理
void initializeItems(GameState& gameState);
void createItemsFromConfig(GameState& gameState, const std::vector<ItemConfig>& itemConfigs);
void initializeItemsWithConfig(GameState& gameState, const std::vector<ItemConfig>& itemConfigs);
void createItemPlatforms(PlatformSystem& platformSystem, const std::vector<ItemConfig>& itemConfigs);

// 可変長のアイテム生成用のヘルパー関数
void createItems(GameState& gameState, PlatformSystem& platformSystem,
                std::initializer_list<std::tuple<std::tuple<float, float, float>, glm::vec3, std::string>> items);

// プラットフォーム生成
void createStaticPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, glm::vec3, std::string>> platforms);

void createPatrolPlatforms(PlatformSystem& platformSystem, const std::vector<PatrolConfig>& patrolConfigs);
void createPatrolPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          const std::vector<std::vector<glm::vec3>>& paths,
                          const std::string& baseDescription = "巡回足場");
void createPatrolPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::vector<std::tuple<float, float, float>>, std::string>> patrols);

void createMovingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, glm::vec3, float, std::string>> platforms);

void createFlyingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          const std::vector<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, float, float, std::string>>& platforms);

void createCyclingDisappearingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                       const std::vector<CyclingDisappearingConfig>& configs);

void createConsecutiveCyclingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                      std::initializer_list<std::tuple<std::tuple<float, float, float>, int, float, std::tuple<float, float, float>, glm::vec3, float, float, float, float, std::tuple<float, float, float>, bool>> platforms);

} // namespace StageUtils
