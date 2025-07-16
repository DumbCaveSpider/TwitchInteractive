#pragma once

#include "../TwitchCommandManager.hpp"
#include "../handler/NotificationSettingsPopup.hpp"
#include "../HandbookPopup.hpp"

#include <vector>
#include <string>

#include <Geode/Geode.hpp>

namespace cocos2d { class CCObject; class CCNode; }

struct TwitchCommandAction;

// Forward declaration for CommandListPopup
class CommandListPopup;

class CommandSettingsPopup : public geode::Popup<TwitchCommand> {
    protected:
    TextInput* m_notificationInput = nullptr;
    CCMenuItemToggler* m_killPlayerCheckbox = nullptr;
    TwitchCommand m_command = TwitchCommand("", "", 0, {});
    CommandListPopup* m_parent = nullptr;
    bool setup(TwitchCommand command) override;
    void onSave(cocos2d::CCObject* sender);
    void onCloseBtn(cocos2d::CCObject* sender);
    void onHandbookBtn(cocos2d::CCObject* sender);
public:
    static CommandSettingsPopup* create(TwitchCommand command, CommandListPopup* parent);
    static CommandSettingsPopup* create(TwitchCommand command);
    void onAlertSettings(cocos2d::CCObject* sender);
    void onCameraSettingsClicked(cocos2d::CCObject* sender);
    void onMoveActionUp(cocos2d::CCObject* sender);
    void onMoveActionDown(cocos2d::CCObject* sender);
    void onColorPlayerSettings(cocos2d::CCObject* sender);
    float m_actionSectionHeight = 0.f;
    std::unordered_map<int, std::string> m_notificationActionTexts;
    std::string getNotificationText() const;
    std::vector<std::string> m_commandActions;
    cocos2d::CCNode* m_actionContent = nullptr;
    void updateNotificationNextTextLabel(int actionIdx, const std::string& nextText);
    void updateNotificationNextTextLabel(int actionIdx, const std::string& nextText, NotificationIconType iconType);
    void updateColorPlayerLabel(int actionIdx);
    void refreshActionsList();
    void onEventInfoBtn(cocos2d::CCObject* sender);
    void onAddEventAction(cocos2d::CCObject* sender);
    void onRemoveAction(cocos2d::CCObject* sender);
    void onProfileUserSettings(cocos2d::CCObject* sender);
    void onNotificationSettings(cocos2d::CCObject* sender);
    void onJumpSettings(cocos2d::CCObject* sender);
    void onKeyCodeSettings(cocos2d::CCObject* sender);
    void onProfileSettings(cocos2d::CCObject* sender);
    void onMoveSettings(cocos2d::CCObject* sender);
    void onEditCameraSettings(cocos2d::CCObject* sender);
    void updateKeyCodeNextTextLabel(int actionIdx, const std::string& nextKey);

};
