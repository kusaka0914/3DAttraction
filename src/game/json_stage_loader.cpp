#include "json_stage_loader.h"
#include "../core/utils/stage_utils.h"
#include "../core/constants/debug_config.h"
#include "../core/constants/stage_constants.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

// ファイル存在チェック関数（シンプル版）
bool JsonStageLoader::fileExists(const std::string& filename) {
    printf("DEBUG: Checking if file exists: %s\n", filename.c_str());
    
    std::ifstream file(filename);
    bool exists = file.good();
    file.close();
    
    printf("DEBUG: File %s exists: %s\n", filename.c_str(), exists ? "YES" : "NO");
    return exists;
}

bool JsonStageLoader::loadStageFromJSON(const std::string& filename, GameState& gameState, PlatformSystem& platformSystem) {
    // 作業ディレクトリを確認（getcwdを使わない方法）
    FILE* pipe = popen("pwd", "r");
    if (pipe) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            printf("DEBUG: Current working directory: %s", buffer);
        }
        pclose(pipe);
    }
    if (!fileExists(filename)) {
        printf("ERROR: Stage file not found: %s\n", filename.c_str());
        return false;
    }
    
    try {
        std::ifstream file(filename);
        nlohmann::json root;
        file >> root;
        
        DEBUG_PRINTF("DEBUG: Loading stage from JSON: %s\n", filename.c_str());
        
        // ステージ情報を読み込み
        if (root.contains("stageInfo")) {
            if (!parseStageInfo(root["stageInfo"], gameState)) {
                printf("ERROR: Failed to parse stage info\n");
                return false;
            }
        }
        
        // アイテムを読み込み
        if (root.contains("items")) {
            if (!parseItems(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse items\n");
                return false;
            }
        }
        
        // 静的足場を読み込み
        if (root.contains("staticPlatforms")) {
            if (!parseStaticPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse static platforms\n");
                return false;
            }
        }
        
        // 巡回足場を読み込み
        if (root.contains("patrolPlatforms")) {
            if (!parsePatrolPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse patrol platforms\n");
                return false;
            }
        }
        
        // 動く足場を読み込み
        if (root.contains("movingPlatforms")) {
            if (!parseMovingPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse moving platforms\n");
                return false;
            }
        }
        
        // 回転足場を読み込み
        if (root.contains("rotatingPlatforms")) {
            if (!parseRotatingPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse rotating platforms\n");
                return false;
            }
        }
        
        // 消失足場を読み込み
        if (root.contains("disappearingPlatforms")) {
            if (!parseDisappearingPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse disappearing platforms\n");
                return false;
            }
        }
        
        // サイクリング消失足場を読み込み
        if (root.contains("cyclingDisappearingPlatforms")) {
            printf("DEBUG: Found cyclingDisappearingPlatforms in JSON\n");
            if (!parseDisappearingPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse cycling disappearing platforms\n");
                return false;
            }
        } else {
            printf("DEBUG: No cyclingDisappearingPlatforms found in JSON\n");
        }
        
        // 連続サイクリング足場を読み込み
        if (root.contains("consecutiveCyclingPlatforms")) {
            printf("DEBUG: Found consecutiveCyclingPlatforms in JSON\n");
            if (!parseConsecutiveCyclingPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse consecutive cycling platforms\n");
                return false;
            }
        } else {
            printf("DEBUG: No consecutiveCyclingPlatforms found in JSON\n");
        }
        
        // 飛行足場を読み込み
        if (root.contains("flyingPlatforms")) {
            if (!parseFlyingPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse flying platforms\n");
                return false;
            }
        }
        
        // テレポート足場を読み込み
        if (root.contains("teleportPlatforms")) {
            if (!parseTeleportPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse teleport platforms\n");
                return false;
            }
        }
        
        // ジャンプパッドを読み込み
        if (root.contains("jumpPads")) {
            if (!parseJumpPads(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse jump pads\n");
                return false;
            }
        }
        
        // ステージ選択エリアを読み込み（動的色変更）
        if (root.contains("stageSelectionAreas")) {
            if (!parseStageSelectionAreas(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse stage selection areas\n");
                return false;
            }
        }
        
        // 条件付きサイクリングディスアピアリングプラットフォームを読み込み
        if (root.contains("conditionalCyclingDisappearingPlatforms")) {
            if (!parseConditionalCyclingDisappearingPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse conditional cycling disappearing platforms\n");
                return false;
            }
        }
        
        // 条件付きパトロールプラットフォームを読み込み
        if (root.contains("conditionalPatrolPlatforms")) {
            if (!parseConditionalPatrolPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse conditional patrol platforms\n");
                return false;
            }
        }
        
        // 条件付き飛んでくる足場を読み込み
        if (root.contains("conditionalFlyingPlatforms")) {
            if (!parseConditionalFlyingPlatforms(root, gameState, platformSystem)) {
                printf("ERROR: Failed to parse conditional flying platforms\n");
                return false;
            }
        }
        
        DEBUG_PRINTF("DEBUG: Successfully loaded stage from JSON: %s\n", filename.c_str());
        return true;
        
    } catch (const std::exception& e) {
        printf("ERROR: Failed to parse JSON file %s: %s\n", filename.c_str(), e.what());
        return false;
    }
}

bool JsonStageLoader::loadStageInfoFromJSON(const std::string& filename, GameState& gameState) {
    if (!fileExists(filename)) {
        printf("ERROR: Stage file not found: %s\n", filename.c_str());
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
        printf("ERROR: Failed to parse JSON file %s: %s\n", filename.c_str(), e.what());
        return false;
    }
}

bool JsonStageLoader::parseStageInfo(const nlohmann::json& stageInfo, GameState& gameState) {
    try {
        if (stageInfo.contains("playerStartPosition")) {
            auto pos = stageInfo["playerStartPosition"];
            gameState.playerPosition = glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            );
        }
        
        if (stageInfo.contains("goalPosition")) {
            auto pos = stageInfo["goalPosition"];
            gameState.goalPosition = glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            );
        }
        
        // json_stage_loader.cpp の parseStageInfo 関数内
        if (stageInfo.contains("timeLimit")) {
            gameState.timeLimit = stageInfo["timeLimit"].get<float>();
            gameState.remainingTime = gameState.timeLimit;
            printf("Time limit: %.1f seconds\n", gameState.timeLimit);
        }
        
        DEBUG_PRINTF("DEBUG: Parsed stage info - Player: (%.1f, %.1f, %.1f), Goal: (%.1f, %.1f, %.1f), Time: %.1f\n",
                    gameState.playerPosition.x, gameState.playerPosition.y, gameState.playerPosition.z,
                    gameState.goalPosition.x, gameState.goalPosition.y, gameState.goalPosition.z,
                    gameState.timeLimit);
        
        return true;
        
    } catch (const std::exception& e) {
        printf("ERROR: Failed to parse stage info: %s\n", e.what());
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
        printf("ERROR: Failed to parse items: %s\n", e.what());
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
        printf("ERROR: Failed to parse static platforms: %s\n", e.what());
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
        printf("ERROR: Failed to parse patrol platforms: %s\n", e.what());
        return false;
    }
}

bool JsonStageLoader::parseMovingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    // MovingPlatformsの解析（将来の拡張用）
    DEBUG_PRINTF("DEBUG: Moving platforms parsing not implemented yet\n");
    return true;
}

bool JsonStageLoader::parseRotatingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    // RotatingPlatformsの解析（将来の拡張用）
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
    
    // std::vector版のオーバーロードを使用
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
    // TeleportPlatformsの解析（将来の拡張用）
    DEBUG_PRINTF("DEBUG: Teleport platforms parsing not implemented yet\n");
    return true;
}

bool JsonStageLoader::parseJumpPads(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    // JumpPadsの解析（将来の拡張用）
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
            
            // アンロック状態に応じて色を動的に決定
            glm::vec3 color;
            if (gameState.unlockedStages.count(stageNumber) && gameState.unlockedStages[stageNumber]) {
                color = glm::vec3(0.2f, 1.0f, 0.2f); // 緑色（アンロック済み）
            } else {
                color = glm::vec3(0.5f, 0.5f, 0.5f); // 灰色（ロック中）
            }
            
            platformSystem.addPlatform(GameState::StaticPlatform(position, size, color));
            printf("Created stage %d selection area at (%.1f, %.1f, %.1f) with color (%.1f, %.1f, %.1f)\n",
                   stageNumber, position.x, position.y, position.z, color.r, color.g, color.b);
        }
        
        DEBUG_PRINTF("DEBUG: Created %zu stage selection areas from JSON\n", root["stageSelectionAreas"].size());
        return true;
        
    } catch (const std::exception& e) {
        printf("ERROR: Failed to parse stage selection areas: %s\n", e.what());
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
            
            // 条件をチェック
            if (condition == "stage3Cleared") {
                shouldAdd = (gameState.stageStars.count(3) && gameState.stageStars.at(3) > 0);
            }
            // 他の条件も追加可能
            // else if (condition == "stage4Cleared") {
            //     shouldAdd = (gameState.stageStars.count(4) && gameState.stageStars.at(4) > 0);
            // }
            
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
        printf("ERROR: Failed to parse conditional cycling disappearing platforms: %s\n", e.what());
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
            
            // 条件をチェック
            if (condition == "stage2Cleared") {
                shouldAdd = (gameState.stageStars.count(2) && gameState.stageStars.at(2) > 0);
            }
            // 他の条件も追加可能
            // else if (condition == "stage4Cleared") {
            //     shouldAdd = (gameState.stageStars.count(4) && gameState.stageStars.at(4) > 0);
            // }
            
            if (shouldAdd) {
                std::vector<PatrolPlatformConfig> configs;
                for (const auto& platform : conditionalGroup["platforms"]) {
                    PatrolPlatformConfig config;
                    config.description = platform["description"].get<std::string>();
                    
                    // パトロールポイントを解析
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
        printf("ERROR: Failed to parse conditional patrol platforms: %s\n", e.what());
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
            
            // 条件をチェック
            if (condition == "stage4Cleared") {
                shouldAdd = (gameState.stageStars.count(4) && gameState.stageStars.at(4) > 0);
            }
            // 他の条件も追加可能
            // else if (condition == "stage5Cleared") {
            //     shouldAdd = (gameState.stageStars.count(5) && gameState.stageStars.at(5) > 0);
            // }
            
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
        printf("ERROR: Failed to parse conditional flying platforms: %s\n", e.what());
        return false;
    }
}
