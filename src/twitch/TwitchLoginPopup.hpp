#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class TwitchLoginPopup : public Popup<> {
protected:
    CCMenu* m_loginMenu = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    CCMenu* m_loggedInMenu = nullptr;

    void onLoginPressed(CCObject* sender);
    void onRefreshPressed(CCObject* sender);
    void updateStatusCallback();
public:
    static TwitchLoginPopup* create();
};
