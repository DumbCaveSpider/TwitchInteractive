#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class TwitchDashboard : public Popup<> {
protected:
    bool setup() override;
    void onClose(CCObject* sender) override;
    
public:
    static TwitchDashboard* create();
};
