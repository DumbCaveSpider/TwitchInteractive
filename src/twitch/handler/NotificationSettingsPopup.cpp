#include "NotificationSettingsPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;
using namespace cocos2d;

bool NotificationSettingsPopup::setup(std::string notificationText) {
    m_notificationText = notificationText;

    setTitle("Edit Notification");
    setID("notification-settings-popup");

    float y = 100.f;
    float x = m_mainLayer->getContentSize().width / 2;

    // Text input for notification
    m_input = geode::TextInput::create(180, "Notification text", "bigFont.fnt");
    m_input->setID("notification-input");
    m_input->setString(notificationText.c_str());
    m_input->setPosition(x, y);
    m_input->setScale(0.7f);

    m_mainLayer->addChild(m_input);

    // Save button 
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(NotificationSettingsPopup::onSave)
    );
    saveBtn->setID("notification-save-btn");

    auto menu = CCMenu::create();
    menu->addChild(saveBtn);
    menu->setPosition(x, y - 40.f);

    m_mainLayer->addChild(menu);

    return true;
};

void NotificationSettingsPopup::onSave(cocos2d::CCObject* sender) {
    std::string text = m_input ? m_input->getString() : "";
    if (m_onSelect) m_onSelect(text);

    onClose(nullptr);
};

NotificationSettingsPopup* NotificationSettingsPopup::create(const std::string& notificationText, std::function<void(const std::string&)> onSelect) {
    auto ret = new NotificationSettingsPopup();
    ret->m_onSelect = onSelect;

    if (ret && ret->initAnchored(220.f, 160.f, notificationText)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};