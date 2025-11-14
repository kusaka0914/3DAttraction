/**
 * @file error_handler.h
 * @brief エラーハンドラー
 * @details エラーメッセージとログの管理を提供します。
 */
#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdarg>

/**
 * @brief エラーハンドラー
 * @details エラーメッセージとログの管理を提供します。
 */
class ErrorHandler {
public:
    /**
     * @brief GLFWエラーを処理する
     * @param operation 操作名
     * @return 常にfalse
     */
    static bool handleGLFWError(const std::string& operation) {
        std::cerr << "GLFW Error during " << operation << std::endl;
        return false;
    }
    
    /**
     * @brief レンダラーエラーを処理する
     * @param operation 操作名
     * @return 常にfalse
     */
    static bool handleRendererError(const std::string& operation) {
        std::cerr << "Renderer Error during " << operation << std::endl;
        return false;
    }
    
    /**
     * @brief エラーログを出力する
     * @param message エラーメッセージ
     */
    static void logError(const std::string& message) {
        std::cerr << "ERROR: " << message << std::endl;
    }
    
    /**
     * @brief 情報ログを出力する
     * @param message 情報メッセージ
     */
    static void logInfo(const std::string& message) {
        std::cout << "INFO: " << message << std::endl;
    }
    
    /**
     * @brief デバッグログを出力する
     * @param message デバッグメッセージ
     */
    static void logDebug(const std::string& message) {
        std::cout << "DEBUG: " << message << std::endl;
    }
    
    /**
     * @brief フォーマット付きエラーログを出力する
     * @details printf形式のフォーマット文字列を使用してエラーメッセージを出力します。
     * @param format フォーマット文字列
     * @param ... フォーマット引数
     */
    static void logErrorFormat(const char* format, ...) {
        char buffer[1024];
        va_list args;
        va_start(args, format);
        #ifdef _WIN32
            vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
        #else
            vsnprintf(buffer, sizeof(buffer), format, args);
        #endif
        va_end(args);
        std::cerr << "ERROR: " << buffer << std::endl;
    }
};
