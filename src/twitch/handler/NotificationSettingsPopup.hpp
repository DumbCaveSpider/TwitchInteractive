#pragma once

#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <functional>

class NotificationSettingsPopup : public geode::Popup<std::string> {
protected:
    std::string m_notificationText;
    std::function<void(const std::string&)> m_onSelect;
    bool setup(std::string notificationText) override;
    void onSave(cocos2d::CCObject* sender);
    geode::TextInput* m_input = nullptr;
public:
    static NotificationSettingsPopup* create(const std::string& notificationText, std::function<void(const std::string&)> onSelect);
};
