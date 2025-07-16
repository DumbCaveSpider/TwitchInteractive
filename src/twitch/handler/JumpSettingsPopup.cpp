#include "JumpSettingsPopup.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace cocos2d;

bool JumpSettingsPopup::setup(int actionIndex) {
    m_actionIndex = actionIndex;

    setTitle("Edit Jump Action");
    setID("jump-settings-popup");

    float y = 190.f;
    float x = m_mainLayer->getContentSize().width / 2;
    float spacing = 50.f;

    m_noElasticity = true;

    // Player 1 button
    auto p1Btn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Player 1", "bigFont.fnt", "GJ_button_01.png", 0.7f),
        this, menu_selector(JumpSettingsPopup::onPlayer1)
    );
    p1Btn->setPosition(x, y);

    // Player 2 button
    auto p2Btn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Player 2", "bigFont.fnt", "GJ_button_01.png", 0.7f),
        this, menu_selector(JumpSettingsPopup::onPlayer2)
    );
    p2Btn->setPosition(x, y - spacing);

    // Both Players button
    auto bothBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Both Players", "bigFont.fnt", "GJ_button_01.png", 0.7f),
        this, menu_selector(JumpSettingsPopup::onBoth)
    );
    bothBtn->setPosition(x, y - 2 * spacing);

    float checkboxY = y - 3 * spacing;
    float checkboxX = x;

    m_holdJumpCheckbox = CCMenuItemToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        this,
        menu_selector(JumpSettingsPopup::onToggleHoldJump)
    );
    m_holdJumpCheckbox->setPosition(checkboxX, checkboxY);

    auto checkboxLabel = CCLabelBMFont::create("Hold Jump", "bigFont.fnt");
    checkboxLabel->setScale(0.4f);
    checkboxLabel->setAnchorPoint({ 0.5f, 1.0f });
    checkboxLabel->setPosition(checkboxX, checkboxY - 18.f);

    m_isHoldJump = false;

    if (m_restoreHold && m_holdJumpCheckbox) {
        m_holdJumpCheckbox->toggle(true);
        m_isHoldJump = true;
    };

    // Button Menu
    auto menu = CCMenu::create();
    menu->setPosition(0, 0);

    menu->addChild(p1Btn);
    menu->addChild(p2Btn);
    menu->addChild(bothBtn);

    menu->addChild(m_holdJumpCheckbox);
    menu->addChild(checkboxLabel);

    m_mainLayer->addChild(menu);

    return true;
};

/*
2 player jump settings
*/

void JumpSettingsPopup::onPlayer1(CCObject*) {
    if (m_onSelect)
        m_onSelect(1, m_holdJumpCheckbox && m_holdJumpCheckbox->isOn());
    onClose(nullptr);
};

void JumpSettingsPopup::onPlayer2(CCObject*) {
    if (m_onSelect)
        m_onSelect(2, m_holdJumpCheckbox && m_holdJumpCheckbox->isOn());
    onClose(nullptr);
};

void JumpSettingsPopup::onBoth(CCObject*) {
    if (m_onSelect)
        m_onSelect(3, m_holdJumpCheckbox && m_holdJumpCheckbox->isOn());
    onClose(nullptr);
};

void JumpSettingsPopup::onToggleHoldJump(CCObject*) {
    m_isHoldJump = m_holdJumpCheckbox && m_holdJumpCheckbox->isOn();
};

/*
create the popup
*/

JumpSettingsPopup* JumpSettingsPopup::create(int actionIndex, bool restoreHold, std::function<void(int, bool)> onSelect) {
    auto ret = new JumpSettingsPopup();
    ret->m_onSelect = onSelect;
    ret->m_restoreHold = restoreHold;

    if (ret && ret->initAnchored(220.f, 250.f, actionIndex)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};