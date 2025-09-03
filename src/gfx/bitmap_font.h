#pragma once

#include <map>
#include <vector>
#include <string>

namespace gfx {

class BitmapFont {
public:
    BitmapFont();
    ~BitmapFont();
    
    // フォントの初期化
    void initialize();
    
    // 文字データの取得
    const std::vector<bool>& getCharacter(char c) const;
    
    // フォントの初期化状態確認
    bool isInitialized() const;
    
    // 指定文字の存在確認
    bool hasCharacter(char c) const;

private:
    // 文字データの格納
    std::map<char, std::vector<bool>> characters;
    
    // 初期化状態
    bool initialized;
    
    // 各文字のビットマップデータを定義
    void defineCharacters();
};

} // namespace gfx
