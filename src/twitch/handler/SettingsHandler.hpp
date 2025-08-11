#pragma once
#include <cocos2d.h>
#include <string>
#include <vector>

#include "../command/CommandSettingsPopup.hpp"

namespace SettingsHandler
{
    void handleAlertSettings(CommandSettingsPopup *parent, cocos2d::CCObject *sender);
    void handleProfileSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleKeyCodeSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleColorPlayerSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleJumpSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleMoveSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleNotificationSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleCameraSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleEditCameraSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleScalePlayerSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleSoundEffectSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleGravitySettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleSpeedSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
    void handleJumpscareSettings(CommandSettingsPopup *popup, cocos2d::CCObject *sender);
}
