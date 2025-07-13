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
public:
    static HandbookPopup* create();
};
