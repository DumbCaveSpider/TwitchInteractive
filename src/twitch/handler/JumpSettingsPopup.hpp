#pragma once

#include <functional>
#include <Geode/ui/Popup.hpp>

namespace cocos2d { class CCObject; }

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
