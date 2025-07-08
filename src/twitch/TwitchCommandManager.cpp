#include "TwitchCommandManager.hpp"

#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>
#include <algorithm>

using namespace geode::prelude;

TwitchCommandManager* TwitchCommandManager::getInstance() {
    static TwitchCommandManager instance;
    return &instance;
};

void TwitchCommandManager::addCommand(const TwitchCommand& command) {
    // Check if command already exists
    auto it = std::find_if(m_commands.begin(), m_commands.end(),
                           [&command](const TwitchCommand& cmd) { return cmd.name == command.name; });

    if (it != m_commands.end()) {
        // Update existing command
        *it = command;
        log::info("Updated command: {}", command.name);
    } else {
        // Add new command
        m_commands.push_back(command);
        log::info("Added new command: {}", command.name);
    };
};

void TwitchCommandManager::removeCommand(const std::string& name) {
    size_t initialSize = m_commands.size();
    auto it = std::remove_if(m_commands.begin(), m_commands.end(),
                             [&name](const TwitchCommand& cmd) { return cmd.name == name; });

    if (it != m_commands.end()) {
        m_commands.erase(it, m_commands.end());
        log::info("Removed command: {} (removed {} command(s))", name, initialSize - m_commands.size());
    } else {
        log::warn("Command '{}' not found for removal", name);
    };
};

void TwitchCommandManager::enableCommand(const std::string& name, bool enable) {
    auto it = std::find_if(m_commands.begin(), m_commands.end(),
                           [&name](const TwitchCommand& cmd) { return cmd.name == name; });

    if (it != m_commands.end()) {
        it->enabled = enable;
        log::info("Command {} {}", name, enable ? "enabled" : "disabled");
    };
};

std::vector<TwitchCommand>& TwitchCommandManager::getCommands() {
    return m_commands;
};

void TwitchCommandManager::startListening() {
    if (m_isListening) {
        log::warn("Already listening to chat messages");
        return;
    };

    auto api = TwitchChatAPI::get();
    if (!api) {
        log::error("TwitchChatAPI is not available for command listening");
        return;
    };

    // Register callback for chat messages
    api->registerOnMessageCallback([this](const ChatMessage& chatMessage) {
        handleChatMessage(chatMessage);
                                   });

    m_isListening = true;
    log::info("Started listening for Twitch chat commands");
};

void TwitchCommandManager::stopListening() {
    if (!m_isListening) {
        return;
    };

    // Note: TwitchChatAPI might not have an unregister method, so we'll just mark as not listening
    m_isListening = false;
    log::info("Stopped listening for Twitch chat commands");
};

bool TwitchCommandManager::isListening() const {
    return m_isListening;
};

void TwitchCommandManager::handleChatMessage(const ChatMessage& chatMessage) {
    std::string message = chatMessage.getMessage();
    std::string username = chatMessage.getUsername();
    std::string messageID = chatMessage.getMessageID();

    // Log username and message ID whenever a message is received
    log::info("Chat message received - Username: {}, Message ID: {}, Message: {}", username, messageID, message);

    // Check if message starts with command prefix
    if (message.empty() || message[0] != '!') {
        return;
    };

    // Extract command name (everything after ! until first space)
    std::string commandName;
    std::string commandArgs;

    size_t spacePos = message.find(' ');
    if (spacePos != std::string::npos) {
        commandName = message.substr(1, spacePos - 1);
        commandArgs = message.substr(spacePos + 1);
    } else {
        commandName = message.substr(1);
    };

    log::debug("Processing command: '{}' with args: '{}' from user: {}", commandName, commandArgs, username);

    // Find matching command
    auto it = std::find_if(m_commands.begin(), m_commands.end(),
                           [&commandName](const TwitchCommand& cmd) {
                               return cmd.name == commandName && cmd.enabled;
                           });

    if (it != m_commands.end()) {
        log::info("Executing command: {} for user: {} (Message ID: {})", commandName, username, messageID);

        // Execute command callback if it exists
        if (it->callback) {
            it->callback(commandArgs);
        };

        // For now, just log the response since we don't have a send method
        if (!it->response.empty()) {
            // Replace placeholders in response
            std::string response = it->response;
            size_t pos = response.find("{username}");

            if (pos != std::string::npos) {
                response.replace(pos, 10, username);
            };

            // Log the response (in a real implementation, this would be sent to chat)
            log::info("Command response: {}", response);
        };
    } else {
        log::debug("Command '{}' not found or disabled", commandName);
    };
};

TwitchCommandManager::~TwitchCommandManager() {
    // Clean up when destroyed
    stopListening();
    m_commands.clear();
    log::debug("TwitchCommandManager destructor called");
};
