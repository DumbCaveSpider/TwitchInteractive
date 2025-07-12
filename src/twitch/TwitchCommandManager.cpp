#include "TwitchCommandManager.hpp"

#include "TwitchDashboard.hpp"
#include "command/events/PlayLayerEvent.hpp"

#include <algorithm>
#include <unordered_map>

#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>

matjson::Value TwitchCommandAction::toJson() const {
    matjson::Value v = matjson::Value::object();
    v["type"] = as<int>(type);
    v["arg"] = arg;
    v["index"] = index;

    return v;
};

TwitchCommandAction TwitchCommandAction::fromJson(const matjson::Value& v) {
    CommandActionType type = CommandActionType::Notification;
    std::string arg = "";
    int index = 0;

    if (v.contains("type") && v["type"].asInt().ok())type = as<CommandActionType>(v["type"].asInt().unwrap());
    if (v.contains("arg") && v["arg"].asString().ok()) arg = v["arg"].asString().unwrap();
    if (v.contains("index") && v["index"].asInt().ok()) index = as<int>(v["index"].asInt().unwrap());

    return TwitchCommandAction(type, arg, index);
};

using namespace geode::prelude;

// Save commands to file
void TwitchCommandManager::saveCommands() {
    std::vector<matjson::Value> arrVec;
    for (const auto& cmd : m_commands) arrVec.push_back(cmd.toJson());

    matjson::Value arr(arrVec);
    std::ofstream ofs(getSavePath());
    if (ofs) ofs << arr.dump(2);
};

// NOTE: Update TwitchCommand definition to use std::vector<TwitchCommandAction> for actions

void TwitchCommandManager::loadCommands() {
    std::ifstream ifs(getSavePath());
    if (!ifs) return;

    auto result = matjson::parse(ifs);
    if (!result) return;

    auto arr = result.unwrap();
    if (!arr.isArray()) return;

    m_commands.clear();
    for (size_t i = 0; i < arr.size(); ++i) m_commands.push_back(TwitchCommand::fromJson(arr[i]));
};

// Deserialize a TwitchCommand from matjson::Value
TwitchCommand TwitchCommand::fromJson(const matjson::Value& v) {
    std::string name = (v.contains("name") && v["name"].asString().ok()) ? v["name"].asString().unwrap() : "";
    std::string description = (v.contains("description") && v["description"].asString().ok()) ? v["description"].asString().unwrap() : "";
    std::string response = (v.contains("response") && v["response"].asString().ok()) ? v["response"].asString().unwrap() : "";

    int cooldown = (v.contains("cooldown") && v["cooldown"].asInt().ok()) ? as<int>(v["cooldown"].asInt().unwrap()) : 0;
    bool enabled = (v.contains("enabled") && v["enabled"].asBool().ok()) ? v["enabled"].asBool().unwrap() : true;

    std::vector<TwitchCommandAction> actions;

    if (v.contains("actions") && v["actions"].isArray()) {
        auto& actionsArr = v["actions"];
        for (size_t i = 0; i < actionsArr.size(); ++i) actions.push_back(TwitchCommandAction::fromJson(actionsArr[i]));
    };

    TwitchCommand cmd(name, description, response, cooldown, actions);
    cmd.enabled = enabled;

    return cmd;
};

// Get the path to the commands save file
std::string TwitchCommandManager::getSavePath() const {
    // Save in the mod's directory as commands.json
    return "commands.json";
};

matjson::Value TwitchCommand::toJson() const {
    matjson::Value v = matjson::Value::object();
    v["name"] = name;
    v["description"] = description;
    v["response"] = response;
    v["cooldown"] = cooldown;
    v["enabled"] = enabled;

    std::vector<matjson::Value> actionsVec;

    for (const auto& action : actions) actionsVec.push_back(action.toJson());

    v["actions"] = matjson::Value(actionsVec);

    return v;
};

TwitchCommandManager* TwitchCommandManager::getInstance() {
    static TwitchCommandManager instance;
    static bool loaded = false;

    if (!loaded) {
        instance.loadCommands();
        loaded = true;
    };

    return &instance;
};

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
    };

    saveCommands();
};

void TwitchCommandManager::removeCommand(const std::string& name) {
    auto it = std::remove_if(m_commands.begin(), m_commands.end(),
                             [&name](const TwitchCommand& cmd) { return cmd.name == name; });

    if (it != m_commands.end()) {
        m_commands.erase(it, m_commands.end());
        log::info("Removed command: {}", name);
        saveCommands();
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
        saveCommands();
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
    if (message.empty() || message[0] != '!') return;

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
        if (it->cooldown > 0) {
            commandCooldowns[commandName] = now + it->cooldown;
            log::info("Command '{}' is now on cooldown for {}s", commandName, it->cooldown);
        };

        log::info("Executing command: {} for user: {} (Message ID: {})", commandName, username, messageID);

        // Notify dashboard to trigger cooldown for this command
        if (TwitchDashboard* dashboard = dynamic_cast<TwitchDashboard*>(CCDirector::sharedDirector()->getRunningScene()->getChildByID("twitch-dashboard-popup"))) dashboard->triggerCommandCooldown(commandName);

        // --- Sequential Action Execution with Wait Support ---
        // Use std::function to allow recursion

        // --- Sequential Action Execution with Wait Support ---

        struct ActionContext : public CCObject {
            std::vector<TwitchCommandAction> actions;
            size_t index = 0;
            std::string commandName;
            std::string username;
            std::string commandArgs;
            TwitchCommandManager* manager = nullptr;

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
                log::info("[TwitchCommandManager] Executing action {}: type={}, arg={}, index={}", ctx->index, (int)action.type, action.arg, action.index);
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
                    if (action.arg == "kill_player") {
                        log::info("[TwitchCommandManager] Triggering kill player event for command: {}", ctx->commandName);
                        PlayLayerEvent::killPlayer();
                    } else if (action.arg.rfind("jump:", 0) == 0) {
                        // Parse player index from arg (jump:1 or jump:2)
                        int playerIdx = 1;
                        std::string idxStr = action.arg.substr(5);
                        if (!idxStr.empty() && (idxStr.find_first_not_of("-0123456789") == std::string::npos)) {
                            playerIdx = std::stoi(idxStr);
                        }
                        log::info("[TwitchCommandManager] Triggering jump event for player {} (command: {})", playerIdx, ctx->commandName);
                        PlayLayerEvent::jumpPlayer(playerIdx);
                    } else if (action.arg.rfind("keycode:", 0) == 0) {
                        // Parse key string and duration from arg (keycode:<key>|<duration>)
                        std::string keyStr = action.arg.substr(8);
                        float duration = 0.f;
                        size_t pipePos = keyStr.find("|");
                        if (pipePos != std::string::npos) {
                            std::string durStr = keyStr.substr(pipePos + 1);
                            keyStr = keyStr.substr(0, pipePos);
                            try { duration = std::stof(durStr); } catch (...) { duration = 0.f; }
                        }
                        log::info("[TwitchCommandManager] Triggering keycode event: '{}' (duration: {}, command: {})", keyStr, duration, ctx->commandName);
                        PlayLayerEvent::pressKey(keyStr, duration);
                    }
                };

                if (action.type == CommandActionType::Notification) {
                    // Parse icon type and text: "<iconInt>:<text>"
                    int iconTypeInt = 1; // Default to Info
                    std::string notifText = action.arg;
                    size_t colonPos = action.arg.find(":");
                    if (colonPos != std::string::npos) {
                        iconTypeInt = std::stoi(action.arg.substr(0, colonPos));
                        notifText = action.arg.substr(colonPos + 1);
                    }
                    // Only show notification if text is not empty
                    if (!notifText.empty()) {
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
                }


                // Add more action types here as needed
                ctx->index++;
                ctx->execute(ctx);
            };
        };

        // Collect non-default actions in order
        std::vector<TwitchCommandAction> orderedActions;
        for (const auto& action : it->actions) {
            // Only skip truly default/empty actions (all fields default)
            if (action.type == CommandActionType::Notification && action.arg.empty() && action.index == 0) continue;
            if (action.type == CommandActionType::Event && action.arg.empty() && action.index == 0) continue;
            if (action.type == CommandActionType::Wait && action.arg.empty() && action.index == 0) continue;
            if (action.type == CommandActionType::Keybind && action.arg.empty() && action.index == 0) continue;
            if (action.type == CommandActionType::Chat && action.arg.empty() && action.index == 0) continue;
            orderedActions.push_back(action);
        };

        if (!orderedActions.empty()) {

            // Debug log: print action order before execution (use ostringstream for MSVC compatibility)
            std::ostringstream orderLog;
            orderLog << "[TwitchCommandManager] Action order for command '" << commandName << "': ";
            for (size_t i = 0; i < orderedActions.size(); ++i) {
                const auto& a = orderedActions[i];
                orderLog << "[" << i << "] type=" << (int)a.type << ", arg=" << a.arg << ", index=" << a.index << "; ";
            };

            std::string orderLogStr = orderLog.str();
            log::debug("{}", orderLogStr);

            auto* ctx = new ActionContext();
            ctx->actions = orderedActions;
            ctx->index = 0;
            ctx->commandName = commandName;
            ctx->username = username;
            ctx->commandArgs = commandArgs;
            ctx->manager = this;

            ctx->execute(ctx);
        };

        // Execute command callback if it exists
        if (it->callback) it->callback(commandArgs);

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