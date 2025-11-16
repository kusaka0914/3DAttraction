#include "save_manager.h"
#include "../core/utils/resource_path.h"
#include <fstream>
#include <iostream>
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

std::string SaveManager::getSaveFilePath() {
    if (fileExists("assets/save/save_data.json")) {
        return "assets/save/save_data.json";
    }
    if (fileExists("../assets/save/save_data.json")) {
        return "../assets/save/save_data.json";
    }
    return "assets/save/save_data.json";
}

bool SaveManager::fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

bool SaveManager::saveFileExists() {
    return fileExists(getSaveFilePath());
}

bool SaveManager::saveGameData(const GameState& gameState) {
    try {
        std::string dirPath = "assets/save";
        #ifdef _WIN32
            if (_access(dirPath.c_str(), 0) != 0) {
                _mkdir(dirPath.c_str());
            }
        #else
            struct stat info;
            if (stat(dirPath.c_str(), &info) != 0) {
                mkdir(dirPath.c_str(), 0755);
            }
        #endif
        
        if (!fileExists(dirPath + "/.gitkeep")) {
            dirPath = "../assets/save";
            #ifdef _WIN32
                if (_access(dirPath.c_str(), 0) != 0) {
                    _mkdir(dirPath.c_str());
                }
            #else
                if (stat(dirPath.c_str(), &info) != 0) {
                    mkdir(dirPath.c_str(), 0755);
                }
            #endif
        }
        
        nlohmann::json saveData;
        
        saveData["totalStars"] = gameState.progress.totalStars;
        
        nlohmann::json stageStarsJson;
        for (const auto& [stageNumber, stars] : gameState.progress.stageStars) {
            stageStarsJson[std::to_string(stageNumber)] = stars;
        }
        saveData["stageStars"] = stageStarsJson;
        
        nlohmann::json unlockedStagesJson;
        for (const auto& [stageNumber, isUnlocked] : gameState.progress.unlockedStages) {
            unlockedStagesJson[std::to_string(stageNumber)] = isUnlocked;
        }
        saveData["unlockedStages"] = unlockedStagesJson;
        
        nlohmann::json timeAttackRecordsJson;
        for (const auto& [stageNumber, bestTime] : gameState.progress.timeAttackRecords) {
            timeAttackRecordsJson[std::to_string(stageNumber)] = bestTime;
        }
        saveData["timeAttackRecords"] = timeAttackRecordsJson;
        
        saveData["showStage0Tutorial"] = gameState.ui.showStage0Tutorial;
        
        saveData["isGameCleared"] = gameState.progress.isGameCleared;
        
        saveData["hasShownSecretStarExplanationUI"] = gameState.progress.hasShownSecretStarExplanationUI;
        
        nlohmann::json secretStarClearedJson;
        for (const auto& [stageNumber, clearedTypes] : gameState.progress.secretStarCleared) {
            nlohmann::json typesArray = nlohmann::json::array();
            for (auto type : clearedTypes) {
                typesArray.push_back(static_cast<int>(type));
            }
            secretStarClearedJson[std::to_string(stageNumber)] = typesArray;
        }
        saveData["secretStarCleared"] = secretStarClearedJson;
        
        std::string filePath = "assets/save/save_data.json";
        std::ofstream file(filePath);
        if (!file.is_open()) {
            filePath = "../assets/save/save_data.json";
            file.open(filePath);
            if (!file.is_open()) {
                std::cerr << "Failed to open save file for writing: " << filePath << std::endl;
                return false;
            }
        }
        
        file << saveData.dump(2); // インデント2で整形して保存
        file.close();
        
        std::cout << "Game data saved successfully to: " << filePath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving game data: " << e.what() << std::endl;
        return false;
    }
}

bool SaveManager::loadGameData(GameState& gameState) {
    try {
        std::string filePath = "assets/save/save_data.json";
        if (!fileExists(filePath)) {
            filePath = "../assets/save/save_data.json";
            if (!fileExists(filePath)) {
                std::cout << "Save file not found. Starting with default data." << std::endl;
                return false;
            }
        }
        
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open save file for reading: " << filePath << std::endl;
            return false;
        }
        
        nlohmann::json saveData;
        file >> saveData;
        file.close();
        
        if (saveData.contains("totalStars") && saveData["totalStars"].is_number()) {
            gameState.progress.totalStars = saveData["totalStars"];
        }
        
        if (saveData.contains("stageStars") && saveData["stageStars"].is_object()) {
            gameState.progress.stageStars.clear();
            for (auto& [key, value] : saveData["stageStars"].items()) {
                int stageNumber = std::stoi(key);
                int stars = value.get<int>();
                gameState.progress.stageStars[stageNumber] = stars;
            }
        }
        
        if (saveData.contains("unlockedStages") && saveData["unlockedStages"].is_object()) {
            gameState.progress.unlockedStages.clear();
            for (auto& [key, value] : saveData["unlockedStages"].items()) {
                int stageNumber = std::stoi(key);
                bool isUnlocked = value.get<bool>();
                gameState.progress.unlockedStages[stageNumber] = isUnlocked;
            }
        }
        
        if (saveData.contains("timeAttackRecords") && saveData["timeAttackRecords"].is_object()) {
            gameState.progress.timeAttackRecords.clear();
            for (auto& [key, value] : saveData["timeAttackRecords"].items()) {
                int stageNumber = std::stoi(key);
                float bestTime = value.get<float>();
                gameState.progress.timeAttackRecords[stageNumber] = bestTime;
            }
        }
        
        if (saveData.contains("showStage0Tutorial") && saveData["showStage0Tutorial"].is_boolean()) {
            gameState.ui.showStage0Tutorial = saveData["showStage0Tutorial"];
        }
        
        if (saveData.contains("isGameCleared") && saveData["isGameCleared"].is_boolean()) {
            gameState.progress.isGameCleared = saveData["isGameCleared"];
        }
        
        if (saveData.contains("hasShownSecretStarExplanationUI") && saveData["hasShownSecretStarExplanationUI"].is_boolean()) {
            gameState.progress.hasShownSecretStarExplanationUI = saveData["hasShownSecretStarExplanationUI"];
        }
        
        if (saveData.contains("secretStarCleared") && saveData["secretStarCleared"].is_object()) {
            gameState.progress.secretStarCleared.clear();
            for (auto& [key, value] : saveData["secretStarCleared"].items()) {
                int stageNumber = std::stoi(key);
                if (value.is_array()) {
                    std::set<GameProgressState::SecretStarType> clearedTypes;
                    for (auto& typeValue : value) {
                        if (typeValue.is_number()) {
                            int typeInt = typeValue.get<int>();
                            clearedTypes.insert(static_cast<GameProgressState::SecretStarType>(typeInt));
                        }
                    }
                    gameState.progress.secretStarCleared[stageNumber] = clearedTypes;
                }
            }
        }
        
        std::cout << "Game data loaded successfully from: " << filePath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading game data: " << e.what() << std::endl;
        return false;
    }
}

