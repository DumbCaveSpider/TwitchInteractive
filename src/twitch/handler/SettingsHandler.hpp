#pragma once
#include <cocos2d.h>
#include <string>
#include <vector>

#include "../command/CommandSettingsPopup.hpp"

namespace SettingsHandler {
    void handleProfileSettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender);
    void handleKeyCodeSettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender);
    void handleColorPlayerSettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender);
    void handleJumpSettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender);
    void handleMoveSettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender);
    void handleNotificationSettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender);
}
