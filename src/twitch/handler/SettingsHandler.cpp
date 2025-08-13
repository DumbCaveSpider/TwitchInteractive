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
#include "JumpscareSettingsPopup.hpp"
#include "PlayerEffectSettingsPopup.hpp"

#include <algorithm>

using namespace cocos2d;

#include <Geode/utils/string.hpp>

namespace SettingsHandler {
    void handlePlayerEffectSettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = 0;
        if (btn && btn->getUserObject())
            idx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (!popup || idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        std::string& actionStr = popup->m_commandActions[idx];
        // Expect player_effect:<player>:<kind> or legacy player_effect:<kind>
        int player = 1;
        PlayerEffectType eff = PlayerEffectType::Death;
        size_t firstColon = actionStr.find(":");
        if (firstColon != std::string::npos) {
            std::string rest = actionStr.substr(firstColon + 1);
            size_t secondColon = rest.find(":");
            if (secondColon != std::string::npos) {
                std::string pStr = rest.substr(0, secondColon);
                std::string kind = rest.substr(secondColon + 1);
                if (!pStr.empty() && pStr.find_first_not_of("-0123456789") == std::string::npos)
                    player = numFromString<int>(pStr).unwrapOrDefault();
                geode::utils::string::toLowerIP(kind);
                eff = (kind == "spawn" ? PlayerEffectType::Spawn : PlayerEffectType::Death);
            } else {
                std::string kind = rest;
                geode::utils::string::toLowerIP(kind);
                eff = (kind == "spawn" ? PlayerEffectType::Spawn : PlayerEffectType::Death);
            }
        }

        auto win = PlayerEffectSettingsPopup::create(eff, player, [popup, idx](PlayerEffectType newEff, int newPlayer){
            std::string kind = (newEff == PlayerEffectType::Spawn) ? "spawn" : "death";
            // Always save with player index
            popup->m_commandActions[idx] = fmt::format("player_effect:{}:{}", std::max(1, std::min(2, newPlayer)), kind);
            popup->refreshActionsList();
        });
        if (win) win->show();
    }
    void handleJumpscareSettings(CommandSettingsPopup* popup, cocos2d::CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = 0;
        if (btn && btn->getUserObject())
            idx = static_cast<CCInteger*>(btn->getUserObject())->getValue();
        if (!popup || idx < 0 || idx >= static_cast<int>(popup->m_commandActions.size()))
            return;

        std::string& actionStr = popup->m_commandActions[idx];
        // Format: jumpscare:<url>:<fade>[:<scale>]
        std::string url;
        float fade = 0.5f;
        float scaleMul = 1.0f;
        size_t firstColon = actionStr.find(":");
        size_t secondColon = (firstColon != std::string::npos ? actionStr.find(":", firstColon + 1) : std::string::npos);
        size_t thirdColon = (secondColon != std::string::npos ? actionStr.find(":", secondColon + 1) : std::string::npos);
        if (firstColon != std::string::npos) {
            if (secondColon != std::string::npos) {
                url = actionStr.substr(firstColon + 1, secondColon - firstColon - 1);
                std::string rest = actionStr.substr(secondColon + 1);
                std::string fadeStr;
                std::string scaleStr;
                if (thirdColon != std::string::npos) {
                    fadeStr = actionStr.substr(secondColon + 1, thirdColon - secondColon - 1);
                    scaleStr = actionStr.substr(thirdColon + 1);
                } else {
                    fadeStr = rest;
                }
                if (!fadeStr.empty()) {
                    fadeStr.erase(0, fadeStr.find_first_not_of(" \t\n\r"));
                    fadeStr.erase(fadeStr.find_last_not_of(" \t\n\r") + 1);
                    if (auto parsed = numFromString<float>(fadeStr)) fade = parsed.unwrap();
                }
                if (!scaleStr.empty()) {
                    scaleStr.erase(0, scaleStr.find_first_not_of(" \t\n\r"));
                    scaleStr.erase(scaleStr.find_last_not_of(" \t\n\r") + 1);
                    if (auto parsed = numFromString<float>(scaleStr)) scaleMul = parsed.unwrap();
                }
            } else {
                url = actionStr.substr(firstColon + 1);
            }
        }

    auto popupWindow = JumpscareSettingsPopup::create(url, fade, scaleMul, [popup, idx](const std::string& newUrl, float newFade, float newScale) {
            std::string safeUrl = newUrl; // could be empty
            float safeFade = newFade;
            float safeScale = (newScale <= 0.f ? 1.f : newScale);
            popup->m_commandActions[idx] = fmt::format("jumpscare:{}:{:.2f}:{:.2f}", safeUrl, safeFade, safeScale);
            popup->refreshActionsList();
        });
        if (popupWindow) popupWindow->show();
    }
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
    geode::utils::string::toLowerIP(actionStrLower);

        float gravity = 1.0f;
        float duration = 2.0f;
        size_t firstColon = actionStr.find(":");
        size_t secondColon = actionStr.find(":", firstColon + 1);
        if (firstColon != std::string::npos && secondColon != std::string::npos) {
            std::string gravityStr = actionStr.substr(firstColon + 1, secondColon - firstColon - 1);
            std::string durationStr = actionStr.substr(secondColon + 1);
            if (!gravityStr.empty())
                gravityStr.erase(0, gravityStr.find_first_not_of(" \t\n\r"));
                gravityStr.erase(gravityStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedGravity = numFromString<float>(gravityStr);
                if (parsedGravity)
                    gravity = parsedGravity.unwrap();
            if (!durationStr.empty())
                durationStr.erase(0, durationStr.find_first_not_of(" \t\n\r"));
                durationStr.erase(durationStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedDuration = numFromString<float>(durationStr);
                if (parsedDuration)
                    duration = parsedDuration.unwrap();
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
                zoomStr.erase(0, zoomStr.find_first_not_of(" \t\n\r"));
                zoomStr.erase(zoomStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedZoom = numFromString<float>(zoomStr);
                if (parsedZoom)
                    zoom = parsedZoom.unwrap();
            if (!xStr.empty())
                xStr.erase(0, xStr.find_first_not_of(" \t\n\r"));
                xStr.erase(xStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedX = numFromString<float>(xStr);
                if (parsedX)
                    x = parsedX.unwrap();
            if (!yStr.empty())
                yStr.erase(0, yStr.find_first_not_of(" \t\n\r"));
                yStr.erase(yStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedY = numFromString<float>(yStr);
                if (parsedY)
                    y = parsedY.unwrap();
            if (!durStr.empty())
                durStr.erase(0, durStr.find_first_not_of(" \t\n\r"));
                durStr.erase(durStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedDur = numFromString<float>(durStr);
                if (parsedDur)
                    duration = parsedDur.unwrap();
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
                speedStr.erase(0, speedStr.find_first_not_of(" \t\n\r"));
                speedStr.erase(speedStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedSpeed = numFromString<float>(speedStr);
                if (parsedSpeed)
                    speed = parsedSpeed.unwrap();
            if (!durationStr.empty())
                durationStr.erase(0, durationStr.find_first_not_of(" \t\n\r"));
                durationStr.erase(durationStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedDur = numFromString<float>(durationStr);
                if (parsedDur)
                    duration = parsedDur.unwrap();
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

    geode::utils::string::toLowerIP(actionStrLower);

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

    geode::utils::string::toLowerIP(actionStrLower);

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

        // Extract key and duration separately
        std::string keyPart;
        std::string durationPart;
        size_t firstColon = actionStr.find(":");
        size_t secondColon = actionStr.find(":", firstColon + 1);

        if (firstColon != std::string::npos && secondColon != std::string::npos) {
            keyPart = actionStr.substr(firstColon + 1, secondColon - firstColon - 1);
            durationPart = actionStr.substr(secondColon + 1);
        } else {
            keyPart = actionStr.substr(firstColon + 1);
        }

        KeyCodesSettingsPopup::create(
            keyPart + ":" + durationPart,
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

    geode::utils::string::toLowerIP(actionStrLower);

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
                val.erase(0, val.find_first_not_of(" \t\n\r"));
                val.erase(val.find_last_not_of(" \t\n\r") + 1);
                auto parsedJump = numFromString<int>(val);
                if (parsedJump)
                    jumpPlayerValue = parsedJump.unwrap();
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

    geode::utils::string::toLowerIP(actionStrLower);

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
                playerStr.erase(0, playerStr.find_first_not_of(" \t\n\r"));
                playerStr.erase(playerStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedPlayer = numFromString<int>(playerStr);
                if (parsedPlayer)
                    player = parsedPlayer.unwrap();

            moveRight = (dirStr == "right");

            if (!distStr.empty() && distStr.find_first_not_of("-0123456789.") == std::string::npos)
                distStr.erase(0, distStr.find_first_not_of(" \t\n\r"));
                distStr.erase(distStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedDist = numFromString<float>(distStr);
                if (parsedDist)
                    distance = parsedDist.unwrap();
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

    geode::utils::string::toLowerIP(actionStrLower);

        if (actionStrLower.rfind("notification", 0) != 0)
            return;

        int iconTypeInt = 1;
        std::string notifText;
        float notifTime = 0.0f;

        size_t firstColon = actionStr.find(":");
        size_t secondColon = actionStr.find(":", firstColon + 1);
    size_t thirdColon = std::string::npos;

        if (firstColon != std::string::npos && secondColon != std::string::npos) {
            {
                std::string iconTypeStr = actionStr.substr(firstColon + 1, secondColon - firstColon - 1);
                iconTypeStr.erase(0, iconTypeStr.find_first_not_of(" \t\n\r"));
                iconTypeStr.erase(iconTypeStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedIcon = numFromString<int>(iconTypeStr);
                if (parsedIcon)
                    iconTypeInt = parsedIcon.unwrap();
            }
            // check for optional :time
            thirdColon = actionStr.rfind(":");
            if (thirdColon != std::string::npos && thirdColon > secondColon) {
                notifText = actionStr.substr(secondColon + 1, thirdColon - secondColon - 1);
                std::string timeStr = actionStr.substr(thirdColon + 1);
                timeStr.erase(0, timeStr.find_first_not_of(" \t\n\r"));
                timeStr.erase(timeStr.find_last_not_of(" \t\n\r") + 1);
                auto parsedTime = numFromString<float>(timeStr);
                if (parsedTime)
                    notifTime = parsedTime.unwrap();
            } else {
                notifText = actionStr.substr(secondColon + 1);
            }
        } else if (actionStr.length() > 13) {
            notifText = actionStr.substr(13);
        } else {
            notifText = "";
        };

        NotificationSettingsPopup::create(
            notifText,
            [popup, idx](const std::string& newText, NotificationIconType newIconType, float newTime) {
                popup->updateNotificationNextTextLabel(idx, newText, newIconType, newTime);
                popup->refreshActionsList();
            },
            static_cast<NotificationIconType>(iconTypeInt),
            notifTime)
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
                scaleStr.erase(0, scaleStr.find_first_not_of(" \t\n\r"));
                scaleStr.erase(scaleStr.find_last_not_of(" \t\n\r") + 1);
                auto parsed = numFromString<float>(scaleStr);
                if (parsed && parsed.unwrap() > 0.0f)
                    scaleValue = parsed.unwrap();
            };

            if (secondColon != std::string::npos) {
                std::string timeStr = actionStr.substr(secondColon + 1);

                if (!timeStr.empty()) {
                    timeStr.erase(0, timeStr.find_first_not_of(" \t\n\r"));
                    timeStr.erase(timeStr.find_last_not_of(" \t\n\r") + 1);
                    auto parsedTime = numFromString<float>(timeStr);
                    if (parsedTime && parsedTime.unwrap() >= 0.0f)
                        timeValue = parsedTime.unwrap();
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
        // Expect either sound_effect:<sound> or sound_effect:<sound>:<speed>:<volume>:<pitch>:<start>:<end>
        size_t firstColon = actionIdRaw.find(":");
        if (firstColon != std::string::npos && firstColon + 1 < actionIdRaw.size()) {
            std::string rest = actionIdRaw.substr(firstColon + 1);
            size_t nextColon = rest.find(":");
            if (nextColon == std::string::npos) {
                soundName = rest;
            } else {
                soundName = rest.substr(0, nextColon);
            }
            if (soundName.empty()) soundName = "secretKey.ogg";
        }

        // Show the SoundSettingsPopup and update the value and label on save
        auto popupWindow = SoundSettingsPopup::create(popup, actionIdx, soundName, [popup, actionIdx](const std::string& newParamPart) {
            // newParamPart is '<sound>:<speed>:<volume>:<pitch>:<start>:<end>'
            popup->m_commandActions[actionIdx] = "sound_effect:" + newParamPart;
            popup->refreshActionsList();
                                                      });

        if (popupWindow) popupWindow->show();
    };
};