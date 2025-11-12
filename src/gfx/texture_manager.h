#pragma once

#include <string>
#include <map>
#include <ctime>

// OpenGL型の前方宣言
typedef unsigned int GLuint;

namespace gfx {

class TextureManager {
public:
    // テクスチャを読み込んでIDを返す
    static GLuint loadTexture(const std::string& filename);
    
    // テクスチャをバインド
    static void bindTexture(GLuint textureID);
    
    // テクスチャをクリーンアップ
    static void cleanup();
    
    // テクスチャが存在するかチェック
    static bool hasTexture(const std::string& filename);
    
    // ファイル監視（自動リロード用）
    static void checkAndReloadTextures();
    
private:
    static std::map<std::string, GLuint> textures;
    static std::map<std::string, std::time_t> textureModTimes;  // ファイルパスと更新時刻のマップ
    static GLuint loadTextureFromFile(const std::string& filename);
    static std::time_t getFileModificationTime(const std::string& filepath);
    static void reloadTexture(const std::string& filename);
};

} // namespace gfx
