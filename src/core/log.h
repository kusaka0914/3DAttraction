#pragma once

#include <iostream>
#include <memory>

namespace core {

class Log {
public:
    static void init() { std::cout << "Log system initialized" << std::endl; }
    static void shutdown() { std::cout << "Log system shutting down" << std::endl; }

    static void* getLogger() { return nullptr; }
};

} // namespace core

// ログマクロ（簡易版）
#define LOG_TRACE(...)    std::cout << "[TRACE] " << __VA_ARGS__ << std::endl
#define LOG_DEBUG(...)    std::cout << "[DEBUG] " << __VA_ARGS__ << std::endl
#define LOG_INFO(...)     std::cout << "[INFO] " << __VA_ARGS__ << std::endl
#define LOG_WARN(...)     std::cout << "[WARN] " << __VA_ARGS__ << std::endl
#define LOG_ERROR(...)    std::cout << "[ERROR] " << __VA_ARGS__ << std::endl
#define LOG_CRITICAL(...) std::cout << "[CRITICAL] " << __VA_ARGS__ << std::endl
