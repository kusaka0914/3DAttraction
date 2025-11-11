#include "audio_manager.h"
#include <iostream>
#include <filesystem>
#ifdef _WIN32
    #include <SDL.h>
    #include <sys/stat.h>
    #include <io.h>
#else
    #include <SDL2/SDL.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace io {

AudioManager::AudioManager() 
    : m_initialized(false)
    , m_masterVolume(1.0f)
    , m_bgmVolume(0.4f)
    , m_sfxVolume(1.0f)
    , m_bgmPlaying(false)
    , m_bgmPaused(false)
    , m_bgmMusic(nullptr)
    , m_bgmModTime(0)
{
}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // SDL初期化
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // SDL_mixer初期化
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Failed to initialize SDL_mixer: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    
    m_initialized = true;
    
    // SE音量を最大に設定
    setSFXVolume(1.0f);
    
    // BGM音量を設定
    setBGMVolume(0.4f);
    
    std::cout << "AudioManager initialized successfully" << std::endl;
    return true;
}

void AudioManager::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    stopBGM();
    
    // BGMのクリーンアップ
    if (m_bgmMusic) {
        Mix_FreeMusic(m_bgmMusic);
        m_bgmMusic = nullptr;
    }
    
    // SFXのクリーンアップ
    for (auto& pair : m_sfxChunks) {
        Mix_FreeChunk(pair.second);
    }
    m_sfxChunks.clear();
    
    // SDL_mixer終了
    Mix_CloseAudio();
    SDL_Quit();
    
    m_initialized = false;
    std::cout << "AudioManager shutdown" << std::endl;
}

bool AudioManager::loadBGM(const std::string& filename) {
    if (!m_initialized) {
        std::cerr << "AudioManager not initialized" << std::endl;
        return false;
    }
    
    // ファイルの存在確認
    if (!std::filesystem::exists(filename)) {
        std::cerr << "BGM file not found: " << filename << std::endl;
        return false;
    }
    
    // 既存のBGMをクリーンアップ
    if (m_bgmMusic) {
        Mix_FreeMusic(m_bgmMusic);
        m_bgmMusic = nullptr;
    }
    
    // 新しいBGMを読み込み
    m_bgmMusic = Mix_LoadMUS(filename.c_str());
    if (!m_bgmMusic) {
        std::cerr << "Failed to load BGM: " << filename << " - " << Mix_GetError() << std::endl;
        return false;
    }
    
    m_currentBGM = filename;
    // 更新時刻を記録
    m_bgmModTime = getFileModificationTime(filename);
    std::cout << "BGM loaded: " << filename << std::endl;
    return true;
}

void AudioManager::playBGM() {
    if (!m_initialized || !m_bgmMusic) {
        return;
    }
    
    if (!m_bgmPlaying) {
        if (Mix_PlayMusic(m_bgmMusic, -1) == -1) { // -1 = 無限ループ
            std::cerr << "Failed to play BGM: " << Mix_GetError() << std::endl;
            return;
        }
        m_bgmPlaying = true;
        m_bgmPaused = false;
        std::cout << "BGM started playing" << std::endl;
    }
}

void AudioManager::stopBGM() {
    if (!m_initialized) {
        return;
    }
    
    if (m_bgmPlaying) {
        Mix_HaltMusic();
        m_bgmPlaying = false;
        m_bgmPaused = false;
        std::cout << "BGM stopped" << std::endl;
    }
}

void AudioManager::pauseBGM() {
    if (!m_initialized) {
        return;
    }
    
    if (m_bgmPlaying && !m_bgmPaused) {
        Mix_PauseMusic();
        m_bgmPaused = true;
        std::cout << "BGM paused" << std::endl;
    }
}

void AudioManager::resumeBGM() {
    if (!m_initialized) {
        return;
    }
    
    if (m_bgmPaused) {
        Mix_ResumeMusic();
        m_bgmPaused = false;
        std::cout << "BGM resumed" << std::endl;
    }
}

void AudioManager::setBGMVolume(float volume) {
    m_bgmVolume = std::max(0.0f, std::min(1.0f, volume));
    
    if (m_initialized) {
        int sdlVolume = static_cast<int>(m_bgmVolume * m_masterVolume * MIX_MAX_VOLUME);
        Mix_VolumeMusic(sdlVolume);
    }
}

float AudioManager::getBGMVolume() const {
    return m_bgmVolume;
}

bool AudioManager::isBGMPlaying() const {
    return m_bgmPlaying && !m_bgmPaused;
}

bool AudioManager::loadSFX(const std::string& name, const std::string& filename) {
    if (!m_initialized) {
        std::cerr << "AudioManager not initialized" << std::endl;
        return false;
    }
    
    // ファイルの存在確認
    if (!std::filesystem::exists(filename)) {
        std::cerr << "SFX file not found: " << filename << std::endl;
        return false;
    }
    
    // 既存のSFXをクリーンアップ
    auto it = m_sfxChunks.find(name);
    if (it != m_sfxChunks.end()) {
        Mix_FreeChunk(it->second);
        m_sfxChunks.erase(it);
    }
    
    // 新しいSFXを読み込み
    Mix_Chunk* chunk = Mix_LoadWAV(filename.c_str());
    if (!chunk) {
        std::cerr << "Failed to load SFX: " << filename << " - " << Mix_GetError() << std::endl;
        return false;
    }
    
    // 音量を設定
    int sdlVolume = static_cast<int>(m_sfxVolume * m_masterVolume * MIX_MAX_VOLUME);
    Mix_VolumeChunk(chunk, sdlVolume);
    
    m_sfxChunks[name] = chunk;
    m_sfxFiles[name] = filename;
    // 更新時刻を記録
    m_sfxModTimes[name] = getFileModificationTime(filename);
    std::cout << "SFX loaded: " << name << " -> " << filename << std::endl;
    return true;
}

void AudioManager::playSFX(const std::string& name) {
    if (!m_initialized) {
        return;
    }
    
    auto it = m_sfxChunks.find(name);
    if (it != m_sfxChunks.end()) {
        if (Mix_PlayChannel(-1, it->second, 0) == -1) {
            std::cerr << "Failed to play SFX: " << name << " - " << Mix_GetError() << std::endl;
        } else {
            std::cout << "SFX played: " << name << std::endl;
        }
    } else {
        std::cerr << "SFX not found: " << name << std::endl;
    }
}

void AudioManager::setSFXVolume(float volume) {
    m_sfxVolume = std::max(0.0f, std::min(1.0f, volume));
    
    if (m_initialized) {
        int sdlVolume = static_cast<int>(m_sfxVolume * m_masterVolume * MIX_MAX_VOLUME);
        for (auto& pair : m_sfxChunks) {
            Mix_VolumeChunk(pair.second, sdlVolume);
        }
    }
}

void AudioManager::setMasterVolume(float volume) {
    m_masterVolume = std::max(0.0f, std::min(1.0f, volume));
    
    if (m_initialized) {
        // BGM音量を更新
        int bgmVolume = static_cast<int>(m_bgmVolume * m_masterVolume * MIX_MAX_VOLUME);
        Mix_VolumeMusic(bgmVolume);
        
        // SFX音量を更新
        int sfxVolume = static_cast<int>(m_sfxVolume * m_masterVolume * MIX_MAX_VOLUME);
        for (auto& pair : m_sfxChunks) {
            Mix_VolumeChunk(pair.second, sfxVolume);
        }
    }
}

float AudioManager::getMasterVolume() const {
    return m_masterVolume;
}

std::time_t AudioManager::getFileModificationTime(const std::string& filepath) {
#ifdef _WIN32
    struct _stat fileInfo;
    if (_stat(filepath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
    // 代替パスも試す
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
    // 代替パスも試す
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

void AudioManager::reloadBGM() {
    if (m_currentBGM.empty() || !m_bgmMusic) {
        return;
    }
    
    bool wasPlaying = m_bgmPlaying && !m_bgmPaused;
    
    // 古いBGMを停止・削除
    if (m_bgmPlaying) {
        stopBGM();
    }
    
    // 新しいBGMを読み込み
    if (loadBGM(m_currentBGM)) {
        // 再生中だった場合は再開
        if (wasPlaying) {
            playBGM();
        }
        std::cout << "BGM reloaded: " << m_currentBGM << std::endl;
    }
}

void AudioManager::reloadSFX(const std::string& name) {
    auto it = m_sfxFiles.find(name);
    if (it == m_sfxFiles.end()) {
        return;
    }
    
    const std::string& filename = it->second;
    
    // 再読み込み
    if (loadSFX(name, filename)) {
        std::cout << "SFX reloaded: " << name << " -> " << filename << std::endl;
    }
}

void AudioManager::checkAndReloadAudio() {
    if (!m_initialized) {
        return;
    }
    
    // BGMの監視
    if (!m_currentBGM.empty()) {
        std::time_t currentModTime = getFileModificationTime(m_currentBGM);
        if (currentModTime > 0 && currentModTime != m_bgmModTime && m_bgmModTime > 0) {
            reloadBGM();
        } else if (m_bgmModTime == 0 && currentModTime > 0) {
            // 初回の場合は更新時刻を記録
            m_bgmModTime = currentModTime;
        }
    }
    
    // SFXの監視
    for (auto& pair : m_sfxModTimes) {
        const std::string& name = pair.first;
        std::time_t& lastModTime = pair.second;
        
        auto it = m_sfxFiles.find(name);
        if (it == m_sfxFiles.end()) {
            continue;
        }
        
        const std::string& filename = it->second;
        std::time_t currentModTime = getFileModificationTime(filename);
        if (currentModTime > 0 && currentModTime != lastModTime && lastModTime > 0) {
            reloadSFX(name);
        } else if (lastModTime == 0 && currentModTime > 0) {
            // 初回の場合は更新時刻を記録
            lastModTime = currentModTime;
        }
    }
}

} // namespace io