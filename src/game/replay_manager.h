/**
 * @file replay_manager.h
 * @brief リプレイマネージャー
 * @details リプレイデータの保存と読み込みを管理します。
 */
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state.h"
#include <string>
#include <nlohmann/json.hpp>

/**
 * @brief リプレイマネージャー
 * @details リプレイデータの保存と読み込みを管理します。
 */
class ReplayManager {
public:
    /**
     * @brief リプレイデータをJSONファイルに保存する
     * @details リプレイデータをJSONファイルに保存します。
     * 
     * @param replayData リプレイデータ
     * @param stageNumber ステージ番号
     * @return 保存成功時true
     */
    static bool saveReplay(const ReplayData& replayData, int stageNumber);
    
    /**
     * @brief JSONファイルからリプレイデータを読み込む
     * @details JSONファイルからリプレイデータを読み込みます。
     * 
     * @param replayData リプレイデータ
     * @param stageNumber ステージ番号
     * @return 読み込み成功時true
     */
    static bool loadReplay(ReplayData& replayData, int stageNumber);
    
    /**
     * @brief リプレイファイルのパスを取得する
     * @param stageNumber ステージ番号
     * @return リプレイファイルのパス
     */
    static std::string getReplayFilePath(int stageNumber);
    
    /**
     * @brief リプレイファイルが存在するか確認する
     * @param stageNumber ステージ番号
     * @return 存在する場合true
     */
    static bool replayExists(int stageNumber);
    
private:
    /**
     * @brief ファイルが存在するか確認する
     * @param filename ファイル名
     * @return 存在する場合true
     */
    static bool fileExists(const std::string& filename);
};
