#pragma once

#include <string>
#include <map>

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
    
private:
    static std::map<std::string, GLuint> textures;
    static GLuint loadTextureFromFile(const std::string& filename);
};

} // namespace gfx
