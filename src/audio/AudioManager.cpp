#include "GravityPaint/audio/AudioManager.h"
#include "GravityPaint/Constants.h"
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace GravityPaint {

AudioManager::AudioManager() = default;

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    if (Mix_OpenAudio(SAMPLE_RATE, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("SDL_mixer initialization failed: %s", Mix_GetError());
        return false;
    }

    Mix_AllocateChannels(MAX_CHANNELS);
    m_initialized = true;

    // Generate all sounds programmatically
    generateSounds();

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

    for (auto& pair : m_sounds) {
        if (pair.second) {
            Mix_FreeChunk(pair.second);
        }
    }
    m_sounds.clear();

    for (auto& pair : m_music) {
        if (pair.second) {
            Mix_FreeMusic(pair.second);
        }
    }
    m_music.clear();

    Mix_CloseAudio();
    m_initialized = false;
}

void AudioManager::generateSounds() {
    // Generate all game sounds programmatically
    m_sounds["click"] = generateClick();
    m_sounds["select"] = generateClick();  // Same as click
    m_sounds["bounce"] = generateBounce();
    m_sounds["collision"] = generateBounce();
    m_sounds["goal"] = generateGoal();
    m_sounds["complete"] = generateLevelComplete();
    m_sounds["gameover"] = generateGameOver();
    m_sounds["swipe"] = generateSwipe();
    m_sounds["star"] = generateStar();
    m_sounds["warning"] = generateWarning();
    m_sounds["chain"] = generateGoal();  // Similar to goal
    m_sounds["energy"] = generateSwipe();  // Similar to swipe
    
    SDL_Log("Generated %zu programmatic sounds", m_sounds.size());
    
    // Generate background music
    generateMenuMusic();
    generateGameplayMusic();
    SDL_Log("Generated programmatic music tracks");
}

Mix_Chunk* AudioManager::generateTone(float frequency, float duration, float volume) {
    int samples = static_cast<int>(SAMPLE_RATE * duration);
    int bufferSize = samples * 2 * sizeof(Sint16);  // Stereo
    
    Uint8* buffer = new Uint8[bufferSize];
    Sint16* samples16 = reinterpret_cast<Sint16*>(buffer);
    
    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float envelope = 1.0f;
        
        // Apply fade out in last 20% of sound
        float fadeStart = duration * 0.8f;
        if (t > fadeStart) {
            envelope = 1.0f - (t - fadeStart) / (duration * 0.2f);
        }
        
        // Apply fade in for first 5%
        if (t < duration * 0.05f) {
            envelope *= t / (duration * 0.05f);
        }
        
        float sample = std::sin(2.0f * 3.14159f * frequency * t) * volume * envelope;
        Sint16 value = static_cast<Sint16>(sample * 32767.0f * 0.5f);
        samples16[i * 2] = value;      // Left
        samples16[i * 2 + 1] = value;  // Right
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, bufferSize);
    if (chunk) {
        chunk->allocated = 1;  // Let SDL_mixer free the buffer
    }
    return chunk;
}

Mix_Chunk* AudioManager::generateNoise(float duration, float volume) {
    int samples = static_cast<int>(SAMPLE_RATE * duration);
    int bufferSize = samples * 2 * sizeof(Sint16);
    
    Uint8* buffer = new Uint8[bufferSize];
    Sint16* samples16 = reinterpret_cast<Sint16*>(buffer);
    
    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float envelope = 1.0f - (t / duration);  // Linear fade out
        
        float noise = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f);
        Sint16 value = static_cast<Sint16>(noise * volume * envelope * 32767.0f * 0.3f);
        samples16[i * 2] = value;
        samples16[i * 2 + 1] = value;
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, bufferSize);
    if (chunk) chunk->allocated = 1;
    return chunk;
}

Mix_Chunk* AudioManager::generateSweep(float startFreq, float endFreq, float duration, float volume) {
    int samples = static_cast<int>(SAMPLE_RATE * duration);
    int bufferSize = samples * 2 * sizeof(Sint16);
    
    Uint8* buffer = new Uint8[bufferSize];
    Sint16* samples16 = reinterpret_cast<Sint16*>(buffer);
    
    float phase = 0.0f;
    
    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;
        
        // Exponential frequency sweep
        float freq = startFreq * std::pow(endFreq / startFreq, progress);
        
        // Envelope
        float envelope = 1.0f;
        if (progress > 0.7f) {
            envelope = 1.0f - (progress - 0.7f) / 0.3f;
        }
        if (progress < 0.05f) {
            envelope *= progress / 0.05f;
        }
        
        phase += 2.0f * 3.14159f * freq / SAMPLE_RATE;
        float sample = std::sin(phase) * volume * envelope;
        Sint16 value = static_cast<Sint16>(sample * 32767.0f * 0.5f);
        samples16[i * 2] = value;
        samples16[i * 2 + 1] = value;
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, bufferSize);
    if (chunk) chunk->allocated = 1;
    return chunk;
}

Mix_Chunk* AudioManager::generateClick() {
    // Short, punchy click sound
    float duration = 0.05f;
    int samples = static_cast<int>(SAMPLE_RATE * duration);
    int bufferSize = samples * 2 * sizeof(Sint16);
    
    Uint8* buffer = new Uint8[bufferSize];
    Sint16* samples16 = reinterpret_cast<Sint16*>(buffer);
    
    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float envelope = std::exp(-t * 80.0f);  // Fast decay
        
        // Mix of frequencies for a nice click
        float sample = std::sin(2.0f * 3.14159f * 800.0f * t) * 0.5f +
                       std::sin(2.0f * 3.14159f * 1200.0f * t) * 0.3f +
                       std::sin(2.0f * 3.14159f * 400.0f * t) * 0.2f;
        
        Sint16 value = static_cast<Sint16>(sample * envelope * 32767.0f * 0.4f);
        samples16[i * 2] = value;
        samples16[i * 2 + 1] = value;
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, bufferSize);
    if (chunk) chunk->allocated = 1;
    return chunk;
}

Mix_Chunk* AudioManager::generateBounce() {
    // Bouncy "boing" sound with pitch drop
    float duration = 0.15f;
    int samples = static_cast<int>(SAMPLE_RATE * duration);
    int bufferSize = samples * 2 * sizeof(Sint16);
    
    Uint8* buffer = new Uint8[bufferSize];
    Sint16* samples16 = reinterpret_cast<Sint16*>(buffer);
    
    float phase = 0.0f;
    
    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;
        
        // Frequency drops from 600 to 200 Hz
        float freq = 600.0f - 400.0f * progress;
        float envelope = std::exp(-t * 15.0f);
        
        phase += 2.0f * 3.14159f * freq / SAMPLE_RATE;
        float sample = std::sin(phase) * envelope;
        
        Sint16 value = static_cast<Sint16>(sample * 32767.0f * 0.5f);
        samples16[i * 2] = value;
        samples16[i * 2 + 1] = value;
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, bufferSize);
    if (chunk) chunk->allocated = 1;
    return chunk;
}

Mix_Chunk* AudioManager::generateGoal() {
    // Triumphant rising tone
    float duration = 0.3f;
    int samples = static_cast<int>(SAMPLE_RATE * duration);
    int bufferSize = samples * 2 * sizeof(Sint16);
    
    Uint8* buffer = new Uint8[bufferSize];
    Sint16* samples16 = reinterpret_cast<Sint16*>(buffer);
    
    float phase1 = 0.0f, phase2 = 0.0f;
    
    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;
        
        // Rising frequency
        float freq1 = 400.0f + 400.0f * progress;
        float freq2 = 600.0f + 400.0f * progress;
        
        float envelope = 1.0f;
        if (progress > 0.7f) {
            envelope = 1.0f - (progress - 0.7f) / 0.3f;
        }
        
        phase1 += 2.0f * 3.14159f * freq1 / SAMPLE_RATE;
        phase2 += 2.0f * 3.14159f * freq2 / SAMPLE_RATE;
        
        float sample = (std::sin(phase1) * 0.6f + std::sin(phase2) * 0.4f) * envelope;
        Sint16 value = static_cast<Sint16>(sample * 32767.0f * 0.5f);
        samples16[i * 2] = value;
        samples16[i * 2 + 1] = value;
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, bufferSize);
    if (chunk) chunk->allocated = 1;
    return chunk;
}

Mix_Chunk* AudioManager::generateLevelComplete() {
    // Happy ascending arpeggio
    float duration = 0.8f;
    int samples = static_cast<int>(SAMPLE_RATE * duration);
    int bufferSize = samples * 2 * sizeof(Sint16);
    
    Uint8* buffer = new Uint8[bufferSize];
    Sint16* samples16 = reinterpret_cast<Sint16*>(buffer);
    
    // C major arpeggio: C4, E4, G4, C5
    float notes[] = {261.63f, 329.63f, 392.00f, 523.25f};
    float noteLength = duration / 4.0f;
    
    float phase = 0.0f;
    
    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        int noteIndex = std::min(3, static_cast<int>(t / noteLength));
        float noteTime = t - noteIndex * noteLength;
        
        float freq = notes[noteIndex];
        float envelope = std::exp(-noteTime * 5.0f);
        
        // Global fade out
        if (t > duration * 0.8f) {
            envelope *= 1.0f - (t - duration * 0.8f) / (duration * 0.2f);
        }
        
        phase += 2.0f * 3.14159f * freq / SAMPLE_RATE;
        float sample = std::sin(phase) * envelope;
        
        // Add harmonics for richness
        sample += std::sin(phase * 2.0f) * envelope * 0.3f;
        sample += std::sin(phase * 3.0f) * envelope * 0.1f;
        
        Sint16 value = static_cast<Sint16>(sample * 32767.0f * 0.4f);
        samples16[i * 2] = value;
        samples16[i * 2 + 1] = value;
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, bufferSize);
    if (chunk) chunk->allocated = 1;
    return chunk;
}

Mix_Chunk* AudioManager::generateGameOver() {
    // Sad descending tones
    float duration = 0.6f;
    int samples = static_cast<int>(SAMPLE_RATE * duration);
    int bufferSize = samples * 2 * sizeof(Sint16);
    
    Uint8* buffer = new Uint8[bufferSize];
    Sint16* samples16 = reinterpret_cast<Sint16*>(buffer);
    
    float phase1 = 0.0f, phase2 = 0.0f;
    
    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;
        
        // Descending minor interval
        float freq1 = 400.0f - 150.0f * progress;
        float freq2 = 300.0f - 100.0f * progress;
        
        float envelope = 1.0f - progress * 0.5f;
        if (progress > 0.8f) {
            envelope *= 1.0f - (progress - 0.8f) / 0.2f;
        }
        
        phase1 += 2.0f * 3.14159f * freq1 / SAMPLE_RATE;
        phase2 += 2.0f * 3.14159f * freq2 / SAMPLE_RATE;
        
        float sample = (std::sin(phase1) * 0.5f + std::sin(phase2) * 0.5f) * envelope;
        Sint16 value = static_cast<Sint16>(sample * 32767.0f * 0.5f);
        samples16[i * 2] = value;
        samples16[i * 2 + 1] = value;
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, bufferSize);
    if (chunk) chunk->allocated = 1;
    return chunk;
}

Mix_Chunk* AudioManager::generateSwipe() {
    // Whoosh sound - filtered noise with frequency sweep
    float duration = 0.2f;
    int samples = static_cast<int>(SAMPLE_RATE * duration);
    int bufferSize = samples * 2 * sizeof(Sint16);
    
    Uint8* buffer = new Uint8[bufferSize];
    Sint16* samples16 = reinterpret_cast<Sint16*>(buffer);
    
    float phase = 0.0f;
    
    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;
        
        // Frequency sweep up then down
        float freq = 200.0f + 600.0f * std::sin(progress * 3.14159f);
        
        float envelope = std::sin(progress * 3.14159f);  // Fade in and out
        
        phase += 2.0f * 3.14159f * freq / SAMPLE_RATE;
        
        // Mix sine with noise for whoosh effect
        float noise = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * 0.3f;
        float sample = (std::sin(phase) * 0.7f + noise) * envelope;
        
        Sint16 value = static_cast<Sint16>(sample * 32767.0f * 0.4f);
        samples16[i * 2] = value;
        samples16[i * 2 + 1] = value;
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, bufferSize);
    if (chunk) chunk->allocated = 1;
    return chunk;
}

Mix_Chunk* AudioManager::generateStar() {
    // Sparkly chime sound
    float duration = 0.25f;
    int samples = static_cast<int>(SAMPLE_RATE * duration);
    int bufferSize = samples * 2 * sizeof(Sint16);
    
    Uint8* buffer = new Uint8[bufferSize];
    Sint16* samples16 = reinterpret_cast<Sint16*>(buffer);
    
    // High frequencies for sparkle
    float freqs[] = {1200.0f, 1500.0f, 1800.0f, 2400.0f};
    
    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float envelope = std::exp(-t * 12.0f);
        
        float sample = 0.0f;
        for (int f = 0; f < 4; ++f) {
            sample += std::sin(2.0f * 3.14159f * freqs[f] * t) * (0.4f - f * 0.08f);
        }
        
        Sint16 value = static_cast<Sint16>(sample * envelope * 32767.0f * 0.3f);
        samples16[i * 2] = value;
        samples16[i * 2 + 1] = value;
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, bufferSize);
    if (chunk) chunk->allocated = 1;
    return chunk;
}

Mix_Chunk* AudioManager::generateWarning() {
    // Urgent beep-beep
    float duration = 0.3f;
    int samples = static_cast<int>(SAMPLE_RATE * duration);
    int bufferSize = samples * 2 * sizeof(Sint16);
    
    Uint8* buffer = new Uint8[bufferSize];
    Sint16* samples16 = reinterpret_cast<Sint16*>(buffer);
    
    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        
        // Two beeps
        bool beepOn = (t < 0.1f) || (t > 0.15f && t < 0.25f);
        float envelope = beepOn ? 1.0f : 0.0f;
        
        // Soften edges
        float beepTime = std::fmod(t, 0.15f);
        if (beepTime < 0.01f) envelope *= beepTime / 0.01f;
        if (beepTime > 0.09f && beepTime < 0.1f) envelope *= (0.1f - beepTime) / 0.01f;
        
        float sample = std::sin(2.0f * 3.14159f * 880.0f * t) * envelope;
        Sint16 value = static_cast<Sint16>(sample * 32767.0f * 0.5f);
        samples16[i * 2] = value;
        samples16[i * 2 + 1] = value;
    }
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, bufferSize);
    if (chunk) chunk->allocated = 1;
    return chunk;
}

void AudioManager::playSound(SoundEffect effect, float volume) {
    playSound(getSoundEffectName(effect), volume);
}

void AudioManager::playSound(const std::string& name, float volume) {
    if (!m_initialized || m_muted) return;

    auto it = m_sounds.find(name);
    if (it == m_sounds.end() || !it->second) {
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
    // Sounds are now generated programmatically in initialize()
    // This function can be used to load external audio files if needed
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

std::vector<Uint8> AudioManager::generateMusicTrack(const std::vector<float>& melody,
                                                     const std::vector<float>& bass,
                                                     float bpm, float duration) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * duration);
    std::vector<Uint8> buffer(totalSamples * 2 * sizeof(Sint16));
    Sint16* samples = reinterpret_cast<Sint16*>(buffer.data());
    
    float beatDuration = 60.0f / bpm;
    float phase1 = 0.0f, phase2 = 0.0f, phase3 = 0.0f;
    
    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float loopT = std::fmod(t, duration);
        
        // Current beat and position within beat
        int beat = static_cast<int>(loopT / beatDuration) % static_cast<int>(melody.size());
        float beatProgress = std::fmod(loopT, beatDuration) / beatDuration;
        
        // Melody (square wave for chiptune feel)
        float melodyFreq = melody[beat % melody.size()];
        if (melodyFreq > 0) {
            phase1 += 2.0f * 3.14159f * melodyFreq / SAMPLE_RATE;
            // Soft square wave
            float melodyWave = std::sin(phase1) > 0 ? 0.3f : -0.3f;
            // Add slight sine for smoothness
            melodyWave = melodyWave * 0.7f + std::sin(phase1) * 0.3f;
            // Envelope per note
            float env = 1.0f - beatProgress * 0.5f;
            melodyWave *= env * 0.25f;
            
            samples[i * 2] += static_cast<Sint16>(melodyWave * 32767.0f);
            samples[i * 2 + 1] += static_cast<Sint16>(melodyWave * 32767.0f);
        }
        
        // Bass (triangle wave)
        float bassFreq = bass[beat % bass.size()];
        if (bassFreq > 0) {
            phase2 += 2.0f * 3.14159f * bassFreq / SAMPLE_RATE;
            // Triangle wave
            float bassWave = std::asin(std::sin(phase2)) * 2.0f / 3.14159f;
            float bassEnv = 1.0f - beatProgress * 0.3f;
            bassWave *= bassEnv * 0.2f;
            
            samples[i * 2] += static_cast<Sint16>(bassWave * 32767.0f);
            samples[i * 2 + 1] += static_cast<Sint16>(bassWave * 32767.0f);
        }
        
        // Subtle arpeggio/pad (sine wave, very quiet)
        phase3 += 2.0f * 3.14159f * (melodyFreq > 0 ? melodyFreq * 2.0f : 440.0f) / SAMPLE_RATE;
        float pad = std::sin(phase3) * 0.08f;
        samples[i * 2] += static_cast<Sint16>(pad * 32767.0f);
        samples[i * 2 + 1] += static_cast<Sint16>(pad * 32767.0f);
    }
    
    return buffer;
}

void AudioManager::generateMenuMusic() {
    // Calm, ambient melody in C major pentatonic
    // Notes: C4=262, D4=294, E4=330, G4=392, A4=440, C5=523
    std::vector<float> melody = {
        262, 0, 330, 0, 392, 0, 330, 0,      // C - E - G - E
        440, 0, 392, 0, 330, 0, 262, 0,      // A - G - E - C
        294, 0, 392, 0, 440, 0, 392, 0,      // D - G - A - G
        330, 0, 294, 0, 262, 0, 0, 0         // E - D - C - rest
    };
    
    std::vector<float> bass = {
        131, 0, 0, 0, 131, 0, 0, 0,          // C2
        110, 0, 0, 0, 110, 0, 0, 0,          // A1
        147, 0, 0, 0, 147, 0, 0, 0,          // D2
        131, 0, 0, 0, 131, 0, 0, 0           // C2
    };
    
    float bpm = 70.0f;  // Slow, relaxed tempo
    float duration = static_cast<float>(melody.size()) * (60.0f / bpm);
    
    auto buffer = generateMusicTrack(melody, bass, bpm, duration);
    
    // Save to temp file and load as music (SDL_mixer requires file for music)
    std::string tempPath = "menu_music.raw";
    SDL_RWops* rw = SDL_RWFromFile(tempPath.c_str(), "wb");
    if (rw) {
        SDL_RWwrite(rw, buffer.data(), 1, buffer.size());
        SDL_RWclose(rw);
        
        // Create WAV header and proper file
        // For simplicity, we'll store as a looping chunk instead
    }
    
    // Store as a chunk that can be played on loop
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(new Uint8[buffer.size()], static_cast<Uint32>(buffer.size()));
    if (chunk) {
        std::memcpy(chunk->abuf, buffer.data(), buffer.size());
        chunk->allocated = 1;
        chunk->alen = static_cast<Uint32>(buffer.size());
        m_sounds["menu_music"] = chunk;
    }
}

void AudioManager::generateGameplayMusic() {
    // More energetic, upbeat melody
    // Using A minor for slight tension: A=440, C=523, D=587, E=659, G=784
    std::vector<float> melody = {
        440, 0, 523, 0, 587, 0, 523, 0,      // A - C - D - C
        659, 0, 587, 0, 523, 0, 440, 0,      // E - D - C - A
        523, 0, 659, 0, 784, 0, 659, 0,      // C - E - G - E
        587, 0, 523, 0, 440, 0, 0, 0,        // D - C - A - rest
        440, 0, 587, 0, 523, 0, 440, 0,      // A - D - C - A
        659, 0, 523, 0, 587, 0, 659, 0,      // E - C - D - E
        784, 0, 659, 0, 587, 0, 523, 0,      // G - E - D - C
        440, 0, 523, 0, 440, 0, 0, 0         // A - C - A - rest
    };
    
    std::vector<float> bass = {
        110, 0, 0, 0, 110, 0, 0, 0,          // A1
        110, 0, 0, 0, 130, 0, 0, 0,          // A1, C2
        130, 0, 0, 0, 130, 0, 0, 0,          // C2
        147, 0, 0, 0, 110, 0, 0, 0,          // D2, A1
        110, 0, 0, 0, 147, 0, 0, 0,          // A1, D2
        165, 0, 0, 0, 130, 0, 0, 0,          // E2, C2
        196, 0, 0, 0, 165, 0, 0, 0,          // G2, E2
        110, 0, 0, 0, 110, 0, 0, 0           // A1
    };
    
    float bpm = 100.0f;  // Moderate upbeat tempo
    float duration = static_cast<float>(melody.size()) * (60.0f / bpm);
    
    auto buffer = generateMusicTrack(melody, bass, bpm, duration);
    
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(new Uint8[buffer.size()], static_cast<Uint32>(buffer.size()));
    if (chunk) {
        std::memcpy(chunk->abuf, buffer.data(), buffer.size());
        chunk->allocated = 1;
        chunk->alen = static_cast<Uint32>(buffer.size());
        m_sounds["gameplay_music"] = chunk;
    }
}

} // namespace GravityPaint
