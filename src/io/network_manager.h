/**
 * @file network_manager.h
 * @brief ネットワーク通信を管理するクラス
 * @details TCPソケットを使用したローカルネットワーク通信を提供します。
 * ホスト/クライアントの役割分担、接続管理、メッセージの送受信を担当します。
 */
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <glm/glm.hpp>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

/**
 * @brief ネットワークメッセージのタイプ
 */
enum class NetworkMessageType {
    CONNECT_REQUEST,      /**< @brief 接続リクエスト */
    CONNECT_RESPONSE,     /**< @brief 接続レスポンス */
    PLAYER_INPUT,         /**< @brief プレイヤー入力 */
    GAME_STATE_UPDATE,    /**< @brief ゲーム状態更新 */
    PLATFORM_STATE_UPDATE, /**< @brief プラットフォーム状態更新 */
    GOAL_REACHED,         /**< @brief ゴール到達 */
    DISCONNECT            /**< @brief 切断 */
};

/**
 * @brief プレイヤー入力データ
 */
struct PlayerInputData {
    glm::vec3 moveDirection;
    bool jumpPressed;
    bool jumpJustPressed;
    float deltaTime;
};

/**
 * @brief プレイヤー状態データ（同期用）
 */
struct PlayerStateData {
    glm::vec3 position;
    glm::vec3 velocity;
    bool isMovingBackward;
    bool isShowingFrontTexture;
};

/**
 * @brief プラットフォームタイプ（同期用）
 */
enum class PlatformType : int {
    STATIC = 0,
    MOVING = 1,
    ROTATING = 2,
    PATROLLING = 3,
    CYCLE_DISAPPEARING = 4,
    OTHER = 5
};

/**
 * @brief プラットフォーム状態データ（同期用）
 */
struct PlatformStateData {
    int platformIndex;  /**< @brief プラットフォームインデックス */
    PlatformType platformType;  /**< @brief プラットフォームタイプ */
    glm::vec3 position;  /**< @brief 位置 */
    float rotationAngle;  /**< @brief 回転角度（RotatingPlatform用） */
    float moveTimer;  /**< @brief 移動タイマー（MovingPlatform用） */
    bool hasPlayerOnBoard;  /**< @brief プレイヤーが乗っているか（MovingPlatform用） */
    int currentPatrolIndex;  /**< @brief 現在の巡回インデックス（PatrollingPlatform用） */
    float patrolTimer;  /**< @brief 巡回タイマー（PatrollingPlatform用） */
    float cycleTimer;  /**< @brief サイクルタイマー（CycleDisappearingPlatform用） */
    bool isCurrentlyVisible;  /**< @brief 現在表示されているか（CycleDisappearingPlatform用） */
    float blinkAlpha;  /**< @brief 点滅アルファ値（CycleDisappearingPlatform用） */
};

/**
 * @brief ネットワークマネージャー
 * @details TCPソケットを使用したローカルネットワーク通信を管理します。
 * ホスト/クライアントの役割分担、接続管理、メッセージの送受信を担当します。
 */
class NetworkManager {
public:
    /**
     * @brief コンストラクタ
     */
    NetworkManager();
    
    /**
     * @brief デストラクタ
     */
    ~NetworkManager();
    
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
    bool isConnected() const { return isConnected_; }
    
    /**
     * @brief ホストかどうかを取得する
     * @return ホストの場合true
     */
    bool isHost() const { return isHost_; }
    
    /**
     * @brief プレイヤー入力を送信する
     * @param input 入力データ
     * @return 成功時true
     */
    bool sendPlayerInput(const PlayerInputData& input);
    
    /**
     * @brief ゲーム状態を送信する（ホストのみ）
     * @param playerState プレイヤー状態
     * @param remotePlayerState リモートプレイヤー状態
     * @return 成功時true
     */
    bool sendGameStateUpdate(const PlayerStateData& playerState, const PlayerStateData& remotePlayerState);
    
    /**
     * @brief ゴール到達を送信する
     * @param playerId プレイヤーID（0=ローカル、1=リモート）
     * @param clearTime クリアタイム
     * @return 成功時true
     */
    bool sendGoalReached(int playerId, float clearTime);
    
    /**
     * @brief 受信したプレイヤー入力を取得する
     * @param input 入力データを受け取る変数
     * @return 新しい入力がある場合true
     */
    bool getReceivedPlayerInput(PlayerInputData& input);
    
    /**
     * @brief 受信したゲーム状態を取得する
     * @param playerState プレイヤー状態を受け取る変数
     * @param remotePlayerState リモートプレイヤー状態を受け取る変数
     * @return 新しい状態がある場合true
     */
    bool getReceivedGameState(PlayerStateData& playerState, PlayerStateData& remotePlayerState);
    
    /**
     * @brief ゴール到達通知を受信したかどうかを取得する
     * @param playerId プレイヤーIDを受け取る変数
     * @param clearTime クリアタイムを受け取る変数
     * @return ゴール到達通知がある場合true
     */
    bool getGoalReached(int& playerId, float& clearTime);
    
    /**
     * @brief プラットフォーム状態を送信する（ホストのみ）
     * @param platformStates プラットフォーム状態のベクター
     * @return 成功時true
     */
    bool sendPlatformStates(const std::vector<PlatformStateData>& platformStates);
    
    /**
     * @brief 受信したプラットフォーム状態を取得する
     * @param platformStates プラットフォーム状態を受け取るベクター
     * @return 新しい状態がある場合true
     */
    bool getReceivedPlatformStates(std::vector<PlatformStateData>& platformStates);
    
    /**
     * @brief 接続状態変更のコールバックを設定する
     * @param callback コールバック関数（接続時: true、切断時: false）
     */
    void setConnectionCallback(std::function<void(bool)> callback);
    
    /**
     * @brief ローカルIPアドレスを取得する
     * @return ローカルIPアドレス（取得失敗時は空文字列）
     */
    static std::string getLocalIPAddress();

private:
    /**
     * @brief ソケットを初期化する
     * @return 成功時true
     */
    bool initializeSocket();
    
    /**
     * @brief ソケットをクリーンアップする
     */
    void cleanupSocket();
    
    /**
     * @brief 受信スレッドの処理
     */
    void receiveThread();
    
    /**
     * @brief メッセージを送信する
     * @param type メッセージタイプ
     * @param data データ
     * @param dataSize データサイズ
     * @return 成功時true
     */
    bool sendMessage(NetworkMessageType type, const void* data, size_t dataSize);
    
    /**
     * @brief メッセージを受信する
     * @param type メッセージタイプを受け取る変数
     * @param data データを受け取るバッファ
     * @param maxDataSize 最大データサイズ
     * @param actualDataSize 実際のデータサイズを受け取る変数
     * @return 成功時true
     */
    bool receiveMessage(NetworkMessageType& type, void* data, size_t maxDataSize, size_t& actualDataSize);
    
    std::atomic<bool> isConnected_;
    std::atomic<bool> isHost_;
    std::atomic<bool> shouldStop_;
    
#ifdef _WIN32
    SOCKET serverSocket_;
    SOCKET clientSocket_;
    static constexpr SOCKET INVALID_SOCKET_VALUE = INVALID_SOCKET;
#else
    int serverSocket_;
    int clientSocket_;
    static constexpr int INVALID_SOCKET_VALUE = -1;
#endif
    
    std::thread receiveThread_;
    std::mutex receiveMutex_;
    
    PlayerInputData receivedInput_;
    bool hasNewInput_;
    
    PlayerStateData receivedPlayerState_;
    PlayerStateData receivedRemotePlayerState_;
    bool hasNewGameState_;
    
    int goalReachedPlayerId_;
    float goalReachedClearTime_;
    bool hasGoalReached_;
    
    std::vector<PlatformStateData> receivedPlatformStates_;
    bool hasNewPlatformStates_;
    
    std::function<void(bool)> connectionCallback_;
    
#ifdef _WIN32
    WSADATA wsaData_;
    bool wsaInitialized_;
#endif
};

