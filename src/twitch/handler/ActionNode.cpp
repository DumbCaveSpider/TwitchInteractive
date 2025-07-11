#include "ActionNode.hpp"
#include <cocos2d.h>
#include <Geode/Geode.hpp>


ActionNode* ActionNode::create(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale) {
    auto ret = new ActionNode();
    if (ret && ret->init(labelText, target, selector, checkboxScale)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}


bool ActionNode::init(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale) {
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
    auto actionMenu = cocos2d::CCMenu::create();
    actionMenu->addChild(m_checkbox);
    actionMenu->setPosition(0, 0);
    this->addChild(actionMenu);
    this->addChild(m_label);
    return true;
}
