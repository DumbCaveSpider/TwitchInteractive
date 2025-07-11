#pragma once
#include <cocos2d.h>
#include <Geode/Geode.hpp>

using namespace cocos2d;
using namespace geode::prelude;

class ActionNode : public CCNode {
protected:
    CCMenuItemToggler* m_checkbox = nullptr;
    CCLabelBMFont* m_label = nullptr;

    bool init(const std::string& labelText, CCObject* target, SEL_MenuHandler selector, float checkboxScale);
public:
    static ActionNode* create(const std::string& labelText, CCObject* target, SEL_MenuHandler selector, float checkboxScale = 0.7f);
    CCMenuItemToggler* getCheckbox() const { return m_checkbox; }
    CCLabelBMFont* getLabel() const { return m_label; }
};
