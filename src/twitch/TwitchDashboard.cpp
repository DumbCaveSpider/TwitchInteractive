#include "TwitchDashboard.hpp"
#include "TwitchCommandManager.hpp"
#include "CommandNode.hpp"
#include "CommandInputPopup.hpp"

#include <Geode/Geode.hpp>
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>
#include <unordered_set>

using namespace geode::prelude;

bool TwitchDashboard::setup() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    setTitle("Twitch Dashboard");
    auto layerSize = m_mainLayer->getContentSize();

    // No need to scale here since create() already handles proper sizing

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
    try {
        auto twitchMod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api");

        if (twitchMod) {
            auto savedChannel = twitchMod->getSavedValue<std::string>("twitch-channel");

            if (!savedChannel.empty()) channelName = savedChannel;
        };
    } catch (const std::exception& e) {
        log::error("Exception while getting Twitch channel name: {}", e.what());
    } catch (...) {
        log::error("Unknown exception while getting Twitch channel name");
    };

    // Create welcome label
    std::string welcomeText = "Welcome " + channelName + "!";
    m_welcomeLabel = CCLabelBMFont::create(welcomeText.c_str(), "bigFont.fnt");
    m_welcomeLabel->setPosition(layerSize.width / 5, layerSize.height - 22);
    m_welcomeLabel->setScale(0.3f);
    m_welcomeLabel->setID("welcome-label");

    m_mainLayer->addChild(m_welcomeLabel);

    // Create single scroll layer for commands (centered)
    float scrollWidth = layerSize.width * 0.9f;  // Use 90% of width for single column
    float scrollHeight = layerSize.height - 80.f; // Leave space for buttons

    // Create background as parent container
    auto scrollBg = CCScale9Sprite::create("square02_001.png");
    scrollBg->setContentSize(CCSize(scrollWidth, scrollHeight));
    scrollBg->setOpacity(50);
    scrollBg->setID("commands-background");
    scrollBg->setPosition(layerSize.width / 2, layerSize.height / 2); // Center position

    m_mainLayer->addChild(scrollBg);

    // Create a ScrollLayer for commands
    m_commandScrollLayer = ScrollLayer::create(CCSize(scrollWidth, scrollHeight));
    m_commandScrollLayer->setID("commands-scroll");

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

    // Setup commands list
    setupCommandsList();

    // Add command input area
    setupCommandInput();

    // Register message callback for custom commands
    setupCommandListening();

    log::debug("TwitchDashboard opened successfully for channel: {}", channelName);
    return true;
};

void TwitchDashboard::setupCommandsList() {
    // Clear existing commands
    m_commandLayer->removeAllChildren();

    // Add some default commands
    auto commandManager = TwitchCommandManager::getInstance();

    if (commandManager->getCommands().empty()) {
        // Add default commands
        TwitchCommand helloCmd("hello", "Greets the user", "Hello {username}!");
        commandManager->addCommand(helloCmd);

        TwitchCommand helpCmd("help", "Shows available commands", "Available commands: !hello, !help");
        commandManager->addCommand(helpCmd);
    };

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
        // Add to the command layer
        m_commandLayer->addChild(CommandNode::create(this, command, m_mainLayer->getContentWidth() * 0.9f));
    };

    // Let the layout position all items
    m_commandLayer->updateLayout();
};

void TwitchDashboard::setupCommandInput() {
    // Create "Add Command" button that opens a popup
    m_commandControlsMenu = CCMenu::create();
    m_commandControlsMenu->setID("command-controls-menu");

    // Set content size to be same width as popup and 25 in height
    auto layerSize = m_mainLayer->getContentSize();
    m_commandControlsMenu->setContentSize(CCSize(layerSize.width, 25.f));

    // Create the button with the same height as the menu
    auto addCommandBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Add Command", "goldFont.fnt", "GJ_button_01.png", 0.5f),
        this,
        menu_selector(TwitchDashboard::onAddCustomCommand));
    addCommandBtn->setID("add-command-btn");
    addCommandBtn->setPosition(m_commandControlsMenu->getContentWidth() / 2, m_commandControlsMenu->getContentHeight() / 2); // Center vertically in the menu

    // Set content size to match the menu's height
    auto btnSprite = static_cast<ButtonSprite*>(addCommandBtn->getNormalImage());

    if (btnSprite) {
        auto btnSize = btnSprite->getContentSize();
        btnSprite->setContentSize(CCSize(btnSize.width, 25.0f));
    };

    m_commandControlsMenu->addChild(addCommandBtn);

    // Position the menu at the bottom center of the screen
    m_commandControlsMenu->setPosition(0.f, 10.f);

    m_mainLayer->addChild(m_commandControlsMenu);
};

void TwitchDashboard::setupCommandListening() {
    auto api = TwitchChatAPI::get();

    if (!api) {
        log::error("TwitchChatAPI is not available for command listening");
        return;
    };

    // Register message callback to listen for custom commands
    // Note: The API design doesn't let us unregister callbacks, but since this only runs once
    // during setup, we won't get duplicate registrations unless the dashboard is opened multiple times
    api->registerOnMessageCallback([this](const ChatMessage& chatMessage) {
        std::string message = chatMessage.getMessage();
        std::string username = chatMessage.getUsername();
        std::string messageId = chatMessage.getMessageID();

        // Check if message starts with '!' (command prefix)
        if (message.empty() || message[0] != '!') return;

        // Extract command name (everything after '!' until first space)
        std::string command = message.substr(1); // Remove '!'
        size_t spacePos = command.find(' ');
        std::string commandName = command.substr(0, spacePos);
        std::string args = (spacePos != std::string::npos) ? command.substr(spacePos + 1) : "";

        // Get command manager and check if command exists
        auto commandManager = TwitchCommandManager::getInstance();
        auto& commands = commandManager->getCommands();

        // Track if we've already processed this message ID to prevent duplicate notifications
        static std::unordered_set<std::string> processedMessageIds;

        // Check if we've already processed this message
        if (processedMessageIds.find(messageId) != processedMessageIds.end()) {
            // Already processed this message, skip to prevent duplicates
            return;
        };

        // Add to processed set
        processedMessageIds.insert(messageId);

        // Limit the size of the processed set to prevent memory growth
        if (processedMessageIds.size() > 100) processedMessageIds.clear();  // Simple approach: just clear when it gets too big

        for (const auto& cmd : commands) {
            if (cmd.name == commandName && cmd.enabled) {
                log::info("Command '{}' triggered by user '{}' (message ID: {}) with args: '{}'",
                          commandName, username, messageId, args);

                // Show notification
                Notification::create(
                    fmt::format("Command '{}' triggered by user '{}'", commandName, username),
                    NotificationIcon::Success
                )->show();

                // Execute command callback if available
                if (cmd.callback) cmd.callback(args);
                break;
            };
        }; });

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

        // Create a new command that logs when triggered
        TwitchCommand newCmd(commandName, commandDesc, "Custom command: " + commandDesc);
        newCmd.callback = [commandName, commandDesc](const std::string& args) {
            log::info("Custom command '{}' ({}) triggered with args: '{}'", commandName, commandDesc, args);
            };

        commandManager->addCommand(newCmd);
        refreshCommandsList();

        log::info("Added custom command: {} - {}", commandName, commandDesc);

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
    scaleFactor = std::min(scaleFactor, 1.0f);           // Don't scale up, only down if needed

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
    
    // Check if this is an actual edit or just opening the edit popup
    if (originalName == newName && newDesc.find('|') == std::string::npos) {
        // This is the first call to just open the edit popup with the current command data
        m_commandToDelete = originalName; // Store the command name for reference
        
        // Find the command to edit
        for (const auto& cmd : commandManager->getCommands()) {
            if (cmd.name == originalName) {
                // Open the command input popup in edit mode
                auto popup = CommandInputPopup::createForEdit(
                    originalName, 
                    cmd.description,
                    [this](const std::string& originalName, const std::string& nameAndDesc) {
                        // Parse nameAndDesc which contains both name and description separated by '|'
                        size_t separatorPos = nameAndDesc.find('|');
                        if (separatorPos != std::string::npos) {
                            std::string newName = nameAndDesc.substr(0, separatorPos);
                            std::string newDesc = nameAndDesc.substr(separatorPos + 1);
                            this->handleCommandEdit(originalName, newName, newDesc);
                        }
                    }
                );

                if (popup) popup->show();
                break;
            }
        }
        
        return;
    }
    
    // This is the actual edit with new values
    // Find the old command
    bool foundOld = false;
    TwitchCommand oldCommand("temp", "temp", "temp"); // Temporary default values
    
    for (const auto& cmd : commandManager->getCommands()) {
        if (cmd.name == originalName) {
            oldCommand = cmd;
            foundOld = true;
            break;
        }
    }
    
    if (!foundOld) {
        log::error("Could not find command to edit: {}", originalName);
        return;
    }
    
    // Remove the old command
    commandManager->removeCommand(originalName);
    
    // Create a new command with the updated values
    TwitchCommand newCmd(newName, newDesc, "Custom command: " + newDesc);
    newCmd.callback = [newName, newDesc](const std::string& args) {
        log::info("Custom command '{}' ({}) triggered with args: '{}'", newName, newDesc, args);
    };
    newCmd.enabled = oldCommand.enabled;
    
    // Add the new command
    commandManager->addCommand(newCmd);
    
    log::info("Updated command: {} -> {} ({})", originalName, newName, newDesc);
    
    // Refresh the commands list
    refreshCommandsList();
    
    // Show success message
    FLAlertLayer::create(
        "Success",
        ("Command '!" + originalName + "' updated successfully!").c_str(),
        "OK"
    )->show();
}

void TwitchDashboard::onEditCommand(CCObject* sender) {
    // Handle the edit button click
    auto button = static_cast<CCMenuItemSpriteExtra*>(sender);
    if (!button) return;
    
    // The command name should be stored in the button's tag or parent node
    auto node = static_cast<CommandNode*>(button->getParent()->getParent());
    if (!node) return;
    
    log::info("Edit button clicked, opening edit popup");
}