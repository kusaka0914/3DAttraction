#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state.h"
#include <string>
#include <nlohmann/json.hpp>

/**
 * @brief セーブマネージャー
 * @details ゲームデータの保存と読み込みを管理します。
 */
class SaveManager {
public:
    /**
     * @brief ゲームデータをJSONファイルに保存する
     * @details ゲーム状態をJSONファイルに保存します。
     * 
     * @param gameState ゲーム状態
     * @return 保存成功時true
     */
    static bool saveGameData(const GameState& gameState);
    
    /**
     * @brief JSONファイルからゲームデータを読み込む
     * @details JSONファイルからゲーム状態を読み込みます。
     * 
     * @param gameState ゲーム状態
     * @return 読み込み成功時true
     */
    static bool loadGameData(GameState& gameState);
    
    /**
     * @brief セーブファイルのパスを取得する
     * @return セーブファイルのパス
     */
    static std::string getSaveFilePath();
    
    /**
     * @brief セーブファイルが存在するか確認する
     * @return 存在する場合true
     */
    static bool saveFileExists();
    
private:
    /**
     * @brief ファイルが存在するか確認する
     * @param filename ファイル名
     * @return 存在する場合true
     */
    static bool fileExists(const std::string& filename);
};
