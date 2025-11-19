#include "online_leaderboard_manager.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>

// 静的メンバ変数の定義
std::string OnlineLeaderboardManager::baseUrl = "http://localhost:3000";
std::string OnlineLeaderboardManager::playerName = "Player";
bool OnlineLeaderboardManager::onlineEnabled = true;

// libcurl用のコールバック関数
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append((char*)contents, totalSize);
    return totalSize;
}

void OnlineLeaderboardManager::setBaseUrl(const std::string& url) {
    baseUrl = url;
}

void OnlineLeaderboardManager::setPlayerName(const std::string& name) {
    playerName = name;
    printf("ONLINE: setPlayerName called with: [%s] (length: %zu), playerName is now: [%s]\n", 
           name.c_str(), name.length(), playerName.c_str());
}

std::string OnlineLeaderboardManager::getPlayerName() {
    return playerName;
}

bool OnlineLeaderboardManager::isOnlineEnabled() {
    return onlineEnabled;
}

bool OnlineLeaderboardManager::loadConfigFromFile() {
    // まずassets/config/leaderboard_config.jsonを試す（buildフォルダから実行される場合）
    std::string configPath = "../assets/config/leaderboard_config.json";
    std::ifstream file(configPath);
    
    // 見つからない場合はassets/config/leaderboard_config.jsonを直接試す
    if (!file.is_open()) {
        configPath = "assets/config/leaderboard_config.json";
        file.open(configPath);
    }
    
    // それでも見つからない場合はbuild/assets/config/leaderboard_config.jsonを試す
    if (!file.is_open()) {
        configPath = "build/assets/config/leaderboard_config.json";
        file.open(configPath);
    }
    
    if (!file.is_open()) {
        printf("Leaderboard Config: File not found, using default values (localhost:3000)\n");
        return false;
    }
    
    try {
        nlohmann::json jsonData;
        file >> jsonData;
        file.close();
        
        if (jsonData.contains("baseUrl") && jsonData["baseUrl"].is_string()) {
            baseUrl = jsonData["baseUrl"].get<std::string>();
            printf("Leaderboard Config: Loaded baseUrl from %s: %s\n", configPath.c_str(), baseUrl.c_str());
        }
        
        if (jsonData.contains("enabled") && jsonData["enabled"].is_boolean()) {
            onlineEnabled = jsonData["enabled"].get<bool>();
            printf("Leaderboard Config: Online enabled: %s\n", onlineEnabled ? "true" : "false");
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Leaderboard Config: Failed to parse JSON: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool OnlineLeaderboardManager::httpGet(const std::string& url, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    response.clear();
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // 5秒タイムアウト
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L); // 3秒接続タイムアウト
    
    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK || httpCode != 200) {
        std::cerr << "HTTP GET failed: " << curl_easy_strerror(res) << " (HTTP " << httpCode << ")" << std::endl;
        return false;
    }
    
    return true;
}

bool OnlineLeaderboardManager::httpPost(const std::string& url, const std::string& jsonData, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    response.clear();
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L);
    
    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK || httpCode != 200) {
        std::cerr << "HTTP POST failed: " << curl_easy_strerror(res) << " (HTTP " << httpCode << ")" << std::endl;
        return false;
    }
    
    return true;
}

bool OnlineLeaderboardManager::parseLeaderboardJson(const std::string& jsonStr, std::vector<LeaderboardEntry>& entries) {
    try {
        auto json = nlohmann::json::parse(jsonStr);
        
        if (!json.contains("records") || !json["records"].is_array()) {
            return false;
        }
        
        entries.clear();
        for (const auto& record : json["records"]) {
            LeaderboardEntry entry;
            entry.id = record.value("id", 0);
            entry.playerName = record.value("playerName", "");
            entry.time = record.value("time", 0.0f);
            entry.timestamp = record.value("timestamp", "");
            entry.hasReplay = record.value("hasReplay", false);
            
            // デバッグ: 取得したデータをログ出力
            std::cout << "DEBUG: Parsed entry - id: " << entry.id 
                      << ", playerName: [" << entry.playerName 
                      << "] (length: " << entry.playerName.length() << "), time: " << entry.time 
                      << ", hasReplay: " << (entry.hasReplay ? "true" : "false") << std::endl;
            
            entries.push_back(entry);
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

bool OnlineLeaderboardManager::parseGlobalTopJson(const std::string& jsonStr, std::map<int, LeaderboardEntry>& topRecords) {
    try {
        auto json = nlohmann::json::parse(jsonStr);
        
        if (!json.contains("topRecords") || !json["topRecords"].is_object()) {
            return false;
        }
        
        topRecords.clear();
        for (auto& [key, value] : json["topRecords"].items()) {
            int stageNumber = std::stoi(key);
            LeaderboardEntry entry;
            entry.playerName = value.value("playerName", "");
            entry.time = value.value("time", 0.0f);
            entry.timestamp = value.value("timestamp", "");
            topRecords[stageNumber] = entry;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

void OnlineLeaderboardManager::fetchLeaderboard(int stageNumber, 
                                                 std::function<void(const std::vector<LeaderboardEntry>&)> callback) {
    if (!onlineEnabled) {
        if (callback) {
            callback(std::vector<LeaderboardEntry>());
        }
        return;
    }
    
    // 別スレッドで非同期実行
    std::thread([stageNumber, callback]() {
        std::string url = baseUrl + "/api/leaderboard/" + std::to_string(stageNumber);
        std::string response;
        
        std::vector<LeaderboardEntry> entries;
        if (httpGet(url, response)) {
            parseLeaderboardJson(response, entries);
        }
        
        if (callback) {
            callback(entries);
        }
    }).detach();
}

void OnlineLeaderboardManager::submitTime(int stageNumber, float time, 
                                          std::function<void(bool)> callback,
                                          const ReplayData* replayData) {
    if (!onlineEnabled) {
        if (callback) {
            callback(false);
        }
        return;
    }
    
    // 別スレッドで非同期実行
    std::thread([stageNumber, time, callback, replayData]() {
        std::string url = baseUrl + "/api/leaderboard";
        
        // プレイヤー名を処理（空または非ASCII文字のみの場合は"UNKNOWN"に変換、大文字に変換）
        std::string processedPlayerName = playerName;
        printf("ONLINE: Submitting time - original playerName: [%s] (length: %zu)\n", 
               playerName.c_str(), playerName.length());
        if (processedPlayerName.empty()) {
            processedPlayerName = "UNKNOWN";
        } else {
            // ASCII文字のみを抽出し、大文字に変換
            std::string asciiName;
            for (char c : processedPlayerName) {
                if (c >= 32 && c <= 126) { // ASCII文字範囲
                    // 小文字を大文字に変換
                    if (c >= 'a' && c <= 'z') {
                        asciiName += (c - 'a' + 'A');
                    } else {
                        asciiName += c;
                    }
                }
            }
            
            if (asciiName.empty()) {
                // ASCII文字が1つもない場合は"UNKNOWN"に変換
                processedPlayerName = "UNKNOWN";
            } else {
                processedPlayerName = asciiName;
            }
        }
        
        nlohmann::json jsonData;
        jsonData["stageNumber"] = stageNumber;
        jsonData["time"] = time;
        jsonData["playerName"] = processedPlayerName;
        
        // リプレイデータがあれば追加
        if (replayData != nullptr) {
            nlohmann::json replayJson;
            replayJson["stageNumber"] = replayData->stageNumber;
            replayJson["clearTime"] = replayData->clearTime;
            replayJson["recordedDate"] = replayData->recordedDate;
            replayJson["frameRate"] = replayData->frameRate;
            
            nlohmann::json framesArray = nlohmann::json::array();
            for (const auto& frame : replayData->frames) {
                nlohmann::json frameJson;
                frameJson["timestamp"] = frame.timestamp;
                frameJson["playerPosition"] = {frame.playerPosition.x, frame.playerPosition.y, frame.playerPosition.z};
                frameJson["playerVelocity"] = {frame.playerVelocity.x, frame.playerVelocity.y, frame.playerVelocity.z};
                if (!frame.itemCollectedStates.empty()) {
                    frameJson["itemCollectedStates"] = frame.itemCollectedStates;
                }
                framesArray.push_back(frameJson);
            }
            replayJson["frames"] = framesArray;
            jsonData["replayData"] = replayJson;
            
            printf("ONLINE: Submitting time with replay data (%zu frames)\n", replayData->frames.size());
        }
        
        printf("ONLINE: Submitting time - processed playerName: [%s] (length: %zu)\n", 
               processedPlayerName.c_str(), processedPlayerName.length());
        
        std::string jsonStr = jsonData.dump();
        printf("ONLINE: JSON payload size: %zu bytes\n", jsonStr.length());
        std::string response;
        
        bool success = httpPost(url, jsonStr, response);
        
        if (success) {
            try {
                auto json = nlohmann::json::parse(response);
                success = json.value("success", false);
            } catch (...) {
                success = false;
            }
        }
        
        if (callback) {
            callback(success);
        }
    }).detach();
}

void OnlineLeaderboardManager::fetchReplay(int leaderboardId,
                                          std::function<void(const ReplayData*)> callback) {
    if (!onlineEnabled) {
        if (callback) {
            callback(nullptr);
        }
        return;
    }
    
    // 別スレッドで非同期実行
    std::thread([leaderboardId, callback]() {
        std::string url = baseUrl + "/api/replay/" + std::to_string(leaderboardId);
        std::string response;
        
        ReplayData* replayData = nullptr;
        if (httpGet(url, response)) {
            try {
                auto json = nlohmann::json::parse(response);
                if (json.value("success", false) && json.contains("replayData")) {
                    replayData = new ReplayData();
                    const auto& replayJson = json["replayData"];
                    
                    replayData->stageNumber = replayJson.value("stageNumber", 0);
                    replayData->clearTime = replayJson.value("clearTime", 0.0f);
                    replayData->recordedDate = replayJson.value("recordedDate", "");
                    replayData->frameRate = replayJson.value("frameRate", 0.1f);
                    
                    if (replayJson.contains("frames") && replayJson["frames"].is_array()) {
                        for (const auto& frameJson : replayJson["frames"]) {
                            ReplayFrame frame;
                            frame.timestamp = frameJson.value("timestamp", 0.0f);
                            
                            if (frameJson.contains("playerPosition") && frameJson["playerPosition"].is_array() && frameJson["playerPosition"].size() == 3) {
                                frame.playerPosition.x = frameJson["playerPosition"][0];
                                frame.playerPosition.y = frameJson["playerPosition"][1];
                                frame.playerPosition.z = frameJson["playerPosition"][2];
                            }
                            
                            if (frameJson.contains("playerVelocity") && frameJson["playerVelocity"].is_array() && frameJson["playerVelocity"].size() == 3) {
                                frame.playerVelocity.x = frameJson["playerVelocity"][0];
                                frame.playerVelocity.y = frameJson["playerVelocity"][1];
                                frame.playerVelocity.z = frameJson["playerVelocity"][2];
                            }
                            
                            if (frameJson.contains("itemCollectedStates") && frameJson["itemCollectedStates"].is_array()) {
                                for (const auto& state : frameJson["itemCollectedStates"]) {
                                    frame.itemCollectedStates.push_back(state.get<bool>());
                                }
                            }
                            
                            replayData->frames.push_back(frame);
                        }
                    }
                    
                    printf("ONLINE: Loaded replay data for leaderboard ID %d (%zu frames)\n", 
                           leaderboardId, replayData->frames.size());
                }
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse replay JSON: " << e.what() << std::endl;
            }
        }
        
        if (callback) {
            callback(replayData);
        }
        
        // メモリを解放（コールバックでコピーを作成する想定）
        if (replayData) {
            delete replayData;
        }
    }).detach();
}

void OnlineLeaderboardManager::fetchGlobalTopRecords(
    std::function<void(const std::map<int, LeaderboardEntry>&)> callback) {
    if (!onlineEnabled) {
        if (callback) {
            callback(std::map<int, LeaderboardEntry>());
        }
        return;
    }
    
    // 別スレッドで非同期実行
    std::thread([callback]() {
        std::string url = baseUrl + "/api/leaderboard/global/top";
        std::string response;
        
        std::map<int, LeaderboardEntry> topRecords;
        if (httpGet(url, response)) {
            parseGlobalTopJson(response, topRecords);
        }
        
        if (callback) {
            callback(topRecords);
        }
    }).detach();
}

