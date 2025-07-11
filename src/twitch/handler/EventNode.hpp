#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class EventNode : public CCNode {
public:
    CCMenuItemToggler* m_checkbox = nullptr;
    CCLabelBMFont* m_label = nullptr;

    static EventNode* create(const std::string& labelText, CCObject* target, SEL_MenuHandler selector, float checkboxScale = 0.6f);
    bool init(const std::string& labelText, CCObject* target, SEL_MenuHandler selector, float checkboxScale);
};
