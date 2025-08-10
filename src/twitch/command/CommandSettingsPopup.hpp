#pragma once

#include "../TwitchCommandManager.hpp"
#include "../handler/NotificationSettingsPopup.hpp"
#include "../HandbookPopup.hpp"

#include <vector>
#include <string>

#include <Geode/Geode.hpp>

namespace cocos2d
{
    class CCObject;
    class CCNode;
}

struct TwitchCommandAction;

class CommandListPopup;

class CommandSettingsPopup : public geode::Popup<TwitchCommand>
{
protected:
    geode::TextInput *m_notificationInput = nullptr;
    CCMenuItemToggler *m_killPlayerCheckbox = nullptr;
    TwitchCommand m_command = TwitchCommand("", "", 0, {});
    CommandListPopup *m_parent = nullptr;
    std::string m_eventSearchString;
    std::string m_lastEventSearchString;
    geode::TextInput* m_eventSearchInput = nullptr;
    std::function<void(const std::string&)> m_refreshEventNodeList = nullptr;
    void onEventSearchPoll(float);

    bool setup(TwitchCommand command) override;
    void onSave(cocos2d::CCObject *sender);
    void onCloseBtn(cocos2d::CCObject *sender);
    void onHandbookBtn(cocos2d::CCObject *sender);
    void onSettingsButtonUnified(cocos2d::CCObject *sender);
    void onSoundEffectSettings(cocos2d::CCObject *sender);

    void onEventInfoBtn(cocos2d::CCObject *sender);
    void onAddEventAction(cocos2d::CCObject *sender);
    void onRemoveAction(cocos2d::CCObject *sender);

    void onMoveActionUp(cocos2d::CCObject *sender);
    void onMoveActionDown(cocos2d::CCObject *sender);

public:
    static CommandSettingsPopup *create(TwitchCommand command, CommandListPopup *parent);
    static CommandSettingsPopup *create(TwitchCommand command);

    void onAlertSettings(cocos2d::CCObject *sender);
    void onColorPlayerSettings(cocos2d::CCObject *sender);
    void onOpenLevelInfoSettings(cocos2d::CCObject *sender);
    void onProfileUserSettings(cocos2d::CCObject *sender);
    void onNotificationSettings(cocos2d::CCObject *sender);
    void onJumpSettings(cocos2d::CCObject *sender);
    void onKeyCodeSettings(cocos2d::CCObject *sender);
    void onProfileSettings(cocos2d::CCObject *sender);
    void onMoveSettings(cocos2d::CCObject *sender);
    void onEditCameraSettings(cocos2d::CCObject *sender);
    void onGravitySettings(CCObject *sender);
    void onSpeedSettings(CCObject *sender);
    
        // Show Cooldown checkbox state and UI
        bool m_showCooldown = false;
        CCMenuItemToggler* m_showCooldownCheckbox = nullptr;
        void onShowCooldownToggled(CCObject* sender);
        bool getShowCooldown() const;
        void setShowCooldown(bool value);

    float m_actionSectionHeight = 0.f;
    std::unordered_map<int, std::string> m_notificationActionTexts;
    std::string getNotificationText() const;
    std::vector<std::string> m_commandActions;
    cocos2d::CCNode *m_actionContent = nullptr;

    void refreshActionsList();

    void updateNotificationNextTextLabel(int actionIdx, const std::string &nextText, NotificationIconType iconType);
    void updateKeyCodeNextTextLabel(int actionIdx, const std::string &nextKey);
    void updateColorPlayerLabel(int actionIdx);

    void onClose(CCObject* sender);
};
