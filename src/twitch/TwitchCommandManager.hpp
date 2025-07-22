#pragma once

#include "command/events/PlayLayerEvent.hpp"

#include <string>
#include <array>
#include <vector>
#include <functional>
#include <unordered_map>

#include <Geode/Geode.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/utils/file.hpp>

#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>

using namespace geode::prelude;

// Command arguments
enum class CommandIdentifiers
{
    Argument = 0,
    Username = 1,
    Displayname = 2,
    UserID = 3,
    Streamer = 4
};

// Enums for the type of callback
enum class CommandActionType
{
    Notification = 0,
    Keybind = 1,
    Chat = 2,
    Event = 3,
    Wait = 4
};

// Helper for countdown logging
struct CountdownLogger : public CCObject
{
    float remaining;
    std::string commandName;
    size_t actionIndex;

    CountdownLogger(float rem, const std::string &cmd, size_t idx) : remaining(rem), commandName(cmd), actionIndex(idx) {};

    void log(CCObject *)
    {
        log::info("Wait countdown for command '{}', action {}: {:.2f} second(s) remaining", commandName, actionIndex, remaining);
    };
};

// A quick command action
struct TwitchCommandAction
{
    matjson::Value toJson() const;
    static TwitchCommandAction fromJson(const matjson::Value &v);
    CommandActionType type = CommandActionType::Notification; // Type of callback
    std::string arg = "";                                     // A string to pass to the callback
    float index = 0.f;                                        // Priority order

    TwitchCommandAction(
        CommandActionType actType = CommandActionType::Notification,
        const std::string &actArg = "",
        float actIndex = 0.f) : type(actType), arg(actArg), index(actIndex) {};
};

// Template for a command
struct TwitchCommand
{
    matjson::Value toJson() const;
    static TwitchCommand fromJson(const matjson::Value &v);
    std::string name;        // All lowercase name of the command
    std::string description; // Brief description of the command

    std::vector<TwitchCommandAction> actions; // List of actions in order

    // User/role restrictions
    std::string allowedUser;
    bool allowVip = false;
    bool allowMod = false;
    bool allowStreamer = false;
    bool allowSubscriber = false;

    bool enabled = true; // If the command is enabled
    int cooldown = 0;    // Cooldown in seconds

    std::function<void(const std::string &)> callback; // Custom callback

    TwitchCommand(
        const std::string &cmdName = "",
        const std::string &cmdDesc = "",
        int cmdCooldown = 0,
        std::vector<TwitchCommandAction> cmdActions = {},
        const std::string &allowedUser_ = "",
        bool allowVip_ = false,
        bool allowMod_ = false,
        bool allowStreamer_ = false,
        bool allowSubscriber_ = false) : name(cmdName), description(cmdDesc), cooldown(cmdCooldown), actions(cmdActions),
                                         allowedUser(allowedUser_), allowVip(allowVip_), allowMod(allowMod_), allowStreamer(allowStreamer_), allowSubscriber(allowSubscriber_) {};
};

class TwitchCommandManager
{
private:
    std::vector<TwitchCommand> m_commands;
    bool m_isListening = false;
    void loadCommands();
    std::string getSavePath() const;

public:
    static TwitchCommandManager *getInstance();
    ~TwitchCommandManager();

    void addCommand(const TwitchCommand &command);
    void removeCommand(const std::string &name);
    void enableCommand(const std::string &name, bool enable);
    std::vector<TwitchCommand> &getCommands();

    void saveCommands();

    void handleChatMessage(const ChatMessage &chatMessage);
};

extern std::unordered_map<std::string, time_t> commandCooldowns;

// Sequential Action Execution
// Future: Use an enum for identifiers (e.g., ${arg}, ${username}, etc.) (here in this file btw! i think)
struct ActionContext : public CCObject
{
    std::vector<TwitchCommandAction> actions;
    size_t index = 0;
    std::string commandName;
    std::string username;
    std::string displayName;
    std::string userID;
    std::string commandArgs;
    std::string streamerUsername;
    TwitchCommandManager *manager = nullptr;

    // Helper to replace identifiers in action arguments
    std::string replaceIdentifiers(const std::string &input)
    {
        std::string result = input;

        // Replace all ${arg} with commandArgs
        size_t pos = 0;
        while ((pos = result.find("${arg}", pos)) != std::string::npos)
        {
            result.replace(pos, 6, commandArgs);
            pos += commandArgs.length();
        };

        // Replace all ${username} with username
        pos = 0;
        while ((pos = result.find("${username}", pos)) != std::string::npos)
        {
            result.replace(pos, 11, username);
            pos += username.length();
        };

        // Replace all ${displayname} with displayName
        pos = 0;
        while ((pos = result.find("${displayname}", pos)) != std::string::npos)
        {
            result.replace(pos, 13, displayName);
            pos += displayName.length();
        };

        // Replace all ${userid} with userID
        pos = 0;
        while ((pos = result.find("${userid}", pos)) != std::string::npos)
        {
            result.replace(pos, 9, userID);
            pos += userID.length();
        };

        // Replace all ${streamer} with the configured Twitch channel (streamer's username)
        pos = 0;
        if (auto twitchMod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api"))
        {
            streamerUsername = twitchMod->getSavedValue<std::string>("twitch-channel");
        };

        while ((pos = result.find("${streamer}", pos)) != std::string::npos)
        {
            result.replace(pos, 11, streamerUsername);
            pos += streamerUsername.length();
        };

        return result;
    };

    void execute(CCObject *obj)
    {
        auto *ctx = as<ActionContext *>(obj);
        if (!ctx || ctx->index >= ctx->actions.size())
        {
            if (ctx)
                ctx->release();
            return;
        }

        // Debug log: print the full action order and current action
        std::ostringstream orderLog;
        orderLog << "Action order for command '" << ctx->commandName << "': ";
        for (size_t i = 0; i < ctx->actions.size(); ++i)
        {
            const auto &a = ctx->actions[i];
            orderLog << "[" << i << "] type=" << (int)a.type << ", arg=" << a.arg << ", index=" << a.index;
            if (i == ctx->index)
                orderLog << " <-- executing";
            orderLog << "; ";
        }
        log::debug("{}", orderLog.str());

        const auto &action = ctx->actions[ctx->index];
        std::string processedArg = ctx->replaceIdentifiers(action.arg);
        log::info("Executing action {}: type={}, arg={}, index={}", ctx->index, (int)action.type, processedArg, action.index);

        // Handle Wait type
        if (action.type == CommandActionType::Wait)
        {
            float delay = action.index;
            if (delay <= 0.f && !processedArg.empty() && processedArg.find_first_not_of("-.0123456789") == std::string::npos)
            {
                delay = std::stof(processedArg);
            }
            delay = std::round(delay * 1000.0f) / 1000.0f;
            if (delay > 0.f)
            {
                log::info("Waiting for {:.2f} seconds before next action (command '{}', action {})", delay, ctx->commandName, ctx->index);
                if (auto scene = CCDirector::sharedDirector()->getRunningScene())
                {
                    int intDelay = static_cast<int>(delay);
                    for (int i = 1; i <= intDelay; ++i)
                    {
                        auto logger = new CountdownLogger(intDelay - i + 1, ctx->commandName, ctx->index);
                        scene->runAction(CCSequence::create(
                            CCDelayTime::create(static_cast<float>(i)),
                            CCCallFuncO::create(logger, callfuncO_selector(CountdownLogger::log), logger),
                            nullptr));
                    }
                }
                ctx->index++;
                auto seq = CCSequence::create(
                    CCDelayTime::create(delay),
                    CCCallFuncO::create(ctx, callfuncO_selector(ActionContext::execute), ctx),
                    nullptr);
                if (auto scene = CCDirector::sharedDirector()->getRunningScene())
                {
                    ctx->retain();
                    scene->runAction(seq);
                }
                else
                {
                    ctx->execute(ctx);
                }
                return;
            }
        }

        // Handle other Event types
        if (action.type == CommandActionType::Event)
        {
            if (processedArg == "kill_player")
            {
                log::info("Triggering kill player event for command: {}", ctx->commandName);
                PlayLayerEvent::killPlayer();
            }
            else if (processedArg == "reverse_player")
            {
                log::info("Triggering reverse player event for command: {}", ctx->commandName);
                PlayLayerEvent::reversePlayer();
            }
            else if (processedArg.rfind("edit_camera:", 0) == 0)
            {
                log::info("Triggering edit camera event: {}", processedArg);
                PlayLayerEvent::setCameraFromString(processedArg);
            }
            else if (processedArg.rfind("sound_effect:", 0) == 0 || processedArg.rfind("sound:", 0) == 0)
            {
                std::string soundName = "";
                size_t colonPos = processedArg.find(":");
                if (colonPos != std::string::npos && colonPos + 1 < processedArg.size())
                    soundName = processedArg.substr(colonPos + 1);
                if (!soundName.empty())
                {
                    log::info("Playing sound effect '{}' (command: {})", soundName, ctx->commandName);
                    auto audioEngine = FMODAudioEngine::sharedEngine();
                    if (audioEngine != nullptr)
                        audioEngine->playEffect(soundName);
                }
                else
                {
                    log::warn("Sound effect action triggered but no sound name set (command: {})", ctx->commandName);
                }
            }
            else if (processedArg.rfind("scale_player:", 0) == 0)
            {
                int playerIdx = 1;
                float scale = 1.0f;
                float time = 0.0f;
                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);
                size_t thirdColon = (secondColon != std::string::npos) ? processedArg.find(":", secondColon + 1) : std::string::npos;
                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string scaleStr, timeStr;
                    // If three colons, treat as scale_player:<player>:<scale>:<time>
                    if (thirdColon != std::string::npos)
                    {
                        std::string playerStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                        scaleStr = processedArg.substr(secondColon + 1, thirdColon - secondColon - 1);
                        timeStr = processedArg.substr(thirdColon + 1);
                        if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos)
                            playerIdx = std::stoi(playerStr);
                        if (!scaleStr.empty() && scaleStr.find_first_not_of("-.0123456789") == std::string::npos)
                            scale = std::stof(scaleStr);
                        if (!timeStr.empty() && timeStr.find_first_not_of("-.0123456789") == std::string::npos)
                            time = std::stof(timeStr);
                    }
                    else
                    {
                        // scale_player:<scale>:<time> (no player index)
                        scaleStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                        timeStr = processedArg.substr(secondColon + 1);
                        if (!scaleStr.empty() && scaleStr.find_first_not_of("-.0123456789") == std::string::npos)
                            scale = std::stof(scaleStr);
                        if (!timeStr.empty() && timeStr.find_first_not_of("-.0123456789") == std::string::npos)
                            time = std::stof(timeStr);
                    }
                }
                else if (firstColon != std::string::npos)
                {
                    std::string scaleStr = processedArg.substr(firstColon + 1);
                    if (!scaleStr.empty() && scaleStr.find_first_not_of("-.0123456789") == std::string::npos)
                        scale = std::stof(scaleStr);
                }
                log::info("Setting scale for player {} to {} (time: {}, command: {})", playerIdx, scale, time, ctx->commandName);
                PlayLayerEvent::scalePlayer(playerIdx, scale, time);
            }
            else if (processedArg.rfind("alert_popup:", 0) == 0)
            {
                std::string title = "-", desc = "-";
                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    title = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    desc = processedArg.substr(secondColon + 1);
                    if (title.empty())
                        title = "-";
                    if (desc.empty())
                        desc = "-";
                }
                log::info("Showing alert popup: title='{}', desc='{}' (command: {})", title, desc, ctx->commandName);
                FLAlertLayer::create(title.c_str(), desc.c_str(), "OK")->show();
            }
            else if (processedArg == "stop_all_sounds")
            {
                log::info("Stopping all sound effects (command: {})", ctx->commandName);
                auto audioEngine = FMODAudioEngine::sharedEngine();
                if (audioEngine != nullptr)
                    audioEngine->stopAllEffects();
            }
            else if (processedArg.rfind("jump:", 0) == 0)
            {
                int playerIdx = 1;
                bool hold = false;
                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string playerStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string typeStr = processedArg.substr(secondColon + 1);
                    if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos)
                        playerIdx = std::stoi(playerStr);
                    if (typeStr == "hold")
                        hold = true;
                }
                if (hold)
                {
                    PlayLayerEvent::jumpPlayerHold(playerIdx);
                }
                else
                {
                    PlayLayerEvent::jumpPlayerTap(playerIdx);
                }
            }
            else if (processedArg.rfind("move:", 0) == 0)
            {
                int playerIdx = 1;
                bool moveRight = true;
                float distance = 0.f;
                bool validDistance = false;
                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);
                size_t thirdColon = processedArg.find(":", secondColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos && thirdColon != std::string::npos)
                {
                    std::string playerStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string dirStr = processedArg.substr(secondColon + 1, thirdColon - secondColon - 1);
                    std::string distStr = processedArg.substr(thirdColon + 1);
                    if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos)
                        playerIdx = std::stoi(playerStr);
                    if (dirStr == "left")
                        moveRight = false;
                    if (!distStr.empty() && distStr.find_first_not_of("-.0123456789") == std::string::npos)
                    {
                        distance = std::stof(distStr);
                        validDistance = true;
                    }
                }
                if (!validDistance)
                {
                    log::warn("Ignoring move action: invalid distance value (command: {})", ctx->commandName);
                }
                else
                {
                    log::info("Triggering move event for player {} direction {} distance {} (command: {})", playerIdx, moveRight ? "right" : "left", distance, ctx->commandName);
                    PlayLayerEvent::movePlayer(playerIdx, moveRight, distance);
                }
            }
            else if (processedArg.rfind("color_player:", 0) == 0)
            {
                int playerIdx = 1;
                std::string colorStr;
                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string playerStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    colorStr = processedArg.substr(secondColon + 1);
                    if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos)
                        playerIdx = std::stoi(playerStr);
                }
                else if (firstColon != std::string::npos)
                {
                    colorStr = processedArg.substr(firstColon + 1);
                }
                cocos2d::ccColor3B color = parseColorString(colorStr);
                log::info("Setting color for player {} to {} (command: {})", playerIdx, colorStr, ctx->commandName);
                PlayLayerEvent::setPlayerColor(playerIdx, color);
            }
            else if (processedArg.rfind("profile:", 0) == 0)
            {
                int accountIdInt = 0;
                size_t firstColon = processedArg.find(":");
                if (firstColon != std::string::npos)
                {
                    std::string idStr = processedArg.substr(firstColon + 1);
                    if (!idStr.empty() && idStr.find_first_not_of("-0123456789") == std::string::npos)
                        accountIdInt = std::stoi(idStr);
                }
                if (auto page = ProfilePage::create(accountIdInt, false))
                    page->show();
            }
        }

        // Handle Notification type
        if (action.type == CommandActionType::Notification)
        {
            int iconTypeInt = 1;
            std::string notifText;
            std::string argStr = action.arg;
            if (argStr.size() >= 13)
            {
                std::string prefix = argStr.substr(0, 13);
                std::string prefixLower = prefix;
                std::transform(prefixLower.begin(), prefixLower.end(), prefixLower.begin(), ::tolower);
                if (prefixLower == "notification:")
                {
                    std::string rest = argStr.substr(13);
                    size_t colonPos = rest.find(":");
                    if (colonPos != std::string::npos)
                    {
                        std::string iconPart = rest.substr(0, colonPos);
                        if (!iconPart.empty() && iconPart.find_first_not_of("0123456789") == std::string::npos)
                        {
                            iconTypeInt = std::stoi(iconPart);
                            notifText = rest.substr(colonPos + 1);
                        }
                        else
                        {
                            notifText = rest.substr(colonPos + 1);
                        }
                    }
                    else
                    {
                        notifText = rest;
                    }
                }
                else
                {
                    size_t colonPos = argStr.find(":");
                    if (colonPos != std::string::npos)
                    {
                        std::string iconPart = argStr.substr(0, colonPos);
                        if (!iconPart.empty() && iconPart.find_first_not_of("0123456789") == std::string::npos)
                        {
                            iconTypeInt = std::stoi(iconPart);
                            notifText = argStr.substr(colonPos + 1);
                        }
                        else
                        {
                            notifText = argStr.substr(colonPos + 1);
                        }
                    }
                    else
                    {
                        notifText = argStr;
                    }
                }
            }
            else
            {
                size_t colonPos = argStr.find(":");
                if (colonPos != std::string::npos)
                {
                    std::string iconPart = argStr.substr(0, colonPos);
                    if (!iconPart.empty() && iconPart.find_first_not_of("0123456789") == std::string::npos)
                    {
                        iconTypeInt = std::stoi(iconPart);
                        notifText = argStr.substr(colonPos + 1);
                    }
                    else
                    {
                        notifText = argStr.substr(colonPos + 1);
                    }
                }
                else
                {
                    notifText = argStr;
                }
            }
            notifText = ctx->replaceIdentifiers(notifText);
            notifText.erase(0, notifText.find_first_not_of(" \t\n\r"));
            notifText.erase(notifText.find_last_not_of(" \t\n\r") + 1);
            NotificationIcon icon = NotificationIcon::Info;
            switch (iconTypeInt)
            {
            case 0:
                icon = NotificationIcon::None;
                break;
            case 1:
                icon = NotificationIcon::Info;
                break;
            case 2:
                icon = NotificationIcon::Success;
                break;
            case 3:
                icon = NotificationIcon::Warning;
                break;
            case 4:
                icon = NotificationIcon::Error;
                break;
            case 5:
                icon = NotificationIcon::Loading;
                break;
            }
            log::info("Showing notification: {} (icon: {}, command: {})", notifText, iconTypeInt, ctx->commandName);
            Notification::create(notifText, icon)->show();
        }

        // Add more action types here as needed
        ctx->index++;
        ctx->execute(ctx);
    }
};