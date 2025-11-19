/**
 * @file multiplayer_manager.h
 * @brief マルチプレイを管理するクラス
 * @details ネットワーク通信とゲーム状態の同期を統合的に管理します。
 */
#pragma once

#include "../io/network_manager.h"
#include "game_state.h"
#include "platform_system.h"
#include <GLFW/glfw3.h>

namespace io {
    class AudioManager;
}

class StageManager;

/**
 * @brief マルチプレイマネージャー
 * @details ネットワーク通信とゲーム状態の同期を統合的に管理します。
 * ホスト/クライアントの役割分担、入力の送受信、ゲーム状態の同期を担当します。
 */
class MultiplayerManager {
public:
    /**
     * @brief コンストラクタ
     */
    MultiplayerManager();
    
    /**
     * @brief デストラクタ
     */
    ~MultiplayerManager();
    
    /**
     * @brief ホストとしてサーバーを開始する
     * @param port ポート番号
     * @return 成功時true
     */
    bool startHost(int port = 12345);
    
    /**
     * @brief クライアントとしてサーバーに接続する
     * @param ipAddress サーバーのIPアドレス
     * @param port ポート番号
     * @return 成功時true
     */
    bool connectToHost(const std::string& ipAddress, int port = 12345);
    
    /**
     * @brief 接続を切断する
     */
    void disconnect();
    
    /**
     * @brief 接続状態を取得する
     * @return 接続中の場合true
     */
    bool isConnected() const { return networkManager_.isConnected(); }
    
    /**
     * @brief ホストかどうかを取得する
     * @return ホストの場合true
     */
    bool isHost() const { return networkManager_.isHost(); }
    
    /**
     * @brief マルチプレイモードを更新する
     * @details 入力の送受信、ゲーム状態の同期を実行します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @param deltaTime デルタタイム
     * @param audioManager オーディオマネージャー
     */
    void update(GLFWwindow* window, GameState& gameState, PlatformSystem& platformSystem, float deltaTime, io::AudioManager& audioManager);
    
    /**
     * @brief プレイヤー入力を送信する
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param deltaTime デルタタイム
     */
    void sendPlayerInput(GLFWwindow* window, const GameState& gameState, float deltaTime);
    
    /**
     * @brief リモートプレイヤーの入力を適用する
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @param deltaTime デルタタイム
     * @param audioManager オーディオマネージャー
     */
    void applyRemotePlayerInput(GameState& gameState, PlatformSystem& platformSystem, float deltaTime, io::AudioManager& audioManager);
    
    /**
     * @brief ゲーム状態を送信する（ホストのみ）
     * @param gameState ゲーム状態
     */
    void sendGameState(const GameState& gameState);
    
    /**
     * @brief リモートプレイヤーの状態を更新する
     * @param gameState ゲーム状態
     * @param deltaTime デルタタイム
     */
    void updateRemotePlayer(GameState& gameState, float deltaTime);
    
    /**
     * @brief ゴール到達を送信する
     * @param playerId プレイヤーID（0=ローカル、1=リモート）
     * @param clearTime クリアタイム
     */
    void sendGoalReached(int playerId, float clearTime);
    
    /**
     * @brief ゴール到達通知をチェックする
     * @param gameState ゲーム状態
     */
    void checkGoalReached(GameState& gameState);
    
    /**
     * @brief ステージ選択を送信する（ホストのみ）
     * @param stageNumber ステージ番号
     */
    void sendStageSelection(int stageNumber);
    
    /**
     * @brief ステージ選択通知をチェックする（クライアントのみ）
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     * @param window GLFWウィンドウ
     * @return ステージ選択通知があった場合true
     */
    bool checkStageSelection(GameState& gameState, StageManager& stageManager, PlatformSystem& platformSystem, std::function<void()> resetStageStartTime, GLFWwindow* window);
    
    /**
     * @brief プラットフォーム状態を送信する（ホストのみ）
     * @param platformSystem プラットフォームシステム
     */
    void sendPlatformStates(const PlatformSystem& platformSystem);
    
    /**
     * @brief 受信したプラットフォーム状態を適用する（クライアントのみ）
     * @param platformSystem プラットフォームシステム
     */
    void applyPlatformStates(PlatformSystem& platformSystem);

private:
    NetworkManager networkManager_;
    
    /**
     * @brief プレイヤー状態をPlayerStateDataに変換する
     * @param playerState プレイヤー状態
     * @return PlayerStateData
     */
    PlayerStateData convertToPlayerStateData(const PlayerState& playerState);
    
    /**
     * @brief PlayerStateDataをプレイヤー状態に適用する
     * @param playerState プレイヤー状態
     * @param data PlayerStateData
     */
    void applyPlayerStateData(PlayerState& playerState, const PlayerStateData& data);
};

