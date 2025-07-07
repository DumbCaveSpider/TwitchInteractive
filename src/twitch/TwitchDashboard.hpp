#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class TwitchDashboard : public Popup<> {
protected:
    CCMenu* m_buttonMenu = nullptr;

    CCLabelBMFont* m_welcomeLabel = nullptr;
    CCLabelBMFont* m_tokenLabel = nullptr;

    CCScrollView* m_tokenScrollView = nullptr;

    bool setup() override;
    void onClose(CCObject* sender) override;
    void onGetTokenPressed(CCObject* sender);
public:
    static TwitchDashboard* create();
};