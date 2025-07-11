#include "JumpSettingsPopup.hpp"

#include <fmt/core.h>
#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace cocos2d;

bool JumpSettingsPopup::setup(int actionIndex) {
    m_actionIndex = actionIndex;

    setTitle(fmt::format("Jump Action #{}", m_actionIndex));
    setID("jump-settings-popup");

    float y = 140.f;
    float x = m_mainLayer->getContentSize().width / 2;
    float spacing = 50.f;

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

    // Menu
    auto menu = CCMenu::create();
    menu->addChild(p1Btn);
    menu->addChild(p2Btn);
    menu->addChild(bothBtn);
    menu->setPosition(0, 0);

    m_mainLayer->addChild(menu);

    return true;
}

void JumpSettingsPopup::onPlayer1(CCObject*) { if (m_onSelect) m_onSelect(1); onClose(nullptr); }
void JumpSettingsPopup::onPlayer2(CCObject*) { if (m_onSelect) m_onSelect(2); onClose(nullptr); }
void JumpSettingsPopup::onBoth(CCObject*) { if (m_onSelect) m_onSelect(3); onClose(nullptr); }

JumpSettingsPopup* JumpSettingsPopup::create(int actionIndex, std::function<void(int)> onSelect) {
    auto ret = new JumpSettingsPopup();
    ret->m_onSelect = onSelect;

    if (ret && ret->initAnchored(220.f, 200.f, actionIndex)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};