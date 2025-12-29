#pragma once

#include "GravityPaint/Types.h"
#include <string>
#include <functional>

namespace GravityPaint {

class PhysicsWorld;

class Objective {
public:
    Objective(ObjectiveType type);
    virtual ~Objective() = default;

    virtual void update(float deltaTime, PhysicsWorld* physics) = 0;
    virtual bool isComplete() const = 0;
    virtual float getProgress() const = 0;
    virtual std::string getDescription() const = 0;

    ObjectiveType getType() const { return m_type; }
    bool isFailed() const { return m_failed; }

protected:
    ObjectiveType m_type;
    bool m_failed = false;
};

// Guide objects to the goal zone
class ReachGoalObjective : public Objective {
public:
    ReachGoalObjective(int requiredCount);
    
    void update(float deltaTime, PhysicsWorld* physics) override;
    bool isComplete() const override;
    float getProgress() const override;
    std::string getDescription() const override;

    void setRequiredCount(int count) { m_requiredCount = count; }
    int getRequiredCount() const { return m_requiredCount; }
    int getCurrentCount() const { return m_currentCount; }

private:
    int m_requiredCount;
    int m_currentCount = 0;
};

// Collect specific items
class CollectItemsObjective : public Objective {
public:
    CollectItemsObjective(int requiredCount);
    
    void update(float deltaTime, PhysicsWorld* physics) override;
    bool isComplete() const override;
    float getProgress() const override;
    std::string getDescription() const override;

    void addCollectedItem() { m_currentCount++; }

private:
    int m_requiredCount;
    int m_currentCount = 0;
};

// Complete within time limit
class TimeChallengeObjective : public Objective {
public:
    TimeChallengeObjective(float timeLimit, int requiredGoals);
    
    void update(float deltaTime, PhysicsWorld* physics) override;
    bool isComplete() const override;
    float getProgress() const override;
    std::string getDescription() const override;

    float getRemainingTime() const { return m_timeRemaining; }

private:
    float m_timeLimit;
    float m_timeRemaining;
    int m_requiredGoals;
    int m_currentGoals = 0;
};

// Create chain reactions
class ChainReactionObjective : public Objective {
public:
    ChainReactionObjective(int requiredChainLength);
    
    void update(float deltaTime, PhysicsWorld* physics) override;
    bool isComplete() const override;
    float getProgress() const override;
    std::string getDescription() const override;

    void recordCollision();
    void resetChain();

private:
    int m_requiredChainLength;
    int m_currentChainLength = 0;
    int m_maxChainLength = 0;
    float m_chainTimeout = 0.0f;
};

// Minimize gravity strokes used
class MinimizeStrokesObjective : public Objective {
public:
    MinimizeStrokesObjective(int maxStrokes, int requiredGoals);
    
    void update(float deltaTime, PhysicsWorld* physics) override;
    bool isComplete() const override;
    float getProgress() const override;
    std::string getDescription() const override;

    void addStroke();
    int getStrokesUsed() const { return m_strokesUsed; }
    int getMaxStrokes() const { return m_maxStrokes; }

private:
    int m_maxStrokes;
    int m_requiredGoals;
    int m_strokesUsed = 0;
    int m_currentGoals = 0;
};

// Maximize energy at goal
class MaximizeEnergyObjective : public Objective {
public:
    MaximizeEnergyObjective(float requiredEnergy);
    
    void update(float deltaTime, PhysicsWorld* physics) override;
    bool isComplete() const override;
    float getProgress() const override;
    std::string getDescription() const override;

    void addEnergy(float energy) { m_totalEnergy += energy; }

private:
    float m_requiredEnergy;
    float m_totalEnergy = 0.0f;
};

} // namespace GravityPaint
