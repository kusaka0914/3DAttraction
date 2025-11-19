#include "network_manager.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#else
#include <sys/select.h>
#include <errno.h>
#include <ifaddrs.h>
#include <netdb.h>
#endif

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
        int acceptAttempts = 0;
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
            } else {
                // 非ブロッキングモードなので、接続がない場合はエラーになる
#ifdef _WIN32
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS) {
                    // 予期しないエラーの場合のみログ出力
                    std::cerr << "Host: accept error: " << error << std::endl;
                }
#else
                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                    // 予期しないエラーの場合のみログ出力
                    std::cerr << "Host: accept error: " << errno << " (" << strerror(errno) << ")" << std::endl;
                }
#endif
            }
            
            acceptAttempts++;
            if (acceptAttempts % 50 == 0) { // 5秒ごと（100ms * 50）
                std::cout << "Host: Still waiting for client connection... (attempts: " << acceptAttempts << ")" << std::endl;
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
    
    // 非ブロッキングモードに設定
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(clientSocket_, FIONBIO, &mode);
#else
    int flags = fcntl(clientSocket_, F_GETFL, 0);
    fcntl(clientSocket_, F_SETFL, flags | O_NONBLOCK);
#endif
    
    // 非ブロッキング接続を試行
    int connectResult = connect(clientSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr));
#ifdef _WIN32
    if (connectResult == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS) {
            std::cerr << "Failed to connect to server (error: " << error << ")" << std::endl;
            closesocket(clientSocket_);
            clientSocket_ = INVALID_SOCKET;
            return false;
        }
    }
#else
    if (connectResult < 0 && errno != EINPROGRESS) {
        std::cerr << "Failed to connect to server (error: " << errno << ")" << std::endl;
        close(clientSocket_);
        clientSocket_ = -1;
        return false;
    }
#endif
    
    // 接続処理を別スレッドで実行（メインスレッドをブロックしない）
    isHost_ = false;
    
    std::thread([this, ipAddress, port]() {
        // 接続が完了するまで待機（タイムアウト付き）
        const int TIMEOUT_MS = 150000; // 150秒
        auto startTime = std::chrono::steady_clock::now();
        int selectAttempts = 0;
        
        while (!shouldStop_ && std::chrono::steady_clock::now() - startTime < std::chrono::milliseconds(TIMEOUT_MS)) {
            // まず接続エラーをチェック
#ifdef _WIN32
            int so_error = 0;
            int len = sizeof(so_error);
            if (getsockopt(clientSocket_, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len) == 0) {
                if (so_error != 0) {
                    std::cerr << "Client: Connection error detected: " << so_error << std::endl;
                    closesocket(clientSocket_);
                    clientSocket_ = INVALID_SOCKET;
                    if (connectionCallback_) {
                        connectionCallback_(false);
                    }
                    return;
                }
            }
#else
            int so_error = 0;
            socklen_t len = sizeof(so_error);
            if (getsockopt(clientSocket_, SOL_SOCKET, SO_ERROR, &so_error, &len) == 0) {
                if (so_error != 0) {
                    std::cerr << "Client: Connection error detected: " << so_error << " (" << strerror(so_error) << ")" << std::endl;
                    close(clientSocket_);
                    clientSocket_ = -1;
                    if (connectionCallback_) {
                        connectionCallback_(false);
                    }
                    return;
                }
            }
#endif
            
            fd_set writeSet;
            FD_ZERO(&writeSet);
#ifdef _WIN32
            FD_SET(clientSocket_, &writeSet);
            timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000; // 100ms
            int selectResult = select(0, nullptr, &writeSet, nullptr, &timeout);
            if (selectResult > 0 && FD_ISSET(clientSocket_, &writeSet)) {
                // 接続完了を再確認
                so_error = 0;
                len = sizeof(so_error);
                if (getsockopt(clientSocket_, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len) == 0 && so_error == 0) {
                    std::cout << "Client: select indicates connection ready" << std::endl;
                    break;
                } else {
                    std::cerr << "Client: Connection failed after select: " << so_error << std::endl;
                    closesocket(clientSocket_);
                    clientSocket_ = INVALID_SOCKET;
                    if (connectionCallback_) {
                        connectionCallback_(false);
                    }
                    return;
                }
            } else if (selectResult < 0) {
                int error = WSAGetLastError();
                std::cerr << "Client: select error: " << error << std::endl;
            }
#else
            FD_SET(clientSocket_, &writeSet);
            timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000; // 100ms
            int selectResult = select(clientSocket_ + 1, nullptr, &writeSet, nullptr, &timeout);
            if (selectResult > 0 && FD_ISSET(clientSocket_, &writeSet)) {
                // 接続完了を確認
                so_error = 0;
                len = sizeof(so_error);
                if (getsockopt(clientSocket_, SOL_SOCKET, SO_ERROR, &so_error, &len) == 0 && so_error == 0) {
                    std::cout << "Client: select indicates connection ready" << std::endl;
                    break;
                } else {
                    std::cerr << "Client: Connection failed after select: " << so_error << " (" << strerror(so_error) << ")" << std::endl;
                    close(clientSocket_);
                    clientSocket_ = -1;
                    if (connectionCallback_) {
                        connectionCallback_(false);
                    }
                    return;
                }
            } else if (selectResult < 0) {
                std::cerr << "Client: select error: " << errno << " (" << strerror(errno) << ")" << std::endl;
            }
#endif
            selectAttempts++;
            if (selectAttempts % 100 == 0) { // 1秒ごと（10ms * 100）
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - startTime).count();
                std::cout << "Client: Still connecting to " << ipAddress << ":" << port 
                          << " (elapsed: " << elapsed << "s, attempts: " << selectAttempts << ")" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        // 接続が完了したか確認
#ifdef _WIN32
        int so_error;
        int len = sizeof(so_error);
        if (!shouldStop_ && getsockopt(clientSocket_, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len) == 0 && so_error == 0) {
            // クライアントソケットも非ブロッキングモードに設定
            u_long mode = 1;
            ioctlsocket(clientSocket_, FIONBIO, &mode);
            
            isConnected_ = true;
            shouldStop_ = false;
            
            std::cout << "Client: Connected to server" << std::endl;
            
            // 受信スレッドを開始
            receiveThread_ = std::thread(&NetworkManager::receiveThread, this);
            
            if (connectionCallback_) {
                connectionCallback_(true);
            }
        } else {
            std::cerr << "Failed to connect to server (timeout or error)" << std::endl;
            closesocket(clientSocket_);
            clientSocket_ = INVALID_SOCKET;
            if (connectionCallback_) {
                connectionCallback_(false);
            }
        }
#else
        int so_error = 0;
        socklen_t len = sizeof(so_error);
        if (!shouldStop_ && getsockopt(clientSocket_, SOL_SOCKET, SO_ERROR, &so_error, &len) == 0 && so_error == 0) {
            // クライアントソケットも非ブロッキングモードに設定
            int socketFlags = fcntl(clientSocket_, F_GETFL, 0);
            fcntl(clientSocket_, F_SETFL, socketFlags | O_NONBLOCK);
            
            isConnected_ = true;
            shouldStop_ = false;
            
            std::cout << "Client: Connected to server" << std::endl;
            
            // 受信スレッドを開始
            receiveThread_ = std::thread(&NetworkManager::receiveThread, this);
            
            if (connectionCallback_) {
                connectionCallback_(true);
            }
        } else {
            std::cerr << "Failed to connect to server (timeout or error: " << so_error << ")" << std::endl;
            close(clientSocket_);
            clientSocket_ = -1;
            if (connectionCallback_) {
                connectionCallback_(false);
            }
        }
#endif
    }).detach();
    
    // 即座に返す（接続処理は別スレッドで実行中）
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

std::string NetworkManager::getLocalIPAddress() {
    std::string ipAddress;
    
#ifdef _WIN32
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        struct hostent* host = gethostbyname(hostname);
        if (host != nullptr && host->h_addr_list[0] != nullptr) {
            struct in_addr addr;
            std::memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
            char* ipStr = inet_ntoa(addr);
            if (ipStr != nullptr) {
                ipAddress = ipStr;
            }
        }
    }
#else
    // macOS/Linux用: getifaddrsを使用
    struct ifaddrs* ifaddrsList = nullptr;
    if (getifaddrs(&ifaddrsList) == 0) {
        for (struct ifaddrs* ifa = ifaddrsList; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) {
                continue;
            }
            
            // IPv4アドレスのみを取得
            if (ifa->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in* sin = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
                char ipStr[INET_ADDRSTRLEN];
                if (inet_ntop(AF_INET, &sin->sin_addr, ipStr, INET_ADDRSTRLEN) != nullptr) {
                    // ループバックアドレス（127.0.0.1）を除外
                    if (std::string(ipStr) != "127.0.0.1") {
                        ipAddress = ipStr;
                        break; // 最初の非ループバックIPv4アドレスを使用
                    }
                }
            }
        }
        freeifaddrs(ifaddrsList);
    }
#endif
    
    return ipAddress;
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
    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // データがまだない（これは正常）
            return false;
        }
        // その他のエラー
        isConnected_ = false;
        return false;
    }
#endif
    
    if (received == 0) {
        // 接続が閉じられた
        isConnected_ = false;
        return false;
    }
    
    if (received != static_cast<int>(sizeof(header))) {
        // 部分的なヘッダー受信（通常は発生しないが、念のため）
        return false;
    }
    
    type = header.type;
    actualDataSize = header.dataSize;
    
    // データサイズの検証
    if (actualDataSize > maxDataSize) {
        std::cerr << "Received data size (" << actualDataSize << ") exceeds max size (" << maxDataSize << ")" << std::endl;
        return false;
    }
    
    // データを受信
    if (actualDataSize > 0) {
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
            // 部分的なデータ受信
            return false;
        }
#else
        ssize_t received = recv(socket, reinterpret_cast<char*>(data), actualDataSize, MSG_DONTWAIT);
        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return false;
            }
            isConnected_ = false;
            return false;
        }
        if (received != static_cast<int>(actualDataSize)) {
            // 部分的なデータ受信
            return false;
        }
#endif
    }
    
    return true;
}

