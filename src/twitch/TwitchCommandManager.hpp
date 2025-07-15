#pragma once

#include "command/events/PlayLayerEvent.hpp"

#include <string>
#include <array>
#include <vector>
#include <functional>
#include <unordered_map>


#include <Geode/Geode.hpp>
#include <Geode/loader/dirs.hpp>
#include <Geode/utils/file.hpp>

#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>

using namespace geode::prelude;

// Command arguments
enum class CommandIdentifiers {
    Argument = 0,
    Username = 1,
    Displayname = 2,
    UserID = 3,
    Streamer = 4
};

// Enums for the type of callback
enum class CommandActionType {
    Notification = 0,
    Keybind = 1,
    Chat = 2,
    Event = 3,
    Wait = 4
};

// Helper for countdown logging
struct CountdownLogger : public CCObject {
    int remaining;
    std::string commandName;
    size_t actionIndex;

    CountdownLogger(int rem, const std::string& cmd, size_t idx) : remaining(rem), commandName(cmd), actionIndex(idx) {};

    void log(CCObject*) {
        log::info("[TwitchCommandManager] Wait countdown for command '{}', action {}: {} second(s) remaining", commandName, actionIndex, remaining);
    };
};

// A quick command action
struct TwitchCommandAction {
    matjson::Value toJson() const;
    static TwitchCommandAction fromJson(const matjson::Value& v);
    CommandActionType type = CommandActionType::Notification; // Type of callback
    std::string arg = ""; // A string to pass to the callback
    int index = 0; // Priority order

    TwitchCommandAction(
        CommandActionType actType = CommandActionType::Notification,
        const std::string& actArg = "",
        int actIndex = 0
    ) : type(actType), arg(actArg), index(actIndex) {};
};

// Template for a command
struct TwitchCommand {
    matjson::Value toJson() const;
    static TwitchCommand fromJson(const matjson::Value& v);
    std::string name; // All lowercase name of the command
    std::string description; // Brief description of the command


    std::vector<TwitchCommandAction> actions; // List of actions in order

    // User/role restrictions
    std::string allowedUser;
    bool allowVip = false;
    bool allowMod = false;
    bool allowStreamer = false;
    bool allowSubscriber = false;

    bool enabled = true; // If the command is enabled
    int cooldown = 0; // Cooldown in seconds

    std::function<void(const std::string&)> callback; // Custom callback
    std::string response; // Custom chat response

    TwitchCommand(
        const std::string& cmdName = "",
        const std::string& cmdDesc = "",
        const std::string& cmdResponse = "",
        int cmdCooldown = 0,
        std::vector<TwitchCommandAction> cmdActions = {},
        const std::string& allowedUser_ = "",
        bool allowVip_ = false,
        bool allowMod_ = false,
        bool allowStreamer_ = false,
        bool allowSubscriber_ = false
    ) : name(cmdName), description(cmdDesc), response(cmdResponse), cooldown(cmdCooldown), actions(cmdActions),
        allowedUser(allowedUser_), allowVip(allowVip_), allowMod(allowMod_), allowStreamer(allowStreamer_), allowSubscriber(allowSubscriber_) {};
};

class TwitchCommandManager {
private:
    std::vector<TwitchCommand> m_commands;
    bool m_isListening = false;
    void loadCommands();
    std::string getSavePath() const;

public:
    static TwitchCommandManager* getInstance();
    ~TwitchCommandManager();

    void addCommand(const TwitchCommand& command);
    void removeCommand(const std::string& name);
    void enableCommand(const std::string& name, bool enable);
    std::vector<TwitchCommand>& getCommands();

    void saveCommands();

    void handleChatMessage(const ChatMessage& chatMessage);
};

extern std::unordered_map<std::string, time_t> commandCooldowns;

// Sequential Action Execution
// Future: Use an enum for identifiers (e.g., ${arg}, ${username}, etc.) (here in this file btw! i think)
struct ActionContext : public CCObject {
    std::vector<TwitchCommandAction> actions;
    size_t index = 0;
    std::string commandName;
    std::string username;
    std::string displayName;
    std::string userID;
    std::string commandArgs;
    std::string streamerUsername;
    TwitchCommandManager* manager = nullptr;

    // Helper to replace identifiers in action arguments
    std::string replaceIdentifiers(const std::string& input) {
        std::string result = input;

        // Replace all ${arg} with commandArgs
        size_t pos = 0;
        while ((pos = result.find("${arg}", pos)) != std::string::npos) {
            result.replace(pos, 6, commandArgs);
            pos += commandArgs.length();
        };

        // Replace all ${username} with username
        pos = 0;
        while ((pos = result.find("${username}", pos)) != std::string::npos) {
            result.replace(pos, 11, username);
            pos += username.length();
        };

        // Replace all ${displayname} with displayName
        pos = 0;
        while ((pos = result.find("${displayname}", pos)) != std::string::npos) {
            result.replace(pos, 13, displayName);
            pos += displayName.length();
        };

        // Replace all ${userid} with userID
        pos = 0;
        while ((pos = result.find("${userid}", pos)) != std::string::npos) {
            result.replace(pos, 9, userID);
            pos += userID.length();
        };

        // Replace all ${streamer} with the configured Twitch channel (streamer's username)
        pos = 0;
        if (auto twitchMod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api")) {
            streamerUsername = twitchMod->getSavedValue<std::string>("twitch-channel");
        };

        while ((pos = result.find("${streamer}", pos)) != std::string::npos) {
            result.replace(pos, 11, streamerUsername);
            pos += streamerUsername.length();
        };

        return result;
    };

    void execute(CCObject* obj) {
        auto* ctx = as<ActionContext*>(obj);
        if (!ctx || ctx->index >= ctx->actions.size()) {
            if (ctx) ctx->release();
            return;
        };

        // Debug log: print the full action order and current action
        std::ostringstream orderLog;
        orderLog << "[TwitchCommandManager] Action order for command '" << ctx->commandName << "': ";
        for (size_t i = 0; i < ctx->actions.size(); ++i) {
            const auto& a = ctx->actions[i];
            orderLog << "[" << i << "] type=" << (int)a.type << ", arg=" << a.arg << ", index=" << a.index;
            if (i == ctx->index) orderLog << " <-- executing";
            orderLog << "; ";
        };

        log::debug("{}", orderLog.str());

        const auto& action = ctx->actions[ctx->index];
        // Replace identifiers in action.arg before use
        std::string processedArg = ctx->replaceIdentifiers(action.arg);
        log::info("[TwitchCommandManager] Executing action {}: type={}, arg={}, index={}", ctx->index, (int)action.type, processedArg, action.index);
        if (action.type == CommandActionType::Wait) {
            int delay = action.index;
            if (delay > 0) {
                log::info("[TwitchCommandManager] Waiting for {} seconds before next action (command '{}', action {})", delay, ctx->commandName, ctx->index);
                // Countdown log for each second using CCCallFuncO, only for this wait action
                if (auto scene = CCDirector::sharedDirector()->getRunningScene()) {
                    for (int i = 1; i <= delay; ++i) {
                        auto logger = new CountdownLogger(delay - i + 1, ctx->commandName, ctx->index);
                        scene->runAction(CCSequence::create(
                            CCDelayTime::create(as<float>(i)),
                            CCCallFuncO::create(logger, callfuncO_selector(CountdownLogger::log), logger),
                            nullptr
                        ));
                    };
                };

                ctx->index++;
                auto seq = CCSequence::create(
                    CCDelayTime::create(as<float>(delay)),
                    CCCallFuncO::create(ctx, callfuncO_selector(ActionContext::execute), ctx),
                    nullptr
                );

                if (auto scene = CCDirector::sharedDirector()->getRunningScene()) {
                    ctx->retain(); // Retain for the callback
                    scene->runAction(seq);
                } else {
                    ctx->execute(ctx);
                };

                return;
            };
        };

        if (action.type == CommandActionType::Event) {
            if (processedArg == "kill_player") {
                log::info("[TwitchCommandManager] Triggering kill player event for command: {}", ctx->commandName);
                PlayLayerEvent::killPlayer();
            } else if (processedArg.rfind("edit_camera:", 0) == 0) {
                // Format: edit_camera:<skew>:<rot>:<scale>:<time>
                log::info("[TwitchCommandManager] Triggering edit camera event: {}", processedArg);
                PlayLayerEvent::setCameraFromString(processedArg);
            } else if (processedArg.rfind("jump:", 0) == 0) {
                // Format: jump:<playerIdx>:hold|tap
                int playerIdx = 1;
                bool hold = false;
                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos) {
                    std::string playerStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string typeStr = processedArg.substr(secondColon + 1);
                    if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos) {
                        playerIdx = std::stoi(playerStr);
                    }
                    if (typeStr == "hold") hold = true;
                }
                if (hold) {
                    PlayLayerEvent::jumpPlayerHold(playerIdx);
                } else {
                    PlayLayerEvent::jumpPlayerTap(playerIdx);
                }
            } else if (processedArg.rfind("move:", 0) == 0) {
                // Format: move:<playerIdx>:left|right:<distance>
                int playerIdx = 1;
                bool moveRight = true;
                float distance = 0.f;
                bool validDistance = false;
                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);
                size_t thirdColon = processedArg.find(":", secondColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos && thirdColon != std::string::npos) {
                    std::string playerStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string dirStr = processedArg.substr(secondColon + 1, thirdColon - secondColon - 1);
                    std::string distStr = processedArg.substr(thirdColon + 1);
                    if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos) {
                        playerIdx = std::stoi(playerStr);
                    }
                    if (dirStr == "left") moveRight = false;
                    if (!distStr.empty() && distStr.find_first_not_of("-.0123456789") == std::string::npos) {
                        distance = std::stof(distStr);
                        validDistance = true;
                    }
                }
                if (!validDistance) {
                    log::warn("[TwitchCommandManager] Ignoring move action: invalid distance value (command: {})", ctx->commandName);
                } else {
                    log::info("[TwitchCommandManager] Triggering move event for player {} direction {} distance {} (command: {})", playerIdx, moveRight ? "right" : "left", distance, ctx->commandName);
                    PlayLayerEvent::movePlayer(playerIdx, moveRight, distance);
                }
            } else if (processedArg.rfind("color_player:", 0) == 0) {
                // Format: color_player:<playerIdx>:R,G,B
                int playerIdx = 1;
                std::string colorStr;
                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos) {
                    std::string playerStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    colorStr = processedArg.substr(secondColon + 1);
                    if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos) {
                        playerIdx = std::stoi(playerStr);
                    }
                } else if (firstColon != std::string::npos) {
                    colorStr = processedArg.substr(firstColon + 1);
                }
                cocos2d::ccColor3B color = parseColorString(colorStr);
                log::info("[TwitchCommandManager] Setting color for player {} to {} (command: {})", playerIdx, colorStr, ctx->commandName);
                PlayLayerEvent::setPlayerColor(playerIdx, color);
            } else if (processedArg.rfind("keycode:", 0) == 0) {
                // Format: keycode:<key>:<duration>
                std::string keyStr;
                float duration = 0.f;
                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos) {
                    keyStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string durStr = processedArg.substr(secondColon + 1);
                    if (!durStr.empty() && durStr.find_first_not_of("-.0123456789") == std::string::npos) {
                        duration = std::stof(durStr);
                    }
                } else if (firstColon != std::string::npos) {
                    keyStr = processedArg.substr(firstColon + 1);
                }
                PlayLayerEvent::pressKey(keyStr, duration);
            } else if (processedArg.rfind("profile:", 0) == 0) {
                // Format: profile:<accountIdInt>
                int accountIdInt = 0;
                size_t firstColon = processedArg.find(":");
                if (firstColon != std::string::npos) {
                    std::string idStr = processedArg.substr(firstColon + 1);
                    if (!idStr.empty() && idStr.find_first_not_of("-0123456789") == std::string::npos) {
                        accountIdInt = std::stoi(idStr);
                    }
                }
                if (auto page = ProfilePage::create(accountIdInt, false)) page->show();
            }
        };

        if (action.type == CommandActionType::Notification) {
            // Always parse notification action as "notification:<iconInt>:<text>" (force lowercase prefix)
            int iconTypeInt = 1;
            std::string notifText;
            std::string argStr = action.arg;
            // Always force prefix to lowercase for parsing
            if (argStr.size() >= 13) {
                std::string prefix = argStr.substr(0, 13);
                std::string prefixLower = prefix;
                std::transform(prefixLower.begin(), prefixLower.end(), prefixLower.begin(), ::tolower);
                if (prefixLower == "notification:") {
                    // Remove prefix
                    std::string rest = argStr.substr(13); // after 'notification:'
                    size_t colonPos = rest.find(":");
                    if (colonPos != std::string::npos) {
                        std::string iconPart = rest.substr(0, colonPos);
                        if (!iconPart.empty() && iconPart.find_first_not_of("0123456789") == std::string::npos) {
                            iconTypeInt = std::stoi(iconPart);
                            notifText = rest.substr(colonPos + 1);
                        } else {
                            notifText = rest.substr(colonPos + 1);
                        }
                    } else {
                        notifText = rest;
                    }
                } else {
                    // Fallback: try to parse as <iconInt>:<text>
                    size_t colonPos = argStr.find(":");
                    if (colonPos != std::string::npos) {
                        std::string iconPart = argStr.substr(0, colonPos);
                        if (!iconPart.empty() && iconPart.find_first_not_of("0123456789") == std::string::npos) {
                            iconTypeInt = std::stoi(iconPart);
                            notifText = argStr.substr(colonPos + 1);
                        } else {
                            notifText = argStr.substr(colonPos + 1);
                        }
                    } else {
                        notifText = argStr;
                    }
                }
            } else {
                // Fallback: try to parse as <iconInt>:<text>
                size_t colonPos = argStr.find(":");
                if (colonPos != std::string::npos) {
                    std::string iconPart = argStr.substr(0, colonPos);
                    if (!iconPart.empty() && iconPart.find_first_not_of("0123456789") == std::string::npos) {
                        iconTypeInt = std::stoi(iconPart);
                        notifText = argStr.substr(colonPos + 1);
                    } else {
                        notifText = argStr.substr(colonPos + 1);
                    }
                } else {
                    notifText = argStr;
                }
            }
            // Replace identifiers in notifText
            notifText = ctx->replaceIdentifiers(notifText);
            // Trim whitespace
            notifText.erase(0, notifText.find_first_not_of(" \t\n\r"));
            notifText.erase(notifText.find_last_not_of(" \t\n\r") + 1);
            NotificationIcon icon = NotificationIcon::Info;
            switch (iconTypeInt) {
            case 0: icon = NotificationIcon::None; break;
            case 1: icon = NotificationIcon::Info; break;
            case 2: icon = NotificationIcon::Success; break;
            case 3: icon = NotificationIcon::Warning; break;
            case 4: icon = NotificationIcon::Error; break;
            case 5: icon = NotificationIcon::Loading; break;
            }
            log::info("[TwitchCommandManager] Showing notification: {} (icon: {}, command: {})", notifText, iconTypeInt, ctx->commandName);
            Notification::create(notifText, icon)->show();
        }

        // Add more action types here as needed
        ctx->index++;
        ctx->execute(ctx);
    };
};