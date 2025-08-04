#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include "../../handler/GravitySettingsPopup.hpp"

using namespace geode::prelude;


class PlayerObjectEvent : public CCNode {
public:
    static PlayerObjectEvent* create(PlayerObject* player, float gravity, float duration, float speed = -1.0f, float speedDuration = 0.0f);
    bool init(PlayerObject* player, float gravity, float duration, float speed = -1.0f, float speedDuration = 0.0f);
    void applyGravity();
    void resetGravityCallback(float);
    void applySpeed();
    void resetSpeedCallback(float);
private:
    PlayerObject* m_player;
    float m_gravity;
    float m_duration;
    float m_resetGravity;
    float m_speed; // Used to set m_player->m_playerSpeed
    float m_speedDuration;
    float m_resetSpeed;
};
