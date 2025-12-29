#include "GravityPaint/core/ResourceManager.h"
#include "GravityPaint/Constants.h"
#include <fstream>
#include <sstream>

namespace GravityPaint {

std::string ResourceManager::s_emptyString;

ResourceManager::ResourceManager() = default;

ResourceManager::~ResourceManager() {
    shutdown();
}

bool ResourceManager::initialize(SDL_Renderer* renderer) {
    m_renderer = renderer;
    return m_renderer != nullptr;
}

void ResourceManager::shutdown() {
    unloadAll();
    m_renderer = nullptr;
}

bool ResourceManager::loadTexture(const std::string& name, const std::string& filepath) {
    if (!m_renderer) return false;

    // For now, create a simple colored texture as placeholder
    // In production, use SDL_image to load actual files
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 64, 64, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if (!surface) {
        SDL_Log("Failed to create surface for texture: %s", name.c_str());
        return false;
    }

    SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 255, 255, 255, 255));

    SDL_Texture* sdlTexture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);

    if (!sdlTexture) {
        SDL_Log("Failed to create texture: %s - %s", name.c_str(), SDL_GetError());
        return false;
    }

    auto texture = std::make_unique<Texture>();
    texture->sdlTexture = sdlTexture;
    SDL_QueryTexture(sdlTexture, nullptr, nullptr, &texture->width, &texture->height);

    m_textures[name] = std::move(texture);
    return true;
}

Texture* ResourceManager::getTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ResourceManager::unloadTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        if (it->second->sdlTexture) {
            SDL_DestroyTexture(it->second->sdlTexture);
        }
        m_textures.erase(it);
    }
}

bool ResourceManager::loadSound(const std::string& name, const std::string& filepath) {
    Mix_Chunk* chunk = Mix_LoadWAV(filepath.c_str());
    if (!chunk) {
        SDL_Log("Failed to load sound: %s - %s", filepath.c_str(), Mix_GetError());
        return false;
    }

    auto sound = std::make_unique<Sound>();
    sound->chunk = chunk;
    m_sounds[name] = std::move(sound);
    return true;
}

Sound* ResourceManager::getSound(const std::string& name) {
    auto it = m_sounds.find(name);
    if (it != m_sounds.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ResourceManager::unloadSound(const std::string& name) {
    auto it = m_sounds.find(name);
    if (it != m_sounds.end()) {
        if (it->second->chunk) {
            Mix_FreeChunk(it->second->chunk);
        }
        m_sounds.erase(it);
    }
}

bool ResourceManager::loadMusic(const std::string& name, const std::string& filepath) {
    Mix_Music* music = Mix_LoadMUS(filepath.c_str());
    if (!music) {
        SDL_Log("Failed to load music: %s - %s", filepath.c_str(), Mix_GetError());
        return false;
    }

    auto musicData = std::make_unique<Music>();
    musicData->music = music;
    m_music[name] = std::move(musicData);
    return true;
}

Music* ResourceManager::getMusic(const std::string& name) {
    auto it = m_music.find(name);
    if (it != m_music.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ResourceManager::unloadMusic(const std::string& name) {
    auto it = m_music.find(name);
    if (it != m_music.end()) {
        if (it->second->music) {
            Mix_FreeMusic(it->second->music);
        }
        m_music.erase(it);
    }
}

bool ResourceManager::loadLevelData(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        SDL_Log("Failed to open level file: %s", filepath.c_str());
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    
    // Extract filename as key
    size_t lastSlash = filepath.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? filepath.substr(lastSlash + 1) : filepath;
    
    m_levelData[filename] = buffer.str();
    return true;
}

const std::string& ResourceManager::getLevelData(const std::string& name) const {
    auto it = m_levelData.find(name);
    if (it != m_levelData.end()) {
        return it->second;
    }
    return s_emptyString;
}

bool ResourceManager::saveProgress(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        SDL_Log("Failed to open save file: %s", filepath.c_str());
        return false;
    }

    // Simple save format - in production use proper serialization
    // For now just return success
    return true;
}

bool ResourceManager::loadProgress(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return false; // No save file exists yet
    }

    // Simple load format - in production use proper serialization
    return true;
}

void ResourceManager::preloadAll() {
    // Preload common assets
    // In production, load from asset manifest
}

void ResourceManager::unloadAll() {
    for (auto& pair : m_textures) {
        if (pair.second->sdlTexture) {
            SDL_DestroyTexture(pair.second->sdlTexture);
        }
    }
    m_textures.clear();

    for (auto& pair : m_sounds) {
        if (pair.second->chunk) {
            Mix_FreeChunk(pair.second->chunk);
        }
    }
    m_sounds.clear();

    for (auto& pair : m_music) {
        if (pair.second->music) {
            Mix_FreeMusic(pair.second->music);
        }
    }
    m_music.clear();

    m_levelData.clear();
}

size_t ResourceManager::getMemoryUsage() const {
    size_t total = 0;
    
    for (const auto& pair : m_textures) {
        total += pair.second->width * pair.second->height * 4; // Approximate
    }
    
    // Sounds and music memory would need to be tracked separately
    
    return total;
}

} // namespace GravityPaint
