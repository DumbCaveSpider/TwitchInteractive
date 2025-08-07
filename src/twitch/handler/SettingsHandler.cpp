#include "SettingsHandler.hpp"
#include "KeyCodesSettingsPopup.hpp"
#include "CameraSettingsPopup.hpp"
#include "ProfileSettingsPopup.hpp"
#include "MoveSettingsPopup.hpp"
#include "JumpSettingsPopup.hpp"
#include "ColorPlayerSettingsPopup.hpp"
#include "AlertSettingsPopup.hpp"
#include "ScaleSettingsPopup.hpp"
#include "SoundSettingsPopup.hpp"
#include "GravitySettingsPopup.hpp"
#include "SpeedSettingsPopup.hpp"

#include <algorithm>

using namespace cocos2d;

namespace SettingsHandler {
    // Process the gravity action settings
    void handleGravitySettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = 0;
        if (btn && btn->getUserObject())
            idx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (!popup || idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        std::string& actionStr = popup->m_commandActions[idx];
        std::string actionStrLower = actionStr;
        std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);

        float gravity = 1.0f;
        float duration = 2.0f;
        size_t firstColon = actionStr.find(":");
        size_t secondColon = actionStr.find(":", firstColon + 1);
        if (firstColon != std::string::npos && secondColon != std::string::npos) {
            std::string gravityStr = actionStr.substr(firstColon + 1, secondColon - firstColon - 1);
            std::string durationStr = actionStr.substr(secondColon + 1);
            if (!gravityStr.empty())
                gravity = numFromString<float>(gravityStr).unwrapOrDefault();
            if (!durationStr.empty())
                duration = numFromString<float>(durationStr).unwrapOrDefault();
        }

        auto popupWindow = GravitySettingsPopup::create(
            idx,
            gravity,
            duration,
            [popup, idx](float newGravity, float newDuration) {
                popup->m_commandActions[idx] = fmt::format("gravity:{:.2f}:{:.2f}", newGravity, newDuration);
                popup->refreshActionsList();
            });
        if (popupWindow) {
            popupWindow->show();
        }
    }
    // Process the edit camera action settings
    void handleEditCameraSettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int actionIdx = 0;

        if (btn && btn->getUserObject())
            actionIdx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (!popup || actionIdx < 0 || actionIdx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        // Parse current values from m_commandActions[actionIdx]
        float zoom = 0.0f, x = 0.0f, y = 1.0f, duration = 0.0f;
        const std::string& actionIdRaw = popup->m_commandActions[actionIdx];

        size_t firstColon = actionIdRaw.find(":");

        size_t secondColon = actionIdRaw.find(":", firstColon + 1);
        size_t thirdColon = actionIdRaw.find(":", secondColon + 1);
        size_t fourthColon = actionIdRaw.find(":", thirdColon + 1);

        if (firstColon != std::string::npos && secondColon != std::string::npos && thirdColon != std::string::npos && fourthColon != std::string::npos) {
            std::string zoomStr = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);

            std::string xStr = actionIdRaw.substr(secondColon + 1, thirdColon - secondColon - 1);
            std::string yStr = actionIdRaw.substr(thirdColon + 1, fourthColon - thirdColon - 1);

            std::string durStr = actionIdRaw.substr(fourthColon + 1);

            if (!zoomStr.empty())
                zoom = numFromString<float>(zoomStr).unwrapOrDefault();
            if (!xStr.empty())
                x = numFromString<float>(xStr).unwrapOrDefault();
            if (!yStr.empty())
                y = numFromString<float>(yStr).unwrapOrDefault();
            if (!durStr.empty())
                duration = numFromString<float>(durStr).unwrapOrDefault();
        };

        // Show the CameraSettingsPopup and update the value and label on save
        auto popupWindow = CameraSettingsPopup::create(zoom, x, y, duration, [popup, actionIdx](float newZoom, float newX, float newY, float newDuration) {
            popup->m_commandActions[actionIdx] = fmt::format("edit_camera:{:.2f}:{:.2f}:{:.2f}:{:.2f}", newZoom, newX, newY, newDuration);

            // Refresh the action node UI after saving
            popup->refreshActionsList(); });

        if (popupWindow)
            popupWindow->show();
    };

    void handleSpeedSettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = 0;
        if (btn && btn->getUserObject())
            idx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (!popup || idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        std::string& actionStr = popup->m_commandActions[idx];
        float speed = 1.0f, duration = 0.5f;
        size_t firstColon = actionStr.find(":");
        size_t secondColon = actionStr.find(":", firstColon + 1);
        if (firstColon != std::string::npos && secondColon != std::string::npos) {
            std::string speedStr = actionStr.substr(firstColon + 1, secondColon - firstColon - 1);
            std::string durationStr = actionStr.substr(secondColon + 1);
            if (!speedStr.empty())
                speed = numFromString<float>(speedStr).unwrapOrDefault();
            if (!durationStr.empty())
                duration = numFromString<float>(durationStr).unwrapOrDefault();
        }

        auto popupWindow = SpeedSettingsPopup::create(
            idx,
            speed,
            duration,
            [popup, idx](float newSpeed, float newDuration) {
                auto buf_speed = fmt::format("speed_player:{:.2f}:{:.2f}", newSpeed, newDuration);
                popup->m_commandActions[idx] = buf_speed;
                popup->refreshActionsList();
            });
        if (popupWindow) {
            popupWindow->show();
        }
    }

    // Process the alert popup action settings
    void handleAlertSettings(CommandSettingsPopup* parent, cocos2d::CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int actionIdx = 0;

        if (btn && btn->getUserObject())
            actionIdx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (!parent || actionIdx < 0 || actionIdx >= static_cast<int>(parent->m_commandActions.size()))
            return;

        // Parse current values from m_commandActions[actionIdx]
        std::string alertTitle = "-", alertDesc = "-";
        const std::string& actionIdRaw = parent->m_commandActions[actionIdx];

        size_t firstColon = actionIdRaw.find(":");
        size_t secondColon = actionIdRaw.find(":", firstColon + 1);

        if (firstColon != std::string::npos && secondColon != std::string::npos) {
            alertTitle = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
            alertDesc = actionIdRaw.substr(secondColon + 1);
        };

        // Treat '-' as empty for UI prefill
        if (alertTitle == "-")
            alertTitle = "";
        if (alertDesc == "-")
            alertDesc = "";

        // Show the AlertSettingsPopup and update the value and label on save
        auto popup = AlertSettingsPopup::create(alertTitle, alertDesc, [parent, actionIdx](const std::string& newTitle, const std::string& newDesc) {
            std::string safeTitle = newTitle.empty() ? "-" : newTitle;
            std::string safeDesc = newDesc.empty() ? "-" : newDesc;
            std::string newActionStr = "alert_popup:" + safeTitle + ":" + safeDesc;

            parent->m_commandActions[actionIdx] = newActionStr;

            // Refresh the action node UI after saving
            parent->refreshActionsList(); });

        if (popup)
            popup->show();
    };

    // Process the camera action settings
    void handleCameraSettings(CommandSettingsPopup* popup, CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = 0;

        if (btn && btn->getUserObject())
            idx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        std::string& actionStr = popup->m_commandActions[idx];
        std::string actionStrLower = actionStr;

        std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);

        if (actionStrLower.rfind("edit_camera", 0) != 0 && actionStrLower.rfind("edit camera", 0) != 0)
            return;

        float skew = 0.f, rot = 0.f, scale = 1.f, time = 0.f;
        size_t colonPos = actionStr.find(":");

        if (colonPos != std::string::npos && colonPos + 1 < actionStr.size()) {
            std::string values = actionStr.substr(colonPos + 1);
            sscanf(values.c_str(), "%f,%f,%f,%f", &skew, &rot, &scale, &time);
        };

        CameraSettingsPopup::create(skew, rot, scale, time, [popup, idx](float newSkew, float newRot, float newScale, float newTime) {

            popup->m_commandActions[idx] = fmt::format("edit_camera:{:.2f},{:.2f},{:.2f},{:.2f}", newSkew, newRot, newScale, newTime);
            popup->refreshActionsList(); })
            ->show();
    };

    // Process the profile action settings
    void handleProfileSettings(CommandSettingsPopup* popup, CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = 0;

        if (btn->getUserObject())
            idx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        std::string& actionStr = popup->m_commandActions[idx];
        std::string actionStrLower = actionStr;

        std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);

        if (actionStrLower.rfind("profile", 0) != 0)
            return;

        std::string accountId = "7689052";
        size_t colonPos = actionStr.find(":");

        if (colonPos != std::string::npos && colonPos + 1 < actionStr.size()) {
            accountId = actionStr.substr(colonPos + 1);
            if (accountId.empty())
                accountId = "7689052";
        };

        ProfileSettingsPopup::create(
            accountId,
            [popup, idx](const std::string& newAccountId) {
                if (idx >= 0 && idx < static_cast<int>(popup->m_commandActions.size())) {
                    popup->m_commandActions[idx] = "profile:" + newAccountId;
                    popup->refreshActionsList();
                }
            })
            ->show();
    };

    // Process the keybind action settings
    void handleKeyCodeSettings(CommandSettingsPopup* popup, CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = 0;

        if (btn->getUserObject())
            idx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        std::string& actionStr = popup->m_commandActions[idx];

        if (actionStr.rfind("keycode", 0) != 0)
            return;

        std::string keyValue;
        size_t colonPos = actionStr.find(":");

        if (colonPos != std::string::npos && colonPos + 1 < actionStr.size()) {
            keyValue = actionStr.substr(colonPos + 1);
        } else {
            keyValue = "";
        };

        KeyCodesSettingsPopup::create(
            keyValue,
            [popup, idx](const std::string& newKeyWithDuration) {
                popup->updateKeyCodeNextTextLabel(idx, newKeyWithDuration);
                popup->refreshActionsList();
            })
            ->show();
    };

    // Process the player color action settings
    void handleColorPlayerSettings(CommandSettingsPopup* popup, CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = 0;

        if (btn && btn->getUserObject())
            idx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        std::string& actionStr = popup->m_commandActions[idx];
        std::string actionStrLower = actionStr;

        std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);

        if (actionStrLower.rfind("color_player", 0) != 0 && actionStrLower.rfind("color player", 0) != 0)
            return;
        ccColor3B color = { 255, 255, 255 };

        size_t colonPos = actionStr.find(":");

        if (colonPos != std::string::npos && colonPos + 1 < actionStr.size()) {
            std::string colorStr = actionStr.substr(colonPos + 1);
            int r = 255, g = 255, b = 255;
            sscanf(colorStr.c_str(), "%d,%d,%d", &r, &g, &b);
            color = { static_cast<GLubyte>(r), static_cast<GLubyte>(g), static_cast<GLubyte>(b) };
        };

        ColorPlayerSettingsPopup::create(color, [popup, idx](const ccColor3B& newColor) {
            auto buf_color = fmt::format("{},{},{}", newColor.r, newColor.g, newColor.b);

            popup->m_commandActions[idx] = std::string("color_player:") + buf_color;

            popup->updateColorPlayerLabel(idx);
            popup->refreshActionsList(); })
            ->show();
    };

    // Process the jump action settings
    void handleJumpSettings(CommandSettingsPopup* popup, CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = 0;

        if (btn->getUserObject())
            idx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size()))
            return;

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
            };

            if (!val.empty() && val.find_first_not_of("-0123456789") == std::string::npos)
                jumpPlayerValue = numFromString<int>(val).unwrapOrDefault();
        };

        JumpSettingsPopup::create(
            jumpPlayerValue,
            isHold,
            [popup, idx](int newPlayer, bool hold) {
                if (idx >= 0 && idx < static_cast<int>(popup->m_commandActions.size())) {
                    std::string action = "jump:" + std::to_string(newPlayer);

                    if (hold)
                        action += ":hold";

                    popup->m_commandActions[idx] = action;
                    popup->refreshActionsList();
                };
            })
            ->show();
    };

    void handleMoveSettings(CommandSettingsPopup* popup, CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = 0;

        if (btn->getUserObject())
            idx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        std::string& actionStr = popup->m_commandActions[idx];
        std::string actionStrLower = actionStr;

        std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);

        if (actionStrLower.rfind("move", 0) != 0)
            return;

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
            };

            if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos)
                player = numFromString<int>(playerStr).unwrapOrDefault();

            moveRight = (dirStr == "right");

            if (!distStr.empty() && distStr.find_first_not_of("-0123456789.") == std::string::npos)
                distance = numFromString<float>(distStr).unwrapOrDefault();
        };

        auto popupMove = MoveSettingsPopup::create(
            player,
            moveRight,
            [popup, idx](int newPlayer, bool newMoveRight, float newDistance) {
                if (idx >= 0 && idx < static_cast<int>(popup->m_commandActions.size())) {
                    std::string dirStr = newMoveRight ? "right" : "left";
                    popup->m_commandActions[idx] = "move:" + std::to_string(newPlayer) + ":" + dirStr + ":" + std::to_string(newDistance);
                    popup->refreshActionsList();
                }
            });

        if (popupMove) {
            popupMove->setDistance(distance);

            if (auto input = popupMove->getDistanceInput()) {
                auto distBuf = fmt::format("{:.5f}", distance);
                input->setString(distBuf);
            };

            popupMove->show();
        };
    };

    void handleNotificationSettings(CommandSettingsPopup* popup, CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = 0;

        if (btn->getUserObject())
            idx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        std::string& actionStr = popup->m_commandActions[idx];
        std::string actionStrLower = actionStr;

        std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);

        if (actionStrLower.rfind("notification", 0) != 0)
            return;

        int iconTypeInt = 1;

        std::string notifText;

        size_t firstColon = actionStr.find(":");
        size_t secondColon = actionStr.find(":", firstColon + 1);

        if (firstColon != std::string::npos && secondColon != std::string::npos) {
            iconTypeInt = numFromString<int>(actionStr.substr(firstColon + 1, secondColon - firstColon - 1)).unwrapOrDefault();
            notifText = actionStr.substr(secondColon + 1);
        } else if (actionStr.length() > 13) {
            notifText = actionStr.substr(13);
        } else {
            notifText = "";
        };

        NotificationSettingsPopup::create(
            notifText,
            [popup, idx](const std::string& newText, NotificationIconType newIconType) {
                popup->updateNotificationNextTextLabel(idx, newText, newIconType);
                popup->refreshActionsList();
            },
            static_cast<NotificationIconType>(iconTypeInt))
            ->show();
    };

    // Process the scale player action settings
    void handleScalePlayerSettings(CommandSettingsPopup* parent, cocos2d::CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int actionIdx = 0;

        if (btn && btn->getUserObject())
            actionIdx = static_cast<CCInteger*>(btn->getUserObject())->getValue();

        if (actionIdx < 0 || actionIdx >= static_cast<int>(parent->m_commandActions.size()))
            return;

        std::string& actionStr = parent->m_commandActions[actionIdx];

        float scaleValue = 1.0f;
        float timeValue = 0.5f;

        size_t firstColon = actionStr.find(":");
        size_t secondColon = actionStr.find(":", firstColon + 1);

        if (firstColon != std::string::npos) {
            std::string scaleStr = actionStr.substr(firstColon + 1, (secondColon != std::string::npos ? secondColon - firstColon - 1 : std::string::npos));
            if (!scaleStr.empty()) {
                float parsed = numFromString<float>(scaleStr).unwrapOrDefault();
                if (parsed > 0.0f)
                    scaleValue = parsed;
            };

            if (secondColon != std::string::npos) {
                std::string timeStr = actionStr.substr(secondColon + 1);

                if (!timeStr.empty()) {
                    float parsedTime = numFromString<float>(timeStr).unwrapOrDefault();
                    if (parsedTime >= 0.0f)
                        timeValue = parsedTime;
                };
            };
        };

        auto popup = ScaleSettingsPopup::create(parent, actionIdx, scaleValue, timeValue, [parent, actionIdx](float newScale, float newTime) {
            // Only allow positive, non-zero scale values
            float safeScale = (newScale > 0.0f) ? newScale : 1.0f;
            float safeTime = (newTime >= 0.0f) ? newTime : 0.5f;

            parent->m_commandActions[actionIdx] = "scale_player:" + fmt::format("{:.2f}", safeScale) + ":" + fmt::format("{:.2f}", safeTime);
            parent->refreshActionsList();
                                                });

        if (popup) popup->show();
    };

    // Process the sound effect action settings
    void handleSoundEffectSettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int actionIdx = 0;

        if (btn && btn->getUserObject())
            actionIdx = static_cast<CCInteger*>(btn->getUserObject())->getValue();

        if (!popup || actionIdx < 0 || actionIdx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        // Parse current sound effect from m_commandActions[actionIdx]
        std::string actionIdRaw = popup->m_commandActions[actionIdx];
        std::string soundName = "secretKey.ogg";

        size_t colon = actionIdRaw.find(":");

        if (colon != std::string::npos && colon + 1 < actionIdRaw.size()) {
            soundName = actionIdRaw.substr(colon + 1);
            if (soundName.empty()) soundName = "secretKey.ogg";
        };

        // Show the SoundSettingsPopup and update the value and label on save
        auto popupWindow = SoundSettingsPopup::create(popup, actionIdx, soundName, [popup, actionIdx](const std::string& newSound) {
            popup->m_commandActions[actionIdx] = "sound_effect:" + newSound;
            popup->refreshActionsList();
                                                      });

        if (popupWindow) popupWindow->show();
    };
};