#pragma once

#include <glm/glm.hpp>
#include <string>

typedef unsigned int GLuint;

namespace gfx {

/**
 * @brief 3Dレンダラー
 * @details 3Dオブジェクトの描画を統合的に管理します。
 */
class Renderer3D {
public:
    Renderer3D();
    ~Renderer3D();

    /**
     * @brief 3Dキューブを描画する
     * @param position 位置
     * @param color 色
     * @param size サイズ
     */
    void renderCube(const glm::vec3& position, const glm::vec3& color, float size);
    
    /**
     * @brief 回転したボックスを描画する
     * @param position 位置
     * @param color 色
     * @param size サイズ
     * @param rotationAxis 回転軸
     * @param rotationAngle 回転角度
     */
    void renderRotatedBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                          const glm::vec3& rotationAxis, float rotationAngle);
    
    /**
     * @brief アルファ付きボックスを描画する
     * @param position 位置
     * @param color 色
     * @param size サイズ
     * @param alpha アルファ値
     */
    void renderBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha);
    
    /**
     * @brief 回転したアルファ付きボックスを描画する
     * @param position 位置
     * @param color 色
     * @param size サイズ
     * @param rotationAxis 回転軸
     * @param rotationAngle 回転角度
     * @param alpha アルファ値
     */
    void renderRotatedBoxWithAlpha(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, 
                                  const glm::vec3& rotationAxis, float rotationAngle, float alpha);
    
    /**
     * @brief リアルなボックスを描画する
     * @param position 位置
     * @param color 色
     * @param size サイズ
     * @param alpha アルファ値
     */
    void renderRealisticBox(const glm::vec3& position, const glm::vec3& color, const glm::vec3& size, float alpha);
    
    /**
     * @brief テクスチャ付きボックスを描画する
     * @param position 位置
     * @param size サイズ
     * @param textureID テクスチャID
     */
    void renderTexturedBox(const glm::vec3& position, const glm::vec3& size, GLuint textureID);
    
    /**
     * @brief テクスチャ付きボックスを描画する（前面と他の面で異なるテクスチャ）
     * @param position 位置
     * @param size サイズ
     * @param frontTextureID 前面テクスチャID
     * @param otherTextureID 他の面のテクスチャID
     */
    void renderTexturedBox(const glm::vec3& position, const glm::vec3& size, GLuint frontTextureID, GLuint otherTextureID);
    
    /**
     * @brief アルファ付きテクスチャボックスを描画する
     * @param position 位置
     * @param size サイズ
     * @param textureID テクスチャID
     * @param alpha アルファ値
     */
    void renderTexturedBoxWithAlpha(const glm::vec3& position, const glm::vec3& size, GLuint textureID, float alpha);
    
    /**
     * @brief 回転したテクスチャボックスを描画する
     * @param position 位置
     * @param size サイズ
     * @param textureID テクスチャID
     * @param rotationAxis 回転軸
     * @param rotationAngle 回転角度
     */
    void renderTexturedRotatedBox(const glm::vec3& position, const glm::vec3& size, GLuint textureID, 
                                 const glm::vec3& rotationAxis, float rotationAngle);
    
    /**
     * @brief 3D星を描画する
     * @param position 位置
     * @param color 色
     * @param scale スケール
     */
    void renderStar3D(const glm::vec3& position, const glm::vec3& color, float scale);
    
    /**
     * @brief 3D数字を描画する
     * @param position 位置
     * @param number 数字
     * @param color 色
     * @param scale スケール
     */
    void renderNumber3D(const glm::vec3& position, int number, const glm::vec3& color, float scale);
    
    /**
     * @brief 3D Xマークを描画する
     * @param position 位置
     * @param color 色
     * @param scale スケール
     */
    void renderXMark3D(const glm::vec3& position, const glm::vec3& color, float scale);

private:
    void drawCube(const glm::mat4& model, const glm::vec3& color);
    void drawTexturedCube(const glm::mat4& model, GLuint textureID, float alpha = 1.0f);
    void drawTexturedCubeWithFrontFace(const glm::mat4& model, GLuint frontTextureID, GLuint otherTextureID);
};

} // namespace gfx
