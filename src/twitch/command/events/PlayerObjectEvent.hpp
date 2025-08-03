#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include "../../handler/GravitySettingsPopup.hpp"

using namespace geode::prelude;

class PlayerObjectEvent : public CCNode {
public:
    static PlayerObjectEvent* create(PlayerObject* player, float gravity, float duration);
    bool init(PlayerObject* player, float gravity, float duration);
    void applyGravity();
    void resetGravityCallback(float);
private:
    PlayerObject* m_player;
    float m_gravity;
    float m_duration;
    float m_resetGravity;
};
