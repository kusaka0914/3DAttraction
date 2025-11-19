#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "replay_manager.h"
#include "../core/error_handler.h"
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
    std::string filename = "assets/replays/stage" + std::to_string(stageNumber) + "_best.json";
    
    if (!fileExists(filename)) {
        filename = "../assets/replays/stage" + std::to_string(stageNumber) + "_best.json";
    }
    
    return filename;
}

bool ReplayManager::replayExists(int stageNumber) {
    std::string filepath = getReplayFilePath(stageNumber);
    return fileExists(filepath);
}

bool ReplayManager::saveReplay(const ReplayData& replayData, int stageNumber) {
    try {
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
        
        nlohmann::json framesArray = nlohmann::json::array();
        for (const auto& frame : replayData.frames) {
            nlohmann::json frameJson;
            frameJson["timestamp"] = frame.timestamp;
            frameJson["playerPosition"] = {frame.playerPosition.x, frame.playerPosition.y, frame.playerPosition.z};
            frameJson["playerVelocity"] = {frame.playerVelocity.x, frame.playerVelocity.y, frame.playerVelocity.z};
            frameJson["timeScale"] = frame.timeScale;
            
            if (!frame.itemCollectedStates.empty()) {
                frameJson["itemCollectedStates"] = frame.itemCollectedStates;
            }
            
            framesArray.push_back(frameJson);
        }
        jsonData["frames"] = framesArray;
        
        std::string filepath = getReplayFilePath(stageNumber);
        std::ofstream file(filepath);
        if (!file.is_open()) {
            filepath = "../assets/replays/stage" + std::to_string(stageNumber) + "_best.json";
            file.open(filepath);
            if (!file.is_open()) {
                ErrorHandler::logErrorFormat("Failed to open replay file for writing: %s", filepath.c_str());
                return false;
            }
        }
        
        file << jsonData.dump(2);  // インデント付きで保存
        file.close();
        
        printf("REPLAY: Saved replay for stage %d (%zu frames, %.2fs)\n", 
               stageNumber, replayData.frames.size(), replayData.clearTime);
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::logErrorFormat("Failed to save replay: %s", e.what());
        return false;
    }
}

bool ReplayManager::loadReplay(ReplayData& replayData, int stageNumber) {
    try {
        std::string filepath = getReplayFilePath(stageNumber);
        if (!fileExists(filepath)) {
            printf("REPLAY: Replay file not found: %s\n", filepath.c_str());
            return false;
        }
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            ErrorHandler::logErrorFormat("Failed to open replay file: %s", filepath.c_str());
            return false;
        }
        
        nlohmann::json jsonData;
        file >> jsonData;
        file.close();
        
        replayData.stageNumber = jsonData["stageNumber"].get<int>();
        replayData.clearTime = jsonData["clearTime"].get<float>();
        replayData.recordedDate = jsonData["recordedDate"].get<std::string>();
        replayData.frameRate = jsonData.contains("frameRate") ? jsonData["frameRate"].get<float>() : 0.1f;
        
        replayData.frames.clear();
        if (jsonData.contains("frames") && jsonData["frames"].is_array()) {
            for (const auto& frameJson : jsonData["frames"]) {
                ReplayFrame frame;
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
                
                // timeScaleは後方互換性のため、存在しない場合は1.0fをデフォルト値とする
                frame.timeScale = frameJson.contains("timeScale") ? frameJson["timeScale"].get<float>() : 1.0f;
                
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
        ErrorHandler::logErrorFormat("Failed to load replay: %s", e.what());
        return false;
    }
}

