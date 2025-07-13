#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Loader.hpp>
#include <cocos2d.h>
using namespace geode::prelude;
using namespace cocos2d;

class HandbookPopup : public Popup<> {
protected:
    bool setup() override;
    void onEventsBtn(cocos2d::CCObject* sender);
    void onActionBtn(cocos2d::CCObject* sender);
    void onIdentifiersBtn(cocos2d::CCObject* sender);
    void onDashboardBtn(cocos2d::CCObject* sender);
    void onCommandsBtn(cocos2d::CCObject* sender);
public:
    static HandbookPopup* create();
};
