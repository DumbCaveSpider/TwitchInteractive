#pragma once
#include <Geode/Geode.hpp>
#include <functional>

using namespace geode::prelude;

class MoveSettingsPopup : public geode::Popup<> {
protected:
    int m_player = 1;
    bool m_moveRight = true;
    std::function<void(int, bool)> m_callback;

    bool setup() override;
    void onPlayerLeft(CCObject* sender);
    void onPlayerRight(CCObject* sender);
    void onDirectionLeft(CCObject* sender);
    void onDirectionRight(CCObject* sender);
    void onSave(CCObject* sender);

public:
    static MoveSettingsPopup* create(int player, bool moveRight, std::function<void(int, bool)> callback);
};
