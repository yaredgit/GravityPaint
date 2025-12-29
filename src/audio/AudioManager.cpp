#include "Vectoria/audio/AudioManager.h"
#include "Vectoria/Constants.h"
#include <SDL.h>

namespace Vectoria {

AudioManager::AudioManager() = default;

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    if (Mix_OpenAudio(AUDIO_FREQUENCY, MIX_DEFAULT_FORMAT, AUDIO_CHANNELS, AUDIO_CHUNK_SIZE) < 0) {
        SDL_Log("SDL_mixer initialization failed: %s", Mix_GetError());
        return false;
    }

    Mix_AllocateChannels(MAX_CHANNELS);
    m_initialized = true;

    // Set initial volumes
    setMasterVolume(m_masterVolume);
    setMusicVolume(m_musicVolume);
    setSFXVolume(m_sfxVolume);

    return true;
}

void AudioManager::shutdown() {
    if (!m_initialized) return;

    stopAllSounds();
    stopMusic();

    // Unload all sounds
    for (auto& pair : m_sounds) {
        if (pair.second) {
            Mix_FreeChunk(pair.second);
        }
    }
    m_sounds.clear();

    // Unload all music
    for (auto& pair : m_music) {
        if (pair.second) {
            Mix_FreeMusic(pair.second);
        }
    }
    m_music.clear();

    Mix_CloseAudio();
    m_initialized = false;
}

void AudioManager::playSound(SoundEffect effect, float volume) {
    playSound(getSoundEffectName(effect), volume);
}

void AudioManager::playSound(const std::string& name, float volume) {
    if (!m_initialized || m_muted) return;

    auto it = m_sounds.find(name);
    if (it == m_sounds.end() || !it->second) {
        // Sound not loaded - this is expected for now since we don't have actual audio files
        return;
    }

    int channel = getNextChannel();
    float finalVolume = m_masterVolume * m_sfxVolume * volume;
    
    Mix_Volume(channel, static_cast<int>(finalVolume * MIX_MAX_VOLUME));
    Mix_PlayChannel(channel, it->second, 0);
}

void AudioManager::stopSound(int channel) {
    if (channel >= 0 && channel < MAX_CHANNELS) {
        Mix_HaltChannel(channel);
    }
}

void AudioManager::stopAllSounds() {
    Mix_HaltChannel(-1);
}

void AudioManager::playMusic(const std::string& name, bool loop) {
    if (!m_initialized || m_muted) return;

    auto it = m_music.find(name);
    if (it == m_music.end() || !it->second) {
        return;
    }

    m_currentMusic = name;
    Mix_PlayMusic(it->second, loop ? -1 : 1);
}

void AudioManager::stopMusic() {
    Mix_HaltMusic();
    m_currentMusic.clear();
}

void AudioManager::pauseMusic() {
    Mix_PauseMusic();
}

void AudioManager::resumeMusic() {
    Mix_ResumeMusic();
}

void AudioManager::fadeOutMusic(int milliseconds) {
    Mix_FadeOutMusic(milliseconds);
}

void AudioManager::fadeInMusic(const std::string& name, int milliseconds, bool loop) {
    if (!m_initialized || m_muted) return;

    auto it = m_music.find(name);
    if (it == m_music.end() || !it->second) {
        return;
    }

    m_currentMusic = name;
    Mix_FadeInMusic(it->second, loop ? -1 : 1, milliseconds);
}

bool AudioManager::isMusicPlaying() const {
    return Mix_PlayingMusic() != 0;
}

void AudioManager::setMasterVolume(float volume) {
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    
    // Update music volume
    setMusicVolume(m_musicVolume);
}

void AudioManager::setMusicVolume(float volume) {
    m_musicVolume = std::clamp(volume, 0.0f, 1.0f);
    
    float finalVolume = m_masterVolume * m_musicVolume;
    Mix_VolumeMusic(static_cast<int>(finalVolume * MIX_MAX_VOLUME));
}

void AudioManager::setSFXVolume(float volume) {
    m_sfxVolume = std::clamp(volume, 0.0f, 1.0f);
}

void AudioManager::setMuted(bool muted) {
    m_muted = muted;
    
    if (muted) {
        Mix_Volume(-1, 0);
        Mix_VolumeMusic(0);
    } else {
        setMasterVolume(m_masterVolume);
    }
}

bool AudioManager::loadSound(const std::string& name, const std::string& filepath) {
    if (!m_initialized) return false;

    Mix_Chunk* chunk = Mix_LoadWAV(filepath.c_str());
    if (!chunk) {
        SDL_Log("Failed to load sound '%s': %s", filepath.c_str(), Mix_GetError());
        return false;
    }

    // Unload existing sound with same name
    auto it = m_sounds.find(name);
    if (it != m_sounds.end() && it->second) {
        Mix_FreeChunk(it->second);
    }

    m_sounds[name] = chunk;
    return true;
}

bool AudioManager::loadMusic(const std::string& name, const std::string& filepath) {
    if (!m_initialized) return false;

    Mix_Music* music = Mix_LoadMUS(filepath.c_str());
    if (!music) {
        SDL_Log("Failed to load music '%s': %s", filepath.c_str(), Mix_GetError());
        return false;
    }

    // Unload existing music with same name
    auto it = m_music.find(name);
    if (it != m_music.end() && it->second) {
        Mix_FreeMusic(it->second);
    }

    m_music[name] = music;
    return true;
}

void AudioManager::unloadSound(const std::string& name) {
    auto it = m_sounds.find(name);
    if (it != m_sounds.end()) {
        if (it->second) {
            Mix_FreeChunk(it->second);
        }
        m_sounds.erase(it);
    }
}

void AudioManager::unloadMusic(const std::string& name) {
    auto it = m_music.find(name);
    if (it != m_music.end()) {
        if (it->second) {
            Mix_FreeMusic(it->second);
        }
        m_music.erase(it);
    }
}

void AudioManager::preloadAll() {
    // Preload all sound effects
    // In production, load from asset manifest
    loadSound("collision", std::string(SOUND_PATH) + "collision.wav");
    loadSound("goal", std::string(SOUND_PATH) + "goal.wav");
    loadSound("swipe", std::string(SOUND_PATH) + "swipe.wav");
    loadSound("chain", std::string(SOUND_PATH) + "chain.wav");
    loadSound("complete", std::string(SOUND_PATH) + "complete.wav");
    loadSound("star", std::string(SOUND_PATH) + "star.wav");
    loadSound("click", std::string(SOUND_PATH) + "click.wav");
    loadSound("select", std::string(SOUND_PATH) + "select.wav");
    loadSound("warning", std::string(SOUND_PATH) + "warning.wav");
    loadSound("gameover", std::string(SOUND_PATH) + "gameover.wav");
    loadSound("energy", std::string(SOUND_PATH) + "energy.wav");
    loadSound("bounce", std::string(SOUND_PATH) + "bounce.wav");

    // Preload music
    loadMusic("menu", std::string(SOUND_PATH) + "menu.ogg");
    loadMusic("gameplay", std::string(SOUND_PATH) + "gameplay.ogg");
}

int AudioManager::getNextChannel() {
    int channel = m_nextChannel;
    m_nextChannel = (m_nextChannel + 1) % MAX_CHANNELS;
    return channel;
}

const char* AudioManager::getSoundEffectName(SoundEffect effect) const {
    switch (effect) {
        case SoundEffect::Collision: return "collision";
        case SoundEffect::GoalReached: return "goal";
        case SoundEffect::GravitySwipe: return "swipe";
        case SoundEffect::ChainReaction: return "chain";
        case SoundEffect::LevelComplete: return "complete";
        case SoundEffect::StarEarned: return "star";
        case SoundEffect::ButtonClick: return "click";
        case SoundEffect::MenuSelect: return "select";
        case SoundEffect::TimeWarning: return "warning";
        case SoundEffect::GameOver: return "gameover";
        case SoundEffect::EnergyTransfer: return "energy";
        case SoundEffect::Bounce: return "bounce";
        default: return "unknown";
    }
}

} // namespace Vectoria
