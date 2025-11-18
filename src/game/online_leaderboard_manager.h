/**
 * @file online_leaderboard_manager.h
 * @brief オンラインランキングマネージャー
 * @details オンラインランキングAPIとの通信を管理します。タイムアタック記録の送信とランキングデータの取得を行います。
 */
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <string>
#include <vector>
#include <map>
#include <functional>

/**
 * @brief ランキングエントリ
 * @details ランキングに表示される1つの記録を表します。
 */
struct LeaderboardEntry {
    std::string playerName;  /**< @brief プレイヤー名 */
    float time;  /**< @brief クリアタイム（秒） */
    std::string timestamp;  /**< @brief 記録日時 */
};

/**
 * @brief オンラインランキングマネージャー
 * @details オンラインランキングAPIとの通信を管理します。
 * タイムアタック記録の送信とランキングデータの取得を非同期で行います。
 */
class OnlineLeaderboardManager {
public:
    /**
     * @brief APIベースURLを設定する
     * @details デフォルトは http://localhost:3000 です。
     * 
     * @param baseUrl APIベースURL
     */
    static void setBaseUrl(const std::string& baseUrl);
    
    /**
     * @brief プレイヤー名を設定する
     * @details タイム記録送信時に使用されます。
     * 
     * @param playerName プレイヤー名
     */
    static void setPlayerName(const std::string& playerName);
    
    /**
     * @brief ステージ別ランキングを取得する
     * @details 非同期でAPIからランキングデータを取得します。
     * 
     * @param stageNumber ステージ番号（1-5）
     * @param callback 取得完了時のコールバック関数（成功時: entries, 失敗時: 空配列）
     */
    static void fetchLeaderboard(int stageNumber, 
                                 std::function<void(const std::vector<LeaderboardEntry>&)> callback);
    
    /**
     * @brief タイム記録を送信する
     * @details 非同期でAPIにタイム記録を送信します。
     * 
     * @param stageNumber ステージ番号（1-5）
     * @param time クリアタイム（秒）
     * @param callback 送信完了時のコールバック関数（成功時: true, 失敗時: false）
     */
    static void submitTime(int stageNumber, float time, 
                          std::function<void(bool)> callback = nullptr);
    
    /**
     * @brief 全ステージのトップ記録を取得する
     * @details 非同期でAPIから全ステージのトップ記録を取得します。
     * 
     * @param callback 取得完了時のコールバック関数（成功時: topRecords, 失敗時: 空マップ）
     */
    static void fetchGlobalTopRecords(
        std::function<void(const std::map<int, LeaderboardEntry>&)> callback);
    
    /**
     * @brief オンライン機能が有効かどうかを確認する
     * @details ネットワーク接続やAPIの可用性を確認します。
     * 
     * @return オンライン機能が有効な場合true
     */
    static bool isOnlineEnabled();
    
    /**
     * @brief 設定ファイルから設定を読み込む
     * @details assets/config/leaderboard_config.jsonからbaseUrlとenabledを読み込みます。
     * ファイルが見つからない場合はデフォルト値（localhost:3000）を使用します。
     * 
     * @return 設定ファイルの読み込みに成功した場合true
     */
    static bool loadConfigFromFile();

private:
    static std::string baseUrl;  /**< @brief APIベースURL */
    static std::string playerName;  /**< @brief プレイヤー名 */
    static bool onlineEnabled;  /**< @brief オンライン機能が有効かどうか */
    
    /**
     * @brief HTTP GETリクエストを送信する
     * @param url リクエストURL
     * @param response レスポンスデータを受け取る文字列
     * @return 成功時true
     */
    static bool httpGet(const std::string& url, std::string& response);
    
    /**
     * @brief HTTP POSTリクエストを送信する
     * @param url リクエストURL
     * @param jsonData JSONデータ
     * @param response レスポンスデータを受け取る文字列
     * @return 成功時true
     */
    static bool httpPost(const std::string& url, const std::string& jsonData, std::string& response);
    
    /**
     * @brief JSON文字列をLeaderboardEntryのベクターに変換する
     * @param jsonStr JSON文字列
     * @param entries 変換結果を受け取るベクター
     * @return 成功時true
     */
    static bool parseLeaderboardJson(const std::string& jsonStr, std::vector<LeaderboardEntry>& entries);
    
    /**
     * @brief JSON文字列をグローバルトップ記録のマップに変換する
     * @param jsonStr JSON文字列
     * @param topRecords 変換結果を受け取るマップ
     * @return 成功時true
     */
    static bool parseGlobalTopJson(const std::string& jsonStr, std::map<int, LeaderboardEntry>& topRecords);
};

