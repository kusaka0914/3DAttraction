#include "stage_utils.h"
#include "../../game/game_state.h"
#include "../../game/platform_system.h"
#include <iostream>

namespace StageUtils {

void initializeItems(GameState& gameState) {
    gameState.items.clear();
    gameState.collectedItems = 0;
}

void createItemsFromConfig(GameState& gameState, const std::vector<ItemConfig>& itemConfigs) {
    for (int i = 0; i < itemConfigs.size(); i++) {
        GameState::Item item;
        item.position = itemConfigs[i].position + glm::vec3(0, 1, 0);
        item.size = glm::vec3(0.5f, 0.5f, 0.5f);
        item.color = itemConfigs[i].color;
        item.isCollected = false;
        item.itemId = i + 1;
        item.rotationAngle = 0.0f;
        item.bobHeight = 0.0f;
        item.bobTimer = 0.0f;
        gameState.items.push_back(item);
        
        printf("Created %s at position (%.1f, %.1f, %.1f)\n", 
               itemConfigs[i].description.c_str(), 
               item.position.x, item.position.y, item.position.z);
    }
}

void initializeItemsWithConfig(GameState& gameState, const std::vector<ItemConfig>& itemConfigs) {
    initializeItems(gameState);
    createItemsFromConfig(gameState, itemConfigs);
}

void createItemPlatforms(PlatformSystem& platformSystem, const std::vector<ItemConfig>& itemConfigs) {
    for (int i = 0; i < itemConfigs.size(); i++) {
        platformSystem.addPlatform(GameState::StaticPlatform(
            itemConfigs[i].position, glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.6f, 1.0f)
        ));
    }
}

void createItems(GameState& gameState, PlatformSystem& platformSystem,
                std::initializer_list<std::tuple<std::tuple<float, float, float>, glm::vec3, std::string>> items) {
    std::vector<ItemConfig> itemConfigs;
    for (const auto& item : items) {
        const auto& pos = std::get<0>(item);
        itemConfigs.push_back({
            glm::vec3(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos)),
            std::get<1>(item),
            std::get<2>(item)
        });
    }
    initializeItemsWithConfig(gameState, itemConfigs);
    createItemPlatforms(platformSystem, itemConfigs);
}

void createStaticPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, glm::vec3, std::string>> platforms) {
    for (const auto& platform : platforms) {
        const auto& pos = std::get<0>(platform);
        const auto& size = std::get<1>(platform);
        glm::vec3 position = glm::vec3(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        glm::vec3 color = std::get<2>(platform);
        std::string description = std::get<3>(platform);
        
        platformSystem.addPlatform(GameState::StaticPlatform(position, sizeVec, color));
        printf("Created %s at position (%.1f, %.1f, %.1f)\n", 
               description.c_str(), position.x, position.y, position.z);
    }
}

void createPatrolPlatforms(PlatformSystem& platformSystem, const std::vector<PatrolConfig>& patrolConfigs) {
    for (const auto& config : patrolConfigs) {
        platformSystem.addPlatform(GameState::PatrollingPlatform(
            config.points[0], glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.2f, 0.8f, 0.8f),
            config.points, 2.0f
        ));
        printf("Created %s with %zu patrol points\n", config.description.c_str(), config.points.size());
    }
}

void createPatrolPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          const std::vector<std::vector<glm::vec3>>& paths,
                          const std::string& baseDescription) {
    std::vector<PatrolConfig> patrolConfigs;
    patrolConfigs.reserve(paths.size());
    for (size_t i = 0; i < paths.size(); i++) {
        patrolConfigs.push_back({ paths[i], baseDescription + std::to_string(i + 1) });
    }
    createPatrolPlatforms(platformSystem, patrolConfigs);
}

void createPatrolPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::vector<std::tuple<float, float, float>>, std::string>> patrols) {
    for (const auto& patrol : patrols) {
        const auto& points = std::get<0>(patrol);
        std::string description = std::get<1>(patrol);
        
        std::vector<glm::vec3> glmPoints;
        for (const auto& point : points) {
            glmPoints.push_back(glm::vec3(std::get<0>(point), std::get<1>(point), std::get<2>(point)));
        }
        
        platformSystem.addPlatform(GameState::PatrollingPlatform(
            glmPoints[0], glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.2f, 0.8f, 0.8f),
            glmPoints, 2.0f
        ));
        printf("Created %s with %zu patrol points\n", description.c_str(), glmPoints.size());
    }
}

void createMovingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, glm::vec3, float, std::string>> platforms) {
    for (const auto& platform : platforms) {
        const auto& startPos = std::get<0>(platform);
        const auto& endPos = std::get<1>(platform);
        const auto& size = std::get<2>(platform);
        glm::vec3 startPosition = glm::vec3(std::get<0>(startPos), std::get<1>(startPos), std::get<2>(startPos));
        glm::vec3 endPosition = glm::vec3(std::get<0>(endPos), std::get<1>(endPos), std::get<2>(endPos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        glm::vec3 color = std::get<3>(platform);
        float speed = std::get<4>(platform);
        std::string description = std::get<5>(platform);
        
        platformSystem.addPlatform(GameState::MovingPlatform(startPosition, sizeVec, color, endPosition, speed));
        printf("Created %s from (%.1f, %.1f, %.1f) to (%.1f, %.1f, %.1f)\n", 
               description.c_str(), startPosition.x, startPosition.y, startPosition.z,
               endPosition.x, endPosition.y, endPosition.z);
    }
}

void createFlyingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          const std::vector<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, float, float, std::string>>& platforms) {
    for (const auto& platform : platforms) {
        const auto& pos = std::get<0>(platform);
        const auto& size = std::get<1>(platform);
        const auto& spawn = std::get<2>(platform);
        const auto& target = std::get<3>(platform);
        glm::vec3 position = glm::vec3(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        glm::vec3 spawnPosition = glm::vec3(std::get<0>(spawn), std::get<1>(spawn), std::get<2>(spawn));
        glm::vec3 targetPosition = glm::vec3(std::get<0>(target), std::get<1>(target), std::get<2>(target));
        glm::vec3 color = glm::vec3(0.8f, 0.2f, 0.2f);
        float speed = std::get<4>(platform);
        float range = std::get<5>(platform);
        std::string description = std::get<6>(platform);
        
        platformSystem.addPlatform(GameState::FlyingPlatform(position, sizeVec, color, spawnPosition, targetPosition, speed, range));
        printf("Created %s at (%.1f, %.1f, %.1f) with spawn at (%.1f, %.1f, %.1f)\n", 
               description.c_str(), position.x, position.y, position.z,
               spawnPosition.x, spawnPosition.y, spawnPosition.z);
    }
}

void createCyclingDisappearingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                       const std::vector<CyclingDisappearingConfig>& configs) {
    for (const auto& config : configs) {
        platformSystem.addPlatform(GameState::CycleDisappearingPlatform(
            config.position, config.size, config.color,
            config.visibleTime, config.invisibleTime, config.initialTimer
        ));
        printf("Created %s at (%.1f, %.1f, %.1f) with cycle %.1fs visible, %.1fs invisible\n", 
               config.description.c_str(),
               config.position.x, config.position.y, config.position.z,
               config.visibleTime, config.invisibleTime);
    }
}

void createConsecutiveCyclingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                      std::initializer_list<std::tuple<std::tuple<float, float, float>, int, float, std::tuple<float, float, float>, glm::vec3, float, float, float, float, std::tuple<float, float, float>, bool>> platforms) {
    for (const auto& platform : platforms) {
        const auto& startPos = std::get<0>(platform);
        int count = std::get<1>(platform);
        float spacing = std::get<2>(platform);
        const auto& size = std::get<3>(platform);
        glm::vec3 color = std::get<4>(platform);
        float visibleTime = std::get<5>(platform);
        float invisibleTime = std::get<6>(platform);
        float blinkTime = std::get<7>(platform);
        float delay = std::get<8>(platform);
        const auto& direction = std::get<9>(platform);
        bool reverseTimer = std::get<10>(platform);
        
        glm::vec3 startPosition = glm::vec3(std::get<0>(startPos), std::get<1>(startPos), std::get<2>(startPos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        glm::vec3 directionVec = glm::vec3(std::get<0>(direction), std::get<1>(direction), std::get<2>(direction));
        
        for (int i = 0; i < count; i++) {
            glm::vec3 position = startPosition + directionVec * (i * spacing);
            float initialTimer;
            if (reverseTimer) {
                initialTimer = (count - 1 - i) * delay;  // 逆順のタイマー
            } else {
                initialTimer = i * delay;  // 通常のタイマー
            }
            platformSystem.addPlatform(GameState::CycleDisappearingPlatform(
                position, sizeVec, color,
                visibleTime + invisibleTime, visibleTime, blinkTime, initialTimer
            ));
        }
        printf("Created %d consecutive cycling platforms from (%.1f, %.1f, %.1f) in direction (%.1f, %.1f, %.1f) with %.1f spacing (reverse timer: %s)\n",
               count, startPosition.x, startPosition.y, startPosition.z, directionVec.x, directionVec.y, directionVec.z, spacing, reverseTimer ? "true" : "false");
    }
}

} // namespace StageUtils
