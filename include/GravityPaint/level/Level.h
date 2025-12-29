#pragma once

#include "GravityPaint/Types.h"
#include "GravityPaint/Constants.h"
#include "GravityPaint/level/Objective.h"
#include <vector>
#include <string>
#include <memory>

namespace GravityPaint {

struct SpawnPoint {
    Vec2 position;
    ObjectType objectType;
    float delay = 0.0f;
    float size = 1.0f;
    float energy = 50.0f;
    Color color = Color::white();
};

struct ObstacleData {
    Vec2 position;
    Vec2 size;
    float rotation = 0.0f;
    bool isCircle = false;
    bool isDeformable = false;
    bool isMoving = false;
    Vec2 moveDirection;
    float moveSpeed = 0.0f;
    float moveDistance = 0.0f;
    Color color = Color(100, 100, 120);
};

struct GravityZoneData {
    Vec2 position;
    Vec2 size;
    ZoneType type;
    Vec2 direction;
    float strength = 1.0f;
    Color color;
};

class Level {
public:
    Level();
    ~Level() = default;

    bool load(const std::string& filepath);
    bool loadFromString(const std::string& data);
    void reset();

    // Level info
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    // Dimensions
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    void setDimensions(float width, float height);

    // Time
    float getTimeLimit() const { return m_timeLimit; }
    void setTimeLimit(float limit) { m_timeLimit = limit; }

    // Spawn points
    const std::vector<SpawnPoint>& getSpawnPoints() const { return m_spawnPoints; }
    void addSpawnPoint(const SpawnPoint& spawn);
    void clearSpawnPoints() { m_spawnPoints.clear(); }

    // Goal zone
    Rect getGoalZone() const { return m_goalZone; }
    void setGoalZone(const Rect& zone) { m_goalZone = zone; }

    // Obstacles
    const std::vector<ObstacleData>& getObstacles() const { return m_obstacles; }
    void addObstacle(const ObstacleData& obstacle);
    void clearObstacles() { m_obstacles.clear(); }

    // Gravity zones
    const std::vector<GravityZoneData>& getGravityZones() const { return m_gravityZones; }
    void addGravityZone(const GravityZoneData& zone);
    void clearGravityZones() { m_gravityZones.clear(); }

    // Objectives
    Objective* getObjective() const { return m_objective.get(); }
    void setObjective(std::unique_ptr<Objective> objective);

    // Star thresholds
    int getStarThreshold(int star) const;
    void setStarThresholds(int one, int two, int three);
    int calculateStars(int score) const;

    // Difficulty
    int getDifficulty() const { return m_difficulty; }
    void setDifficulty(int difficulty) { m_difficulty = difficulty; }

    // Tutorial
    bool isTutorial() const { return m_isTutorial; }
    void setTutorial(bool tutorial) { m_isTutorial = tutorial; }
    const std::vector<std::string>& getTutorialSteps() const { return m_tutorialSteps; }
    void addTutorialStep(const std::string& step) { m_tutorialSteps.push_back(step); }

    // Background
    Color getBackgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const Color& color) { m_backgroundColor = color; }

    // Max strokes allowed
    int getMaxStrokes() const { return m_maxStrokes; }
    void setMaxStrokes(int max) { m_maxStrokes = max; }

private:
    bool parseJSON(const std::string& json);

    int m_id = 0;
    std::string m_name;
    float m_width = DEFAULT_SCREEN_WIDTH;
    float m_height = DEFAULT_SCREEN_HEIGHT;
    float m_timeLimit = LEVEL_TIME_LIMIT;

    std::vector<SpawnPoint> m_spawnPoints;
    Rect m_goalZone = Rect(0, 0, 0, 0);
    std::vector<ObstacleData> m_obstacles;
    std::vector<GravityZoneData> m_gravityZones;

    std::unique_ptr<Objective> m_objective;
    int m_starThresholds[3] = {100, 200, 300};

    int m_difficulty = 1;
    bool m_isTutorial = false;
    std::vector<std::string> m_tutorialSteps;

    Color m_backgroundColor = Color(15, 15, 30);
    int m_maxStrokes = MAX_ACTIVE_STROKES;
};

} // namespace GravityPaint
