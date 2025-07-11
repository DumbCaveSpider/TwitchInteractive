#include "ActionNode.hpp"

#include <cocos2d.h>
#include <Geode/Geode.hpp>

ActionNode* ActionNode::create(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale,
                               cocos2d::CCObject* moveTarget, SEL_MenuHandler moveUpSelector, SEL_MenuHandler moveDownSelector, int actionIndex, bool canMoveUp, bool canMoveDown) {
    auto ret = new ActionNode();
    if (ret && ret->init(labelText, target, selector, checkboxScale, moveTarget, moveUpSelector, moveDownSelector, actionIndex, canMoveUp, canMoveDown)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ActionNode::init(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale,
                      cocos2d::CCObject* moveTarget, SEL_MenuHandler moveUpSelector, SEL_MenuHandler moveDownSelector, int actionIndex, bool canMoveUp, bool canMoveDown) {
    if (!cocos2d::CCNode::init()) return false;

    setContentSize(cocos2d::CCSize(370.f, 32.f));

    // Up/Down arrows
    m_upBtn = nullptr;
    m_downBtn = nullptr;

    if (moveTarget && (moveUpSelector || moveDownSelector)) {
        // Up arrow
        auto upSprite = cocos2d::CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        upSprite->setScale(0.35f);
        upSprite->setRotation(0.f);

        m_upBtn = ::CCMenuItemSpriteExtra::create(upSprite, moveTarget, moveUpSelector);
        m_upBtn->setPosition(38.f, 23.f);
        m_upBtn->setRotation(90.f);
        m_upBtn->setUserObject(cocos2d::CCInteger::create(actionIndex));
        m_upBtn->setEnabled(canMoveUp);
        m_upBtn->setVisible(canMoveUp);

        // Down arrow
        auto downSprite = cocos2d::CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        downSprite->setScale(0.35f);
        downSprite->setRotation(180.f);

        m_downBtn = ::CCMenuItemSpriteExtra::create(downSprite, moveTarget, moveDownSelector);
        m_downBtn->setPosition(38.f, 9.f);
        m_downBtn->setRotation(90.f);
        m_downBtn->setUserObject(cocos2d::CCInteger::create(actionIndex));
        m_downBtn->setEnabled(canMoveDown);
        m_downBtn->setVisible(canMoveDown);

        // Menu for arrows
        auto arrowMenu = cocos2d::CCMenu::create();
        arrowMenu->addChild(m_upBtn);
        arrowMenu->addChild(m_downBtn);
        arrowMenu->setPosition(0, 0);

        addChild(arrowMenu);
    };

    // Label
    m_label = cocos2d::CCLabelBMFont::create(labelText.c_str(), "bigFont.fnt");
    m_label->setScale(0.5f);
    m_label->setAnchorPoint({ 0, 0.5f });
    m_label->setAlignment(cocos2d::kCCTextAlignmentLeft);
    m_label->setPosition(60.f, 16.f); // shift right for arrows

    addChild(m_label);

    return true;
}

void ActionNode::setMoveEnabled(bool canMoveUp, bool canMoveDown) {
    if (m_upBtn) m_upBtn->setEnabled(canMoveUp);
    if (m_downBtn) m_downBtn->setEnabled(canMoveDown);
};