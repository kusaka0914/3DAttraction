#include "game_app.h"
#include "../core/log.h"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        // ログシステムの初期化
        core::Log::init();
        
        LOG_INFO("Starting Vulkan3D - Portal Physics Puzzle");
        
        // アプリケーションの作成と初期化
        app::GameApp app;
        
        if (!app.initialize()) {
            LOG_ERROR("Failed to initialize application");
            return -1;
        }
        
        // メインループの実行
        app.run();
        
        LOG_INFO("Application finished successfully");
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        LOG_CRITICAL("Fatal error: {}", e.what());
        return -1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        LOG_CRITICAL("Unknown fatal error occurred");
        return -1;
    }
    
    // ログシステムの終了
    core::Log::shutdown();
    
    return 0;
}
