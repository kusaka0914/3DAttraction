#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "json_stage_loader.h"
#include "../core/utils/stage_utils.h"
#include "../core/constants/debug_config.h"
#include "../core/constants/stage_constants.h"
#include "../core/error_handler.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <cstdio>
#endif

bool JsonStageLoader::fileExists(const std::string& filename) {
    printf("DEBUG: Checking if file exists: %s\n", filename.c_str());
    
    std::ifstream file(filename);
    bool exists = file.good();
    file.close();
    
    printf("DEBUG: File %s exists: %s\n", filename.c_str(), exists ? "YES" : "NO");
    return exists;
}

bool JsonStageLoader::loadStageFromJSON(const std::string& filename, GameState& gameState, PlatformSystem& platformSystem) {
    #ifdef _WIN32
        char buffer[256];
        if (GetCurrentDirectoryA(256, buffer)) {
            printf("DEBUG: Current working directory: %s\n", buffer);
        }
    #else
        FILE* pipe = popen("pwd", "r");
        if (pipe) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), pipe)) {
                printf("DEBUG: Current working directory: %s", buffer);
            }
            pclose(pipe);
        }
    #endif
    if (!fileExists(filename)) {
        ErrorHandler::logErrorFormat("Stage file not found: %s", filename.c_str());
        return false;
    }
    
    try {
        std::ifstream file(filename);
        nlohmann::json root;
        file >> root;
        
        DEBUG_PRINTF("DEBUG: Loading stage from JSON: %s\n", filename.c_str());
        
        if (root.contains("stageInfo")) {
            if (!parseStageInfo(root["stageInfo"], gameState)) {
                ErrorHandler::logError("Failed to parse stage info");
                return false;
            }
        }
        
        if (root.contains("items")) {
            if (!parseItems(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse items");
                return false;
            }
        }
        
        if (root.contains("staticPlatforms")) {
            if (!parseStaticPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse static platforms");
                return false;
            }
        }
        
        if (root.contains("patrolPlatforms")) {
            if (!parsePatrolPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse patrol platforms");
                return false;
            }
        }
        
        if (root.contains("movingPlatforms")) {
            if (!parseMovingPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse moving platforms");
                return false;
            }
        }
        
        if (root.contains("rotatingPlatforms")) {
            if (!parseRotatingPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse rotating platforms");
                return false;
            }
        }
        
        if (root.contains("disappearingPlatforms")) {
            if (!parseDisappearingPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse disappearing platforms");
                return false;
            }
        }
        
        if (root.contains("cyclingDisappearingPlatforms")) {
            printf("DEBUG: Found cyclingDisappearingPlatforms in JSON\n");
            if (!parseDisappearingPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse cycling disappearing platforms");
                return false;
            }
        } else {
            printf("DEBUG: No cyclingDisappearingPlatforms found in JSON\n");
        }
        
        if (root.contains("consecutiveCyclingPlatforms")) {
            printf("DEBUG: Found consecutiveCyclingPlatforms in JSON\n");
            if (!parseConsecutiveCyclingPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse consecutive cycling platforms");
                return false;
            }
        } else {
            printf("DEBUG: No consecutiveCyclingPlatforms found in JSON\n");
        }
        
        if (root.contains("flyingPlatforms")) {
            if (!parseFlyingPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse flying platforms");
                return false;
            }
        }
        
        if (root.contains("teleportPlatforms")) {
            if (!parseTeleportPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse teleport platforms");
                return false;
            }
        }
        
        if (root.contains("jumpPads")) {
            if (!parseJumpPads(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse jump pads");
                return false;
            }
        }
        
        if (root.contains("stageSelectionAreas")) {
            if (!parseStageSelectionAreas(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse stage selection areas");
                return false;
            }
        }
        
        if (root.contains("leaderboardArea")) {
            if (!parseLeaderboardArea(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse leaderboard area");
                return false;
            }
        }
        
        if (root.contains("conditionalCyclingDisappearingPlatforms")) {
            if (!parseConditionalCyclingDisappearingPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse conditional cycling disappearing platforms");
                return false;
            }
        }
        
        if (root.contains("conditionalPatrolPlatforms")) {
            if (!parseConditionalPatrolPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse conditional patrol platforms");
                return false;
            }
        }
        
        if (root.contains("conditionalFlyingPlatforms")) {
            if (!parseConditionalFlyingPlatforms(root, gameState, platformSystem)) {
                ErrorHandler::logError("Failed to parse conditional flying platforms");
                return false;
            }
        }
        
        DEBUG_PRINTF("DEBUG: Successfully loaded stage from JSON: %s\n", filename.c_str());
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to parse JSON file %s: %s", filename.c_str(), e.what());
        return false;
    }
}

bool JsonStageLoader::loadStageInfoFromJSON(const std::string& filename, GameState& gameState) {
    if (!fileExists(filename)) {
        ErrorHandler::logErrorFormat("Stage file not found: %s", filename.c_str());
        return false;
    }
    
    try {
        std::ifstream file(filename);
        nlohmann::json root;
        file >> root;
        
        if (root.contains("stageInfo")) {
            return parseStageInfo(root["stageInfo"], gameState);
        }
        
        return false;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to parse JSON file %s: %s", filename.c_str(), e.what());
        return false;
    }
}

bool JsonStageLoader::parseStageInfo(const nlohmann::json& stageInfo, GameState& gameState) {
    try {
        if (stageInfo.contains("playerStartPosition")) {
            auto pos = stageInfo["playerStartPosition"];
            gameState.player.position = glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            );
        }
        
        if (stageInfo.contains("goalPosition")) {
            auto pos = stageInfo["goalPosition"];
            gameState.progress.goalPosition = glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            );
        }
        
        if (stageInfo.contains("timeLimit")) {
            gameState.progress.timeLimit = stageInfo["timeLimit"].get<float>();
            gameState.progress.remainingTime = gameState.progress.timeLimit;
            printf("Time limit: %.1f seconds\n", gameState.progress.timeLimit);
        }
        
        DEBUG_PRINTF("DEBUG: Parsed stage info - Player: (%.1f, %.1f, %.1f), Goal: (%.1f, %.1f, %.1f), Time: %.1f\n",
                    gameState.player.position.x, gameState.player.position.y, gameState.player.position.z,
                    gameState.progress.goalPosition.x, gameState.progress.goalPosition.y, gameState.progress.goalPosition.z,
                    gameState.progress.timeLimit);
        
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to parse stage info: %s", e.what());
        return false;
    }
}

bool JsonStageLoader::parseItems(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    try {
        std::vector<ItemConfig> itemConfigs;
        
        for (const auto& item : root["items"]) {
            ItemConfig config;
            
            auto pos = item["position"];
            config.position = glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            );
            
            auto color = item["color"];
            config.color = glm::vec3(
                color[0].get<float>(),
                color[1].get<float>(),
                color[2].get<float>()
            );
            
            config.description = item["description"].get<std::string>();
            itemConfigs.push_back(config);
        }
        
        StageUtils::createItemsFromConfig(gameState, itemConfigs);
        StageUtils::createItemPlatforms(platformSystem, itemConfigs);
        DEBUG_PRINTF("DEBUG: Created %zu items and their platforms from JSON\n", itemConfigs.size());
        
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to parse items: %s", e.what());
        return false;
    }
}

bool JsonStageLoader::parseStaticPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    try {
        std::vector<StaticPlatformConfig> platformConfigs;
        
        for (const auto& platform : root["staticPlatforms"]) {
            StaticPlatformConfig config;
            
            auto pos = platform["position"];
            config.position = glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            );
            
            auto size = platform["size"];
            config.size = glm::vec3(
                size[0].get<float>(),
                size[1].get<float>(),
                size[2].get<float>()
            );
            
            auto color = platform["color"];
            config.color = glm::vec3(
                color[0].get<float>(),
                color[1].get<float>(),
                color[2].get<float>()
            );
            
            config.description = platform["description"].get<std::string>();
            platformConfigs.push_back(config);
        }
        
        StageUtils::createStaticPlatformsFromConfig(gameState, platformSystem, platformConfigs);
        DEBUG_PRINTF("DEBUG: Created %zu static platforms from JSON\n", platformConfigs.size());
        
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to parse static platforms: %s", e.what());
        return false;
    }
}

bool JsonStageLoader::parsePatrolPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    try {
        std::vector<PatrolPlatformConfig> platformConfigs;
        
        for (const auto& platform : root["patrolPlatforms"]) {
            PatrolPlatformConfig config;
            
            for (const auto& point : platform["patrolPoints"]) {
                glm::vec3 patrolPoint(
                    point[0].get<float>(),
                    point[1].get<float>(),
                    point[2].get<float>()
                );
                config.points.push_back(patrolPoint);
            }
            
            config.description = platform["description"].get<std::string>();
            platformConfigs.push_back(config);
        }
        
        StageUtils::createPatrolPlatformsFromConfig(gameState, platformSystem, platformConfigs);
        DEBUG_PRINTF("DEBUG: Created %zu patrol platforms from JSON\n", platformConfigs.size());
        
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to parse patrol platforms: %s", e.what());
        return false;
    }
}

bool JsonStageLoader::parseMovingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    DEBUG_PRINTF("DEBUG: Moving platforms parsing not implemented yet\n");
    return true;
}

bool JsonStageLoader::parseRotatingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    DEBUG_PRINTF("DEBUG: Rotating platforms parsing not implemented yet\n");
    return true;
}

bool JsonStageLoader::parseDisappearingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("cyclingDisappearingPlatforms")) {
        return true; // オプショナル
    }
    
    std::vector<CyclingDisappearingConfig> configs;
    for (const auto& platform : root["cyclingDisappearingPlatforms"]) {
        CyclingDisappearingConfig config;
        config.position = glm::vec3(
            platform["position"][0].get<float>(),
            platform["position"][1].get<float>(),
            platform["position"][2].get<float>()
        );
        config.size = glm::vec3(
            platform["size"][0].get<float>(),
            platform["size"][1].get<float>(),
            platform["size"][2].get<float>()
        );
        config.color = glm::vec3(
            platform["color"][0].get<float>(),
            platform["color"][1].get<float>(),
            platform["color"][2].get<float>()
        );
        config.visibleTime = platform["visibleTime"].get<float>();
        config.invisibleTime = platform["invisibleTime"].get<float>();
        config.initialTimer = platform["initialTimer"].get<float>();
        config.description = platform["description"].get<std::string>();
        configs.push_back(config);
    }
    
    StageUtils::createCyclingDisappearingPlatforms(gameState, platformSystem, configs);
    DEBUG_PRINTF("DEBUG: Created %zu cycling disappearing platforms from JSON\n", configs.size());
    return true;
}

bool JsonStageLoader::parseConsecutiveCyclingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("consecutiveCyclingPlatforms")) {
        return true; // オプショナル
    }
    
    std::vector<std::tuple<std::tuple<float, float, float>, int, float, std::tuple<float, float, float>, glm::vec3, float, float, float, float, std::tuple<float, float, float>, bool>> consecutiveConfigs;
    
    for (const auto& platform : root["consecutiveCyclingPlatforms"]) {
        glm::vec3 startPosition = glm::vec3(
            platform["startPosition"][0].get<float>(),
            platform["startPosition"][1].get<float>(),
            platform["startPosition"][2].get<float>()
        );
        int count = platform["count"].get<int>();
        float spacing = platform["spacing"].get<float>();
        glm::vec3 size = glm::vec3(
            platform["size"][0].get<float>(),
            platform["size"][1].get<float>(),
            platform["size"][2].get<float>()
        );
        glm::vec3 color = glm::vec3(
            platform["color"][0].get<float>(),
            platform["color"][1].get<float>(),
            platform["color"][2].get<float>()
        );
        float visibleTime = platform["visibleTime"].get<float>();
        float invisibleTime = platform["invisibleTime"].get<float>();
        float blinkTime = platform["blinkTime"].get<float>();
        float delay = platform["delay"].get<float>();
        glm::vec3 direction = glm::vec3(
            platform["direction"][0].get<float>(),
            platform["direction"][1].get<float>(),
            platform["direction"][2].get<float>()
        );
        bool reverseTimer = platform["reverseTimer"].get<bool>();
        
        consecutiveConfigs.push_back({
            {startPosition.x, startPosition.y, startPosition.z}, 
            count, 
            spacing, 
            {size.x, size.y, size.z}, 
            color, 
            visibleTime, invisibleTime, blinkTime, delay, 
            {direction.x, direction.y, direction.z}, 
            reverseTimer
        });
    }
    
    StageUtils::createConsecutiveCyclingPlatforms(gameState, platformSystem, consecutiveConfigs);
    DEBUG_PRINTF("DEBUG: Created %zu consecutive cycling platform groups from JSON\n", consecutiveConfigs.size());
    return true;
}

bool JsonStageLoader::parseFlyingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("flyingPlatforms")) {
        return true; // オプショナル
    }
    
    std::vector<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, float, float, std::string>> flyingPlatforms;
    
    for (const auto& platform : root["flyingPlatforms"]) {
        glm::vec3 position = glm::vec3(
            platform["position"][0].get<float>(),
            platform["position"][1].get<float>(),
            platform["position"][2].get<float>()
        );
        glm::vec3 size = glm::vec3(
            platform["size"][0].get<float>(),
            platform["size"][1].get<float>(),
            platform["size"][2].get<float>()
        );
        glm::vec3 spawnPosition = glm::vec3(
            platform["spawnPosition"][0].get<float>(),
            platform["spawnPosition"][1].get<float>(),
            platform["spawnPosition"][2].get<float>()
        );
        glm::vec3 targetPosition = glm::vec3(
            platform["targetPosition"][0].get<float>(),
            platform["targetPosition"][1].get<float>(),
            platform["targetPosition"][2].get<float>()
        );
        float speed = platform["speed"].get<float>();
        float range = platform["range"].get<float>();
        std::string description = platform["description"].get<std::string>();
        
        flyingPlatforms.push_back({
            {position.x, position.y, position.z},
            {size.x, size.y, size.z},
            {spawnPosition.x, spawnPosition.y, spawnPosition.z},
            {targetPosition.x, targetPosition.y, targetPosition.z},
            speed,
            range,
            description
        });
    }
    
    StageUtils::createFlyingPlatforms(gameState, platformSystem, flyingPlatforms);
    DEBUG_PRINTF("DEBUG: Created %zu flying platforms from JSON\n", flyingPlatforms.size());
    return true;
}

bool JsonStageLoader::parseTeleportPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    DEBUG_PRINTF("DEBUG: Teleport platforms parsing not implemented yet\n");
    return true;
}

bool JsonStageLoader::parseJumpPads(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    DEBUG_PRINTF("DEBUG: Jump pads parsing not implemented yet\n");
    return true;
}

bool JsonStageLoader::parseStageSelectionAreas(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    try {
        if (!root.contains("stageSelectionAreas")) {
            return true; // オプショナル
        }
        
        for (const auto& area : root["stageSelectionAreas"]) {
            int stageNumber = area["stageNumber"].get<int>();
            glm::vec3 position = glm::vec3(
                area["position"][0].get<float>(),
                area["position"][1].get<float>(),
                area["position"][2].get<float>()
            );
            glm::vec3 size = glm::vec3(
                area["size"][0].get<float>(),
                area["size"][1].get<float>(),
                area["size"][2].get<float>()
            );
            
            glm::vec3 color;
            if (gameState.progress.unlockedStages.count(stageNumber) && gameState.progress.unlockedStages[stageNumber]) {
                color = glm::vec3(0.2f, 1.0f, 0.2f); // 緑色（アンロック済み）
            } else {
                color = glm::vec3(0.5f, 0.5f, 0.5f); // 灰色（ロック中）
            }
            
            platformSystem.addPlatform(StaticPlatform(position, size, color));
            printf("Created stage %d selection area at (%.1f, %.1f, %.1f) with color (%.1f, %.1f, %.1f)\n",
                   stageNumber, position.x, position.y, position.z, color.r, color.g, color.b);
        }
        
        DEBUG_PRINTF("DEBUG: Created %zu stage selection areas from JSON\n", root["stageSelectionAreas"].size());
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to parse stage selection areas: %s", e.what());
        return false;
    }
}

bool JsonStageLoader::parseLeaderboardArea(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    try {
        if (!root.contains("leaderboardArea")) {
            return true; // オプショナル
        }
        
        const auto& area = root["leaderboardArea"];
        glm::vec3 position = glm::vec3(
            area["position"][0].get<float>(),
            area["position"][1].get<float>(),
            area["position"][2].get<float>()
        );
        glm::vec3 size = glm::vec3(
            area["size"][0].get<float>(),
            area["size"][1].get<float>(),
            area["size"][2].get<float>()
        );
        
        gameState.ui.leaderboardPosition = position;
        
        // ランキングボードをプラットフォームとして描画（青色で表示）
        glm::vec3 color = glm::vec3(0.2f, 0.4f, 1.0f); // 青色
        platformSystem.addPlatform(StaticPlatform(position, size, color));
        printf("Created leaderboard area at (%.1f, %.1f, %.1f) with color (%.1f, %.1f, %.1f)\n",
               position.x, position.y, position.z, color.r, color.g, color.b);
        
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to parse leaderboard area: %s", e.what());
        return false;
    }
}

bool JsonStageLoader::parseConditionalCyclingDisappearingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("conditionalCyclingDisappearingPlatforms")) {
        return true; // オプショナル
    }
    
    try {
        for (const auto& conditionalGroup : root["conditionalCyclingDisappearingPlatforms"]) {
            std::string condition = conditionalGroup["condition"].get<std::string>();
            bool shouldAdd = false;
            
            if (condition == "stage3Cleared") {
                shouldAdd = (gameState.progress.stageStars.count(3) && gameState.progress.stageStars.at(3) > 0);
            }
            
            if (shouldAdd) {
                std::vector<CyclingDisappearingConfig> configs;
                for (const auto& platform : conditionalGroup["platforms"]) {
                    CyclingDisappearingConfig config;
                    config.position = glm::vec3(platform["position"][0].get<float>(), platform["position"][1].get<float>(), platform["position"][2].get<float>());
                    config.size = glm::vec3(platform["size"][0].get<float>(), platform["size"][1].get<float>(), platform["size"][2].get<float>());
                    config.color = glm::vec3(platform["color"][0].get<float>(), platform["color"][1].get<float>(), platform["color"][2].get<float>());
                    config.visibleTime = platform["visibleTime"].get<float>();
                    config.invisibleTime = platform["invisibleTime"].get<float>();
                    config.initialTimer = platform["initialTimer"].get<float>();
                    config.description = platform["description"].get<std::string>();
                    configs.push_back(config);
                }
                
                StageUtils::createCyclingDisappearingPlatforms(gameState, platformSystem, configs);
                DEBUG_PRINTF("DEBUG: Created %zu conditional cycling disappearing platforms for condition: %s\n", configs.size(), condition.c_str());
            } else {
                DEBUG_PRINTF("DEBUG: Condition '%s' not met, skipping conditional platforms\n", condition.c_str());
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to parse conditional cycling disappearing platforms: %s", e.what());
        return false;
    }
}

bool JsonStageLoader::parseConditionalPatrolPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("conditionalPatrolPlatforms")) {
        return true; // オプショナル
    }
    
    try {
        for (const auto& conditionalGroup : root["conditionalPatrolPlatforms"]) {
            std::string condition = conditionalGroup["condition"].get<std::string>();
            bool shouldAdd = false;
            
            if (condition == "stage2Cleared") {
                shouldAdd = (gameState.progress.stageStars.count(2) && gameState.progress.stageStars.at(2) > 0);
            }
            
            if (shouldAdd) {
                std::vector<PatrolPlatformConfig> configs;
                for (const auto& platform : conditionalGroup["platforms"]) {
                    PatrolPlatformConfig config;
                    config.description = platform["description"].get<std::string>();
                    
                    for (const auto& point : platform["patrolPoints"]) {
                        glm::vec3 patrolPoint = glm::vec3(
                            point[0].get<float>(),
                            point[1].get<float>(),
                            point[2].get<float>()
                        );
                        config.points.push_back(patrolPoint);
                    }
                    
                    configs.push_back(config);
                }
                
                StageUtils::createPatrolPlatformsFromConfig(gameState, platformSystem, configs);
                DEBUG_PRINTF("DEBUG: Created %zu conditional patrol platforms for condition: %s\n", configs.size(), condition.c_str());
            } else {
                DEBUG_PRINTF("DEBUG: Condition '%s' not met, skipping conditional patrol platforms\n", condition.c_str());
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to parse conditional patrol platforms: %s", e.what());
        return false;
    }
}

bool JsonStageLoader::parseConditionalFlyingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("conditionalFlyingPlatforms")) {
        return true; // オプショナル
    }
    
    try {
        for (const auto& conditionalGroup : root["conditionalFlyingPlatforms"]) {
            std::string condition = conditionalGroup["condition"].get<std::string>();
            bool shouldAdd = false;
            
            if (condition == "stage4Cleared") {
                shouldAdd = (gameState.progress.stageStars.count(4) && gameState.progress.stageStars.at(4) > 0);
            }
            
            if (shouldAdd) {
                std::vector<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, float, float, std::string>> flyingPlatforms;
                
                for (const auto& platform : conditionalGroup["platforms"]) {
                    glm::vec3 position = glm::vec3(platform["position"][0].get<float>(), platform["position"][1].get<float>(), platform["position"][2].get<float>());
                    glm::vec3 size = glm::vec3(platform["size"][0].get<float>(), platform["size"][1].get<float>(), platform["size"][2].get<float>());
                    glm::vec3 spawnPosition = glm::vec3(platform["spawnPosition"][0].get<float>(), platform["spawnPosition"][1].get<float>(), platform["spawnPosition"][2].get<float>());
                    glm::vec3 targetPosition = glm::vec3(platform["targetPosition"][0].get<float>(), platform["targetPosition"][1].get<float>(), platform["targetPosition"][2].get<float>());
                    float speed = platform["speed"].get<float>();
                    float range = platform["range"].get<float>();
                    std::string description = platform["description"].get<std::string>();
                    
                    flyingPlatforms.push_back({
                        {position.x, position.y, position.z},
                        {size.x, size.y, size.z},
                        {spawnPosition.x, spawnPosition.y, spawnPosition.z},
                        {targetPosition.x, targetPosition.y, targetPosition.z},
                        speed,
                        range,
                        description
                    });
                }
                
                StageUtils::createFlyingPlatforms(gameState, platformSystem, flyingPlatforms);
                DEBUG_PRINTF("DEBUG: Created %zu conditional flying platforms for condition: %s\n", flyingPlatforms.size(), condition.c_str());
            } else {
                DEBUG_PRINTF("DEBUG: Condition '%s' not met, skipping conditional flying platforms\n", condition.c_str());
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to parse conditional flying platforms: %s", e.what());
        return false;
    }
}

bool JsonStageLoader::saveStageToJSON(const std::string& filename, const GameState& gameState, 
                                     const PlatformSystem& platformSystem, int stageNumber) {
    try {
        nlohmann::json root;
        
        nlohmann::json stageInfo;
        stageInfo["stageNumber"] = stageNumber;
        stageInfo["stageName"] = "Stage " + std::to_string(stageNumber);
        stageInfo["playerStartPosition"] = {
            gameState.player.position.x,
            gameState.player.position.y,
            gameState.player.position.z
        };
        stageInfo["goalPosition"] = {
            gameState.progress.goalPosition.x,
            gameState.progress.goalPosition.y,
            gameState.progress.goalPosition.z
        };
        stageInfo["timeLimit"] = gameState.progress.timeLimit;
        stageInfo["isUnlocked"] = true;
        root["stageInfo"] = stageInfo;
        
        nlohmann::json itemsArray = nlohmann::json::array();
        for (const auto& item : gameState.items.items) {
            if (!item.isCollected) {  // 収集されていないアイテムのみ保存
                nlohmann::json itemJson;
                itemJson["position"] = {item.position.x, item.position.y, item.position.z};
                itemJson["color"] = {item.color.x, item.color.y, item.color.z};
                itemJson["description"] = "Item " + std::to_string(item.itemId);
                itemsArray.push_back(itemJson);
            }
        }
        if (!itemsArray.empty()) {
            root["items"] = itemsArray;
        }
        
        nlohmann::json staticPlatformsArray = nlohmann::json::array();
        nlohmann::json movingPlatformsArray = nlohmann::json::array();
        nlohmann::json rotatingPlatformsArray = nlohmann::json::array();
        nlohmann::json patrolPlatformsArray = nlohmann::json::array();
        nlohmann::json teleportPlatformsArray = nlohmann::json::array();
        nlohmann::json jumpPadsArray = nlohmann::json::array();
        nlohmann::json cyclingDisappearingPlatformsArray = nlohmann::json::array();
        nlohmann::json disappearingPlatformsArray = nlohmann::json::array();
        nlohmann::json flyingPlatformsArray = nlohmann::json::array();
        
        for (const auto& platform : platformSystem.getPlatforms()) {
            std::visit([&](const auto& p) {
                using T = std::decay_t<decltype(p)>;
                nlohmann::json platformJson;
                platformJson["position"] = {p.position.x, p.position.y, p.position.z};
                platformJson["size"] = {p.size.x, p.size.y, p.size.z};
                platformJson["color"] = {p.color.x, p.color.y, p.color.z};
                platformJson["description"] = "";
                
                if constexpr (std::is_same_v<T, StaticPlatform>) {
                    staticPlatformsArray.push_back(platformJson);
                } else if constexpr (std::is_same_v<T, MovingPlatform>) {
                    platformJson["moveTargetPosition"] = {
                        p.moveTargetPosition.x, p.moveTargetPosition.y, p.moveTargetPosition.z
                    };
                    platformJson["moveSpeed"] = p.moveSpeed;
                    movingPlatformsArray.push_back(platformJson);
                } else if constexpr (std::is_same_v<T, RotatingPlatform>) {
                    platformJson["rotationAxis"] = {
                        p.rotationAxis.x, p.rotationAxis.y, p.rotationAxis.z
                    };
                    platformJson["rotationSpeed"] = p.rotationSpeed;
                    rotatingPlatformsArray.push_back(platformJson);
                } else if constexpr (std::is_same_v<T, PatrollingPlatform>) {
                    nlohmann::json pointsArray = nlohmann::json::array();
                    for (const auto& point : p.patrolPoints) {
                        pointsArray.push_back({point.x, point.y, point.z});
                    }
                    platformJson["patrolPoints"] = pointsArray;
                    platformJson["patrolSpeed"] = p.patrolSpeed;
                    patrolPlatformsArray.push_back(platformJson);
                } else if constexpr (std::is_same_v<T, TeleportPlatform>) {
                    platformJson["teleportDestination"] = {
                        p.teleportDestination.x, p.teleportDestination.y, p.teleportDestination.z
                    };
                    platformJson["cooldownTime"] = p.cooldown;
                    teleportPlatformsArray.push_back(platformJson);
                } else if constexpr (std::is_same_v<T, JumpPad>) {
                    platformJson["jumpPower"] = p.jumpPower;
                    jumpPadsArray.push_back(platformJson);
                } else if constexpr (std::is_same_v<T, CycleDisappearingPlatform>) {
                    platformJson["cycleTime"] = p.cycleTime;
                    platformJson["visibleTime"] = p.visibleTime;
                    platformJson["blinkTime"] = p.blinkTime;
                    cyclingDisappearingPlatformsArray.push_back(platformJson);
                } else if constexpr (std::is_same_v<T, DisappearingPlatform>) {
                    disappearingPlatformsArray.push_back(platformJson);
                } else if constexpr (std::is_same_v<T, FlyingPlatform>) {
                    platformJson["spawnPosition"] = {
                        p.spawnPosition.x, p.spawnPosition.y, p.spawnPosition.z
                    };
                    platformJson["targetPosition"] = {
                        p.targetPosition.x, p.targetPosition.y, p.targetPosition.z
                    };
                    platformJson["flySpeed"] = p.flySpeed;
                    platformJson["detectionRange"] = p.detectionRange;
                    flyingPlatformsArray.push_back(platformJson);
                }
            }, platform);
        }
        
        if (!staticPlatformsArray.empty()) {
            root["staticPlatforms"] = staticPlatformsArray;
        }
        if (!movingPlatformsArray.empty()) {
            root["movingPlatforms"] = movingPlatformsArray;
        }
        if (!rotatingPlatformsArray.empty()) {
            root["rotatingPlatforms"] = rotatingPlatformsArray;
        }
        if (!patrolPlatformsArray.empty()) {
            root["patrolPlatforms"] = patrolPlatformsArray;
        }
        if (!teleportPlatformsArray.empty()) {
            root["teleportPlatforms"] = teleportPlatformsArray;
        }
        if (!jumpPadsArray.empty()) {
            root["jumpPads"] = jumpPadsArray;
        }
        if (!cyclingDisappearingPlatformsArray.empty()) {
            root["cyclingDisappearingPlatforms"] = cyclingDisappearingPlatformsArray;
        }
        if (!disappearingPlatformsArray.empty()) {
            root["disappearingPlatforms"] = disappearingPlatformsArray;
        }
        if (!flyingPlatformsArray.empty()) {
            root["flyingPlatforms"] = flyingPlatformsArray;
        }
        
        printf("DEBUG: Attempting to open file for writing: %s\n", filename.c_str());
        std::ofstream file(filename, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            ErrorHandler::logErrorFormat("Failed to open file for writing: %s", filename.c_str());
            return false;
        }
        
        std::string jsonContent = root.dump(2);
        printf("DEBUG: JSON content size: %zu bytes\n", jsonContent.size());
        file << jsonContent;
        file.flush();
        file.close();
        
        std::ifstream verifyFile(filename);
        if (!verifyFile.good()) {
            ErrorHandler::logErrorFormat("File verification failed after writing: %s", filename.c_str());
            return false;
        }
        verifyFile.close();
        
        printf("SUCCESS: Saved stage to %s\n", filename.c_str());
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to save stage to JSON: %s", e.what());
        return false;
    }
}
