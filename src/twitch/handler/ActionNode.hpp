#pragma once

#include <cocos2d.h>
#include <Geode/Geode.hpp>

namespace cocos2d { class CCLabelBMFont; class CCSprite; class CCInteger; }
class CCMenuItemToggler;
typedef void (cocos2d::CCObject::* SEL_MenuHandler)(cocos2d::CCObject*);

class ActionNode : public cocos2d::CCNode {
protected:
    CCMenuItemToggler* m_checkbox = nullptr;
    cocos2d::CCLabelBMFont* m_label = nullptr;
    ::CCMenuItemSpriteExtra* m_upBtn = nullptr;
    ::CCMenuItemSpriteExtra* m_downBtn = nullptr;

    bool init(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale,
              cocos2d::CCObject* moveTarget = nullptr, SEL_MenuHandler moveUpSelector = nullptr, SEL_MenuHandler moveDownSelector = nullptr, int actionIndex = 0, bool canMoveUp = false, bool canMoveDown = false);
public:
    static ActionNode* create(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale = 0.7f,
                              cocos2d::CCObject* moveTarget = nullptr, SEL_MenuHandler moveUpSelector = nullptr, SEL_MenuHandler moveDownSelector = nullptr, int actionIndex = 0, bool canMoveUp = false, bool canMoveDown = false);
    CCMenuItemToggler* getCheckbox() const { return m_checkbox; }
    cocos2d::CCLabelBMFont* getLabel() const { return m_label; }
    void setMoveEnabled(bool canMoveUp, bool canMoveDown);
};
