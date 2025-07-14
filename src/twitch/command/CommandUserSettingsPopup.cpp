#include "CommandUserSettingsPopup.hpp"
#include <Geode/Geode.hpp>
using namespace geode::prelude;
using namespace cocos2d;

bool CommandUserSettingsPopup::setup() {
    setTitle("Command User Settings");
    setID("command-user-settings-popup");

    float y = 160.f;
    float x = m_mainLayer->getContentSize().width / 2;

    // Username input
    m_userInput = geode::TextInput::create(160, "Twitch Username (leave blank for any)", "chatFont.fnt");
    m_userInput->setString(m_allowedUser.c_str());
    m_userInput->setPosition(x, y);
    m_mainLayer->addChild(m_userInput);

    y -= 40.f;


    // Role toggles and labels
    struct RoleTogglerInfo {
        const char* label;
        float posX;
        CCMenuItemToggler** togglerPtr;
        bool initial;
    };
    RoleTogglerInfo togglers[] = {
        {"VIP",      x - 90, &m_vipToggler, m_allowVip},
        {"Mod",      x - 30, &m_modToggler, m_allowMod},
        {"Subscriber", x + 90, &m_subscriberToggler, m_allowSubscriber},
        {"Streamer", x + 30, &m_streamerToggler, m_allowStreamer},
    };
    // Add togglers to a CCMenu for proper touch handling
    auto togglerMenu = CCMenu::create();
    togglerMenu->setPosition(0, 0);
    for (const auto& info : togglers) {
        auto on = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        auto off = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        *info.togglerPtr = CCMenuItemToggler::create(off, on, nullptr, nullptr);
        (*info.togglerPtr)->setPosition(info.posX, y - 10.f);
        (*info.togglerPtr)->toggle(info.initial);
        togglerMenu->addChild(*info.togglerPtr);
        // Add label under toggler
        auto label = CCLabelBMFont::create(info.label, "bigFont.fnt");
        label->setScale(0.3f);
        label->setAnchorPoint({0.5f, 1.0f});
        label->setPosition(info.posX, y - 28.f);
        m_mainLayer->addChild(label);
    }
    m_mainLayer->addChild(togglerMenu);

    // Save button
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(CommandUserSettingsPopup::onSave)
    );
    auto menu = CCMenu::create();
    saveBtn->setPosition(0, 0);
    menu->addChild(saveBtn);
    menu->setPosition(x, y - 80.f);
    m_mainLayer->addChild(menu);

    return true;
}

void CommandUserSettingsPopup::onSave(CCObject* sender) {
    std::string user = m_userInput ? m_userInput->getString() : "";
    bool vip = m_vipToggler ? m_vipToggler->isToggled() : false;
    bool mod = m_modToggler ? m_modToggler->isToggled() : false;
    bool subscriber = m_subscriberToggler ? m_subscriberToggler->isToggled() : false;
    bool streamer = m_streamerToggler ? m_streamerToggler->isToggled() : false;
    if (m_callback) m_callback(user, vip, mod, subscriber, streamer);
    onClose(sender);
}

CommandUserSettingsPopup* CommandUserSettingsPopup::create(const std::string& allowedUser, bool allowVip, bool allowMod, bool allowSubscriber, bool allowStreamer, std::function<void(const std::string&, bool, bool, bool, bool)> callback) {
    auto ret = new CommandUserSettingsPopup();
    ret->m_allowedUser = allowedUser;
    ret->m_allowVip = allowVip;
    ret->m_allowMod = allowMod;
    ret->m_allowSubscriber = allowSubscriber;
    ret->m_allowStreamer = allowStreamer;
    ret->m_callback = callback;
    if (ret && ret->initAnchored(320.f, 220.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
