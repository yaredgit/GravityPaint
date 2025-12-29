#include "Vectoria/level/Level.h"
#include "Vectoria/Constants.h"
#include <fstream>
#include <sstream>

namespace Vectoria {

Level::Level()
    : m_width(DEFAULT_SCREEN_WIDTH)
    , m_height(DEFAULT_SCREEN_HEIGHT)
{
}

bool Level::load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        SDL_Log("Failed to open level file: %s", filepath.c_str());
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return loadFromString(buffer.str());
}

bool Level::loadFromString(const std::string& data) {
    // Simple key-value parsing for now
    // In production, use proper JSON parsing
    reset();
    
    std::istringstream stream(data);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;
        
        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);
        
        if (key == "id") m_id = std::stoi(value);
        else if (key == "name") m_name = value;
        else if (key == "width") m_width = std::stof(value);
        else if (key == "height") m_height = std::stof(value);
        else if (key == "timeLimit") m_timeLimit = std::stof(value);
        else if (key == "difficulty") m_difficulty = std::stoi(value);
        else if (key == "maxStrokes") m_maxStrokes = std::stoi(value);
    }
    
    return true;
}

void Level::reset() {
    m_spawnPoints.clear();
    m_obstacles.clear();
    m_gravityZones.clear();
    m_tutorialSteps.clear();
    m_objective.reset();
}

void Level::setDimensions(float width, float height) {
    m_width = width;
    m_height = height;
}

void Level::addSpawnPoint(const SpawnPoint& spawn) {
    m_spawnPoints.push_back(spawn);
}

void Level::addObstacle(const ObstacleData& obstacle) {
    m_obstacles.push_back(obstacle);
}

void Level::addGravityZone(const GravityZoneData& zone) {
    m_gravityZones.push_back(zone);
}

void Level::setObjective(std::unique_ptr<Objective> objective) {
    m_objective = std::move(objective);
}

int Level::getStarThreshold(int star) const {
    if (star >= 1 && star <= 3) {
        return m_starThresholds[star - 1];
    }
    return 0;
}

void Level::setStarThresholds(int one, int two, int three) {
    m_starThresholds[0] = one;
    m_starThresholds[1] = two;
    m_starThresholds[2] = three;
}

int Level::calculateStars(int score) const {
    if (score >= m_starThresholds[2]) return 3;
    if (score >= m_starThresholds[1]) return 2;
    if (score >= m_starThresholds[0]) return 1;
    return 0;
}

bool Level::parseJSON(const std::string& /*json*/) {
    // JSON parsing would go here
    // Using a lightweight JSON library like nlohmann/json
    return false;
}

} // namespace Vectoria
