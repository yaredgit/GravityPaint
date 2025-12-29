#include "GravityPaint/level/Objective.h"
#include "GravityPaint/physics/PhysicsWorld.h"
#include "GravityPaint/physics/PhysicsObject.h"
#include "GravityPaint/Constants.h"
#include <sstream>

namespace GravityPaint {

Objective::Objective(ObjectiveType type)
    : m_type(type)
{
}

// ReachGoalObjective
ReachGoalObjective::ReachGoalObjective(int requiredCount)
    : Objective(ObjectiveType::ReachGoal)
    , m_requiredCount(requiredCount)
{
}

void ReachGoalObjective::update(float /*deltaTime*/, PhysicsWorld* physics) {
    if (!physics) return;

    auto objectsInGoal = physics->getObjectsInGoal();
    m_currentCount = static_cast<int>(objectsInGoal.size());
}

bool ReachGoalObjective::isComplete() const {
    return m_currentCount >= m_requiredCount;
}

float ReachGoalObjective::getProgress() const {
    if (m_requiredCount <= 0) return 1.0f;
    return static_cast<float>(m_currentCount) / m_requiredCount;
}

std::string ReachGoalObjective::getDescription() const {
    std::stringstream ss;
    ss << "Guide " << m_currentCount << "/" << m_requiredCount << " objects to goal";
    return ss.str();
}

// CollectItemsObjective
CollectItemsObjective::CollectItemsObjective(int requiredCount)
    : Objective(ObjectiveType::CollectItems)
    , m_requiredCount(requiredCount)
{
}

void CollectItemsObjective::update(float /*deltaTime*/, PhysicsWorld* physics) {
    if (!physics) return;

    // Count collected objects
    int collected = 0;
    for (const auto& obj : physics->getObjects()) {
        if (obj->isCollected()) {
            collected++;
        }
    }
    m_currentCount = collected;
}

bool CollectItemsObjective::isComplete() const {
    return m_currentCount >= m_requiredCount;
}

float CollectItemsObjective::getProgress() const {
    if (m_requiredCount <= 0) return 1.0f;
    return static_cast<float>(m_currentCount) / m_requiredCount;
}

std::string CollectItemsObjective::getDescription() const {
    std::stringstream ss;
    ss << "Collect " << m_currentCount << "/" << m_requiredCount << " items";
    return ss.str();
}

// TimeChallengeObjective
TimeChallengeObjective::TimeChallengeObjective(float timeLimit, int requiredGoals)
    : Objective(ObjectiveType::TimeChallenge)
    , m_timeLimit(timeLimit)
    , m_timeRemaining(timeLimit)
    , m_requiredGoals(requiredGoals)
{
}

void TimeChallengeObjective::update(float deltaTime, PhysicsWorld* physics) {
    m_timeRemaining -= deltaTime;
    
    if (m_timeRemaining <= 0) {
        m_failed = true;
    }

    if (physics) {
        m_currentGoals = static_cast<int>(physics->getObjectsInGoal().size());
    }
}

bool TimeChallengeObjective::isComplete() const {
    return m_currentGoals >= m_requiredGoals && m_timeRemaining > 0;
}

float TimeChallengeObjective::getProgress() const {
    if (m_requiredGoals <= 0) return 1.0f;
    return static_cast<float>(m_currentGoals) / m_requiredGoals;
}

std::string TimeChallengeObjective::getDescription() const {
    std::stringstream ss;
    int seconds = static_cast<int>(m_timeRemaining);
    ss << "Time: " << seconds << "s - Goals: " << m_currentGoals << "/" << m_requiredGoals;
    return ss.str();
}

// ChainReactionObjective
ChainReactionObjective::ChainReactionObjective(int requiredChainLength)
    : Objective(ObjectiveType::ChainReaction)
    , m_requiredChainLength(requiredChainLength)
{
}

void ChainReactionObjective::update(float deltaTime, PhysicsWorld* /*physics*/) {
    if (m_chainTimeout > 0) {
        m_chainTimeout -= deltaTime;
        if (m_chainTimeout <= 0) {
            resetChain();
        }
    }
}

bool ChainReactionObjective::isComplete() const {
    return m_maxChainLength >= m_requiredChainLength;
}

float ChainReactionObjective::getProgress() const {
    if (m_requiredChainLength <= 0) return 1.0f;
    return static_cast<float>(m_maxChainLength) / m_requiredChainLength;
}

std::string ChainReactionObjective::getDescription() const {
    std::stringstream ss;
    ss << "Chain reaction: " << m_maxChainLength << "/" << m_requiredChainLength;
    if (m_currentChainLength > 0) {
        ss << " (current: " << m_currentChainLength << ")";
    }
    return ss.str();
}

void ChainReactionObjective::recordCollision() {
    m_currentChainLength++;
    m_chainTimeout = COMBO_TIMEOUT;
    
    if (m_currentChainLength > m_maxChainLength) {
        m_maxChainLength = m_currentChainLength;
    }
}

void ChainReactionObjective::resetChain() {
    m_currentChainLength = 0;
    m_chainTimeout = 0;
}

// MinimizeStrokesObjective
MinimizeStrokesObjective::MinimizeStrokesObjective(int maxStrokes, int requiredGoals)
    : Objective(ObjectiveType::MinimizeStrokes)
    , m_maxStrokes(maxStrokes)
    , m_requiredGoals(requiredGoals)
{
}

void MinimizeStrokesObjective::update(float /*deltaTime*/, PhysicsWorld* physics) {
    if (physics) {
        m_currentGoals = static_cast<int>(physics->getObjectsInGoal().size());
    }

    if (m_strokesUsed > m_maxStrokes && m_currentGoals < m_requiredGoals) {
        m_failed = true;
    }
}

bool MinimizeStrokesObjective::isComplete() const {
    return m_currentGoals >= m_requiredGoals;
}

float MinimizeStrokesObjective::getProgress() const {
    if (m_requiredGoals <= 0) return 1.0f;
    return static_cast<float>(m_currentGoals) / m_requiredGoals;
}

std::string MinimizeStrokesObjective::getDescription() const {
    std::stringstream ss;
    ss << "Strokes: " << m_strokesUsed << "/" << m_maxStrokes;
    ss << " - Goals: " << m_currentGoals << "/" << m_requiredGoals;
    return ss.str();
}

void MinimizeStrokesObjective::addStroke() {
    m_strokesUsed++;
}

// MaximizeEnergyObjective
MaximizeEnergyObjective::MaximizeEnergyObjective(float requiredEnergy)
    : Objective(ObjectiveType::MaximizeEnergy)
    , m_requiredEnergy(requiredEnergy)
{
}

void MaximizeEnergyObjective::update(float /*deltaTime*/, PhysicsWorld* physics) {
    if (!physics) return;

    // Sum energy of objects in goal
    m_totalEnergy = 0;
    for (const auto& obj : physics->getObjectsInGoal()) {
        m_totalEnergy += obj->getEnergy();
    }
}

bool MaximizeEnergyObjective::isComplete() const {
    return m_totalEnergy >= m_requiredEnergy;
}

float MaximizeEnergyObjective::getProgress() const {
    if (m_requiredEnergy <= 0) return 1.0f;
    return std::min(1.0f, m_totalEnergy / m_requiredEnergy);
}

std::string MaximizeEnergyObjective::getDescription() const {
    std::stringstream ss;
    ss << "Energy in goal: " << static_cast<int>(m_totalEnergy) << "/" << static_cast<int>(m_requiredEnergy);
    return ss.str();
}

} // namespace GravityPaint
