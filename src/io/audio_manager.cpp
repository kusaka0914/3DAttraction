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
    , m_mp3Supported(false)
{
}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    
    int initFlags = MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC;
    int initialized = Mix_Init(initFlags);
    m_mp3Supported = (initialized & MIX_INIT_MP3) != 0;
    if ((initialized & initFlags) != initFlags) {
        std::cerr << "Warning: Some audio formats failed to initialize: " << Mix_GetError() << std::endl;
        if (!m_mp3Supported) {
            std::cerr << "Warning: MP3 support is not available. MP3 files will not be playable." << std::endl;
            std::cerr << "Please use OGG format or recompile SDL_mixer with MP3 support." << std::endl;
        }
    }
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Failed to initialize SDL_mixer: " << Mix_GetError() << std::endl;
        Mix_Quit();
        SDL_Quit();
        return false;
    }
    
    m_initialized = true;
    
    setSFXVolume(1.0f);
    
    setBGMVolume(0.4f);
    
    std::cout << "AudioManager initialized successfully" << std::endl;
    return true;
}

void AudioManager::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    stopBGM();
    
    if (m_bgmMusic) {
        Mix_FreeMusic(m_bgmMusic);
        m_bgmMusic = nullptr;
    }
    
    for (auto& pair : m_sfxChunks) {
        Mix_FreeChunk(pair.second);
    }
    m_sfxChunks.clear();
    
    Mix_CloseAudio();
    Mix_Quit();  // Mix_Init()で初期化したフォーマットをクリーンアップ
    SDL_Quit();
    
    m_initialized = false;
    std::cout << "AudioManager shutdown" << std::endl;
}

bool AudioManager::loadBGM(const std::string& filename) {
    if (!m_initialized) {
        std::cerr << "AudioManager not initialized" << std::endl;
        return false;
    }
    
    if (!std::filesystem::exists(filename)) {
        std::cerr << "BGM file not found: " << filename << std::endl;
        return false;
    }
    
    if (m_bgmMusic) {
        Mix_FreeMusic(m_bgmMusic);
        m_bgmMusic = nullptr;
    }
    
    m_bgmMusic = Mix_LoadMUS(filename.c_str());
    if (!m_bgmMusic) {
        std::string error = Mix_GetError();
        if (!m_mp3Supported && filename.find(".mp3") != std::string::npos) {
            static bool mp3ErrorShown = false;
            if (!mp3ErrorShown) {
                std::cerr << "Failed to load BGM (MP3): " << filename << " - MP3 support not available" << std::endl;
                mp3ErrorShown = true;
            }
        } else {
            std::cerr << "Failed to load BGM: " << filename << " - " << error << std::endl;
        }
        return false;
    }
    
    m_currentBGM = filename;
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
    
    if (!std::filesystem::exists(filename)) {
        std::cerr << "SFX file not found: " << filename << std::endl;
        return false;
    }
    
    auto it = m_sfxChunks.find(name);
    if (it != m_sfxChunks.end()) {
        Mix_FreeChunk(it->second);
        m_sfxChunks.erase(it);
    }
    
    Mix_Chunk* chunk = Mix_LoadWAV(filename.c_str());
    if (!chunk) {
        std::string error = Mix_GetError();
        if (!m_mp3Supported && filename.find(".mp3") != std::string::npos) {
            static bool mp3SFXErrorShown = false;
            if (!mp3SFXErrorShown) {
                std::cerr << "Failed to load SFX (MP3): " << filename << " - MP3 support not available" << std::endl;
                mp3SFXErrorShown = true;
            }
        } else {
            std::cerr << "Failed to load SFX: " << filename << " - " << error << std::endl;
        }
        return false;
    }
    
    int sdlVolume = static_cast<int>(m_sfxVolume * m_masterVolume * MIX_MAX_VOLUME);
    Mix_VolumeChunk(chunk, sdlVolume);
    
    m_sfxChunks[name] = chunk;
    m_sfxFiles[name] = filename;
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
        int bgmVolume = static_cast<int>(m_bgmVolume * m_masterVolume * MIX_MAX_VOLUME);
        Mix_VolumeMusic(bgmVolume);
        
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

void AudioManager::reloadBGM() {
    if (m_currentBGM.empty() || !m_bgmMusic) {
        return;
    }
    
    bool wasPlaying = m_bgmPlaying && !m_bgmPaused;
    
    if (m_bgmPlaying) {
        stopBGM();
    }
    
    if (loadBGM(m_currentBGM)) {
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
    
    if (loadSFX(name, filename)) {
        std::cout << "SFX reloaded: " << name << " -> " << filename << std::endl;
    }
}

void AudioManager::checkAndReloadAudio() {
    if (!m_initialized) {
        return;
    }
    
    if (!m_currentBGM.empty()) {
        std::time_t currentModTime = getFileModificationTime(m_currentBGM);
        if (currentModTime > 0 && currentModTime != m_bgmModTime && m_bgmModTime > 0) {
            reloadBGM();
        } else if (m_bgmModTime == 0 && currentModTime > 0) {
            m_bgmModTime = currentModTime;
        }
    }
    
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
            lastModTime = currentModTime;
        }
    }
}

} // namespace io