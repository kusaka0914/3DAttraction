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
    // assets/saveディレクトリに保存（ReplayManagerと同じロジック）
    // まず assets/save/save_data.json を試す
    if (fileExists("assets/save/save_data.json")) {
        return "assets/save/save_data.json";
    }
    // 次に ../assets/save/save_data.json を試す
    if (fileExists("../assets/save/save_data.json")) {
        return "../assets/save/save_data.json";
    }
    // どちらも存在しない場合は、assets/save/save_data.json を返す（新規作成用）
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
        // ディレクトリが存在するか確認し、なければ作成
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
        
        // 代替パスも試す
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
        
        // 星数データを保存
        saveData["totalStars"] = gameState.totalStars;
        
        // 各ステージの星数を保存
        nlohmann::json stageStarsJson;
        for (const auto& [stageNumber, stars] : gameState.stageStars) {
            stageStarsJson[std::to_string(stageNumber)] = stars;
        }
        saveData["stageStars"] = stageStarsJson;
        
        // 解放されたステージを保存
        nlohmann::json unlockedStagesJson;
        for (const auto& [stageNumber, isUnlocked] : gameState.unlockedStages) {
            unlockedStagesJson[std::to_string(stageNumber)] = isUnlocked;
        }
        saveData["unlockedStages"] = unlockedStagesJson;
        
        // タイムアタックのベストタイムを保存
        nlohmann::json timeAttackRecordsJson;
        for (const auto& [stageNumber, bestTime] : gameState.timeAttackRecords) {
            timeAttackRecordsJson[std::to_string(stageNumber)] = bestTime;
        }
        saveData["timeAttackRecords"] = timeAttackRecordsJson;
        
        // 初回チュートリアル表示フラグを保存
        saveData["showStage0Tutorial"] = gameState.showStage0Tutorial;
        
        // JSONファイルに書き込み（ReplayManagerと同じロジック）
        std::string filePath = "assets/save/save_data.json";
        std::ofstream file(filePath);
        if (!file.is_open()) {
            // 代替パスを試す
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
        // 複数のパスを試す（ReplayManagerと同じロジック）
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
        
        // 総獲得星数を読み込み
        if (saveData.contains("totalStars") && saveData["totalStars"].is_number()) {
            gameState.totalStars = saveData["totalStars"];
        }
        
        // 各ステージの星数を読み込み
        if (saveData.contains("stageStars") && saveData["stageStars"].is_object()) {
            gameState.stageStars.clear();
            for (auto& [key, value] : saveData["stageStars"].items()) {
                int stageNumber = std::stoi(key);
                int stars = value.get<int>();
                gameState.stageStars[stageNumber] = stars;
            }
        }
        
        // 解放されたステージを読み込み
        if (saveData.contains("unlockedStages") && saveData["unlockedStages"].is_object()) {
            gameState.unlockedStages.clear();
            for (auto& [key, value] : saveData["unlockedStages"].items()) {
                int stageNumber = std::stoi(key);
                bool isUnlocked = value.get<bool>();
                gameState.unlockedStages[stageNumber] = isUnlocked;
            }
        }
        
        // タイムアタックのベストタイムを読み込み
        if (saveData.contains("timeAttackRecords") && saveData["timeAttackRecords"].is_object()) {
            gameState.timeAttackRecords.clear();
            for (auto& [key, value] : saveData["timeAttackRecords"].items()) {
                int stageNumber = std::stoi(key);
                float bestTime = value.get<float>();
                gameState.timeAttackRecords[stageNumber] = bestTime;
            }
        }
        
        // 初回チュートリアル表示フラグを読み込み
        if (saveData.contains("showStage0Tutorial") && saveData["showStage0Tutorial"].is_boolean()) {
            gameState.showStage0Tutorial = saveData["showStage0Tutorial"];
        }
        
        std::cout << "Game data loaded successfully from: " << filePath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading game data: " << e.what() << std::endl;
        return false;
    }
}

