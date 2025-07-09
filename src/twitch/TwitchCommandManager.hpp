#pragma once

#include <Geode/Geode.hpp>
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>
#include <string>
#include <array>
#include <vector>
#include <functional>
#include <unordered_map>

using namespace geode::prelude;

// Enums for the type of callbacks
enum class CommandActionType {
    Notification = 0, // Show a notification
    Keybind = 1, // Fire a keybind
    Chat = 2 // Reply in chat with a message
};

// A quick command action
struct TwitchCommandAction {
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
    std::string name; // All lowercase name of the command
    std::string description; // Brief description of the command

    std::array<TwitchCommandAction, 10> actions = {}; // List of up to 10 actions in order

    bool enabled = true; // If the command is enabled
    int cooldown = 0; // Cooldown in seconds

    std::function<void(const std::string&)> callback; // Custom callback
    std::string response; // Custom chat response

    TwitchCommand(
        const std::string& cmdName,
        const std::string& cmdDesc,
        const std::string& cmdResponse,
        int cmdCooldown = 0,
        std::array<TwitchCommandAction, 10> cmdActions = {}
    ) : name(cmdName), description(cmdDesc), response(cmdResponse), cooldown(cmdCooldown), actions(cmdActions) {};
};

class TwitchCommandManager {
private:
    std::vector<TwitchCommand> m_commands;
    bool m_isListening = false;

public:
    static TwitchCommandManager* getInstance();
    ~TwitchCommandManager();

    void addCommand(const TwitchCommand& command);
    void removeCommand(const std::string& name);
    void enableCommand(const std::string& name, bool enable);
    std::vector<TwitchCommand>& getCommands();

    void handleChatMessage(const ChatMessage& chatMessage);
};

extern std::unordered_map<std::string, time_t> commandCooldowns;