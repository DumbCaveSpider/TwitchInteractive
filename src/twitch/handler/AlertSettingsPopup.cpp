#include "AlertSettingsPopup.hpp"
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>

using namespace cocos2d;
using namespace geode::prelude;

bool AlertSettingsPopup::setup()
{
    float width = 320.f;
    float height = 180.f;
    setTitle("Alert Popup Settings");
    setID("alert-settings-popup");
    this->m_noElasticity = true;

    auto popupSize = m_mainLayer->getContentSize();
    float margin = 30.f;
    float inputWidth = popupSize.width - margin * 2;
    float inputHeight = 32.f;
    float spacing = 14.f;

    // Centered Y positions
    float centerY = popupSize.height / 2;
    float titleY = centerY + inputHeight / 2 + spacing / 2;
    float descY = centerY - inputHeight / 2 - spacing / 2;
    float btnY = margin;

    // Title input
    m_titleInput = geode::TextInput::create(inputWidth, "Title", "chatFont.fnt");
    m_titleInput->setString(m_initTitle.c_str());
    m_titleInput->setPosition(popupSize.width / 2, titleY);
    m_mainLayer->addChild(m_titleInput);

    // Description input
    m_descInput = geode::TextInput::create(inputWidth, "Content", "chatFont.fnt");
    m_descInput->setString(m_initDesc.c_str());
    m_descInput->setPosition(popupSize.width / 2, descY + 10.f);
    m_mainLayer->addChild(m_descInput);

    // Save button
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(AlertSettingsPopup::onSave));
    saveBtn->setPosition(popupSize.width / 2, btnY);
    auto menu = CCMenu::create();
    menu->addChild(saveBtn);
    menu->setPosition(0, 0);
    m_mainLayer->addChild(menu);
    return true;
}

void AlertSettingsPopup::onSave(cocos2d::CCObject *sender)
{
    if (m_callback)
    {
        std::string title = m_titleInput ? m_titleInput->getString() : "";
        std::string desc = m_descInput ? m_descInput->getString() : "";
        m_callback(title, desc);
    }
    onClose(sender);
}

void AlertSettingsPopup::onClose(cocos2d::CCObject *sender)
{
    this->removeFromParentAndCleanup(true);
}

AlertSettingsPopup *AlertSettingsPopup::create(const std::string &title, const std::string &desc, std::function<void(const std::string &, const std::string &)> callback)
{
    auto ret = new AlertSettingsPopup();
    if (ret)
    {
        ret->m_callback = callback;
        ret->m_initTitle = title;
        ret->m_initDesc = desc;
        if (ret->initAnchored(320.f, 180.f))
        {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
    }
    return nullptr;
}