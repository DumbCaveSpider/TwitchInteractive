#pragma once

#include <Geode/Geode.hpp>
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

using namespace geode::prelude;

struct TwitchCommand {
    std::string name;
    std::string description;
    std::string response;
    bool enabled = true;
    int cooldownSeconds = 0; // Cooldown in seconds
    std::function<void(const std::string&)> callback;

    TwitchCommand(const std::string& cmdName, const std::string& cmdDesc, const std::string& cmdResponse, int cooldown = 0)
        : name(cmdName), description(cmdDesc), response(cmdResponse), cooldownSeconds(cooldown) {}
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