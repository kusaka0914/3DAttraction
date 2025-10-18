#pragma once

#include <string>
#include <map>
#include <memory>
#include <SDL2/SDL_mixer.h>

namespace io {

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    // BGM管理
    bool loadBGM(const std::string& filename);
    void playBGM();
    void stopBGM();
    void pauseBGM();
    void resumeBGM();
    void setBGMVolume(float volume);
    float getBGMVolume() const;
    bool isBGMPlaying() const;
    
    // 効果音管理
    bool loadSFX(const std::string& name, const std::string& filename);
    void playSFX(const std::string& name);
    void setSFXVolume(float volume);
    
    // 全体的な音量制御
    void setMasterVolume(float volume);
    float getMasterVolume() const;
    
    // 音声の初期化・終了
    bool initialize();
    void shutdown();
    
private:
    bool m_initialized;
    float m_masterVolume;
    float m_bgmVolume;
    float m_sfxVolume;
    
    std::string m_currentBGM;
    bool m_bgmPlaying;
    bool m_bgmPaused;
    
    Mix_Music* m_bgmMusic;
    std::map<std::string, Mix_Chunk*> m_sfxChunks;
    std::map<std::string, std::string> m_sfxFiles;
};

} // namespace io
