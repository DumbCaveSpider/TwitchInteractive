#include "ProfileSettingsPopup.hpp"

bool ProfileSettingsPopup::setup() {
    setTitle("Profile Settings");
    setID("profile-settings-popup");

    float y = 60.f;
    float x = m_mainLayer->getContentSize().width / 2;

    m_accountIdInput = TextInput::create(120, "Account ID", "chatFont.fnt");
    m_accountIdInput->setString(m_accountId.c_str());
    m_accountIdInput->setPosition(x, y + 10);
    m_mainLayer->addChild(m_accountIdInput);

    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(ProfileSettingsPopup::onSave)
    );

    auto menu = CCMenu::create();
    saveBtn->setPosition(0, 0);
    menu->addChild(saveBtn);
    menu->setPosition(x, y - 30);
    m_mainLayer->addChild(menu);

    return true;
}

void ProfileSettingsPopup::onSave(CCObject* sender) {
    std::string newId = m_accountIdInput ? m_accountIdInput->getString() : m_accountId;
    if (m_callback) m_callback(newId);
    onClose(sender);
}

ProfileSettingsPopup* ProfileSettingsPopup::create(const std::string& accountId, std::function<void(const std::string&)> callback) {
    auto ret = new ProfileSettingsPopup();
    ret->m_accountId = accountId;
    ret->m_callback = callback;
    if (ret && ret->initAnchored(220.f, 120.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
