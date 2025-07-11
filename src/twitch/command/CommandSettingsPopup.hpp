#pragma once

#include "../TwitchCommandManager.hpp"
#include "../handler/NotificationSettingsPopup.hpp"

#include <vector>
#include <string>

#include <Geode/ui/Popup.hpp>

namespace cocos2d { class CCObject; class CCNode; }

class CommandSettingsPopup : public geode::Popup<TwitchCommand> {
protected:
    TextInput* m_notificationInput = nullptr;
    CCMenuItemToggler* m_killPlayerCheckbox = nullptr;
    TwitchCommand m_command = TwitchCommand("", "", "", 0, {});
    bool setup(TwitchCommand command) override;
    void onSave(cocos2d::CCObject* sender);
    void onCloseBtn(cocos2d::CCObject* sender);

public:
    std::unordered_map<int, std::string> m_notificationActionTexts;
    void updateNotificationNextTextLabel(int actionIdx, const std::string& nextText);
    static CommandSettingsPopup* create(TwitchCommand command);
    std::string getNotificationText() const;
    std::vector<std::string> m_commandActions;
    cocos2d::CCNode* m_actionContent = nullptr;
    float m_actionSectionHeight = 0.f;
    void refreshActionsList();
    void onAddEventAction(cocos2d::CCObject* sender);
    void onRemoveAction(cocos2d::CCObject* sender);
    void onNotificationSettings(cocos2d::CCObject* sender);
    void onJumpSettings(cocos2d::CCObject* sender);
    void onMoveActionUp(cocos2d::CCObject* sender);
    void onMoveActionDown(cocos2d::CCObject* sender);
};
