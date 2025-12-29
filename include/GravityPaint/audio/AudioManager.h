#pragma once

#include "GravityPaint/Types.h"
#include "GravityPaint/Constants.h"
#include <SDL_mixer.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace GravityPaint {

enum class SoundEffect {
    Collision,
    GoalReached,
    GravitySwipe,
    ChainReaction,
    LevelComplete,
    StarEarned,
    ButtonClick,
    MenuSelect,
    TimeWarning,
    GameOver,
    EnergyTransfer,
    Bounce
};

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool initialize();
    void shutdown();

    // Sound effects
    void playSound(SoundEffect effect, float volume = 1.0f);
    void playSound(const std::string& name, float volume = 1.0f);
    void stopSound(int channel);
    void stopAllSounds();

    // Music
    void playMusic(const std::string& name, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void fadeOutMusic(int milliseconds);
    void fadeInMusic(const std::string& name, int milliseconds, bool loop = true);
    bool isMusicPlaying() const;

    // Volume control
    void setMasterVolume(float volume);
    void setMusicVolume(float volume);
    void setSFXVolume(float volume);
    float getMasterVolume() const { return m_masterVolume; }
    float getMusicVolume() const { return m_musicVolume; }
    float getSFXVolume() const { return m_sfxVolume; }

    // Mute
    void setMuted(bool muted);
    bool isMuted() const { return m_muted; }
    void toggleMute() { setMuted(!m_muted); }

    // Resource loading
    bool loadSound(const std::string& name, const std::string& filepath);
    bool loadMusic(const std::string& name, const std::string& filepath);
    void unloadSound(const std::string& name);
    void unloadMusic(const std::string& name);
    void preloadAll();
    
    // Programmatic sound generation
    void generateSounds();

private:
    int getNextChannel();
    const char* getSoundEffectName(SoundEffect effect) const;
    
    // Sound synthesis helpers
    Mix_Chunk* generateTone(float frequency, float duration, float volume = 1.0f);
    Mix_Chunk* generateNoise(float duration, float volume = 1.0f);
    Mix_Chunk* generateSweep(float startFreq, float endFreq, float duration, float volume = 1.0f);
    Mix_Chunk* generateClick();
    Mix_Chunk* generateBounce();
    Mix_Chunk* generateGoal();
    Mix_Chunk* generateLevelComplete();
    Mix_Chunk* generateGameOver();
    Mix_Chunk* generateSwipe();
    Mix_Chunk* generateStar();
    Mix_Chunk* generateWarning();
    
    // Music generation
    void generateMenuMusic();
    void generateGameplayMusic();
    std::vector<Uint8> generateMusicTrack(const std::vector<float>& melody, 
                                           const std::vector<float>& bass,
                                           float bpm, float duration);

    std::unordered_map<std::string, Mix_Chunk*> m_sounds;
    std::unordered_map<std::string, Mix_Music*> m_music;

    std::string m_currentMusic;
    int m_nextChannel = 0;
    static constexpr int MAX_CHANNELS = 16;
    static constexpr int SAMPLE_RATE = 44100;

    float m_masterVolume = MASTER_VOLUME;
    float m_musicVolume = MUSIC_VOLUME;
    float m_sfxVolume = SFX_VOLUME;
    bool m_muted = false;
    bool m_initialized = false;
};

} // namespace GravityPaint
