#include "PlayerObjectEvent.hpp"
#include <Geode/utils/general.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/Geode.hpp>

using namespace geode::prelude;

PlayerObjectEvent *PlayerObjectEvent::create(PlayerObject *player, float gravity, float duration)
{
    auto ret = new PlayerObjectEvent();
    if (ret && ret->init(player, gravity, duration))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool PlayerObjectEvent::init(PlayerObject *player, float gravity, float duration)
{
    m_player = player;
    m_gravity = gravity;
    m_duration = duration;
    return true;
}

void PlayerObjectEvent::applyGravity()
{
    if (!m_player)
    {
        log::warn("[PlayerObjectEvent] m_player is nullptr, cannot apply gravity.");
        return;
    }
    float originalGravity = m_player->m_gravity;
    log::info("[PlayerObjectEvent] Applying gravity {:.2f} to player (was {:.2f})", m_gravity, originalGravity);
    m_player->m_gravity = m_gravity;
    m_resetGravity = originalGravity;
    this->schedule(schedule_selector(PlayerObjectEvent::resetGravityCallback), m_duration, 0, 0);
}

void PlayerObjectEvent::resetGravityCallback(float)
{
    if (m_player)
    {
        log::info("[PlayerObjectEvent] Resetting gravity to {:.2f}", m_resetGravity);
        m_player->m_gravity = m_resetGravity;
    }
    else
    {
        log::warn("[PlayerObjectEvent] m_player is nullptr, cannot reset gravity.");
    }
    this->removeFromParent();
}
