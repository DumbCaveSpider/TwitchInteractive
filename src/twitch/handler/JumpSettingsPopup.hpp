#pragma once
#include <Geode/ui/Popup.hpp>
#include <functional>
#include <cocos2d.h>

class JumpSettingsPopup : public geode::Popup<int> {
protected:
    int m_actionIndex = 1;
    std::function<void(int)> m_onSelect;
    bool setup(int actionIndex) override;
    void onPlayer1(cocos2d::CCObject*);
    void onPlayer2(cocos2d::CCObject*);
    void onBoth(cocos2d::CCObject*);
public:
    static JumpSettingsPopup* create(int actionIndex, std::function<void(int)> onSelect);
};
