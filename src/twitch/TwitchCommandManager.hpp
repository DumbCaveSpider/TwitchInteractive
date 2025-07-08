#pragma once

#include <Geode/Geode.hpp>
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>
#include <string>
#include <vector>
#include <functional>

using namespace geode::prelude;

struct TwitchCommand {
    std::string name;
    std::string description;
    std::string response;
    bool enabled = true;
    std::function<void(const std::string&)> callback;

    TwitchCommand(const std::string& cmdName, const std::string& cmdDesc, const std::string& cmdResponse)
        : name(cmdName), description(cmdDesc), response(cmdResponse) {}
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

    void startListening();
    void stopListening();
    bool isListening() const;

    void handleChatMessage(const ChatMessage& chatMessage);
};