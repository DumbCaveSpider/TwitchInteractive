#pragma once

#include <string>
#include <array>
#include <vector>
#include <functional>
#include <unordered_map>

#include <Geode/Geode.hpp>

#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>

using namespace geode::prelude;

// Enums for the type of callback
enum class CommandActionType {
    Notification = 0,
    Keybind = 1,
    Chat = 2,
    Event = 3,
    Wait = 4
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

    bool enabled = true; // If the command is enabled
    int cooldown = 0; // Cooldown in seconds

    std::function<void(const std::string&)> callback; // Custom callback
    std::string response; // Custom chat response

    TwitchCommand(
        const std::string& cmdName = "",
        const std::string& cmdDesc = "",
        const std::string& cmdResponse = "",
        int cmdCooldown = 0,
        std::vector<TwitchCommandAction> cmdActions = {}
    ) : name(cmdName), description(cmdDesc), response(cmdResponse), cooldown(cmdCooldown), actions(cmdActions) {};
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