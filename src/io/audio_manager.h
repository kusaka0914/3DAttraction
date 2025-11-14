/**
 * @file audio_manager.h
 * @brief オーディオマネージャー
 * @details BGMと効果音の読み込み、再生、音量制御を統合的に管理します。
 */
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

/**
 * @brief オーディオマネージャー
 * @details BGMと効果音の読み込み、再生、音量制御を統合的に管理します。
 */
class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    /**
     * @brief BGMを読み込む
     * @param filename ファイル名
     * @return 読み込み成功時true
     */
    bool loadBGM(const std::string& filename);
    
    /**
     * @brief BGMを再生する
     * @details 現在読み込まれているBGMを再生します。
     */
    void playBGM();
    
    /**
     * @brief BGMを停止する
     * @details 現在再生中のBGMを停止します。
     */
    void stopBGM();
    
    /**
     * @brief BGMを一時停止する
     * @details 現在再生中のBGMを一時停止します。
     */
    void pauseBGM();
    
    /**
     * @brief BGMの再生を再開する
     * @details 一時停止中のBGMの再生を再開します。
     */
    void resumeBGM();
    
    /**
     * @brief BGMの音量を設定する
     * @param volume 音量（0.0～1.0）
     */
    void setBGMVolume(float volume);
    
    /**
     * @brief BGMの音量を取得する
     * @return 音量（0.0～1.0）
     */
    float getBGMVolume() const;
    
    /**
     * @brief BGMが再生中か確認する
     * @return 再生中の場合true
     */
    bool isBGMPlaying() const;
    
    /**
     * @brief 効果音を読み込む
     * @param name 効果音の名前
     * @param filename ファイル名
     * @return 読み込み成功時true
     */
    bool loadSFX(const std::string& name, const std::string& filename);
    
    /**
     * @brief 効果音を再生する
     * @param name 効果音の名前
     */
    void playSFX(const std::string& name);
    
    /**
     * @brief 効果音の音量を設定する
     * @param volume 音量（0.0～1.0）
     */
    void setSFXVolume(float volume);
    
    /**
     * @brief マスター音量を設定する
     * @param volume 音量（0.0～1.0）
     */
    void setMasterVolume(float volume);
    
    /**
     * @brief マスター音量を取得する
     * @return 音量（0.0～1.0）
     */
    float getMasterVolume() const;
    
    /**
     * @brief 音声システムを初期化する
     * @return 初期化成功時true
     */
    bool initialize();
    
    /**
     * @brief 音声システムを終了する
     * @details リソースを解放します。
     */
    void shutdown();
    
    /**
     * @brief 音声ファイルの変更を確認してリロードする
     * @details 音声ファイルが変更されていた場合、自動的にリロードします。
     */
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
    
    std::time_t m_bgmModTime;
    std::map<std::string, std::time_t> m_sfxModTimes;
    
    bool m_mp3Supported;
    
    std::time_t getFileModificationTime(const std::string& filepath);
    void reloadBGM();
    void reloadSFX(const std::string& name);
};

} // namespace io
