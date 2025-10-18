#include "texture_manager.h"
#include <iostream>
#include <fstream>
#include <GL/glew.h>

// STB Image の実装をインクルード
#define STB_IMAGE_IMPLEMENTATION
#include "../../third_party/stb_image.h"

namespace gfx {

std::map<std::string, GLuint> TextureManager::textures;

GLuint TextureManager::loadTexture(const std::string& filename) {
    // 既に読み込まれている場合はキャッシュから返す
    if (textures.find(filename) != textures.end()) {
        return textures[filename];
    }
    
    // ファイルの存在チェック
    std::ifstream file(filename);
    if (!file.good()) {
        std::cerr << "ERROR: Texture file not found: " << filename << std::endl;
        return 0;
    }
    file.close();
    
    // テクスチャを読み込む
    GLuint textureID = loadTextureFromFile(filename);
    if (textureID != 0) {
        textures[filename] = textureID;
        std::cout << "Loaded texture: " << filename << " (ID: " << textureID << ")" << std::endl;
    }
    
    return textureID;
}

GLuint TextureManager::loadTextureFromFile(const std::string& filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "ERROR: Failed to load texture: " << filename << std::endl;
        return 0;
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // テクスチャパラメータを設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // チャンネル数に応じてフォーマットを選択
    GLenum format;
    if (channels == 1) {
        format = GL_LUMINANCE;
    } else if (channels == 3) {
        format = GL_RGB;
    } else if (channels == 4) {
        format = GL_RGBA;
    } else {
        std::cerr << "ERROR: Unsupported number of channels: " << channels << std::endl;
        stbi_image_free(data);
        return 0;
    }
    
    // テクスチャデータをアップロード
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    // メモリを解放
    stbi_image_free(data);
    
    return textureID;
}

void TextureManager::bindTexture(GLuint textureID) {
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void TextureManager::cleanup() {
    for (auto& pair : textures) {
        glDeleteTextures(1, &pair.second);
    }
    textures.clear();
}

bool TextureManager::hasTexture(const std::string& filename) {
    return textures.find(filename) != textures.end();
}

} // namespace gfx
