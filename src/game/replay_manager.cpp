#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "replay_manager.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

bool ReplayManager::fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

std::string ReplayManager::getReplayFilePath(int stageNumber) {
    // assets/replaysディレクトリに保存
    std::string filename = "assets/replays/stage" + std::to_string(stageNumber) + "_best.json";
    
    // 代替パスも試す
    if (!fileExists(filename)) {
        filename = "../assets/replays/stage" + std::to_string(stageNumber) + "_best.json";
    }
    
    return filename;
}

bool ReplayManager::replayExists(int stageNumber) {
    std::string filepath = getReplayFilePath(stageNumber);
    return fileExists(filepath);
}

bool ReplayManager::saveReplay(const GameState::ReplayData& replayData, int stageNumber) {
    try {
        // ディレクトリが存在するか確認し、なければ作成
        std::string dirPath = "assets/replays";
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
            dirPath = "../assets/replays";
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
        
        nlohmann::json jsonData;
        jsonData["stageNumber"] = replayData.stageNumber;
        jsonData["clearTime"] = replayData.clearTime;
        jsonData["recordedDate"] = replayData.recordedDate;
        jsonData["frameRate"] = replayData.frameRate;
        
        // フレームデータを配列に変換
        nlohmann::json framesArray = nlohmann::json::array();
        for (const auto& frame : replayData.frames) {
            nlohmann::json frameJson;
            frameJson["timestamp"] = frame.timestamp;
            frameJson["playerPosition"] = {frame.playerPosition.x, frame.playerPosition.y, frame.playerPosition.z};
            frameJson["playerVelocity"] = {frame.playerVelocity.x, frame.playerVelocity.y, frame.playerVelocity.z};
            
            // アイテムの収集状態を追加（存在する場合）
            if (!frame.itemCollectedStates.empty()) {
                frameJson["itemCollectedStates"] = frame.itemCollectedStates;
            }
            
            framesArray.push_back(frameJson);
        }
        jsonData["frames"] = framesArray;
        
        // ファイルに書き込み
        std::string filepath = getReplayFilePath(stageNumber);
        std::ofstream file(filepath);
        if (!file.is_open()) {
            // 代替パスを試す
            filepath = "../assets/replays/stage" + std::to_string(stageNumber) + "_best.json";
            file.open(filepath);
            if (!file.is_open()) {
                printf("ERROR: Failed to open replay file for writing: %s\n", filepath.c_str());
                return false;
            }
        }
        
        file << jsonData.dump(2);  // インデント付きで保存
        file.close();
        
        printf("REPLAY: Saved replay for stage %d (%zu frames, %.2fs)\n", 
               stageNumber, replayData.frames.size(), replayData.clearTime);
        return true;
        
    } catch (const std::exception& e) {
        printf("ERROR: Failed to save replay: %s\n", e.what());
        return false;
    }
}

bool ReplayManager::loadReplay(GameState::ReplayData& replayData, int stageNumber) {
    try {
        std::string filepath = getReplayFilePath(stageNumber);
        if (!fileExists(filepath)) {
            printf("REPLAY: Replay file not found: %s\n", filepath.c_str());
            return false;
        }
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            printf("ERROR: Failed to open replay file: %s\n", filepath.c_str());
            return false;
        }
        
        nlohmann::json jsonData;
        file >> jsonData;
        file.close();
        
        // データを読み込み
        replayData.stageNumber = jsonData["stageNumber"].get<int>();
        replayData.clearTime = jsonData["clearTime"].get<float>();
        replayData.recordedDate = jsonData["recordedDate"].get<std::string>();
        replayData.frameRate = jsonData.contains("frameRate") ? jsonData["frameRate"].get<float>() : 0.1f;
        
        // フレームデータを読み込み
        replayData.frames.clear();
        if (jsonData.contains("frames") && jsonData["frames"].is_array()) {
            for (const auto& frameJson : jsonData["frames"]) {
                GameState::ReplayFrame frame;
                frame.timestamp = frameJson["timestamp"].get<float>();
                
                auto pos = frameJson["playerPosition"];
                frame.playerPosition = glm::vec3(
                    pos[0].get<float>(),
                    pos[1].get<float>(),
                    pos[2].get<float>()
                );
                
                auto vel = frameJson["playerVelocity"];
                frame.playerVelocity = glm::vec3(
                    vel[0].get<float>(),
                    vel[1].get<float>(),
                    vel[2].get<float>()
                );
                
                // アイテムの収集状態を読み込み（存在する場合）
                if (frameJson.contains("itemCollectedStates") && frameJson["itemCollectedStates"].is_array()) {
                    frame.itemCollectedStates.clear();
                    for (const auto& state : frameJson["itemCollectedStates"]) {
                        frame.itemCollectedStates.push_back(state.get<bool>());
                    }
                }
                
                replayData.frames.push_back(frame);
            }
        }
        
        printf("REPLAY: Loaded replay for stage %d (%zu frames, %.2fs)\n", 
               stageNumber, replayData.frames.size(), replayData.clearTime);
        return true;
        
    } catch (const std::exception& e) {
        printf("ERROR: Failed to load replay: %s\n", e.what());
        return false;
    }
}

