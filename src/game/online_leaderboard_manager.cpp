#include "online_leaderboard_manager.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <iostream>
#include <sstream>

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
}

bool OnlineLeaderboardManager::isOnlineEnabled() {
    return onlineEnabled;
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
            entry.playerName = record.value("playerName", "");
            entry.time = record.value("time", 0.0f);
            entry.timestamp = record.value("timestamp", "");
            
            // デバッグ: 取得したデータをログ出力
            std::cout << "DEBUG: Parsed entry - playerName: [" << entry.playerName 
                      << "] (length: " << entry.playerName.length() << "), time: " << entry.time << std::endl;
            
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
                                          std::function<void(bool)> callback) {
    if (!onlineEnabled) {
        if (callback) {
            callback(false);
        }
        return;
    }
    
    // 別スレッドで非同期実行
    std::thread([stageNumber, time, callback]() {
        std::string url = baseUrl + "/api/leaderboard";
        
        // プレイヤー名を処理（空または非ASCII文字のみの場合は"UNKNOWN"に変換、大文字に変換）
        std::string processedPlayerName = playerName;
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
        
        std::string jsonStr = jsonData.dump();
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

