#pragma once

#include <string>
#include <map>
#include <memory>
#include <ctime>
#ifdef _WIN32
    #include <SDL_mixer.h>
#else
    #include <SDL2/SDL_mixer.h>
#endif

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
    
    // ファイル監視（自動リロード用）
    void checkAndReloadAudio();
    
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
    
    // ファイル監視用
    std::time_t m_bgmModTime;  // BGMの更新時刻
    std::map<std::string, std::time_t> m_sfxModTimes;  // SFXの更新時刻
    
    // MP3サポートの有無
    bool m_mp3Supported;
    
    // ヘルパー関数
    std::time_t getFileModificationTime(const std::string& filepath);
    void reloadBGM();
    void reloadSFX(const std::string& name);
};

} // namespace io
