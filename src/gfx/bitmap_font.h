/**
 * @file bitmap_font.h
 * @brief ビットマップフォント
 * @details ビットマップベースのフォント描画を管理します。
 */
#pragma once

#include <map>
#include <vector>
#include <string>

namespace gfx {

/**
 * @brief ビットマップフォント
 * @details ビットマップベースのフォント描画を管理します。
 */
class BitmapFont {
public:
    BitmapFont();
    ~BitmapFont();
    
    /**
     * @brief フォントを初期化する
     * @details フォントデータを読み込み、初期化します。
     */
    void initialize();
    
    /**
     * @brief 文字データを取得する
     * @param c 取得する文字
     * @return 文字のビットマップデータ
     */
    const std::vector<bool>& getCharacter(char c) const;
    
    /**
     * @brief フォントが初期化済みか確認する
     * @return 初期化済みの場合true
     */
    bool isInitialized() const;
    
    /**
     * @brief 指定文字が存在するか確認する
     * @param c 確認する文字
     * @return 存在する場合true
     */
    bool hasCharacter(char c) const;

private:
    std::map<char, std::vector<bool>> characters;
    bool initialized;
    
    void defineCharacters();
};

} // namespace gfx
