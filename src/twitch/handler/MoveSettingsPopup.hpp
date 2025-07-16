#pragma once
#include <Geode/Geode.hpp>
#include <functional>

using namespace geode::prelude;

class MoveSettingsPopup : public geode::Popup<>
{

protected:
    int m_player = 1;
    bool m_moveRight = true;
    float m_distance = 0.f;
    std::function<void(int, bool, float)> m_callback;

    static CCMenuItemSpriteExtra *playerLeftBtn;
    static CCMenuItemSpriteExtra *playerRightBtn;
    static CCMenuItemSpriteExtra *dirLeftBtn;
    static CCMenuItemSpriteExtra *dirRightBtn;
    static geode::TextInput *distanceInput;

    bool setup() override;
    void onPlayerLeft(CCObject *sender);
    void onPlayerRight(CCObject *sender);
    void onDirectionLeft(CCObject *sender);
    void onDirectionRight(CCObject *sender);
    void onSave(CCObject *sender);

public:
    static MoveSettingsPopup *create(int player, bool moveRight, std::function<void(int, bool, float)> callback);

    // Public getter/setter for distance
    void setDistance(float distance) { m_distance = distance; }
    float getDistance() const { return m_distance; }

    // Public getter for distanceInput
    geode::TextInput *getDistanceInput() const { return distanceInput; }
};
