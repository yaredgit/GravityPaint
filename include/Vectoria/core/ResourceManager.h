#pragma once

#include "Vectoria/Types.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace Vectoria {

struct Texture {
    SDL_Texture* sdlTexture = nullptr;
    int width = 0;
    int height = 0;
};

struct Sound {
    Mix_Chunk* chunk = nullptr;
};

struct Music {
    Mix_Music* music = nullptr;
};

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    bool initialize(SDL_Renderer* renderer);
    void shutdown();

    // Texture management
    bool loadTexture(const std::string& name, const std::string& filepath);
    Texture* getTexture(const std::string& name);
    void unloadTexture(const std::string& name);

    // Sound management
    bool loadSound(const std::string& name, const std::string& filepath);
    Sound* getSound(const std::string& name);
    void unloadSound(const std::string& name);

    // Music management
    bool loadMusic(const std::string& name, const std::string& filepath);
    Music* getMusic(const std::string& name);
    void unloadMusic(const std::string& name);

    // Level data
    bool loadLevelData(const std::string& filepath);
    const std::string& getLevelData(const std::string& name) const;

    // Save/Load game progress
    bool saveProgress(const std::string& filepath);
    bool loadProgress(const std::string& filepath);

    // Utility
    void preloadAll();
    void unloadAll();
    size_t getMemoryUsage() const;

private:
    SDL_Renderer* m_renderer = nullptr;
    
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::unique_ptr<Sound>> m_sounds;
    std::unordered_map<std::string, std::unique_ptr<Music>> m_music;
    std::unordered_map<std::string, std::string> m_levelData;

    static std::string s_emptyString;
};

} // namespace Vectoria
