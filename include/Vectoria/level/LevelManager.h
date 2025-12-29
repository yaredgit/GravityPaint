#pragma once

#include "Vectoria/Types.h"
#include <vector>
#include <memory>
#include <string>

namespace Vectoria {

class Level;
class PhysicsWorld;
class ParticleSystem;

struct LevelProgress {
    int levelId = 0;
    bool completed = false;
    int bestScore = 0;
    int stars = 0;
    float bestTime = 0.0f;
    int attempts = 0;
};

class LevelManager {
public:
    LevelManager();
    ~LevelManager();

    bool initialize();
    void shutdown();

    // Level loading
    bool loadLevel(int levelId);
    bool loadLevelFromFile(const std::string& filepath);
    void unloadCurrentLevel();
    bool reloadCurrentLevel();

    // Level state
    void startLevel();
    void updateLevel(float deltaTime);
    void completeLevel(int score, float time);
    void failLevel();
    void resetLevel();

    // Current level
    Level* getCurrentLevel() const { return m_currentLevel.get(); }
    int getCurrentLevelId() const { return m_currentLevelId; }
    bool isLevelActive() const { return m_levelActive; }
    float getLevelTime() const { return m_levelTime; }

    // Level navigation
    bool hasNextLevel() const;
    bool hasPreviousLevel() const;
    void nextLevel();
    void previousLevel();
    void goToLevel(int levelId);

    // Level count
    int getTotalLevelCount() const { return m_totalLevels; }
    int getUnlockedLevelCount() const;

    // Progress
    const LevelProgress& getLevelProgress(int levelId) const;
    void setLevelProgress(int levelId, const LevelProgress& progress);
    int getTotalStars() const;
    bool isLevelUnlocked(int levelId) const;

    // Spawn management
    void spawnObjects(PhysicsWorld* physics);
    void updateSpawns(float deltaTime, PhysicsWorld* physics);
    bool allObjectsSpawned() const { return m_allSpawned; }

    // Level generation (for endless mode)
    std::unique_ptr<Level> generateProceduralLevel(int difficulty);

    // Save/Load progress
    bool saveProgress(const std::string& filepath);
    bool loadProgress(const std::string& filepath);

private:
    void createBuiltInLevels();
    Level* createTutorialLevel();
    Level* createLevel(int id, int difficulty);
    void updateObjectiveProgress(PhysicsWorld* physics);

    std::unique_ptr<Level> m_currentLevel;
    std::vector<LevelProgress> m_progress;

    int m_currentLevelId = 0;
    int m_totalLevels = 50;
    bool m_levelActive = false;
    float m_levelTime = 0.0f;

    // Spawn state
    std::vector<bool> m_spawnedObjects;
    float m_spawnTimer = 0.0f;
    bool m_allSpawned = false;

    static const LevelProgress s_emptyProgress;
};

} // namespace Vectoria
