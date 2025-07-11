#include <Geode/Geode.hpp>
#include "TwitchDashboard.hpp"

#include "TwitchCommandManager.hpp"

#include "command/CommandNode.hpp"
#include "command/CommandInputPopup.hpp"
#include "command/events/PlayLayerEvent.hpp"

#include <unordered_set>
#include <Geode/Geode.hpp>
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>

using namespace geode::prelude;

extern void resetCommandCooldown(const std::string& commandName);

static bool s_listening = true;
static geode::Mod* getThisMod() {
    return geode::Loader::get()->getLoadedMod("arcticwoof.twitch_interactive");
}
static void loadListenState() {
    if (auto mod = getThisMod()) {
        s_listening = mod->getSavedValue<bool>("command-listen", true);
    }
}
static void saveListenState() {
    if (auto mod = getThisMod()) {
        mod->setSavedValue("command-listen", s_listening);
    }
}

bool TwitchDashboard::setup() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    setTitle("Twitch Dashboard");
    auto layerSize = m_mainLayer->getContentSize();

    // Set ID for the main popup layer
    setID("twitch-dashboard-popup");
    m_mainLayer->setID("twitch-dashboard-main-layer");

    // Check if TwitchChatAPI is available
    auto api = TwitchChatAPI::get();

    if (!api) {
        log::error("TwitchChatAPI is not available in TwitchDashboard::setup");
        return false;
    };

    // Get the Twitch channel name for the welcome message
    std::string channelName = "Unknown";
    auto twitchMod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api");
    if (twitchMod) {
        auto savedChannel = twitchMod->getSavedValue<std::string>("twitch-channel");
        if (!savedChannel.empty()) channelName = savedChannel;
    } else {
        log::error("TwitchChatAPI mod not found while getting Twitch channel name");
    }

    // Create welcome label
    std::string welcomeText = "Welcome " + channelName + "!";
    m_welcomeLabel = CCLabelBMFont::create(welcomeText.c_str(), "bigFont.fnt");
    m_welcomeLabel->setPosition(layerSize.width / 5, layerSize.height - 22);
    m_welcomeLabel->setScale(0.3f);
    m_welcomeLabel->setID("welcome-label");

    m_mainLayer->addChild(m_welcomeLabel);

    // Create single scroll layer for commands
    float scrollWidth = layerSize.width * 0.9f;  // Use 90% of width for single column
    float scrollHeight = layerSize.height - 80.f; // Leave space for buttons

    // Create background as parent container
    auto scrollBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
    scrollBg->setContentSize(CCSize(scrollWidth, scrollHeight));
    scrollBg->setOpacity(50);
    scrollBg->setID("commands-background");
    scrollBg->setPosition(layerSize.width / 2, layerSize.height / 2); // Center position

    m_mainLayer->addChild(scrollBg);

    // Create a ScrollLayer for commands
    m_commandScrollLayer = ScrollLayer::create(CCSize(scrollWidth, scrollHeight - 10.f));
    m_commandScrollLayer->setID("commands-scroll");
    m_commandScrollLayer->setPositionY(5.f);

    // Create a column layout for organizing commands vertically
    auto columnLayout = ColumnLayout::create()
        ->setAxisReverse(true)
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::Center) // Center items horizontally
        ->setAutoGrowAxis(scrollHeight) // Allow vertical growth
        ->setGap(5.0f);

    // Set the layout to the content layer
    m_commandLayer = m_commandScrollLayer->m_contentLayer;
    m_commandLayer->setID("commands-layer");
    m_commandLayer->setLayout(columnLayout);
    m_commandLayer->setContentSize(CCSize(scrollWidth, scrollHeight));

    // Add the scroll layer to the background
    scrollBg->addChild(m_commandScrollLayer);

    setupCommandsList(); // Setup commands list
    setupCommandInput(); // Add command input area
    setupCommandListening();// Register message callback for custom commands

    log::debug("TwitchDashboard opened successfully for channel: {}", channelName);
    return true;
};

void TwitchDashboard::setupCommandsList() {
    // Clear existing commands
    m_commandLayer->removeAllChildren();

    // Add some default commands
    auto commandManager = TwitchCommandManager::getInstance();

    /*
    if (commandManager->getCommands().empty()) {
        // Add default commands with a custom notification as the third argument
        TwitchCommand welcomeCmd(
            "welcome",
            "Welcome to the Twitch Interactive GD Mod!",
            "",
            10
        );
        commandManager->addCommand(welcomeCmd);
    };
*/
    refreshCommandsList();
};

void TwitchDashboard::refreshCommandsList() {
    // Remove all existing command items
    m_commandLayer->removeAllChildren();

    log::debug("Cleared all command nodes");

    auto commandManager = TwitchCommandManager::getInstance();
    auto& commands = commandManager->getCommands();

    // Reset the column layout to ensure proper spacing
    auto columnLayout = ColumnLayout::create()
        ->setAxisReverse(true)
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::Center) // Center items horizontally
        ->setAutoGrowAxis(m_commandScrollLayer->getContentSize().height)  // Allow vertical growth
        ->setGap(7.0f); // Slightly larger gap between items

    m_commandLayer->setLayout(columnLayout);

    // Check if there are no commands to display
    if (commands.empty()) {
        // Create a message when no commands are available
        auto noCommandsLabel = CCLabelBMFont::create("No commands available.\nClick 'Add Command' to create one.", "goldFont.fnt");
        noCommandsLabel->setScale(0.45f);
        noCommandsLabel->setPosition(m_commandLayer->getContentSize().width / 2, m_commandLayer->getContentSize().height / 2);
        noCommandsLabel->setAlignment(kCCTextAlignmentCenter);
        noCommandsLabel->setID("no-commands-label");

        m_commandLayer->addChild(noCommandsLabel);

        log::warn("No commands found");

        return; // Exit early since we don't need to create command items
    } else {
        log::debug("Recreating {} commands", commands.size());
    };

    // Create command items
    for (const auto& command : commands) {
        m_commandLayer->addChild(CommandNode::create(
            this,
            command,
            m_mainLayer->getContentWidth() * 0.9f
        ));
    };

    // Let the layout position all items
    m_commandLayer->updateLayout();
};

void TwitchDashboard::setupCommandInput() {
    // Create "Add Command" button that opens a popup
    m_commandControlsMenu = CCMenu::create();
    m_commandControlsMenu->setID("command-controls-menu");
    m_commandControlsMenu->setScale(0.8f); 

    // Set content size to be same width as popup and 25 in height
    auto layerSize = m_mainLayer->getContentSize();
    m_commandControlsMenu->setContentSize(CCSize(layerSize.width, 25.f));

    // Create the "Add Command" button
    auto addCommandBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Add Command", "bigFont.fnt", "GJ_button_01.png", 0.5f),
        this,
        menu_selector(TwitchDashboard::onAddCustomCommand)
    );
    addCommandBtn->setID("add-command-btn");

    // Create the CommandListen toggle button
    loadListenState();
    auto listenOffSprite = ButtonSprite::create("Listen", "bigFont.fnt", "GJ_button_01.png", 0.5f);
    auto listenOnSprite = ButtonSprite::create("Listen", "bigFont.fnt", "GJ_button_06.png", 0.5f);
    auto listenBtn = CCMenuItemToggler::create(
        listenOnSprite,
        listenOffSprite,
        this,
        menu_selector(TwitchDashboard::onToggleCommandListen)
    );
    listenBtn->setID("command-listen-btn");
    listenBtn->toggle(s_listening);

    // Set content size to match the menu's height
    auto btnSprite = as<ButtonSprite*>(addCommandBtn->getNormalImage());
    if (btnSprite) {
        auto btnSize = btnSprite->getContentSize();
        btnSprite->setContentSize(CCSize(btnSize.width, 25.0f));
    }
    auto listenBtnSprite = as<ButtonSprite*>(listenOnSprite);
    if (listenBtnSprite) {
        auto btnSize = listenBtnSprite->getContentSize();
        listenBtnSprite->setContentSize(CCSize(btnSize.width, 25.0f));
    }
    auto listenBtnOffSprite = as<ButtonSprite*>(listenOffSprite);
    if (listenBtnOffSprite) {
        auto btnSize = listenBtnOffSprite->getContentSize();
        listenBtnOffSprite->setContentSize(CCSize(btnSize.width, 25.0f));
    }

    // Center both buttons as a group
    float totalWidth = addCommandBtn->getContentSize().width + listenBtn->getContentSize().width + 16.0f; // 16px gap
    float startX = (m_commandControlsMenu->getContentSize().width - totalWidth) / 2.0f;
    addCommandBtn->setPosition(startX + addCommandBtn->getContentSize().width / 2.0f, m_commandControlsMenu->getContentHeight() / 2.f);
    listenBtn->setPosition(startX + addCommandBtn->getContentSize().width + 16.0f + listenBtn->getContentSize().width / 2.0f, m_commandControlsMenu->getContentHeight() / 2.f);

    m_commandControlsMenu->addChild(addCommandBtn);
    m_commandControlsMenu->addChild(listenBtn);

    // Position the menu at the bottom center of the screen
    m_commandControlsMenu->setPosition(0.f, 6.25f);

    m_mainLayer->addChild(m_commandControlsMenu);
}


void TwitchDashboard::onToggleCommandListen(CCObject* sender) {
    s_listening = !s_listening;
    saveListenState();
    log::info("CommandListen toggled: {}", s_listening ? "Listening" : "Not Listening");
}

bool TwitchDashboard::isListening() {
    return s_listening;
}

void TwitchDashboard::setupCommandListening() {
    // Ensure listen state is loaded on startup
    loadListenState();
    static bool callbackRegistered = false;
    if (callbackRegistered) {
        log::debug("TwitchDashboard::setupCommandListening: Callback already registered, skipping.");
        return;
    }
    auto api = TwitchChatAPI::get();
    if (!api) {
        log::error("TwitchChatAPI is not available for command listening");
        return;
    }
    api->registerOnMessageCallback([this](const ChatMessage& chatMessage) {
        // Ignore all callbacks if not listening
        if (!s_listening) {
            log::debug("Command ignored: Not Listening");
            return;
        }
        auto commandManager = TwitchCommandManager::getInstance();
        commandManager->handleChatMessage(chatMessage);
    });
    callbackRegistered = true;
    log::info("Command listening setup complete");
};

void TwitchDashboard::onClose(CCObject* sender) {
    // Make sure to unschedule any delayed refreshes when closing
    unschedule(schedule_selector(TwitchDashboard::delayedRefreshCommandsList));

    // Stop any pending actions
    stopAllActions();

    Popup::onClose(sender);
};

void TwitchDashboard::onAddCustomCommand(CCObject* sender) {
    // Open the command input popup
    auto popup = CommandInputPopup::create([this](const std::string& commandName, const std::string& commandDesc) {
        // This callback is called when user adds a command
        auto commandManager = TwitchCommandManager::getInstance();

        // Parse cooldown from commandDesc if present
        std::string desc = commandDesc;
        int cooldown = 0;
        size_t delim = commandDesc.find_last_of('|');

        if (delim != std::string::npos) {
            desc = commandDesc.substr(0, delim);
            std::string cooldownStr = commandDesc.substr(delim + 1);
            cooldown = 0;
            if (!cooldownStr.empty() && (cooldownStr.find_first_not_of("-0123456789") == std::string::npos)) {
                cooldown = std::stoi(cooldownStr);
            }
        }

        // Create a new command that logs when triggered
        TwitchCommand newCmd(commandName, desc, "Custom command: " + desc, cooldown);
        newCmd.callback = [commandName, desc](const std::string& args) {
            log::info("Custom command '{}' ({}) triggered with args: '{}'", commandName, desc, args);
            };

        commandManager->addCommand(newCmd);
        refreshCommandsList();

        log::info("Added custom command: {} - {} (cooldown: {}s)", commandName, desc, cooldown);

        // Show success message
        FLAlertLayer::create(
            "Success",
            ("Command '!" + commandName + "' added successfully!").c_str(),
            "OK"
        )->show();
                                           });

    if (popup) popup->show();
};

void TwitchDashboard::handleCommandDelete(const std::string& commandName) {
    // Delete the command
    auto commandManager = TwitchCommandManager::getInstance();
    commandManager->removeCommand(commandName);

    log::info("Command deleted: {}", commandName);

    // Schedule a refresh with a slightly longer delay to ensure all events are processed
    schedule(schedule_selector(TwitchDashboard::delayedRefreshCommandsList), 0.2f);

    // Show success message
    FLAlertLayer::create(
        "Success",
        ("Command '!" + commandName + "' deleted successfully!").c_str(),
        "OK"
    )->show();
};

void TwitchDashboard::delayedRefreshCommandsList(float dt) {
    // Unschedule to ensure this only runs once
    unschedule(schedule_selector(TwitchDashboard::delayedRefreshCommandsList));

    // Refresh the commands list
    refreshCommandsList();

    // Log for debugging
    log::debug("Commands list refreshed via delayed callback");
};

TwitchDashboard* TwitchDashboard::create() {
    auto ret = new TwitchDashboard();

    // Calculate appropriate size based on window size (same logic as in setup())
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // Start with base size
    float baseWidth = 800.f;
    float baseHeight = 500.f;

    // Calculate scale factor to fit within window
    float scaleX = winSize.width / baseWidth;
    float scaleY = winSize.height / baseHeight;
    float scaleFactor = std::min(scaleX, scaleY) * 0.8f; // Use 80% of available space
    scaleFactor = std::min(scaleFactor, 1.0f); // Don't scale up, only down if needed

    // Apply scale factor to get final size
    float width = baseWidth * scaleFactor;
    float height = baseHeight * scaleFactor;

    // Ensure minimum size
    width = std::max(width, 525.f);
    height = std::max(height, 280.f);

    if (ret && ret->initAnchored(width, height)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};

TwitchDashboard::~TwitchDashboard() {
    log::debug("TwitchDashboard destructor called");
};

void TwitchDashboard::handleCommandEdit(const std::string& originalName, const std::string& newName, const std::string& newDesc) {
    auto commandManager = TwitchCommandManager::getInstance();

    // Always expect newName = name, newDesc = desc|cooldown
    std::string finalName = newName;
    std::string desc = newDesc;

    int cooldown = 0;

    size_t firstSep = newDesc.find('|');
    size_t lastSep = newDesc.rfind('|');

    if (firstSep != std::string::npos && lastSep != std::string::npos && firstSep != lastSep) {
        // Format: desc|cooldown
        desc = newDesc.substr(0, firstSep);
        std::string cooldownStr = newDesc.substr(lastSep + 1);
        cooldown = 0;
        if (!cooldownStr.empty() && (cooldownStr.find_first_not_of("-0123456789") == std::string::npos)) {
            cooldown = std::stoi(cooldownStr);
        }
    } else if (firstSep != std::string::npos) {
        // Format: desc|cooldown (if only one sep)
        desc = newDesc.substr(0, firstSep);
        std::string cooldownStr = newDesc.substr(firstSep + 1);
        cooldown = 0;
        if (!cooldownStr.empty() && (cooldownStr.find_first_not_of("-0123456789") == std::string::npos)) {
            cooldown = std::stoi(cooldownStr);
        }
    }

    // Find the old command
    bool foundOld = false;
    TwitchCommand oldCommand("temp", "temp", "temp"); // Temporary default values

    for (const auto& cmd : commandManager->getCommands()) {
        if (cmd.name == originalName) {
            oldCommand = cmd;
            foundOld = true;
            break;
        };
    };

    if (!foundOld) {
        log::error("Could not find command to edit: {}", originalName);
        return;
    };

    // Remove the old command
    commandManager->removeCommand(originalName);

    // If cooldown changed, reset cooldown for this command
    if (cooldown != oldCommand.cooldown) {
        resetCommandCooldown(originalName);
        log::info("Cooldown for command '{}' was changed. Cooldown reset.", originalName);
    };

    // Create a new command with the updated values
    TwitchCommand newCmd(finalName, desc, "Custom command: " + desc, cooldown);
    newCmd.callback = [finalName, desc](const std::string& args) {
        log::info("Custom command '{}' ({}) triggered with args: '{}'", finalName, desc, args);
        };
    newCmd.enabled = oldCommand.enabled;

    // Add the new command
    commandManager->addCommand(newCmd);
    log::info("Updated command: {} -> {} ({}) (cooldown: {}s)", originalName, finalName, desc, cooldown);

    // Refresh the commands list
    refreshCommandsList();

    // Show success message
    FLAlertLayer::create(
        "Success",
        ("Command '!" + originalName + "' updated successfully!").c_str(),
        "OK"
    )->show();
};

void TwitchDashboard::onEditCommand(CCObject* sender) {
    // Handle the edit button click
    auto button = as<CCMenuItemSpriteExtra*>(sender);
    if (!button) return;

    // The command name should be stored in the button's tag or parent node
    auto node = as<CommandNode*>(button->getParent()->getParent());
    if (!node) return;

    std::string commandName = node->getCommandName();
    auto commandManager = TwitchCommandManager::getInstance();

    for (const auto& cmd : commandManager->getCommands()) {
        if (cmd.name == commandName) {
            // Always pass cooldown, even if 0
            std::string descForEdit = cmd.description + "|" + std::to_string(cmd.cooldown);

            auto popup = CommandInputPopup::createForEdit(
                cmd.name,
                descForEdit,
                [this](const std::string& originalName, const std::string& nameDescCooldown) {
                    // Always parse as name|desc|cooldown
                    size_t firstSep = nameDescCooldown.find('|');
                    size_t lastSep = nameDescCooldown.rfind('|');
                    if (firstSep != std::string::npos && lastSep != std::string::npos && firstSep != lastSep) {
                        std::string newName = nameDescCooldown.substr(0, firstSep);
                        std::string newDesc = nameDescCooldown.substr(firstSep + 1);
                        handleCommandEdit(originalName, newName, newDesc);
                    } else {
                        // Fallback: treat as name|desc
                        handleCommandEdit(originalName, nameDescCooldown.substr(0, firstSep), nameDescCooldown.substr(firstSep + 1));
                    };
                }
            );

            if (popup) popup->show();
            break;
        };
    };
};

void TwitchDashboard::triggerCommandCooldown(const std::string& commandName) {
    if (!m_commandLayer) return;

    for (auto child : CCArrayExt<CCNode*>(m_commandLayer->getChildren())) {
        if (auto node = dynamic_cast<CommandNode*>(child)) {
            if (node->getCommandName() == commandName) {
                node->triggerCommand();
                break;
            };
        };
    };
};