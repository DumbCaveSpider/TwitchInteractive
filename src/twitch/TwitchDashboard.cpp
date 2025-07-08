#include "TwitchDashboard.hpp"
#include "TwitchCommandManager.hpp"
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
    float scrollHeight = layerSize.height - 80; // Leave space for buttons

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

CCMenuItem* TwitchDashboard::createDeleteButton(const std::string& commandName) {
    // Create delete button sprite with proper scaling
    auto deleteSprite = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
    deleteSprite->setScale(0.7f); // Slightly larger icon

    // Create button with proper delegate and selector
    auto deleteBtn = CCMenuItemSpriteExtra::create(
        deleteSprite,
        this,
        menu_selector(TwitchDashboard::onDeleteCommand)
    );
    deleteBtn->setID("delete-btn-" + commandName);

    // Store the command name for deletion in the user object
    deleteBtn->setUserObject(CCString::create(commandName));

    // Make the button have a consistent size for better hit detection
    // This will be exactly the size of our menu (40x40)
    deleteBtn->setContentSize({ 40.0f, 40.0f });

    // Center the sprite within the button for better appearance
    auto btnSprite = deleteBtn->getNormalImage();
    if (btnSprite) btnSprite->setPosition(20.0f, 20.0f); // Center within the 40x40 area

    return deleteBtn;
};

void TwitchDashboard::ensureMenusRegistered() {
    // This function makes sure all delete button menus are properly registered with the touch dispatcher
    auto children = m_commandLayer->getChildren();

    if (!children) return;

    CCObject* child;
    CCARRAY_FOREACH(children, child) {
        auto commandItem = dynamic_cast<CCNode*>(child);

        if (commandItem) {
            // Find the menu within this command item
            auto itemChildren = commandItem->getChildren();

            if (itemChildren) {
                CCObject* itemChild;
                CCARRAY_FOREACH(itemChildren, itemChild) {
                    auto menu = dynamic_cast<CCMenu*>(itemChild);

                    if (menu && menu->getID().find("delete-menu-") != std::string::npos) {
                        menu->registerWithTouchDispatcher();
                        log::debug("Re-registered menu: {}", menu->getID().c_str());
                    };
                };
            };
        };
    };
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

    // Create command items using ColumnLayout
    for (const auto& command : commands) {
        // Create command item container
        auto commandItem = CCNode::create();
        commandItem->setID("command-item-" + command.name);

        // Set standard item height
        const float itemHeight = 40.0f;

        // Calculate width to center within scroll layer
        float scrollWidth = m_commandScrollLayer->getContentSize().width;
        float itemWidth = scrollWidth - 10; // Less margin for better filling

        // Set item size for layout to work properly - use full width of scroll layer
        commandItem->setContentSize(CCSize(scrollWidth, itemHeight));

        // Background - fill the entire command item space
        auto commandBg = CCScale9Sprite::create("square02_small.png");
        commandBg->setContentSize(CCSize(itemWidth, itemHeight - 1));
        commandBg->setPosition(scrollWidth / 2, itemHeight / 2); // Center in the command item
        commandBg->setOpacity(100);

        commandItem->addChild(commandBg);

        // Left side padding
        float leftPadding = 15.f;

        // Command name label - positioned on the left side
        auto nameLabel = CCLabelBMFont::create(("!" + command.name).c_str(), "bigFont.fnt");
        nameLabel->setScale(0.4f);
        nameLabel->setAnchorPoint({ 0.0f, 0.5f }); // Left-aligned
        nameLabel->setPosition(leftPadding, itemHeight / 2 + 5); // Top half of container

        commandItem->addChild(nameLabel);

        // Command description label - positioned on the left side below the name
        auto descLabel = CCLabelBMFont::create(command.description.c_str(), "chatFont.fnt");
        descLabel->setScale(0.35f);
        descLabel->setAnchorPoint({ 0.0f, 0.5f }); // Left-aligned
        descLabel->setPosition(leftPadding, itemHeight / 2 - 8); // Bottom half of container

        commandItem->addChild(descLabel);

        // Create menu with sufficient padding for better touch detection
        auto deleteMenu = CCMenu::create();
        deleteMenu->setID("delete-menu-" + command.name);

        // Make sure menu has consistent size for hit detection
        deleteMenu->setContentSize({ 40, 40 });

        // Create delete button exactly matching the menu's size
        auto deleteBtn = createDeleteButton(command.name);

        // Add button to menu
        deleteMenu->addChild(deleteBtn);

        // Position menu at right side of the item, center vertically
        deleteMenu->setPosition(scrollWidth - 30, itemHeight / 2);

        // Set high touch priority to ensure buttons are clickable
        deleteMenu->setTouchPriority(-130); // Higher priority than default
        // CCMenu doesn't use ccTouchesMode, it has its own handling

        commandItem->addChild(deleteMenu);

        // Add to the command layer - ColumnLayout will handle vertical positioning
        m_commandLayer->addChild(commandItem);
    };

    // Let the layout position all items
    m_commandLayer->updateLayout();

    // Make sure all menus are properly registered after a small delay
    // This gives Cocos2d time to properly layout everything
    runAction(CCSequence::create(
        CCDelayTime::create(0.05f),
        CCCallFunc::create(this, callfunc_selector(TwitchDashboard::ensureMenusRegistered)),
        nullptr
    ));
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

void TwitchDashboard::onDeleteCommand(CCObject* sender) {
    // Sound effect is handled automatically by CCMenuItemSpriteExtra

    // Get the menu item that was clicked
    auto menuItem = static_cast<CCMenuItem*>(sender);

    if (!menuItem || !menuItem->getUserObject()) {
        log::error("Invalid delete button clicked");
        return;
    };

    // Get the command name from the user object
    auto commandNameObj = static_cast<CCString*>(menuItem->getUserObject());
    std::string commandName = commandNameObj->getCString();

    // Temporarily disable the button to prevent double clicks
    menuItem->setEnabled(false);

    // Also find and disable the entire parent menu
    auto parent = menuItem->getParent();
    if (auto menu = typeinfo_cast<CCMenu*>(parent)) menu->setEnabled(false);

    // Find and visually disable the parent command item as well
    auto commandItem = menuItem->getParent()->getParent();
    if (commandItem) commandItem->runAction(CCFadeTo::create(0.2f, 120)); // Add a slight fade effect to indicate deletion

    log::info("Deleting command: {}", commandName);

    // Store the command name to delete in a member variable
    m_commandToDelete = commandName;

    // Process command deletion after a brief delay to allow the click to fully register
    // and for the fade animation to be visible
    runAction(CCSequence::create(
        CCDelayTime::create(0.15f),
        CCCallFunc::create(this, callfunc_selector(TwitchDashboard::processDeleteCommand)),
        nullptr
    ));
};

void TwitchDashboard::processDeleteCommand() {
    // Get the stored command name and clear it
    std::string commandName = m_commandToDelete;
    m_commandToDelete = "";

    if (commandName.empty()) {
        log::error("No command to delete");
        return;
    };

    // Delete the command
    auto commandManager = TwitchCommandManager::getInstance();
    commandManager->removeCommand(commandName);

    log::info("Command deleted: {}", commandName);

    // Schedule a refresh with a slightly longer delay to ensure all events are processed
    schedule(schedule_selector(TwitchDashboard::delayedRefreshCommandsList), 0.2f);
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