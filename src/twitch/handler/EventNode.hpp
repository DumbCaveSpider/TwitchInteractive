#pragma once
#include <Geode/Geode.hpp>
#include <vector>

using namespace geode::prelude;

class CCObject;
struct EventNodeInfo {
    std::string label;
    std::string id;
};

class EventNodeFactory {
public:
    // Returns a list of all event node definitions
    static std::vector<EventNodeInfo> getAllEventNodes();
};

class EventNode : public CCNode {
public:
    CCMenuItemToggler* m_checkbox = nullptr;
    CCLabelBMFont* m_label = nullptr;

    static EventNode* create(const std::string& labelText, CCObject* target, SEL_MenuHandler selector, float checkboxScale = 0.6f);
    bool init(const std::string& labelText, CCObject* target, SEL_MenuHandler selector, float checkboxScale);
};
