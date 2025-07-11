
#include "../TwitchCommandManager.hpp"
#include "../TwitchDashboard.hpp"
#include "events/PlayLayerEvent.hpp"
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>
#include <algorithm>
#include <unordered_map>
#include <fstream>

using namespace geode::prelude;

// Save commands to file
void TwitchCommandManager::saveCommands() {
    using namespace matjson;
    std::vector<matjson::Value> arrVec;
    for (const auto& cmd : m_commands) arrVec.push_back(cmd.toJson());
    Value arr(arrVec);
    std::ofstream ofs(getSavePath());
    if (ofs) ofs << arr.dump(2);
}

// Serialize a TwitchCommand to matjson::Value
matjson::Value TwitchCommand::toJson() const {
    matjson::Value v = matjson::Value::object();
    v["name"] = name;
    v["description"] = description;
    v["response"] = response;
    v["cooldown"] = cooldown;
    v["enabled"] = enabled;
    // TODO: Serialize actions properly if needed
    return v;
}

// Load commands from file
void TwitchCommandManager::loadCommands() {
    using namespace matjson;
    std::ifstream ifs(getSavePath());
    if (!ifs) return;
    auto result = matjson::parse(ifs);
    if (!result) return;
    auto arr = result.unwrap();
    if (!arr.isArray()) return;
    m_commands.clear();
    for (size_t i = 0; i < arr.size(); ++i) {
        m_commands.push_back(TwitchCommand::fromJson(arr[i]));
    }
}

// Deserialize a TwitchCommand from matjson::Value
TwitchCommand TwitchCommand::fromJson(const matjson::Value& v) {
    std::string name = (v.contains("name") && v["name"].asString().ok()) ? v["name"].asString().unwrap() : "";
    std::string description = (v.contains("description") && v["description"].asString().ok()) ? v["description"].asString().unwrap() : "";
    std::string response = (v.contains("response") && v["response"].asString().ok()) ? v["response"].asString().unwrap() : "";
    int cooldown = (v.contains("cooldown") && v["cooldown"].asInt().ok()) ? static_cast<int>(v["cooldown"].asInt().unwrap()) : 0;
    bool enabled = (v.contains("enabled") && v["enabled"].asBool().ok()) ? v["enabled"].asBool().unwrap() : true;
    std::array<TwitchCommandAction, 10> actions = {};
    if (v.contains("actions") && v["actions"].isArray()) {
        auto& actionsArr = v["actions"];
        for (size_t i = 0; i < actionsArr.size() && i < 10; ++i) {
            // You may need to implement TwitchCommandAction::fromJson if not already present
            // For now, just default-construct
            actions[i] = TwitchCommandAction();
        }
    }
    TwitchCommand cmd(name, description, response, cooldown, actions);
    cmd.enabled = enabled;
    return cmd;
}

// Get the path to the commands save file
std::string TwitchCommandManager::getSavePath() const {
    // Save in the mod's directory as commands.json
    return "commands.json";
}
#include "../TwitchCommandManager.hpp"
#include "../TwitchDashboard.hpp"
#include "events/PlayLayerEvent.hpp"

#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>
#include <algorithm>
#include <unordered_map>
#include <fstream>


using namespace geode::prelude;

TwitchCommandManager* TwitchCommandManager::getInstance() {
    static TwitchCommandManager instance;
    static bool loaded = false;
    if (!loaded) {
        instance.loadCommands();
        loaded = true;
    }
    return &instance;
}

void TwitchCommandManager::addCommand(const TwitchCommand& command) {
    // Check if command already exists
    auto it = std::find_if(m_commands.begin(), m_commands.end(),
                           [&command](const TwitchCommand& cmd) { return cmd.name == command.name; });

    if (it != m_commands.end()) {
        *it = command;
        log::info("Updated command: {}", command.name);
    } else {
        m_commands.push_back(command);
        log::info("Added new command: {}", command.name);
    }
    saveCommands();
}

void TwitchCommandManager::removeCommand(const std::string& name) {
    auto it = std::remove_if(m_commands.begin(), m_commands.end(),
                             [&name](const TwitchCommand& cmd) { return cmd.name == name; });
    if (it != m_commands.end()) {
        m_commands.erase(it, m_commands.end());
        log::info("Removed command: {}", name);
        saveCommands();
    } else {
        log::warn("Command '{}' not found for removal", name);
    }
}

void TwitchCommandManager::enableCommand(const std::string& name, bool enable) {
    auto it = std::find_if(m_commands.begin(), m_commands.end(),
                           [&name](const TwitchCommand& cmd) { return cmd.name == name; });
    if (it != m_commands.end()) {
        it->enabled = enable;
        log::info("Command {} {}", name, enable ? "enabled" : "disabled");
        saveCommands();
    }
}

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
        }

        // Set cooldown if needed
        if (it->cooldown > 0) {
            commandCooldowns[commandName] = now + it->cooldown;
            log::info("Command '{}' is now on cooldown for {}s", commandName, it->cooldown);
        }

        log::info("Executing command: {} for user: {} (Message ID: {})", commandName, username, messageID);

        // Notify dashboard to trigger cooldown for this command
        if (TwitchDashboard* dashboard = dynamic_cast<TwitchDashboard*>(CCDirector::sharedDirector()->getRunningScene()->getChildByID("twitch-dashboard-popup"))) {
            dashboard->triggerCommandCooldown(commandName);
        }

        // Execute kill player if enabled for this command
        for (const auto& action : it->actions) {
            log::info("[TwitchCommandManager] Checking action: type={}, arg={}", (int)action.type, action.arg);
            if (action.type == CommandActionType::Event && action.arg == "kill_player") {
                log::info("[TwitchCommandManager] Triggering kill player event for command: {}", commandName);
                PlayLayerEvent::killPlayer();
                break;
            }
        }

        // Execute command callback if it exists
        if (it->callback) {
            it->callback(commandArgs);
        }

        // For now, just log the response since we don't have a send method
        if (!it->response.empty()) {
            // Replace placeholders in response
            std::string response = it->response;
            size_t pos = response.find("{username}");

            if (pos != std::string::npos) response.replace(pos, 10, username);

            // Log the response (in a real implementation, this would be sent to chat)
            log::info("Command response: {}", response);
        }
    } else {
        log::debug("Command '{}' not found or disabled", commandName);
    }
};
TwitchCommandManager::~TwitchCommandManager() {
    m_commands.clear();
    log::debug("TwitchCommandManager destructor called");
}
