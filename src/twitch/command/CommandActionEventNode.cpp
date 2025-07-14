#include <Geode/Geode.hpp>
#include "CommandActionEventNode.hpp"
#include "CommandSettingsPopup.hpp"

using namespace geode::prelude;
using namespace cocos2d;

// Command Node logic
bool CommandActionEventNode::initCommandNode(TwitchDashboard* parent, TwitchCommand command, float width) {
    m_parent = parent;
    m_command = command;
    m_cooldownRemaining = 0;
    m_isOnCooldown = false;

    if (!CCNode::init()) return false;

    const float itemHeight = 40.0f;
    float itemWidth = width - 10;
    setContentSize(CCSize(width, itemHeight));

    m_commandBg = CCScale9Sprite::create("square02_small.png");
    m_commandBg->setContentSize(CCSize(itemWidth, itemHeight - 1));
    m_commandBg->setPosition(width / 2, itemHeight / 2);
    m_commandBg->setOpacity(100);
    addChild(m_commandBg);

    float leftPadding = 15.f;
    auto nameLabel = CCLabelBMFont::create(("!" + m_command.name).c_str(), "bigFont.fnt");
    nameLabel->setScale(0.4f);
    nameLabel->setAnchorPoint({ 0.0f, 0.5f });

    // Make the command name label clickable (copy to clipboard)
    auto nameBtn = CCMenuItemSpriteExtra::create(
        nameLabel,
        this,
        menu_selector(CommandActionEventNode::onCopyCommandName)
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
    }

    // Command description label - positioned on the left side below the name
    auto descLabel = CCLabelBMFont::create(m_command.description.c_str(), "chatFont.fnt");
    descLabel->setScale(0.375f);
    descLabel->setAnchorPoint({ 0.0f, 0.5f });
    descLabel->setPosition(leftPadding, itemHeight / 2 - 8);
    addChild(descLabel);

    // Create menu with sufficient padding for better touch detection
    auto commandEditMenu = CCMenu::create();
    commandEditMenu->setID("command-edit-menu");
    commandEditMenu->ignoreAnchorPointForPosition(false);
    commandEditMenu->setContentSize({ 180, 40 }); // Wider for four buttons

    // Create edit, delete, and settings buttons
    auto editBtn = createEditButton();
    auto deleteBtn = createDeleteButton();
    auto settingsBtn = createSettingsButton();

    // Create enable/disable toggle button
    auto enableOffSprite = ButtonSprite::create("Enabled", "bigFont.fnt", "GJ_button_01.png", 0.3f);
    auto enableOnSprite = ButtonSprite::create("Disabled", "bigFont.fnt", "GJ_button_06.png", 0.3f);
    auto enableToggle = CCMenuItemToggler::create(
        enableOnSprite,
        enableOffSprite,
        this,
        menu_selector(CommandActionEventNode::onToggleEnableCommand)
    );
    enableToggle->setID("enable-command-toggle");
    enableToggle->setContentSize({ 60.0f, 40.0f });

    // Position buttons side by side (settings, edit, delete, enable/disable)
    settingsBtn->setPosition(0, 0);
    editBtn->setPosition(40, 0);
    deleteBtn->setPosition(80, 0);
    enableToggle->setPosition(160, enableToggle->getContentSize().height / 2);

    // Add buttons to menu in new order
    commandEditMenu->addChild(settingsBtn);
    commandEditMenu->addChild(editBtn);
    commandEditMenu->addChild(deleteBtn);
    commandEditMenu->addChild(enableToggle);

    // Set the correct toggle state *after* adding to menu and positioning, to ensure layout is correct
    enableToggle->toggle(m_command.enabled);

    // Position menu at right side of the item, center vertically
    commandEditMenu->setPosition(width - 110, itemHeight / 2);

    commandEditMenu->setTouchPriority(-130);
    addChild(commandEditMenu);
    return true;
}

CommandActionEventNode* CommandActionEventNode::createCommandNode(TwitchDashboard* parent, TwitchCommand command, float width) {
    auto ret = new CommandActionEventNode();

    if (ret && ret->initCommandNode(parent, command, width)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};

void CommandActionEventNode::triggerCommand() {
    if (m_command.cooldown > 0) {
        if (m_isOnCooldown) {
            log::info("Command '{}' is currently on cooldown ({}s remaining)", m_command.name, m_cooldownRemaining);
            return;
        };

        startCooldown();
    };
};

void CommandActionEventNode::startCooldown() {
    m_cooldownRemaining = m_command.cooldown;
    m_isOnCooldown = true;
    schedule(schedule_selector(CommandActionEventNode::updateCooldown), 1.0f);
    updateCooldown(0);
};

void CommandActionEventNode::onCopyCommandName(cocos2d::CCObject* sender) {
    std::string cmd = "!" + m_command.name;
    geode::utils::clipboard::write(cmd);
    Notification::create(fmt::format("Copied '{}' to clipboard!", cmd), NotificationIcon::Success)->show();
};

void CommandActionEventNode::resetCooldown() {
    unschedule(schedule_selector(CommandActionEventNode::updateCooldown));

    m_isOnCooldown = false;
    if (m_commandBg) m_commandBg->setColor({ 255, 255, 255 }); // White

    if (m_command.cooldown > 0) {
        m_cooldownLabel->setString(fmt::format("({}s)", m_command.cooldown).c_str());
    } else {
        m_cooldownLabel->setString("");
    };
};

void CommandActionEventNode::updateCooldown(float dt) {
    if (m_cooldownRemaining > 0) {
        m_cooldownLabel->setString(fmt::format("({}s)", m_cooldownRemaining).c_str());
        m_cooldownRemaining--;
    } else {
        resetCooldown();
    };
};

void CommandActionEventNode::onDeleteCommand(cocos2d::CCObject* sender) {
    auto menuItem = as<CCMenuItem*>(sender);
    runAction(CCFadeTo::create(0.2f, 120));

    log::info("Deleting command: {}", m_command.name);

    m_parent->handleCommandDelete(m_command.name);
};

void CommandActionEventNode::onEditCommand(cocos2d::CCObject* sender) {
    auto menuItem = as<CCMenuItem*>(sender);

    log::info("Editing command: {}", m_command.name);

    m_parent->onEditCommand(sender);
};

void CommandActionEventNode::onSettingsCommand(cocos2d::CCObject* sender) {
    log::info("Settings button clicked for command: {}", m_command.name);
    auto commandManager = TwitchCommandManager::getInstance();

    for (const auto& cmd : commandManager->getCommands()) {
        if (cmd.name == m_command.name) {
            m_command = cmd;
            break;
        };
    };

    auto popup = CommandSettingsPopup::create(m_command);
    if (popup) {
        popup->m_noElasticity = true;
        popup->show();
    };
};

void CommandActionEventNode::onToggleEnableCommand(cocos2d::CCObject* sender) {
    m_command.enabled = !m_command.enabled;

    log::info("Command '{}' enabled state set to {}", m_command.name, m_command.enabled);

    auto commandManager = TwitchCommandManager::getInstance();
    commandManager->enableCommand(m_command.name, m_command.enabled);
};

cocos2d::CCMenuItem* CommandActionEventNode::createSettingsButton() {
    auto settingsBtnSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingsBtnSprite->setScale(0.65f);

    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsBtnSprite,
        this,
        menu_selector(CommandActionEventNode::onSettingsCommand)
    );
    settingsBtn->setID("settings-btn");
    settingsBtn->ignoreAnchorPointForPosition(true);
    settingsBtn->setContentSize({ 40.0f, 40.0f });

    auto btnSprite = settingsBtn->getNormalImage();
    if (btnSprite) btnSprite->setPosition(20.0f, 20.0f);

    return settingsBtn;
}

cocos2d::CCMenuItem* CommandActionEventNode::createEditButton() {
    auto editBtnSprite = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
    editBtnSprite->setScale(0.4f);

    auto editBtn = CCMenuItemSpriteExtra::create(
        editBtnSprite,
        this,
        menu_selector(CommandActionEventNode::onEditCommand)
    );
    editBtn->setID("edit-btn");
    editBtn->ignoreAnchorPointForPosition(true);
    editBtn->setContentSize({ 40.0f, 40.0f });

    auto btnSprite = editBtn->getNormalImage();
    if (btnSprite) btnSprite->setPosition(20.0f, 20.0f);

    return editBtn;
};

cocos2d::CCMenuItem* CommandActionEventNode::createDeleteButton() {
    auto deleteBtnSprite = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
    deleteBtnSprite->setScale(0.7f);

    auto deleteBtn = CCMenuItemSpriteExtra::create(
        deleteBtnSprite,
        this,
        menu_selector(CommandActionEventNode::onDeleteCommand)
    );
    deleteBtn->setID("delete-btn");
    deleteBtn->ignoreAnchorPointForPosition(true);
    deleteBtn->setContentSize({ 40.0f, 40.0f });

    auto btnSprite = deleteBtn->getNormalImage();
    if (btnSprite) btnSprite->setPosition(20.0f, 20.0f);

    return deleteBtn;
};

// Action Node logic
bool CommandActionEventNode::initActionNode(const std::string& labelText, CCObject* target, SEL_MenuHandler selector, float checkboxScale,
                                            CCObject* moveTarget, SEL_MenuHandler moveUpSelector, SEL_MenuHandler moveDownSelector, int actionIndex, bool canMoveUp, bool canMoveDown) {
    if (!CCNode::init()) return false;
    setContentSize(CCSize(370.f, 32.f));
    m_upBtn = nullptr;
    m_downBtn = nullptr;
    if (moveTarget && (moveUpSelector || moveDownSelector)) {
        auto upSprite = CCSprite::createWithSpriteFrameName("edit_upBtn_001.png");
        upSprite->setScale(0.5f);
        upSprite->setRotation(0.f);

        // Create move up button
        m_upBtn = ::CCMenuItemSpriteExtra::create(
            upSprite,
            moveTarget,
            moveUpSelector
        );
        m_upBtn->setPosition(38.f, 23.f);
        m_upBtn->setUserObject(CCInteger::create(actionIndex));
        m_upBtn->setEnabled(canMoveUp);
        m_upBtn->setVisible(canMoveUp);

        auto downSprite = CCSprite::createWithSpriteFrameName("edit_downBtn_001.png");
        downSprite->setScale(0.5f);

        // Create move down button
        m_downBtn = ::CCMenuItemSpriteExtra::create(
            downSprite,
            moveTarget,
            moveDownSelector
        );
        m_downBtn->setPosition(38.f, 9.f);
        m_downBtn->setUserObject(CCInteger::create(actionIndex));
        m_downBtn->setEnabled(canMoveDown);
        m_downBtn->setVisible(canMoveDown);

        auto arrowMenu = CCMenu::create();
        arrowMenu->setPosition(0, 0);

        arrowMenu->addChild(m_upBtn);
        arrowMenu->addChild(m_downBtn);

        addChild(arrowMenu);
    };

    m_label = nullptr;

    // Label
    m_label = CCLabelBMFont::create(labelText.c_str(), "bigFont.fnt");
    m_label->setScale(0.5f);
    m_label->setAnchorPoint({ 0, 0.5f });
    m_label->setAlignment(kCCTextAlignmentLeft);
    m_label->setPosition(50.f, 16.f);

    auto eventMenu = CCMenu::create();
    eventMenu->setPosition(0, 0);

    addChild(eventMenu);
    addChild(m_label);

    return true;
};

CommandActionEventNode* CommandActionEventNode::createActionNode(const std::string& labelText, CCObject* target, SEL_MenuHandler selector, float checkboxScale,
                                                                 CCObject* moveTarget, SEL_MenuHandler moveUpSelector, SEL_MenuHandler moveDownSelector, int actionIndex, bool canMoveUp, bool canMoveDown) {
    auto ret = new CommandActionEventNode();

    if (ret && ret->initActionNode(labelText, target, selector, checkboxScale, moveTarget, moveUpSelector, moveDownSelector, actionIndex, canMoveUp, canMoveDown)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};

// Event Node logic
bool CommandActionEventNode::initEventNode(const std::string& labelText, CCObject* target, SEL_MenuHandler selector, float checkboxScale) {
    if (!CCNode::init()) return false;
    setContentSize(CCSize(370.f, 32.f));

    m_checkbox = CCMenuItemToggler::createWithStandardSprites(target, selector, checkboxScale);
    m_checkbox->setPosition(20.f, 16.f);

    m_label = CCLabelBMFont::create(labelText.c_str(), "bigFont.fnt");
    m_label->setScale(0.5f);
    m_label->setAnchorPoint({ 0, 0.5f });
    m_label->setAlignment(kCCTextAlignmentLeft);
    m_label->setPosition(50.f, 16.f);

    // Store description for FLAlertLayer
    m_eventDescription = "";

    for (const auto& node : getAllEventNodes()) {
        if (node.label == labelText) {
            m_eventDescription = node.description;
            break;
        };
    };

    auto labelNode = CCNode::create();
    labelNode->addChild(m_label);
    labelNode->setPosition(0, 0);

    auto eventMenu = CCMenu::create();
    eventMenu->addChild(m_checkbox);
    eventMenu->setPosition(0, 0);

    addChild(eventMenu);
    addChild(labelNode);

    return true;
};

CommandActionEventNode* CommandActionEventNode::createEventNode(const std::string& labelText, CCObject* target, SEL_MenuHandler selector, float checkboxScale) {
    auto ret = new CommandActionEventNode();

    if (ret && ret->initEventNode(labelText, target, selector, checkboxScale)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};

std::vector<EventNodeInfo> CommandActionEventNode::getAllEventNodes() {
    std::vector<EventNodeInfo> nodes = {
        {"kill_player", "Destroy Player", "Destroy player. Self-explanatory. Don't use this while beating extremes!"},
        {"jump", "Jump", "Force the player to jump. You can set it to also hold jump."},
        {"move", "Move Player", "Move the player left or right. Lets you pick the player, direction and the distance to move."},
        {"wait", "Wait", "Pauses the command sequence for a set amount of time (in seconds). Use as a delay between actions."},
        {"notification", "Notification", "Shows a notification message on the screen. Supports the use of identifiers."},
        {"keycode", "Key Code", "Simulates a key press or release. Accepts a key name as argument (e.g., 'A', 'Space')."},
        {"profile", "Profile", "Opens the Player Profile in-game. Only accepts Account ID only!"},
        {"nothing", "Nothing", "Does nothing at all."},
    };

    return nodes;
};

// Unified interface
bool CommandActionEventNode::init(TwitchCommandAction action, CCSize scrollSize) {
    m_action = action;

    if (!CCNode::create()) return false;

    setContentSize(CCSize(scrollSize.width, 32.f));
    return true;
};

CommandActionEventNode* CommandActionEventNode::create(TwitchCommandAction action, CCSize scrollSize) {
    auto ret = new CommandActionEventNode();

    if (ret && ret->init(action, scrollSize)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};