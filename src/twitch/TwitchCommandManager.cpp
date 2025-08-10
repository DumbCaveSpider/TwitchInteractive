#include "TwitchCommandManager.hpp"

#include "TwitchDashboard.hpp"
#include "command/CommandSettingsPopup.hpp"

#include <algorithm>
#include <unordered_map>

#include <Geode/utils/file.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>

#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>
using namespace geode::prelude;

matjson::Value TwitchCommandAction::toJson() const
{
    matjson::Value v = matjson::Value::object();
    v["type"] = static_cast<int>(type);
    v["arg"] = arg;
    v["index"] = index;

    return v;
};

TwitchCommandAction TwitchCommandAction::fromJson(const matjson::Value &v)
{
    CommandActionType type = CommandActionType::Notification;
    std::string arg = "";
    float index = 0.f;

    if (v.contains("type") && v["type"].asInt().ok())
        type = static_cast<CommandActionType>(v["type"].asInt().unwrap());
    if (v.contains("arg") && v["arg"].asString().ok())
        arg = v["arg"].asString().unwrap();
    if (v.contains("index") && v["index"].isNumber())
    {
        auto result = v["index"].asDouble();
        if (result.ok())
            index = static_cast<float>(result.unwrap());
        else
            index = 0.f;
    };

    return TwitchCommandAction(type, arg, index);
};

// Save commands to file
void TwitchCommandManager::saveCommands()
{
    std::vector<matjson::Value> arrVec;

    for (const auto &cmd : m_commands)
        arrVec.push_back(cmd.toJson());

    matjson::Value arr(arrVec);
    std::string savePath = getSavePath();
    log::debug("[TwitchCommandManager] Saving commands to: {}", savePath);
    std::ofstream ofs(savePath);

    if (ofs)
        ofs << arr.dump(2);
};

// NOTE: Update TwitchCommand definition to use std::vector<TwitchCommandAction> for actions

void TwitchCommandManager::loadCommands()
{
    std::ifstream ifs(getSavePath());
    if (!ifs)
        return;

    auto result = matjson::parse(ifs);
    if (!result)
        return;

    auto arr = result.unwrap();
    if (!arr.isArray())
        return;

    m_commands.clear();
    for (size_t i = 0; i < arr.size(); ++i)
        m_commands.push_back(TwitchCommand::fromJson(arr[i]));
};

// Deserialize a TwitchCommand from matjson::Value
TwitchCommand TwitchCommand::fromJson(const matjson::Value &v)
{
    std::string name = (v.contains("name") && v["name"].asString().ok()) ? v["name"].asString().unwrap() : "";
    std::string description = (v.contains("description") && v["description"].asString().ok()) ? v["description"].asString().unwrap() : "";

    int cooldown = (v.contains("cooldown") && v["cooldown"].asInt().ok()) ? static_cast<int>(v["cooldown"].asInt().unwrap()) : 0;
    bool enabled = (v.contains("enabled") && v["enabled"].asBool().ok()) ? v["enabled"].asBool().unwrap() : true;
    bool showCooldown = (v.contains("showCooldown") && v["showCooldown"].asBool().ok()) ? v["showCooldown"].asBool().unwrap() : false;

    std::vector<TwitchCommandAction> actions;

    if (v.contains("actions") && v["actions"].isArray())
    {
        auto &actionsArr = v["actions"];
        for (size_t i = 0; i < actionsArr.size(); ++i)
            actions.push_back(TwitchCommandAction::fromJson(actionsArr[i]));
    };

    TwitchCommand cmd(name, description, cooldown, actions);
    cmd.enabled = enabled;
    cmd.showCooldown = showCooldown;
    return cmd;
};

std::string TwitchCommandManager::getSavePath() const
{
    // Use Geode's mod save directory for cross-platform compatibility
    std::string saveDir = geode::dirs::getModsSaveDir().string();
    if (!geode::utils::file::createDirectoryAll(saveDir))
    {
        log::warn("[TwitchCommandManager] Failed to create save directory: {}", saveDir);
    }
    return saveDir + "/commands.json";
}

matjson::Value TwitchCommand::toJson() const
{
    matjson::Value v = matjson::Value::object();
    v["name"] = name;
    v["description"] = description;
    v["cooldown"] = cooldown;
    v["enabled"] = enabled;
    v["showCooldown"] = showCooldown;
    std::vector<matjson::Value> actionsVec;
    for (const auto &action : actions)
        actionsVec.push_back(action.toJson());
    v["actions"] = matjson::Value(actionsVec);
    return v;
};

TwitchCommandManager *TwitchCommandManager::getInstance()
{
    static TwitchCommandManager instance;
    static bool loaded = false;

    if (!loaded)
    {
        auto configDirPath = geode::dirs::getModConfigDir();
        log::debug("[TwitchCommandManager] Mod config dir: \"{}\"", configDirPath.string());

        auto sfxDirPath = (configDirPath / "arcticwoof.twitch_interactive" / "sfx").string();
        if (!geode::utils::file::createDirectoryAll(sfxDirPath))
        {
            log::warn("[TwitchCommandManager] Failed to create sfx directory: \"{}\"", sfxDirPath);
        }
        else
        {
            log::debug("[TwitchCommandManager] Ensured sfx directory exists: \"{}\"", sfxDirPath);
        }

        instance.loadCommands();
        loaded = true;
    };

    return &instance;
};

void TwitchCommandManager::addCommand(const TwitchCommand &command)
{
    // Check if command already exists
    auto it = std::find_if(m_commands.begin(), m_commands.end(),
                           [&command](const TwitchCommand &cmd)
                           { return cmd.name == command.name; });

    if (it != m_commands.end())
    {
        *it = command;
        log::info("Updated command: {}", command.name);
    }
    else
    {
        m_commands.push_back(command);
        log::info("Added new command: {}", command.name);
    };

    saveCommands();
};

void TwitchCommandManager::removeCommand(const std::string &name)
{
    auto it = std::remove_if(m_commands.begin(), m_commands.end(),
                             [&name](const TwitchCommand &cmd)
                             { return cmd.name == name; });

    if (it != m_commands.end())
    {
        m_commands.erase(it, m_commands.end());
        log::info("Removed command: {}", name);
        saveCommands();
    }
    else
    {
        log::warn("Command '{}' not found for removal", name);
    };
};

void TwitchCommandManager::enableCommand(const std::string &name, bool enable)
{
    auto it = std::find_if(m_commands.begin(), m_commands.end(),
                           [&name](const TwitchCommand &cmd)
                           { return cmd.name == name; });

    if (it != m_commands.end())
    {
        it->enabled = enable;
        log::info("Command {} {}", name, enable ? "enabled" : "disabled");
        saveCommands();
    };
};

std::vector<TwitchCommand> &TwitchCommandManager::getCommands()
{
    return m_commands;
};

// Add a static map to track cooldowns for each command
std::unordered_map<std::string, time_t> commandCooldowns;

void resetCommandCooldown(const std::string &commandName)
{
    commandCooldowns.erase(commandName);
};

void TwitchCommandManager::handleChatMessage(const ChatMessage &chatMessage)
{
    // Check if CommandListen is enabled; if not, ignore all commands
    if (!TwitchDashboard::isListening())
    {
        log::info("[TwitchCommandManager] CommandListen is OFF. Ignoring all Twitch chat commands.");
        return;
    };

    std::string message = chatMessage.getMessage();
    std::string username = chatMessage.getUsername();
    std::string displayName = chatMessage.getDisplayName();
    std::string userID = chatMessage.getUserID();
    std::string messageID = chatMessage.getMessageID();

    // Log username and message ID whenever a message is received
    log::debug("Chat message received - Username: {}, Message ID: {}, Message: {}", username, messageID, message);

    std::string commandName;
    std::string commandArgs;

    size_t spacePos = message.find(' ');
    if (spacePos != std::string::npos)
    {
        commandName = message.substr(1, spacePos - 1);
        commandArgs = message.substr(spacePos + 1);
    }
    else
    {
        commandName = message.substr(1);
    };

    log::debug("Processing command: '{}' with args: '{}' from user: {}", commandName, commandArgs, username);

    // Find matching command
    auto it = std::find_if(m_commands.begin(), m_commands.end(),
                           [&commandName](const TwitchCommand &cmd)
                           {
                               return cmd.name == commandName && cmd.enabled;
                           });

    if (it != m_commands.end())
    {
        // Role restriction checks
        bool allowed = true;
        // If any role restriction is set, user must match at least one
        bool hasRoleRestriction = !it->allowedUser.empty() || it->allowMod || it->allowVip || it->allowSubscriber || it->allowStreamer;
        if (hasRoleRestriction)
        {
            allowed = false;
            // Check username restriction
            if (!it->allowedUser.empty() && chatMessage.getUsername() == it->allowedUser)
                allowed = true;

            // Check mod
            if (!allowed && it->allowMod && chatMessage.getIsMod())
                allowed = true;

            // Check VIP
            if (!allowed && it->allowVip && chatMessage.getIsVIP())
                allowed = true;

            // Check subscriber
            if (!allowed && it->allowSubscriber && chatMessage.getIsSubscriber())
                allowed = true;

            // Check streamer (require username matches the current channel/login name)
            if (!allowed && it->allowStreamer)
            {
                std::string channelName;

                if (auto twitchMod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api"))
                {
                    // The channel name is usually stored as 'twitch-channel' or similar
                    channelName = twitchMod->getSavedValue<std::string>("twitch-channel");

                    // Fallback: try 'twitch-username' if 'twitch-channel' is empty
                    if (channelName.empty())
                        channelName = twitchMod->getSavedValue<std::string>("twitch-username");
                };

                // Only allow if the user executing the command is the channel owner
                if (!channelName.empty() && chatMessage.getUsername() == channelName)
                    allowed = true;
            };
        };

        if (!allowed)
        {
            log::info("User '{}' is not allowed to execute command '{}' due to role restrictions.", username, commandName);
            return;
        };

        // Check cooldown
        time_t now = time(nullptr);
        auto cooldownIt = commandCooldowns.find(commandName);

        if (cooldownIt != commandCooldowns.end() && cooldownIt->second > now)
        {
            log::info("Command '{}' is currently on cooldown ({}s remaining)", commandName, cooldownIt->second - now);

            // Show cooldown notification if enabled
            bool showCooldown = it->showCooldown;
            // Fallback to the popup setting if open (for live preview/testing)
            if (!showCooldown)
            {
                if (auto *scene = CCDirector::sharedDirector()->getRunningScene())
                {
                    if (auto *popup = scene->getChildByID("command-settings-popup"))
                    {
                        if (auto *cmdPopup = dynamic_cast<CommandSettingsPopup *>(popup))
                        {
                            showCooldown = cmdPopup->getShowCooldown();
                        }
                    }
                }
            }
            if (showCooldown)
            {
                int seconds = static_cast<int>(cooldownIt->second - now);
                geode::Notification::create(fmt::format("{}: {}s cooldown", commandName, seconds))->show();
            }
            return;
        };

        // Set cooldown if needed
        if (it->cooldown > 0)
        {
            commandCooldowns[commandName] = now + it->cooldown;
            log::info("Command '{}' is now on cooldown for {}s", commandName, it->cooldown);
        };

        log::info("Executing command: {} for user: {} (Message ID: {})", commandName, username, messageID);

        // Notify dashboard to trigger cooldown for this command
        if (TwitchDashboard *dashboard = typeinfo_cast<TwitchDashboard *>(CCDirector::sharedDirector()->getRunningScene()->getChildByID("twitch-dashboard-popup")))
            dashboard->triggerCommandCooldown(commandName);

        // Collect all actions in order
        std::vector<TwitchCommandAction> orderedActions = it->actions;
        if (!orderedActions.empty())
        {
            // Debug log: print action order before execution (use ostringstream for MSVC compatibility)
            std::ostringstream orderLog;
            orderLog << "[TwitchCommandManager] Action order for command '" << commandName << "': ";
            for (size_t i = 0; i < orderedActions.size(); ++i)
            {
                const auto &a = orderedActions[i];
                orderLog << "[" << i << "] type=" << (int)a.type << ", arg=" << a.arg << ", index=" << a.index << "; ";
            };

            std::string orderLogStr = orderLog.str();
            log::debug("{}", orderLogStr);

            auto *ctx = new ActionContext();
            ctx->actions = orderedActions;
            ctx->index = 0;
            ctx->commandName = commandName;
            ctx->username = username;
            ctx->displayName = displayName;
            ctx->userID = userID;
            ctx->commandArgs = commandArgs;
            ctx->manager = this;

            ctx->execute(ctx);
        }

        // Execute command callback if it exists
        if (it->callback)
            it->callback(commandArgs);
    }
    else
    {
        log::debug("Command '{}' not found or disabled", commandName);
    };
};
TwitchCommandManager::~TwitchCommandManager()
{
    m_commands.clear();
    log::debug("TwitchCommandManager destructor called");
};