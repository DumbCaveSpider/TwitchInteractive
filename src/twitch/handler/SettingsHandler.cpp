#include "SettingsHandler.hpp"
#include "KeyCodesSettingsPopup.hpp"
#include "CameraSettingsPopup.hpp"
#include "ProfileSettingsPopup.hpp"
#include "MoveSettingsPopup.hpp"
#include "JumpSettingsPopup.hpp"
#include "ColorPlayerSettingsPopup.hpp"
#include "AlertSettingsPopup.hpp"

#include <algorithm>

using namespace cocos2d;

namespace SettingsHandler {

// Handler for Alert Popup settings button
void SettingsHandler::handleAlertSettings(CommandSettingsPopup* parent, cocos2d::CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int actionIdx = 0;
    if (btn && btn->getUserObject()) actionIdx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (!parent || actionIdx < 0 || actionIdx >= static_cast<int>(parent->m_commandActions.size())) return;
    // Parse current values from m_commandActions[actionIdx]
    std::string alertTitle, alertDesc;
    const std::string& actionIdRaw = parent->m_commandActions[actionIdx];
    size_t firstColon = actionIdRaw.find(":");
    size_t secondColon = actionIdRaw.find(":", firstColon + 1);
    if (firstColon != std::string::npos && secondColon != std::string::npos) {
        alertTitle = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
        alertDesc = actionIdRaw.substr(secondColon + 1);
    }
    // Show the AlertSettingsPopup and update the value and label on save
    auto popup = AlertSettingsPopup::create(alertTitle, alertDesc, [parent, actionIdx](const std::string& newTitle, const std::string& newDesc) {
        std::string newActionStr = "alert_popup:" + newTitle + ":" + newDesc;
        parent->m_commandActions[actionIdx] = newActionStr;
        // Update the label in the UI
        auto children = parent->m_actionContent->getChildren();
        if (children && actionIdx >= 0 && actionIdx < children->count()) {
            auto actionNode = as<CCNode*>(children->objectAtIndex(actionIdx));
            if (actionNode) {
                std::string alertLabelId = "alert-popup-action-text-label-" + std::to_string(actionIdx);
                std::string labelText = "Title: " + newTitle + (newDesc.empty() ? "" : (", Desc: " + newDesc));
                if (auto alertLabel = dynamic_cast<CCLabelBMFont*>(actionNode->getChildByID(alertLabelId))) alertLabel->setString(labelText.c_str());
            }
        }
    });
    if (popup) popup->show();
}

void handleCameraSettings(CommandSettingsPopup* popup, CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn && btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size())) return;
    std::string& actionStr = popup->m_commandActions[idx];
    std::string actionStrLower = actionStr;
    std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);
    if (actionStrLower.rfind("edit_camera", 0) != 0 && actionStrLower.rfind("edit camera", 0) != 0) return;
    float skew = 0.f, rot = 0.f, scale = 1.f, time = 0.f;
    size_t colonPos = actionStr.find(":");
    if (colonPos != std::string::npos && colonPos + 1 < actionStr.size()) {
        std::string values = actionStr.substr(colonPos + 1);
        sscanf(values.c_str(), "%f,%f,%f,%f", &skew, &rot, &scale, &time);
    }
    CameraSettingsPopup::create(skew, rot, scale, time, [popup, idx](float newSkew, float newRot, float newScale, float newTime) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.2f,%.2f,%.2f,%.2f", newSkew, newRot, newScale, newTime);
        popup->m_commandActions[idx] = std::string("edit_camera:") + buf;
        popup->refreshActionsList();
    })->show();
}

void handleProfileSettings(CommandSettingsPopup* popup, CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size())) return;
    std::string& actionStr = popup->m_commandActions[idx];
    std::string actionStrLower = actionStr;
    std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);
    if (actionStrLower.rfind("profile", 0) != 0) return;
    std::string accountId = "7689052";
    size_t colonPos = actionStr.find(":");
    if (colonPos != std::string::npos && colonPos + 1 < actionStr.size()) {
        accountId = actionStr.substr(colonPos + 1);
        if (accountId.empty()) accountId = "7689052";
    }
    ProfileSettingsPopup::create(
        accountId,
        [popup, idx](const std::string& newAccountId) {
            if (idx >= 0 && idx < static_cast<int>(popup->m_commandActions.size())) {
                popup->m_commandActions[idx] = "profile:" + newAccountId;
                popup->refreshActionsList();
            }
        }
    )->show();
}

void handleKeyCodeSettings(CommandSettingsPopup* popup, CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= as<int>(popup->m_commandActions.size())) return;
    std::string& actionStr = popup->m_commandActions[idx];
    if (actionStr.rfind("keycode", 0) != 0) return;
    std::string keyValue;
    size_t colonPos = actionStr.find(":");
    if (colonPos != std::string::npos && colonPos + 1 < actionStr.size()) {
        keyValue = actionStr.substr(colonPos + 1);
    } else {
        keyValue = "";
    }
    KeyCodesSettingsPopup::create(
        keyValue,
        [popup, idx](const std::string& newKeyWithDuration) {
            popup->updateKeyCodeNextTextLabel(idx, newKeyWithDuration);
        }
    )->show();
}

void handleColorPlayerSettings(CommandSettingsPopup* popup, CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn && btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size())) return;
    std::string& actionStr = popup->m_commandActions[idx];
    std::string actionStrLower = actionStr;
    std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);
    if (actionStrLower.rfind("color_player", 0) != 0 && actionStrLower.rfind("color player", 0) != 0) return;
    ccColor3B color = {255, 255, 255};
    size_t colonPos = actionStr.find(":");
    if (colonPos != std::string::npos && colonPos + 1 < actionStr.size()) {
        std::string colorStr = actionStr.substr(colonPos + 1);
        int r = 255, g = 255, b = 255;
        sscanf(colorStr.c_str(), "%d,%d,%d", &r, &g, &b);
        color = {static_cast<GLubyte>(r), static_cast<GLubyte>(g), static_cast<GLubyte>(b)};
    }
    ColorPlayerSettingsPopup::create(color, [popup, idx](const ccColor3B& newColor) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d,%d,%d", newColor.r, newColor.g, newColor.b);
        popup->m_commandActions[idx] = std::string("color_player:") + buf;
        popup->updateColorPlayerLabel(idx);
    })->show();
}

void handleJumpSettings(CommandSettingsPopup* popup, CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size())) return;
    std::string actionIdRaw = popup->m_commandActions[idx];
    int jumpPlayerValue = 1;
    bool isHold = false;
    size_t colonPos = actionIdRaw.find(":");
    if (colonPos != std::string::npos && colonPos + 1 < actionIdRaw.size()) {
        std::string val = actionIdRaw.substr(colonPos + 1);
        size_t holdPos = val.find(":hold");
        if (holdPos != std::string::npos) {
            isHold = true;
            val = val.substr(0, holdPos);
        }
        if (!val.empty() && val.find_first_not_of("-0123456789") == std::string::npos) {
            jumpPlayerValue = std::stoi(val);
        }
    }
    JumpSettingsPopup::create(
        jumpPlayerValue,
        isHold,
        [popup, idx](int newPlayer, bool hold) {
            if (idx >= 0 && idx < static_cast<int>(popup->m_commandActions.size())) {
                std::string action = "jump:" + std::to_string(newPlayer);
                if (hold) action += ":hold";
                popup->m_commandActions[idx] = action;
                popup->refreshActionsList();
            }
        }
    )->show();
}

void handleMoveSettings(CommandSettingsPopup* popup, CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size())) return;
    std::string& actionStr = popup->m_commandActions[idx];
    std::string actionStrLower = actionStr;
    std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);
    if (actionStrLower.rfind("move", 0) != 0) return;
    int player = 1;
    bool moveRight = true;
    float distance = 0.f;
    size_t firstColon = actionStr.find(":");
    size_t secondColon = actionStr.find(":", firstColon + 1);
    size_t thirdColon = actionStr.find(":", secondColon + 1);
    if (firstColon != std::string::npos && secondColon != std::string::npos) {
        std::string playerStr = actionStr.substr(firstColon + 1, secondColon - firstColon - 1);
        std::string dirStr;
        std::string distStr;
        if (thirdColon != std::string::npos) {
            dirStr = actionStr.substr(secondColon + 1, thirdColon - secondColon - 1);
            distStr = actionStr.substr(thirdColon + 1);
        } else {
            dirStr = actionStr.substr(secondColon + 1);
        }
        if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos) {
            player = std::stoi(playerStr);
        }
        moveRight = (dirStr == "right");
        if (!distStr.empty() && distStr.find_first_not_of("-0123456789.") == std::string::npos) {
            distance = std::stof(distStr);
        }
    }
    auto popupMove = MoveSettingsPopup::create(
        player,
        moveRight,
        [popup, idx](int newPlayer, bool newMoveRight, float newDistance) {
            if (idx >= 0 && idx < static_cast<int>(popup->m_commandActions.size())) {
                std::string dirStr = newMoveRight ? "right" : "left";
                popup->m_commandActions[idx] = "move:" + std::to_string(newPlayer) + ":" + dirStr + ":" + std::to_string(newDistance);
                popup->refreshActionsList();
            }
        }
    );
    if (popupMove) {
        popupMove->setDistance(distance);
        if (auto input = popupMove->getDistanceInput()) {
            char distBuf[32];
            snprintf(distBuf, sizeof(distBuf), "%.5f", distance);
            input->setString(distBuf);
        }
        popupMove->show();
    }
}

void handleNotificationSettings(CommandSettingsPopup* popup, CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= as<int>(popup->m_commandActions.size())) return;
    std::string& actionStr = popup->m_commandActions[idx];
    std::string actionStrLower = actionStr;
    std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);
    if (actionStrLower.rfind("notification", 0) != 0) return;
    int iconTypeInt = 1;
    std::string notifText;
    size_t firstColon = actionStr.find(":");
    size_t secondColon = actionStr.find(":", firstColon + 1);
    if (firstColon != std::string::npos && secondColon != std::string::npos) {
        iconTypeInt = std::stoi(actionStr.substr(firstColon + 1, secondColon - firstColon - 1));
        notifText = actionStr.substr(secondColon + 1);
    } else if (actionStr.length() > 13) {
        notifText = actionStr.substr(13);
    } else {
        notifText = "";
    }
    NotificationSettingsPopup::create(
        notifText,
        [popup, idx](const std::string& newText, NotificationIconType newIconType) {
            popup->updateNotificationNextTextLabel(idx, newText, newIconType);
        },
        static_cast<NotificationIconType>(iconTypeInt)
    )->show();
}
}