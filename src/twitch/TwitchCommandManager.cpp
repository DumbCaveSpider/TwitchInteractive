#include "TwitchCommandManager.hpp"
#include "TwitchDashboard.hpp"

#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>
#include <algorithm>
#include <unordered_map>

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
        // Update existing command if name or description changed
        if (it->description != command.description) {
            it->description = command.description;
            log::info("Updated command description: {}", command.name);
        };

        // Update other fields as needed (response, callback, enabled, etc.)
        *it = command;
        log::info("Updated command: {}", command.name);
    } else {
        // Add new command
        m_commands.push_back(command);
        log::info("Added new command: {}", command.name);
    };
};

void TwitchCommandManager::removeCommand(const std::string& name) {
    log::info("TwitchCommandManager::removeCommand called with name: '{}'", name);

    // List all commands before removal
    log::info("Current commands before removal:");
    for (const auto& cmd : m_commands) {
        log::info("  - Command: '{}', Description: '{}'", cmd.name, cmd.description);
    };

    size_t initialSize = m_commands.size();
    auto it = std::remove_if(m_commands.begin(), m_commands.end(),
                             [&name](const TwitchCommand& cmd) {
                                 log::info("Checking command: '{}' against '{}'", cmd.name, name);
                                 return cmd.name == name;
                             });

    if (it != m_commands.end()) {
        m_commands.erase(it, m_commands.end());
        log::info("Removed command: '{}' (removed {} command(s))", name, initialSize - m_commands.size());

        // List all commands after removal
        log::info("Current commands after removal:");
        for (const auto& cmd : m_commands) {
            log::info("  - Command: '{}', Description: '{}'", cmd.name, cmd.description);
        };
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

// Add a static map to track cooldowns for each command
std::unordered_map<std::string, time_t> commandCooldowns;

void resetCommandCooldown(const std::string& commandName) {
    commandCooldowns.erase(commandName);
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
        // Check cooldown
        time_t now = time(nullptr);
        auto cooldownIt = commandCooldowns.find(commandName);
        if (cooldownIt != commandCooldowns.end() && cooldownIt->second > now) {
            log::info("Command '{}' is currently on cooldown ({}s remaining)", commandName, cooldownIt->second - now);
            return;
        };

        // Set cooldown if needed
        if (it->cooldownSeconds > 0) {
            commandCooldowns[commandName] = now + it->cooldownSeconds;
            log::info("Command '{}' is now on cooldown for {}s", commandName, it->cooldownSeconds);
        };

        log::info("Executing command: {} for user: {} (Message ID: {})", commandName, username, messageID);

        // Notify dashboard to trigger cooldown for this command
        if (TwitchDashboard* dashboard = dynamic_cast<TwitchDashboard*>(CCDirector::sharedDirector()->getRunningScene()->getChildByID("twitch-dashboard-popup"))) {
            dashboard->triggerCommandCooldown(commandName);
        };

        // Execute command callback if it exists
        if (it->callback) {
            it->callback(commandArgs);
        };

        // For now, just log the response since we don't have a send method
        if (!it->response.empty()) {
            // Replace placeholders in response
            std::string response = it->response;
            size_t pos = response.find("{username}");

            if (pos != std::string::npos) response.replace(pos, 10, username);

            // Log the response (in a real implementation, this would be sent to chat)
            log::info("Command response: {}", response);
        };
    } else {
        log::debug("Command '{}' not found or disabled", commandName);
    };
};

TwitchCommandManager::~TwitchCommandManager() {
    m_commands.clear();
    log::debug("TwitchCommandManager destructor called");
};