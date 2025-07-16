#include "AlertSettingsPopup.hpp"
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>

using namespace cocos2d;
using namespace geode::prelude;


bool AlertSettingsPopup::setup() {
    float width = 320.f;
    float height = 180.f;
    setTitle("Alert Popup Settings");
    setContentSize({width, height});

    m_titleInput = geode::TextInput::create(width - 60, "Title", "chatFont.fnt");
    m_titleInput->setString(m_initTitle.c_str());
    m_titleInput->setPosition(width / 2, height - 60);
    addChild(m_titleInput);

    m_descInput = geode::TextInput::create(width - 60, "Description", "chatFont.fnt");
    m_descInput->setString(m_initDesc.c_str());
    m_descInput->setPosition(width / 2, height - 110);
    addChild(m_descInput);

    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(AlertSettingsPopup::onSave)
    );
    saveBtn->setPosition(width / 2, 35.f);
    auto menu = CCMenu::create();
    menu->addChild(saveBtn);
    menu->setPosition(0, 0);
    addChild(menu);
    return true;
}

void AlertSettingsPopup::onSave(cocos2d::CCObject* sender) {
    if (m_callback) {
        std::string title = m_titleInput ? m_titleInput->getString() : "";
        std::string desc = m_descInput ? m_descInput->getString() : "";
        m_callback(title, desc);
    }
    onClose(sender);
}

void AlertSettingsPopup::onClose(cocos2d::CCObject* sender) {
    this->removeFromParentAndCleanup(true);
}

AlertSettingsPopup* AlertSettingsPopup::create(const std::string& title, const std::string& desc, std::function<void(const std::string&, const std::string&)> callback) {
    auto ret = new AlertSettingsPopup();
    if (ret && ret->initAnchored(320.f, 180.f)) {
        ret->m_callback = callback;
        ret->m_initTitle = title;
        ret->m_initDesc = desc;
        ret->setup();
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}