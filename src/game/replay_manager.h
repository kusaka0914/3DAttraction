#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state.h"
#include <string>
#include <nlohmann/json.hpp>

class ReplayManager {
public:
    // リプレイデータをJSONファイルに保存
    static bool saveReplay(const GameState::ReplayData& replayData, int stageNumber);
    
    // JSONファイルからリプレイデータを読み込み
    static bool loadReplay(GameState::ReplayData& replayData, int stageNumber);
    
    // リプレイファイルのパスを取得
    static std::string getReplayFilePath(int stageNumber);
    
    // リプレイファイルが存在するか確認
    static bool replayExists(int stageNumber);
    
private:
    // JSONファイルの存在確認
    static bool fileExists(const std::string& filename);
};

