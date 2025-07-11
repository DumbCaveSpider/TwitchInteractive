#include "CommandNode.hpp"
#include "CommandSettingsPopup.hpp"

#include "../TwitchDashboard.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace cocos2d;

bool CommandNode::init(TwitchDashboard* parent, TwitchCommand command, float width) {
    m_parent = parent;
    m_command = command;
    m_cooldownRemaining = 0;
    m_isOnCooldown = false;

    if (!CCNode::init()) return false;

    // Set standard item height
    const float itemHeight = 40.0f;

    // Calculate width to center within scroll layer
    float itemWidth = width - 10; // Less margin for better filling

    // Set item size for layout to work properly - use full width of scroll layer
    setContentSize(CCSize(width, itemHeight));

    // Background - fill the entire command item space
    m_commandBg = CCScale9Sprite::create("square02_small.png");
    m_commandBg->setContentSize(CCSize(itemWidth, itemHeight - 1));
    m_commandBg->setPosition(width / 2, itemHeight / 2); // Center in the command item
    m_commandBg->setOpacity(100);

    addChild(m_commandBg);

    // Left side padding
    float leftPadding = 15.f;

    // Command name label - positioned on the left side, clickable
    auto nameLabel = CCLabelBMFont::create(("!" + m_command.name).c_str(), "bigFont.fnt");
    nameLabel->setScale(0.4f);
    nameLabel->setAnchorPoint({ 0.0f, 0.5f }); // Left-aligned

    // Make the command name label clickable (copy to clipboard)
    auto nameBtn = CCMenuItemSpriteExtra::create( // use this class for now to compile for mac
                                                 nameLabel,
                                                 this,
                                                 menu_selector(CommandNode::onCopyCommandName)
    );
    nameBtn->setID("command-name-btn");
    nameBtn->setAnchorPoint({ 0.0f, 0.5f });
    nameBtn->setPosition(leftPadding, (itemHeight / 2.f) + 5.f);

    // Create a menu just so it can be clicked
    auto nameMenu = CCMenu::create();
    nameMenu->setPosition(0, 0);
    nameMenu->setContentSize(nameLabel->getContentSize());

    nameMenu->addChild(nameBtn);
    addChild(nameMenu);

    // Calculate width of nameLabel after scaling
    float nameLabelWidth = nameLabel->getContentSize().width * nameLabel->getScale();
    float cooldownPadding = 8.0f; // Space between name and cooldown

    // Cooldown label - right next to the command name (dynamic position)
    m_cooldownLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_cooldownLabel->setScale(0.4f);
    m_cooldownLabel->setAnchorPoint({ 0.0f, 0.5f });
    m_cooldownLabel->setPosition(leftPadding + nameLabelWidth + cooldownPadding, itemHeight / 2.f + 5.f);

    addChild(m_cooldownLabel);

    // Set initial cooldown label
    if (m_command.cooldown > 0) {
        m_cooldownLabel->setString(fmt::format("({}s)", m_command.cooldown).c_str());
    } else {
        m_cooldownLabel->setString("");
    };

    // Command description label - positioned on the left side below the name
    auto descLabel = CCLabelBMFont::create(m_command.description.c_str(), "chatFont.fnt");
    descLabel->setScale(0.375f);
    descLabel->setAnchorPoint({ 0.0f, 0.5f }); // Left-aligned
    descLabel->setPosition(leftPadding, itemHeight / 2 - 8); // Bottom half of container

    addChild(descLabel);


    // Create menu with sufficient padding for better touch detection
    auto commandEditMenu = CCMenu::create();
    commandEditMenu->setID("command-edit-menu");
    commandEditMenu->ignoreAnchorPointForPosition(false);
    commandEditMenu->setContentSize({ 120, 40 }); // Wider for three buttons

    // Create edit, delete, and settings buttons
    auto editBtn = createEditButton();
    auto deleteBtn = createDeleteButton();
    auto settingsBtn = createSettingsButton();

    // Position buttons side by side (settings, edit, delete)
    settingsBtn->setPosition(0, 0);
    editBtn->setPosition(40, 0);
    deleteBtn->setPosition(80, 0);

    // Add buttons to menu in new order
    commandEditMenu->addChild(settingsBtn);
    commandEditMenu->addChild(editBtn);
    commandEditMenu->addChild(deleteBtn);

    // Position menu at right side of the item, center vertically
    commandEditMenu->setPosition(width - 70, itemHeight / 2);

    commandEditMenu->setTouchPriority(-130);
    addChild(commandEditMenu);
    return true;
};

CCMenuItem* CommandNode::createSettingsButton() {
    // Create settings button sprite with proper scaling
    auto settingsBtnSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingsBtnSprite->setScale(0.65f);

    // Create button with proper delegate and selector
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsBtnSprite,
        this,
        menu_selector(CommandNode::onSettingsCommand)
    );
    settingsBtn->setID("settings-btn");
    settingsBtn->ignoreAnchorPointForPosition(true);
    settingsBtn->setContentSize({ 40.0f, 40.0f });

    auto btnSprite = settingsBtn->getNormalImage();
    if (btnSprite) btnSprite->setPosition(20.0f, 20.0f);

    return settingsBtn;
};

CCMenuItem* CommandNode::createEditButton() {
    // Create edit button sprite with proper scaling
    auto editBtnSprite = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
    editBtnSprite->setScale(0.4f); // Slightly larger icon

    // Create button with proper delegate and selector
    auto editBtn = CCMenuItemSpriteExtra::create(
        editBtnSprite,
        this,
        menu_selector(CommandNode::onEditCommand)
    );
    editBtn->setID("edit-btn");
    editBtn->ignoreAnchorPointForPosition(true);

    // Make the button have a consistent size for better hit detection
    editBtn->setContentSize({ 40.0f, 40.0f });

    // Center the sprite within the button for better appearance
    auto btnSprite = editBtn->getNormalImage();
    if (btnSprite) btnSprite->setPosition(20.0f, 20.0f); // Center within the 40x40 area

    return editBtn;
};

CCMenuItem* CommandNode::createDeleteButton() {
    // Create delete button sprite with proper scaling
    auto deleteBtnSprite = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
    deleteBtnSprite->setScale(0.7f);

    // Create button with proper delegate and selector
    auto deleteBtn = CCMenuItemSpriteExtra::create(
        deleteBtnSprite,
        this,
        menu_selector(CommandNode::onDeleteCommand)
    );
    deleteBtn->setID("delete-btn");
    deleteBtn->ignoreAnchorPointForPosition(true);
    deleteBtn->setContentSize({ 40.0f, 40.0f }); // Make the button have a consistent size for better hit detection

    // Center the sprite within the button for better appearance
    auto btnSprite = deleteBtn->getNormalImage();
    if (btnSprite) btnSprite->setPosition(20.0f, 20.0f); // Center within the 40x40 area

    return deleteBtn;
};

void CommandNode::onDeleteCommand(CCObject* sender) {
    // Get the menu item that was clicked
    auto menuItem = as<CCMenuItem*>(sender);

    // Find and visually disable the parent command item as well
    runAction(CCFadeTo::create(0.2f, 120)); // Add a slight fade effect to indicate deletion

    log::info("Deleting command: {}", m_command.name);

    // Delete the command
    m_parent->handleCommandDelete(m_command.name);
};

void CommandNode::onEditCommand(CCObject* sender) {
    // Get the menu item that was clicked
    auto menuItem = as<CCMenuItem*>(sender);

    log::info("Editing command: {}", m_command.name);

    // Only call the parent's edit handler to show the popup
    m_parent->onEditCommand(sender);
};

CommandNode* CommandNode::create(TwitchDashboard* parent, TwitchCommand command, float width) {
    auto ret = new CommandNode();

    if (!ret || !ret->init(parent, command, width)) {
        CC_SAFE_DELETE(ret);
        return nullptr;
    };

    ret->autorelease();
    return ret;
};

void CommandNode::triggerCommand() {
    if (m_command.cooldown > 0) {
        if (m_isOnCooldown) {
            log::info("Command '{}' is currently on cooldown ({}s remaining)", m_command.name, m_cooldownRemaining);
            return;
        };

        startCooldown();
    };
};

void CommandNode::startCooldown() {
    m_cooldownRemaining = m_command.cooldown;
    m_isOnCooldown = true;

    schedule(schedule_selector(CommandNode::updateCooldown), 1.0f);
    updateCooldown(0);
};

void CommandNode::updateCooldown(float dt) {
    if (m_cooldownRemaining > 0) {
        m_cooldownLabel->setString(fmt::format("({}s)", m_cooldownRemaining).c_str());
        m_cooldownRemaining--;
    } else {
        resetCooldown();
    };
};

void CommandNode::resetCooldown() {
    unschedule(schedule_selector(CommandNode::updateCooldown));
    m_isOnCooldown = false;

    if (m_commandBg) m_commandBg->setColor({ 255, 255, 255 }); // White

    if (m_command.cooldown > 0) {
        m_cooldownLabel->setString(fmt::format("({}s)", m_command.cooldown).c_str());
    } else {
        m_cooldownLabel->setString("");
    };
};

void CommandNode::onCopyCommandName(CCObject* sender) {
    std::string cmd = "!" + m_command.name;

    // Copy to clipboard (Geode API)
    geode::utils::clipboard::write(cmd);
    Notification::create(fmt::format("Copied '{}' to clipboard!", cmd), NotificationIcon::Success)->show();
};

void CommandNode::onSettingsCommand(cocos2d::CCObject* sender) {
    log::info("Settings button clicked for command: {}", m_command.name);

    // Find the command in the manager to ensure it has the latest data
    auto commandManager = TwitchCommandManager::getInstance();
    for (const auto& cmd : commandManager->getCommands()) {
        if (cmd.name == m_command.name) {
            m_command = cmd;
            break;
        };
    };

    auto popup = CommandSettingsPopup::create(m_command);
    popup->m_noElasticity = true;
    popup->show();
};