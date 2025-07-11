#include "EventNode.hpp"
#include <cocos2d.h>
#include <Geode/Geode.hpp>
#include <vector>

std::vector<EventNodeInfo> EventNodeFactory::getAllEventNodes() {
    // Add new events here. For each event, provide id and label
    std::vector<EventNodeInfo> nodes = {
        {"kill_player", "Kill Player"},
        {"jump", "Jump"},
        {"wait", "Wait"},
        {"notification", "Notification"},
    };
    return nodes;
}


EventNode* EventNode::create(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale) {
    auto ret = new EventNode();
    if (ret && ret->init(labelText, target, selector, checkboxScale)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}


bool EventNode::init(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale) {
    if (!cocos2d::CCNode::init()) return false;
    this->setContentSize(cocos2d::CCSize(370.f, 32.f));
    // Checkbox
    m_checkbox = CCMenuItemToggler::createWithStandardSprites(target, selector, checkboxScale);
    m_checkbox->setPosition(20.f, 16.f);
    // Label
    m_label = cocos2d::CCLabelBMFont::create(labelText.c_str(), "bigFont.fnt");
    m_label->setScale(0.5f);
    m_label->setAnchorPoint({0, 0.5f});
    m_label->setAlignment(cocos2d::kCCTextAlignmentLeft);
    m_label->setPosition(36.f, 16.f);
    // Menu for checkbox
    auto eventMenu = cocos2d::CCMenu::create();
    eventMenu->addChild(m_checkbox);
    eventMenu->setPosition(0, 0);
    this->addChild(eventMenu);
    this->addChild(m_label);
    return true;
}
