#pragma once

#include <functional>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

enum class NotificationIconType
{
    None,
    Info,
    Success,
    Warning,
    Error,
    Loading
};

class NotificationSettingsPopup : public geode::Popup<std::string>
{
protected:
    CCMenuItemSpriteExtra *m_leftArrow = nullptr;
    CCMenuItemSpriteExtra *m_rightArrow = nullptr;
    std::string m_notificationText;
    NotificationIconType m_iconType = NotificationIconType::Info;
    std::function<void(const std::string &, NotificationIconType)> m_onSelect;
    bool setup(std::string notificationText) override;
    void onSave(cocos2d::CCObject *sender);
    void onLeftIcon(cocos2d::CCObject *sender);
    void onRightIcon(cocos2d::CCObject *sender);
    geode::TextInput *m_input = nullptr;
    cocos2d::CCLabelBMFont *m_iconLabel = nullptr;
    void updateIconLabel();

public:
    static NotificationSettingsPopup *create(const std::string &notificationText, std::function<void(const std::string &, NotificationIconType)> onSelect, NotificationIconType iconType = NotificationIconType::Info);
};
