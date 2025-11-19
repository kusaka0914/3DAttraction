#include "network_manager.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

NetworkManager::NetworkManager()
    : isConnected_(false)
    , isHost_(false)
    , shouldStop_(false)
#ifdef _WIN32
    , serverSocket_(INVALID_SOCKET)
    , clientSocket_(INVALID_SOCKET)
#else
    , serverSocket_(-1)
    , clientSocket_(-1)
#endif
    , hasNewInput_(false)
    , hasNewGameState_(false)
    , hasGoalReached_(false)
    , goalReachedPlayerId_(-1)
    , goalReachedClearTime_(0.0f)
    , hasNewPlatformStates_(false)
#ifdef _WIN32
    , wsaInitialized_(false)
#endif
{
    initializeSocket();
}

NetworkManager::~NetworkManager() {
    disconnect();
    cleanupSocket();
}

bool NetworkManager::initializeSocket() {
#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }
    wsaInitialized_ = true;
#endif
    return true;
}

void NetworkManager::cleanupSocket() {
#ifdef _WIN32
    if (wsaInitialized_) {
        WSACleanup();
        wsaInitialized_ = false;
    }
#endif
}

bool NetworkManager::startHost(int port) {
    if (isConnected_) {
        disconnect();
    }
    
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (serverSocket_ == INVALID_SOCKET) {
#else
    if (serverSocket_ < 0) {
#endif
        std::cerr << "Failed to create server socket" << std::endl;
        return false;
    }
    
    // ソケットオプション設定（アドレス再利用）
    int opt = 1;
#ifdef _WIN32
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind server socket" << std::endl;
#ifdef _WIN32
        closesocket(serverSocket_);
        serverSocket_ = INVALID_SOCKET;
#else
        close(serverSocket_);
        serverSocket_ = -1;
#endif
        return false;
    }
    
    if (listen(serverSocket_, 1) < 0) {
        std::cerr << "Failed to listen on server socket" << std::endl;
#ifdef _WIN32
        closesocket(serverSocket_);
        serverSocket_ = INVALID_SOCKET;
#else
        close(serverSocket_);
        serverSocket_ = -1;
#endif
        return false;
    }
    
    std::cout << "Host: Waiting for client connection on port " << port << std::endl;
    
    // 非ブロッキングモードに設定
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(serverSocket_, FIONBIO, &mode);
#else
    int flags = fcntl(serverSocket_, F_GETFL, 0);
    fcntl(serverSocket_, F_SETFL, flags | O_NONBLOCK);
#endif
    
    // 接続待ちは別スレッドで行う（startHostは即座に返す）
    isHost_ = true;
    
    // 接続待ちスレッドを開始
    std::thread([this]() {
        while (!shouldStop_ && !isConnected_) {
            sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            clientSocket_ = accept(serverSocket_, (sockaddr*)&clientAddr, &clientAddrLen);
            
#ifdef _WIN32
            if (clientSocket_ != INVALID_SOCKET) {
#else
            if (clientSocket_ >= 0) {
#endif
                isConnected_ = true;
                std::cout << "Host: Client connected from " << inet_ntoa(clientAddr.sin_addr) << std::endl;
                
                // 受信スレッドを開始
                receiveThread_ = std::thread(&NetworkManager::receiveThread, this);
                
                if (connectionCallback_) {
                    connectionCallback_(true);
                }
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
    
    return true;
}

bool NetworkManager::connectToHost(const std::string& ipAddress, int port) {
    if (isConnected_) {
        disconnect();
    }
    
    clientSocket_ = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (clientSocket_ == INVALID_SOCKET) {
#else
    if (clientSocket_ < 0) {
#endif
        std::cerr << "Failed to create client socket" << std::endl;
        return false;
    }
    
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << ipAddress << std::endl;
#ifdef _WIN32
        closesocket(clientSocket_);
        clientSocket_ = INVALID_SOCKET;
#else
        close(clientSocket_);
        clientSocket_ = -1;
#endif
        return false;
    }
    
    std::cout << "Client: Connecting to " << ipAddress << ":" << port << std::endl;
    
    if (connect(clientSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to connect to server" << std::endl;
#ifdef _WIN32
        closesocket(clientSocket_);
        clientSocket_ = INVALID_SOCKET;
#else
        close(clientSocket_);
        clientSocket_ = -1;
#endif
        return false;
    }
    
    // クライアントソケットも非ブロッキングモードに設定
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(clientSocket_, FIONBIO, &mode);
#else
    int flags = fcntl(clientSocket_, F_GETFL, 0);
    fcntl(clientSocket_, F_SETFL, flags | O_NONBLOCK);
#endif
    
    isHost_ = false;
    isConnected_ = true;
    shouldStop_ = false;
    
    std::cout << "Client: Connected to server" << std::endl;
    
    // 受信スレッドを開始
    receiveThread_ = std::thread(&NetworkManager::receiveThread, this);
    
    if (connectionCallback_) {
        connectionCallback_(true);
    }
    
    return true;
}

void NetworkManager::disconnect() {
    shouldStop_ = true;
    
    if (receiveThread_.joinable()) {
        receiveThread_.join();
    }
    
#ifdef _WIN32
    if (clientSocket_ != INVALID_SOCKET) {
        closesocket(clientSocket_);
        clientSocket_ = INVALID_SOCKET;
    }
    
    if (serverSocket_ != INVALID_SOCKET) {
        closesocket(serverSocket_);
        serverSocket_ = INVALID_SOCKET;
    }
#else
    if (clientSocket_ >= 0) {
        close(clientSocket_);
        clientSocket_ = -1;
    }
    
    if (serverSocket_ >= 0) {
        close(serverSocket_);
        serverSocket_ = -1;
    }
#endif
    
    isConnected_ = false;
    
    if (connectionCallback_) {
        connectionCallback_(false);
    }
}

bool NetworkManager::sendPlayerInput(const PlayerInputData& input) {
#ifdef _WIN32
    if (!isConnected_ || clientSocket_ == INVALID_SOCKET) {
#else
    if (!isConnected_ || clientSocket_ < 0) {
#endif
        return false;
    }
    
    return sendMessage(NetworkMessageType::PLAYER_INPUT, &input, sizeof(input));
}

bool NetworkManager::sendGameStateUpdate(const PlayerStateData& playerState, const PlayerStateData& remotePlayerState) {
#ifdef _WIN32
    if (!isConnected_ || !isHost_ || clientSocket_ == INVALID_SOCKET) {
#else
    if (!isConnected_ || !isHost_ || clientSocket_ < 0) {
#endif
        return false;
    }
    
    struct GameStateUpdateData {
        PlayerStateData playerState;
        PlayerStateData remotePlayerState;
    };
    
    GameStateUpdateData data;
    data.playerState = playerState;
    data.remotePlayerState = remotePlayerState;
    
    return sendMessage(NetworkMessageType::GAME_STATE_UPDATE, &data, sizeof(data));
}

bool NetworkManager::sendGoalReached(int playerId, float clearTime) {
#ifdef _WIN32
    if (!isConnected_ || clientSocket_ == INVALID_SOCKET) {
#else
    if (!isConnected_ || clientSocket_ < 0) {
#endif
        return false;
    }
    
    struct GoalReachedData {
        int playerId;
        float clearTime;
    };
    
    GoalReachedData data;
    data.playerId = playerId;
    data.clearTime = clearTime;
    
    return sendMessage(NetworkMessageType::GOAL_REACHED, &data, sizeof(data));
}

bool NetworkManager::getReceivedPlayerInput(PlayerInputData& input) {
    std::lock_guard<std::mutex> lock(receiveMutex_);
    if (hasNewInput_) {
        input = receivedInput_;
        hasNewInput_ = false;
        return true;
    }
    return false;
}

bool NetworkManager::getReceivedGameState(PlayerStateData& playerState, PlayerStateData& remotePlayerState) {
    std::lock_guard<std::mutex> lock(receiveMutex_);
    if (hasNewGameState_) {
        playerState = receivedPlayerState_;
        remotePlayerState = receivedRemotePlayerState_;
        hasNewGameState_ = false;
        return true;
    }
    return false;
}

bool NetworkManager::getGoalReached(int& playerId, float& clearTime) {
    std::lock_guard<std::mutex> lock(receiveMutex_);
    if (hasGoalReached_) {
        playerId = goalReachedPlayerId_;
        clearTime = goalReachedClearTime_;
        hasGoalReached_ = false;
        return true;
    }
    return false;
}

bool NetworkManager::sendPlatformStates(const std::vector<PlatformStateData>& platformStates) {
    if (!isConnected_) {
        return false;
    }
    
    // プラットフォーム数 + プラットフォームデータを送信
    size_t dataSize = sizeof(int) + platformStates.size() * sizeof(PlatformStateData);
    std::vector<char> buffer(dataSize);
    
    // プラットフォーム数を先頭に書き込む
    int platformCount = static_cast<int>(platformStates.size());
    std::memcpy(buffer.data(), &platformCount, sizeof(int));
    
    // プラットフォームデータを書き込む
    size_t offset = sizeof(int);
    for (const auto& platform : platformStates) {
        std::memcpy(buffer.data() + offset, &platform, sizeof(PlatformStateData));
        offset += sizeof(PlatformStateData);
    }
    
    return sendMessage(NetworkMessageType::PLATFORM_STATE_UPDATE, buffer.data(), dataSize);
}

bool NetworkManager::getReceivedPlatformStates(std::vector<PlatformStateData>& platformStates) {
    std::lock_guard<std::mutex> lock(receiveMutex_);
    if (hasNewPlatformStates_) {
        platformStates = receivedPlatformStates_;
        hasNewPlatformStates_ = false;
        return true;
    }
    return false;
}

void NetworkManager::setConnectionCallback(std::function<void(bool)> callback) {
    connectionCallback_ = callback;
}

void NetworkManager::receiveThread() {
    while (!shouldStop_ && isConnected_) {
        NetworkMessageType type;
        char buffer[1024];
        size_t actualSize;
        
        if (receiveMessage(type, buffer, sizeof(buffer), actualSize)) {
            std::lock_guard<std::mutex> lock(receiveMutex_);
            
            switch (type) {
                case NetworkMessageType::PLAYER_INPUT: {
                    if (actualSize >= sizeof(PlayerInputData)) {
                        receivedInput_ = *reinterpret_cast<PlayerInputData*>(buffer);
                        hasNewInput_ = true;
                    }
                    break;
                }
                case NetworkMessageType::GAME_STATE_UPDATE: {
                    struct GameStateUpdateData {
                        PlayerStateData playerState;
                        PlayerStateData remotePlayerState;
                    };
                    if (actualSize >= sizeof(GameStateUpdateData)) {
                        auto* data = reinterpret_cast<GameStateUpdateData*>(buffer);
                        receivedPlayerState_ = data->playerState;
                        receivedRemotePlayerState_ = data->remotePlayerState;
                        hasNewGameState_ = true;
                    }
                    break;
                }
                case NetworkMessageType::GOAL_REACHED: {
                    struct GoalReachedData {
                        int playerId;
                        float clearTime;
                    };
                    if (actualSize >= sizeof(GoalReachedData)) {
                        auto* data = reinterpret_cast<GoalReachedData*>(buffer);
                        goalReachedPlayerId_ = data->playerId;
                        goalReachedClearTime_ = data->clearTime;
                        hasGoalReached_ = true;
                    }
                    break;
                }
                case NetworkMessageType::PLATFORM_STATE_UPDATE: {
                    // プラットフォーム状態の受信
                    // 先頭にプラットフォーム数を読み取る
                    if (actualSize >= sizeof(int)) {
                        int platformCount = *reinterpret_cast<int*>(buffer);
                        size_t offset = sizeof(int);
                        receivedPlatformStates_.clear();
                        
                        for (int i = 0; i < platformCount && offset + sizeof(PlatformStateData) <= actualSize; ++i) {
                            PlatformStateData platformData = *reinterpret_cast<PlatformStateData*>(buffer + offset);
                            receivedPlatformStates_.push_back(platformData);
                            offset += sizeof(PlatformStateData);
                        }
                        hasNewPlatformStates_ = true;
                    }
                    break;
                }
                case NetworkMessageType::DISCONNECT: {
                    isConnected_ = false;
                    shouldStop_ = true;
                    break;
                }
                default:
                    break;
            }
        } else {
            // 接続エラーまたはタイムアウト
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

bool NetworkManager::sendMessage(NetworkMessageType type, const void* data, size_t dataSize) {
#ifdef _WIN32
    SOCKET socket = clientSocket_;
    if (socket == INVALID_SOCKET) {
        return false;
    }
#else
    int socket = clientSocket_;
    if (socket < 0) {
        return false;
    }
#endif
    
    // メッセージヘッダー（タイプ + データサイズ）
    struct MessageHeader {
        NetworkMessageType type;
        size_t dataSize;
    };
    
    MessageHeader header;
    header.type = type;
    header.dataSize = dataSize;
    
    // ヘッダーを送信
#ifdef _WIN32
    int sent = send(socket, reinterpret_cast<const char*>(&header), sizeof(header), 0);
#else
    ssize_t sent = send(socket, reinterpret_cast<const char*>(&header), sizeof(header), 0);
#endif
    if (sent != static_cast<int>(sizeof(header))) {
        return false;
    }
    
    // データを送信
    if (dataSize > 0) {
#ifdef _WIN32
        sent = send(socket, reinterpret_cast<const char*>(data), static_cast<int>(dataSize), 0);
#else
        sent = send(socket, reinterpret_cast<const char*>(data), dataSize, 0);
#endif
        if (sent != static_cast<int>(dataSize)) {
            return false;
        }
    }
    
    return true;
}

bool NetworkManager::receiveMessage(NetworkMessageType& type, void* data, size_t maxDataSize, size_t& actualDataSize) {
#ifdef _WIN32
    SOCKET socket = clientSocket_;
    if (socket == INVALID_SOCKET) {
        return false;
    }
#else
    int socket = clientSocket_;
    if (socket < 0) {
        return false;
    }
#endif
    
    // メッセージヘッダーを受信
    struct MessageHeader {
        NetworkMessageType type;
        size_t dataSize;
    };
    
    MessageHeader header;
#ifdef _WIN32
    int received = recv(socket, reinterpret_cast<char*>(&header), sizeof(header), 0);
    if (received < 0) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS) {
            // データがまだない（これは正常）
            return false;
        }
        // その他のエラー
        isConnected_ = false;
        return false;
    }
#else
    ssize_t received = recv(socket, reinterpret_cast<char*>(&header), sizeof(header), MSG_DONTWAIT);
#endif
    
    if (received <= 0) {
        if (received == 0) {
            // 接続が閉じられた
            isConnected_ = false;
        }
        return false;
    }
    
    if (received != static_cast<int>(sizeof(header))) {
        return false;
    }
    
    type = header.type;
    actualDataSize = header.dataSize;
    
    // データを受信
    if (actualDataSize > 0 && actualDataSize <= maxDataSize) {
#ifdef _WIN32
        int received = recv(socket, reinterpret_cast<char*>(data), static_cast<int>(actualDataSize), 0);
        if (received < 0) {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS) {
                return false;
            }
            isConnected_ = false;
            return false;
        }
        if (received != static_cast<int>(actualDataSize)) {
            return false;
        }
#else
        ssize_t received = recv(socket, reinterpret_cast<char*>(data), actualDataSize, MSG_DONTWAIT);
        if (received != static_cast<int>(actualDataSize)) {
            return false;
        }
#endif
    }
    
    return true;
}

