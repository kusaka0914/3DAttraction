#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state.h"
#include <string>
#include <nlohmann/json.hpp>

class SaveManager {
public:
    // ゲームデータをJSONファイルに保存
    static bool saveGameData(const GameState& gameState);
    
    // JSONファイルからゲームデータを読み込み
    static bool loadGameData(GameState& gameState);
    
    // セーブファイルのパスを取得
    static std::string getSaveFilePath();
    
    // セーブファイルが存在するか確認
    static bool saveFileExists();
    
private:
    // JSONファイルの存在確認
    static bool fileExists(const std::string& filename);
};

