#pragma once

#include <functional>
#include <Geode/ui/Popup.hpp>


namespace cocos2d { class CCObject; }
class CCMenuItemToggler;


class JumpSettingsPopup : public geode::Popup<int> {
protected:
    int m_actionIndex = 1;
    std::function<void(int, bool)> m_onSelect;
    bool m_isHoldJump = false;
    CCMenuItemToggler* m_holdJumpCheckbox = nullptr;
    bool m_restoreHold = false;
    bool setup(int actionIndex) override;
    void onPlayer1(cocos2d::CCObject*);
    void onPlayer2(cocos2d::CCObject*);
    void onBoth(cocos2d::CCObject*);
    void onToggleHoldJump(cocos2d::CCObject*);
public:
    static JumpSettingsPopup* create(int actionIndex, bool restoreHold, std::function<void(int, bool)> onSelect);
};
