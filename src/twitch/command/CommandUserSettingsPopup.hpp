#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <functional>

class CommandUserSettingsPopup : public geode::Popup<> {

protected:
    geode::TextInput* m_userInput = nullptr;
    CCMenuItemToggler* m_vipToggler = nullptr;
    CCMenuItemToggler* m_modToggler = nullptr;
    CCMenuItemToggler* m_streamerToggler = nullptr;
    CCMenuItemToggler* m_subscriberToggler = nullptr;
    std::string m_allowedUser;
    bool m_allowVip = false;
    bool m_allowMod = false;
    bool m_allowStreamer = false;
    bool m_allowSubscriber = false;
    std::function<void(const std::string&, bool, bool, bool, bool)> m_callback;

    bool setup() override;
    void onSave(CCObject* sender);
public:
    static CommandUserSettingsPopup* create(const std::string& allowedUser, bool allowVip, bool allowMod, bool allowStreamer, bool allowSubscriber, std::function<void(const std::string&, bool, bool, bool, bool)> callback);
};
