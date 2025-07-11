#pragma once

#include <vector>
#include <string>
#include <Geode/Geode.hpp>

namespace cocos2d { class CCObject; class CCNode; class CCLabelBMFont; }

namespace cocos2d { class CCLabelBMFont; }
class CCMenuItemToggler;
typedef void (cocos2d::CCObject::* SEL_MenuHandler)(cocos2d::CCObject*);

struct EventNodeInfo {
    std::string id;
    std::string label;
};

class EventNode : public cocos2d::CCNode {
public:
    static EventNode* create(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale = 0.6f);
    bool init(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale);
private:
    cocos2d::CCLabelBMFont* m_label = nullptr;
    CCMenuItemToggler* m_checkbox = nullptr;
};

class EventNodeFactory {
public:
    static std::vector<EventNodeInfo> getAllEventNodes();
    static EventNode* create(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale = 0.6f);
};
