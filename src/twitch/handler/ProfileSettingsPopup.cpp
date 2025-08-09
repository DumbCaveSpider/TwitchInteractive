#include "ProfileSettingsPopup.hpp"

bool ProfileSettingsPopup::setup() {
    setTitle("Profile Settings");
    setID("profile-settings-popup");

    float y = 60.f;
    float x = m_mainLayer->getContentSize().width / 2;

    m_accountIdInput = TextInput::create(120, "Account ID (leave blank for default)", "bigFont.fnt");
    m_accountIdInput->setCommonFilter(CommonFilter::Int);
    m_accountIdInput->setID("account-id-input");
    m_accountIdInput->setString(m_accountId.c_str());
    m_accountIdInput->setPosition(x - 20, y + 10);

    m_mainLayer->addChild(m_accountIdInput);

    this->m_noElasticity = true;

    // Add profile open button next to textbox
    auto openBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Open", "bigFont.fnt", "GJ_button_05.png", 0.3f),
        this,
        menu_selector(ProfileSettingsPopup::onOpenProfile));
    openBtn->setID("profile-open-btn");
    openBtn->setPosition(x + 70, y + 10);

    auto openMenu = CCMenu::create();
    openMenu->setPosition(0, 0);

    openMenu->addChild(openBtn);

    m_mainLayer->addChild(openMenu);

    // Add save button below
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(ProfileSettingsPopup::onSave));
    saveBtn->setID("save-btn");
    saveBtn->setPosition(0, 0);

    auto menu = CCMenu::create();
    menu->setPosition(x, y - 30);

    menu->addChild(saveBtn);

    m_mainLayer->addChild(menu);

    return true;
};

void ProfileSettingsPopup::onOpenProfile(CCObject* sender) {
    std::string idStr = m_accountIdInput ? m_accountIdInput->getString() : m_accountId;

    // Default fallback
    if (idStr.empty())
        idStr = "7689052";

    // Only allow numbers
    if (idStr.find_first_not_of("0123456789") == std::string::npos) {
        int accountIdInt = numFromString<int>(idStr).unwrapOrDefault();
        if (auto page = ProfilePage::create(accountIdInt, false))
            page->show();
    };
};

void ProfileSettingsPopup::onSave(CCObject* sender) {
    std::string newId = m_accountIdInput ? m_accountIdInput->getString() : m_accountId;
    if (m_callback)
        m_callback(newId);

    onClose(sender);
};

ProfileSettingsPopup* ProfileSettingsPopup::create(const std::string& accountId, std::function<void(const std::string&)> callback) {
    auto ret = new ProfileSettingsPopup();

    ret->m_accountId = accountId;
    ret->m_callback = callback;

    if (ret && ret->initAnchored(220.f, 120.f)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};