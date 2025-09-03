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
    
    // フォントの幅と高さ（ピクセル単位）
    static constexpr int CHAR_WIDTH = 8;
    static constexpr int CHAR_HEIGHT = 12;
    
    // 文字間隔とスペース幅
    static constexpr float CHAR_SPACING = 2.0f;
    static constexpr float SPACE_WIDTH = 8.0f;

private:
    // 文字データの格納
    std::map<char, std::vector<bool>> characters;
    
    // 初期化状態
    bool initialized;
    
    // 各文字のビットマップデータを定義
    void defineCharacters();
};

} // namespace gfx
