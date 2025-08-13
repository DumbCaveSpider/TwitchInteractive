#include <Geode/Geode.hpp>
#include "PlayerEffectSettingsPopup.hpp"

using namespace geode::prelude;
using namespace cocos2d;

static const char *toText(PlayerEffectType t)
{
    switch (t)
    {
    case PlayerEffectType::Death:
        return "Death Effect";
    case PlayerEffectType::Spawn:
        return "Spawn Effect";
    }
    return "Death Effect";
}

bool PlayerEffectSettingsPopup::setup()
{
    setTitle("Player Effect Settings");
    setID("player-effect-settings-popup");

    auto popupSize = getContentSize();

    m_mainLayer = CCLayer::create();
    m_mainLayer->setContentSize(popupSize);
    m_mainLayer->setAnchorPoint({0, 0});
    m_mainLayer->setPosition(0, 0);
    this->m_noElasticity = true;
    addChild(m_mainLayer);

    auto layout = ColumnLayout::create()->setGap(10.f)->setAxisAlignment(AxisAlignment::Center)->setCrossAxisAlignment(AxisAlignment::Center);
    m_mainLayer->setLayout(layout);

    float centerX = m_mainLayer->getContentSize().width / 2.0f;
    float centerY = m_mainLayer->getContentSize().height / 2.0f;

    m_valueLabel = CCLabelBMFont::create(toText(getSelected()), "bigFont.fnt");
    m_valueLabel->setScale(0.6f);
    m_valueLabel->setAnchorPoint({0.5f, 0.5f});
    m_valueLabel->setAlignment(kCCTextAlignmentCenter);

    // Left/right arrows aligned horizontally with the text center
    auto leftSpr = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    auto rightSpr = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");

    auto menu = CCMenu::create();
    menu->setPosition(centerX, centerY + 20.f); // align to text baseline

    m_leftBtn = CCMenuItemSpriteExtra::create(leftSpr, this, menu_selector(PlayerEffectSettingsPopup::onLeft));
    m_rightBtn = CCMenuItemSpriteExtra::create(rightSpr, this, menu_selector(PlayerEffectSettingsPopup::onRight));

    // initial positions; will be refined by updateArrowPositions
    m_leftBtn->setPosition(-90.f, 0);
    m_rightBtn->setPosition(90.f, 0);

    // Player selection buttons below the selector
    auto p1Sprite = ButtonSprite::create("Player 1", "bigFont.fnt", "GJ_button_04.png", 0.5f);
    auto p2Sprite = ButtonSprite::create("Player 2", "bigFont.fnt", "GJ_button_04.png", 0.5f);
    m_player1Btn = CCMenuItemSpriteExtra::create(p1Sprite, this, menu_selector(PlayerEffectSettingsPopup::onChoosePlayer1));
    m_player2Btn = CCMenuItemSpriteExtra::create(p2Sprite, this, menu_selector(PlayerEffectSettingsPopup::onChoosePlayer2));
    m_player1Btn->setPosition(-60.f, -40.f);
    m_player2Btn->setPosition(60.f, -40.f);

    auto saveBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f), this, menu_selector(PlayerEffectSettingsPopup::onSaveBtn));
    saveBtn->setPosition(0, -80.f);

    menu->addChild(m_valueLabel);
    menu->addChild(m_leftBtn);
    menu->addChild(m_rightBtn);
    menu->addChild(m_player1Btn);
    menu->addChild(m_player2Btn);
    menu->addChild(saveBtn);

    m_mainLayer->addChild(menu);

    updateArrowPositions();
    updatePlayerButtons();

    return true;
}

void PlayerEffectSettingsPopup::updateValueLabel()
{
    if (m_valueLabel)
        m_valueLabel->setString(toText(getSelected()));
    updateArrowPositions();
}

void PlayerEffectSettingsPopup::onLeft(CCObject *)
{
    // Cycle backward through list
    if (!m_types.empty())
    {
        m_selectedIdx = (m_selectedIdx - 1 + (int)m_types.size()) % (int)m_types.size();
    }
    updateValueLabel();
}

void PlayerEffectSettingsPopup::onRight(CCObject *)
{
    // Cycle forward through list
    if (!m_types.empty())
    {
        m_selectedIdx = (m_selectedIdx + 1) % (int)m_types.size();
    }
    updateValueLabel();
}

void PlayerEffectSettingsPopup::onSaveBtn(CCObject *)
{
    if (m_onSave)
        m_onSave(getSelected(), m_playerIdx);
    this->removeFromParentAndCleanup(true);
}

void PlayerEffectSettingsPopup::onChoosePlayer1(CCObject *)
{
    m_playerIdx = 1;
    updatePlayerButtons();
}

void PlayerEffectSettingsPopup::onChoosePlayer2(CCObject *)
{
    m_playerIdx = 2;
    updatePlayerButtons();
}

void PlayerEffectSettingsPopup::updatePlayerButtons()
{
    // Change textures based on selection: selected -> GJ_button_02.png, unselected -> GJ_button_01.png
    auto apply = [](CCMenuItemSpriteExtra *btn, const char *text, bool selected)
    {
        if (!btn)
            return;
        const char *tex = selected ? "GJ_button_02.png" : "GJ_button_04.png";
        // Replace the normal image with a new ButtonSprite so style updates
        auto newSpr = ButtonSprite::create(text, "bigFont.fnt", tex, 0.5f);
        btn->setNormalImage(newSpr);
    };
    apply(m_player1Btn, "Player 1", m_playerIdx == 1);
    apply(m_player2Btn, "Player 2", m_playerIdx == 2);
}

void PlayerEffectSettingsPopup::updateArrowPositions()
{
    if (!m_valueLabel || !m_leftBtn || !m_rightBtn)
        return;
    // place arrows with fixed padding from label bounds
    float halfWidth = (m_valueLabel->getContentSize().width * m_valueLabel->getScale()) / 2.0f;
    float pad = 24.f;
    m_leftBtn->setPosition(-(halfWidth + pad), 0);
    m_rightBtn->setPosition(+(halfWidth + pad), 0);
}

PlayerEffectSettingsPopup *PlayerEffectSettingsPopup::create(PlayerEffectType initial, int initialPlayer, std::function<void(PlayerEffectType, int)> onSave)
{
    auto ret = new PlayerEffectSettingsPopup();
    if (ret != nullptr)
    {
        // Map initial to index in current list
        int idx = 0;
        for (size_t i = 0; i < ret->m_types.size(); ++i)
        {
            if (ret->m_types[i] == initial)
            {
                idx = static_cast<int>(i);
                break;
            }
        }
        ret->m_selectedIdx = idx;
        ret->m_playerIdx = (initialPlayer == 2 ? 2 : 1);
        ret->m_onSave = onSave;
        if (ret->initAnchored(320.f, 180.f))
        {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
    }
    return nullptr;
}
