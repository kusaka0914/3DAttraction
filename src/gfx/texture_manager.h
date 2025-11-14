/**
 * @file texture_manager.h
 * @brief テクスチャマネージャー
 * @details テクスチャの読み込み、管理、自動リロードを統合的に管理します。
 */
#pragma once

#include <string>
#include <map>
#include <ctime>

typedef unsigned int GLuint;

namespace gfx {

/**
 * @brief テクスチャマネージャー
 * @details テクスチャの読み込み、管理、自動リロードを統合的に管理します。
 */
class TextureManager {
public:
    /**
     * @brief テクスチャを読み込んでIDを返す
     * @details テクスチャファイルを読み込み、OpenGLテクスチャIDを返します。
     * 
     * @param filename ファイル名
     * @return テクスチャID（読み込み失敗時0）
     */
    static GLuint loadTexture(const std::string& filename);
    
    /**
     * @brief テクスチャをバインドする
     * @details 指定されたテクスチャIDをOpenGLにバインドします。
     * 
     * @param textureID テクスチャID
     */
    static void bindTexture(GLuint textureID);
    
    /**
     * @brief テクスチャをクリーンアップする
     * @details 全テクスチャのリソースを解放します。
     */
    static void cleanup();
    
    /**
     * @brief テクスチャが存在するかチェックする
     * @param filename ファイル名
     * @return 存在する場合true
     */
    static bool hasTexture(const std::string& filename);
    
    /**
     * @brief テクスチャファイルの変更を確認してリロードする
     * @details テクスチャファイルが変更されていた場合、自動的にリロードします。
     */
    static void checkAndReloadTextures();
    
private:
    static std::map<std::string, GLuint> textures;
    static std::map<std::string, std::time_t> textureModTimes;
    static GLuint loadTextureFromFile(const std::string& filename);
    static std::time_t getFileModificationTime(const std::string& filepath);
    static void reloadTexture(const std::string& filename);
};

} // namespace gfx
