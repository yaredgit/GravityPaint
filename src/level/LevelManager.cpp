#include "Vectoria/level/LevelManager.h"
#include "Vectoria/level/Level.h"
#include "Vectoria/level/Objective.h"
#include "Vectoria/physics/PhysicsWorld.h"
#include "Vectoria/physics/PhysicsObject.h"
#include "Vectoria/Constants.h"
#include <fstream>
#include <random>

namespace Vectoria {

const LevelProgress LevelManager::s_emptyProgress;

LevelManager::LevelManager() = default;

LevelManager::~LevelManager() {
    shutdown();
}

bool LevelManager::initialize() {
    m_progress.resize(m_totalLevels);
    
    // Mark first level as unlocked
    if (!m_progress.empty()) {
        m_progress[0].levelId = 1;
    }

    createBuiltInLevels();
    
    return true;
}

void LevelManager::shutdown() {
    m_currentLevel.reset();
    m_progress.clear();
}

bool LevelManager::loadLevel(int levelId) {
    if (levelId < 1 || levelId > m_totalLevels) {
        return false;
    }

    m_currentLevel = std::make_unique<Level>();
    m_currentLevelId = levelId;

    // Create procedural level based on ID
    Level* level = createLevel(levelId, 1 + (levelId - 1) / 10);
    if (level) {
        *m_currentLevel = *level;
        delete level;
    }

    m_spawnedObjects.clear();
    m_spawnedObjects.resize(m_currentLevel->getSpawnPoints().size(), false);
    m_allSpawned = false;
    m_spawnTimer = 0;

    return true;
}

bool LevelManager::loadLevelFromFile(const std::string& filepath) {
    m_currentLevel = std::make_unique<Level>();
    if (!m_currentLevel->load(filepath)) {
        m_currentLevel.reset();
        return false;
    }

    m_currentLevelId = m_currentLevel->getId();
    m_spawnedObjects.clear();
    m_spawnedObjects.resize(m_currentLevel->getSpawnPoints().size(), false);
    m_allSpawned = false;
    m_spawnTimer = 0;

    return true;
}

void LevelManager::unloadCurrentLevel() {
    m_currentLevel.reset();
    m_currentLevelId = 0;
    m_levelActive = false;
}

bool LevelManager::reloadCurrentLevel() {
    int id = m_currentLevelId;
    unloadCurrentLevel();
    return loadLevel(id);
}

void LevelManager::startLevel() {
    m_levelActive = true;
    m_levelTime = 0;
    m_spawnTimer = 0;
    
    for (auto& spawned : m_spawnedObjects) {
        spawned = false;
    }
    m_allSpawned = false;
}

void LevelManager::updateLevel(float deltaTime) {
    if (!m_levelActive || !m_currentLevel) return;

    m_levelTime += deltaTime;

    // Update objective
    // (Physics world handles the actual checking)
}

void LevelManager::completeLevel(int score, float time) {
    if (m_currentLevelId < 1 || m_currentLevelId > static_cast<int>(m_progress.size())) {
        return;
    }

    LevelProgress& progress = m_progress[m_currentLevelId - 1];
    progress.levelId = m_currentLevelId;
    progress.completed = true;
    progress.attempts++;

    if (score > progress.bestScore) {
        progress.bestScore = score;
    }

    if (time < progress.bestTime || progress.bestTime == 0) {
        progress.bestTime = time;
    }

    progress.stars = m_currentLevel->calculateStars(score);

    // Unlock next level
    if (m_currentLevelId < m_totalLevels && m_currentLevelId < static_cast<int>(m_progress.size())) {
        m_progress[m_currentLevelId].levelId = m_currentLevelId + 1;
    }

    m_levelActive = false;
}

void LevelManager::failLevel() {
    if (m_currentLevelId >= 1 && m_currentLevelId <= static_cast<int>(m_progress.size())) {
        m_progress[m_currentLevelId - 1].attempts++;
    }
    m_levelActive = false;
}

void LevelManager::resetLevel() {
    m_levelTime = 0;
    m_spawnTimer = 0;
    
    for (auto& spawned : m_spawnedObjects) {
        spawned = false;
    }
    m_allSpawned = false;
}

bool LevelManager::hasNextLevel() const {
    return m_currentLevelId < m_totalLevels;
}

bool LevelManager::hasPreviousLevel() const {
    return m_currentLevelId > 1;
}

void LevelManager::nextLevel() {
    if (hasNextLevel()) {
        loadLevel(m_currentLevelId + 1);
    }
}

void LevelManager::previousLevel() {
    if (hasPreviousLevel()) {
        loadLevel(m_currentLevelId - 1);
    }
}

void LevelManager::goToLevel(int levelId) {
    if (isLevelUnlocked(levelId)) {
        loadLevel(levelId);
    }
}

int LevelManager::getUnlockedLevelCount() const {
    int count = 0;
    for (const auto& progress : m_progress) {
        if (progress.levelId > 0) count++;
    }
    return std::max(1, count);
}

const LevelProgress& LevelManager::getLevelProgress(int levelId) const {
    if (levelId >= 1 && levelId <= static_cast<int>(m_progress.size())) {
        return m_progress[levelId - 1];
    }
    return s_emptyProgress;
}

void LevelManager::setLevelProgress(int levelId, const LevelProgress& progress) {
    if (levelId >= 1 && levelId <= static_cast<int>(m_progress.size())) {
        m_progress[levelId - 1] = progress;
    }
}

int LevelManager::getTotalStars() const {
    int total = 0;
    for (const auto& progress : m_progress) {
        total += progress.stars;
    }
    return total;
}

bool LevelManager::isLevelUnlocked(int levelId) const {
    if (levelId == 1) return true;
    if (levelId < 1 || levelId > static_cast<int>(m_progress.size())) return false;
    
    // Level is unlocked if previous level was completed
    return m_progress[levelId - 2].completed;
}

void LevelManager::spawnObjects(PhysicsWorld* physics) {
    if (!m_currentLevel || !physics) return;

    const auto& spawnPoints = m_currentLevel->getSpawnPoints();
    
    for (size_t i = 0; i < spawnPoints.size(); ++i) {
        const SpawnPoint& spawn = spawnPoints[i];
        
        if (spawn.delay <= 0 && !m_spawnedObjects[i]) {
            PhysicsObject* obj = physics->createObject(spawn.objectType, spawn.position, spawn.size);
            if (obj) {
                obj->setEnergy(spawn.energy);
                obj->setColor(spawn.color);
            }
            m_spawnedObjects[i] = true;
        }
    }
}

void LevelManager::updateSpawns(float deltaTime, PhysicsWorld* physics) {
    if (!m_currentLevel || !physics || m_allSpawned) return;

    m_spawnTimer += deltaTime;
    
    const auto& spawnPoints = m_currentLevel->getSpawnPoints();
    bool allDone = true;

    for (size_t i = 0; i < spawnPoints.size(); ++i) {
        if (m_spawnedObjects[i]) continue;

        const SpawnPoint& spawn = spawnPoints[i];
        
        if (m_spawnTimer >= spawn.delay) {
            PhysicsObject* obj = physics->createObject(spawn.objectType, spawn.position, spawn.size);
            if (obj) {
                obj->setEnergy(spawn.energy);
                obj->setColor(spawn.color);
            }
            m_spawnedObjects[i] = true;
        } else {
            allDone = false;
        }
    }

    m_allSpawned = allDone;
}

std::unique_ptr<Level> LevelManager::generateProceduralLevel(int difficulty) {
    auto level = std::make_unique<Level>();
    
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> distX(100, DEFAULT_SCREEN_WIDTH - 100);
    std::uniform_real_distribution<float> distY(100, DEFAULT_SCREEN_HEIGHT / 2);

    level->setDimensions(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
    level->setTimeLimit(90.0f - difficulty * 5.0f);
    level->setDifficulty(difficulty);
    level->setMaxStrokes(MAX_ACTIVE_STROKES);

    // Goal zone at bottom
    level->setGoalZone(Rect(DEFAULT_SCREEN_WIDTH / 2 - 100, DEFAULT_SCREEN_HEIGHT - 150, 200, 100));

    // Spawn points based on difficulty
    int objectCount = 1 + difficulty / 2;
    for (int i = 0; i < objectCount; ++i) {
        SpawnPoint spawn;
        spawn.position = Vec2(distX(rng), distY(rng));
        spawn.objectType = static_cast<ObjectType>(i % 5);
        spawn.delay = i * 1.5f;
        spawn.size = 1.0f;
        spawn.energy = 50.0f;
        spawn.color = Color::cyan();
        level->addSpawnPoint(spawn);
    }

    // Obstacles based on difficulty
    int obstacleCount = difficulty / 3;
    for (int i = 0; i < obstacleCount; ++i) {
        ObstacleData obstacle;
        obstacle.position = Vec2(distX(rng), DEFAULT_SCREEN_HEIGHT / 2 + distY(rng) / 2);
        obstacle.size = Vec2(100, 20);
        obstacle.isCircle = (i % 2 == 0);
        level->addObstacle(obstacle);
    }

    // Objective
    level->setObjective(std::make_unique<ReachGoalObjective>(objectCount));

    // Star thresholds
    level->setStarThresholds(100, 200, 350);

    return level;
}

bool LevelManager::saveProgress(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    // Simple binary format
    size_t count = m_progress.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& progress : m_progress) {
        file.write(reinterpret_cast<const char*>(&progress), sizeof(LevelProgress));
    }

    return true;
}

bool LevelManager::loadProgress(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    size_t count;
    file.read(reinterpret_cast<char*>(&count), sizeof(count));

    if (count != m_progress.size()) {
        return false; // Version mismatch
    }

    for (auto& progress : m_progress) {
        file.read(reinterpret_cast<char*>(&progress), sizeof(LevelProgress));
    }

    return true;
}

void LevelManager::createBuiltInLevels() {
    // Levels are created procedurally based on ID
}

Level* LevelManager::createTutorialLevel() {
    Level* level = new Level();
    
    level->setId(0);
    level->setName("Tutorial");
    level->setDimensions(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
    level->setTimeLimit(120.0f);
    level->setTutorial(true);
    level->setMaxStrokes(10);

    // Single easy spawn
    SpawnPoint spawn;
    spawn.position = Vec2(DEFAULT_SCREEN_WIDTH / 2, 200);
    spawn.objectType = ObjectType::Ball;
    spawn.size = 1.0f;
    spawn.energy = 50.0f;
    spawn.color = Color::cyan();
    level->addSpawnPoint(spawn);

    // Large goal at bottom
    level->setGoalZone(Rect(DEFAULT_SCREEN_WIDTH / 4, DEFAULT_SCREEN_HEIGHT - 200, 
                            DEFAULT_SCREEN_WIDTH / 2, 150));

    level->setObjective(std::make_unique<ReachGoalObjective>(1));
    level->setStarThresholds(50, 100, 150);

    level->addTutorialStep("Swipe anywhere to create a gravity field!");
    level->addTutorialStep("The ball will be pulled in the direction you swipe.");
    level->addTutorialStep("Guide the ball to the green goal zone to complete the level!");

    return level;
}

Level* LevelManager::createLevel(int id, int difficulty) {
    Level* level = new Level();
    
    std::mt19937 rng(id * 12345);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    level->setId(id);
    level->setName("Level " + std::to_string(id));
    level->setDimensions(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
    level->setTimeLimit(std::max(30.0f, 90.0f - difficulty * 3.0f));
    level->setDifficulty(difficulty);
    level->setMaxStrokes(std::max(3, MAX_ACTIVE_STROKES - difficulty / 5));

    // Goal position varies
    float goalY = DEFAULT_SCREEN_HEIGHT - 100 - (id % 3) * 100;
    float goalX = DEFAULT_SCREEN_WIDTH / 4 + (id % 5) * DEFAULT_SCREEN_WIDTH / 10;
    level->setGoalZone(Rect(goalX, goalY, 180, 100));

    // Object count scales with level
    int objectCount = 1 + id / 5;
    objectCount = std::min(objectCount, 5);

    for (int i = 0; i < objectCount; ++i) {
        SpawnPoint spawn;
        spawn.position = Vec2(
            100 + dist01(rng) * (DEFAULT_SCREEN_WIDTH - 200),
            100 + dist01(rng) * 300
        );
        spawn.objectType = static_cast<ObjectType>(static_cast<int>(dist01(rng) * 5));
        spawn.delay = i * (1.0f + dist01(rng));
        spawn.size = 0.8f + dist01(rng) * 0.4f;
        spawn.energy = 40.0f + dist01(rng) * 30.0f;
        
        // Colorful objects
        spawn.color = Color::fromFloat(
            0.5f + dist01(rng) * 0.5f,
            0.5f + dist01(rng) * 0.5f,
            0.5f + dist01(rng) * 0.5f
        );
        
        level->addSpawnPoint(spawn);
    }

    // Add obstacles for harder levels
    int obstacleCount = std::max(0, (id - 5) / 3);
    for (int i = 0; i < obstacleCount; ++i) {
        ObstacleData obstacle;
        obstacle.position = Vec2(
            100 + dist01(rng) * (DEFAULT_SCREEN_WIDTH - 200),
            DEFAULT_SCREEN_HEIGHT / 3 + dist01(rng) * DEFAULT_SCREEN_HEIGHT / 3
        );
        obstacle.size = Vec2(80 + dist01(rng) * 100, 15 + dist01(rng) * 20);
        obstacle.rotation = dist01(rng) * 0.5f - 0.25f;
        obstacle.isCircle = dist01(rng) > 0.7f;
        obstacle.color = Color(80, 80, 100);
        level->addObstacle(obstacle);
    }

    // Set objective
    level->setObjective(std::make_unique<ReachGoalObjective>(objectCount));

    // Star thresholds scale with difficulty
    int baseScore = 100 + difficulty * 20;
    level->setStarThresholds(baseScore, baseScore * 2, baseScore * 3);

    return level;
}

void LevelManager::updateObjectiveProgress(PhysicsWorld* physics) {
    if (!m_currentLevel || !m_currentLevel->getObjective()) return;
    
    m_currentLevel->getObjective()->update(0, physics);
}

} // namespace Vectoria
