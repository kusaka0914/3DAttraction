#include "texture_manager.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#ifdef _WIN32
    #include <sys/stat.h>
    #include <io.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "../../third_party/stb_image.h"

namespace gfx {

std::map<std::string, GLuint> TextureManager::textures;
std::map<std::string, std::time_t> TextureManager::textureModTimes;

GLuint TextureManager::loadTexture(const std::string& filename) {
    if (textures.find(filename) != textures.end()) {
        return textures[filename];
    }
    
    std::ifstream file(filename);
    if (!file.good()) {
        std::cerr << "ERROR: Texture file not found: " << filename << std::endl;
        return 0;
    }
    file.close();
    
    GLuint textureID = loadTextureFromFile(filename);
    if (textureID != 0) {
        textures[filename] = textureID;
        textureModTimes[filename] = getFileModificationTime(filename);
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
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
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
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
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

std::time_t TextureManager::getFileModificationTime(const std::string& filepath) {
#ifdef _WIN32
    struct _stat fileInfo;
    if (_stat(filepath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
    std::string altPath = filepath;
    if (altPath.find("../") == 0) {
        altPath = altPath.substr(3);
    } else if (altPath.find("assets/") == 0) {
        altPath = "../" + altPath;
    }
    if (_stat(altPath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
#else
    struct stat fileInfo;
    if (stat(filepath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
    std::string altPath = filepath;
    if (altPath.find("../") == 0) {
        altPath = altPath.substr(3);
    } else if (altPath.find("assets/") == 0) {
        altPath = "../" + altPath;
    }
    if (stat(altPath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
#endif
    return 0;
}

void TextureManager::reloadTexture(const std::string& filename) {
    auto it = textures.find(filename);
    if (it == textures.end()) {
        return;
    }
    
    glDeleteTextures(1, &it->second);
    textures.erase(it);
    
    GLuint textureID = loadTextureFromFile(filename);
    if (textureID != 0) {
        textures[filename] = textureID;
        textureModTimes[filename] = getFileModificationTime(filename);
        std::cout << "Reloaded texture: " << filename << " (ID: " << textureID << ")" << std::endl;
    }
}

void TextureManager::checkAndReloadTextures() {
    for (auto& pair : textureModTimes) {
        const std::string& filename = pair.first;
        std::time_t& lastModTime = pair.second;
        
        std::time_t currentModTime = getFileModificationTime(filename);
        if (currentModTime > 0 && currentModTime != lastModTime && lastModTime > 0) {
            reloadTexture(filename);
        } else if (lastModTime == 0 && currentModTime > 0) {
            lastModTime = currentModTime;
        }
    }
}

} // namespace gfx
