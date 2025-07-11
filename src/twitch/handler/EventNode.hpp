#pragma once
#include <Geode/Geode.hpp>
#include <vector>

using namespace geode::prelude;

namespace cocos2d { class CCObject; class CCNode; class CCLabelBMFont; }

class EventNode : public cocos2d::CCNode {
public:
    static EventNode* create(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale = 0.6f);
    bool init(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale);
private:
    cocos2d::CCLabelBMFont* m_label = nullptr;
    CCMenuItemToggler* m_checkbox = nullptr;
};
struct EventNodeInfo {
    std::string id;
    std::string label;
};

class EventNodeFactory {
public:
    // Returns a list of all event node definitions
    static std::vector<EventNodeInfo> getAllEventNodes();
    static EventNode* create(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale = 0.6f);
};
