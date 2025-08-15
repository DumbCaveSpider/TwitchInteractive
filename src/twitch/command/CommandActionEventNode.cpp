#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include "CommandActionEventNode.hpp"
#include "events/PlayerObjectEvent.hpp"
#include "../handler/GravitySettingsPopup.hpp"
#include "CommandSettingsPopup.hpp"
#include "../handler/SettingsHandler.hpp"

using namespace geode::prelude;
using namespace cocos2d;

// Command Node logic
bool CommandActionEventNode::initCommandNode(TwitchDashboard *parent, TwitchCommand command, float width)
{
    m_parent = parent;
    m_command = command;
    m_cooldownRemaining = 0;
    m_isOnCooldown = false;

    if (!CCNode::init())
        return false;

    const float itemHeight = 40.0f;
    float itemWidth = width - 10;

    setContentSize(CCSize(width, itemHeight));

    m_commandBg = CCScale9Sprite::create("square02_small.png");
    m_commandBg->setContentSize(CCSize(itemWidth, itemHeight - 1));
    m_commandBg->setPosition(width / 2, itemHeight / 2);
    m_commandBg->setOpacity(60);

    addChild(m_commandBg);

    float leftPadding = 15.f;
    auto nameLabel = CCLabelBMFont::create(("!" + m_command.name).c_str(), "bigFont.fnt");
    nameLabel->setID("command-name");
    nameLabel->setScale(0.4f);
    nameLabel->setAnchorPoint({0.0f, 0.5f});

    // Make the command name label clickable (copy to clipboard)
    auto nameBtn = CCMenuItemSpriteExtra::create(
        nameLabel,
        this,
        menu_selector(CommandActionEventNode::onCopyCommandName));
    nameBtn->setID("command-name-btn");
    nameBtn->setAnchorPoint({0.0f, 0.5f});
    nameBtn->setPosition(leftPadding, (itemHeight / 2.f));

    // Create a menu just so it can be clicked
    auto nameMenu = CCMenu::create();
    nameMenu->setID("command-name-menu");
    nameMenu->setPosition(0, 9);
    nameMenu->setContentSize(nameLabel->getContentSize());

    nameMenu->addChild(nameBtn);

    addChild(nameMenu);

    // Calculate width of nameLabel after scaling
    float nameLabelWidth = nameLabel->getContentSize().width * nameLabel->getScale();
    float cooldownPadding = 8.0f; // Space between name and cooldown

    // Cooldown label - right next to the command name (dynamic position)
    m_cooldownLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_cooldownLabel->setID("command-cooldown");
    m_cooldownLabel->setScale(0.4f);
    m_cooldownLabel->setAnchorPoint({0.0f, 0.5f});
    m_cooldownLabel->setPosition(leftPadding + nameLabelWidth + cooldownPadding, itemHeight / 2.f + 8.5f);

    addChild(m_cooldownLabel);

    // Set initial cooldown label
    if (m_command.cooldown > 0)
    {
        m_cooldownLabel->setString(fmt::format("({}s)", m_command.cooldown).c_str());
    }
    else
    {
        m_cooldownLabel->setString("");
    };

    // Command description label - positioned on the left side below the name
    auto descLabel = CCLabelBMFont::create(m_command.description.c_str(), "chatFont.fnt");
    descLabel->setID("command-description");
    descLabel->setScale(0.375f);
    descLabel->setAnchorPoint({0.0f, 0.5f});
    descLabel->setPosition(leftPadding, itemHeight / 2 - 2.f);

    addChild(descLabel);

    // Role restriction label & Create and store role label for live updates
    std::string roleText;
    bool anyRole = false;
    if (!m_command.allowedUser.empty())
    {
        roleText += "User: " + m_command.allowedUser;
        anyRole = true;
    };

    if (m_command.allowVip)
    {
        if (anyRole)
            roleText += " | ";
        roleText += "VIP";
        anyRole = true;
    };

    if (m_command.allowMod)
    {
        if (anyRole)
            roleText += " | ";
        roleText += "Mod";
        anyRole = true;
    };

    if (m_command.allowSubscriber)
    {
        if (anyRole)
            roleText += " | ";
        roleText += "Subscriber";
        anyRole = true;
    };

    if (m_command.allowStreamer)
    {
        if (anyRole)
            roleText += " | ";
        roleText += "Streamer";
        anyRole = true;
    };

    if (!anyRole)
        roleText = "Everyone";

    m_roleLabel = CCLabelBMFont::create(roleText.c_str(), "goldFont.fnt");
    m_roleLabel->setID("command-roles");
    m_roleLabel->setScale(0.32f);
    m_roleLabel->setAnchorPoint({0.0f, 1.0f});
    // Place directly under the description label

    float descBottom = itemHeight / 2 - 3 - (descLabel->getContentSize().height * descLabel->getScale()) / 2;

    m_roleLabel->setPosition(leftPadding, descBottom + 1.f);
    m_roleLabel->setScale(0.35f);

    addChild(m_roleLabel);

    // Create menu with sufficient padding for better touch detection
    auto commandEditMenu = CCMenu::create();
    commandEditMenu->setID("command-edit-menu");
    commandEditMenu->ignoreAnchorPointForPosition(false);
    commandEditMenu->setContentSize({180, 40}); // Wider for four buttons

    // Create edit, delete, and settings buttons
    auto editBtn = createEditButton();
    auto deleteBtn = createDeleteButton();
    auto settingsBtn = createSettingsButton();

    auto enableOffSprite = ButtonSprite::create("Enabled", "bigFont.fnt", "GJ_button_01.png", 0.3f);
    auto enableOnSprite = ButtonSprite::create("Disabled", "bigFont.fnt", "GJ_button_06.png", 0.3f);

    // Create enable/disable toggle button
    auto enableToggle = CCMenuItemToggler::create(
        enableOnSprite,
        enableOffSprite,
        this,
        menu_selector(CommandActionEventNode::onToggleEnableCommand));
    enableToggle->setID("enable-command-toggle");
    enableToggle->setContentSize({60.0f, 40.0f});

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
};

CommandActionEventNode *CommandActionEventNode::createCommandNode(TwitchDashboard *parent, TwitchCommand command, float width)
{
    auto ret = new CommandActionEventNode();

    if (ret && ret->initCommandNode(parent, command, width))
    {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};

void CommandActionEventNode::triggerCommand()
{
    if (m_command.cooldown > 0)
    {
        if (m_isOnCooldown)
        {
            log::info("Command '{}' is currently on cooldown ({}s remaining)", m_command.name, m_cooldownRemaining);
            return;
        }
        startCooldown();
    }
};

void CommandActionEventNode::startCooldown()
{
    m_cooldownRemaining = m_command.cooldown;
    m_isOnCooldown = true;

    schedule(schedule_selector(CommandActionEventNode::updateCooldown), 1.0f);
    updateCooldown(0);
};

void CommandActionEventNode::onCopyCommandName(cocos2d::CCObject *sender)
{
    std::string cmd = "!" + m_command.name;
    geode::utils::clipboard::write(cmd);
    Notification::create(fmt::format("Copied '{}' to clipboard!", cmd), NotificationIcon::Success)->show();
};

void CommandActionEventNode::resetCooldown()
{
    unschedule(schedule_selector(CommandActionEventNode::updateCooldown));

    m_isOnCooldown = false;
    if (m_commandBg)
        m_commandBg->setColor({255, 255, 255}); // White

    if (m_command.cooldown > 0)
    {
        m_cooldownLabel->setString(fmt::format("({}s)", m_command.cooldown).c_str());
    }
    else
    {
        m_cooldownLabel->setString("");
    };
};

void CommandActionEventNode::updateCooldown(float dt)
{
    if (m_cooldownRemaining > 0)
    {
        m_cooldownLabel->setString(fmt::format("({}s)", m_cooldownRemaining).c_str());
        m_cooldownRemaining--;
    }
    else
    {
        resetCooldown();
    };
};

void CommandActionEventNode::onDeleteCommand(cocos2d::CCObject *sender)
{
    auto menuItem = static_cast<CCMenuItem *>(sender);
    runAction(CCFadeTo::create(0.2f, 120));

    log::info("Deleting command: {}", m_command.name);

    m_parent->handleCommandDelete(m_command.name);
};

void CommandActionEventNode::onEditCommand(cocos2d::CCObject *sender)
{
    auto menuItem = static_cast<CCMenuItem *>(sender);

    log::info("Editing command: {}", m_command.name);

    m_parent->onEditCommand(sender);
};

void CommandActionEventNode::onSettingsCommand(cocos2d::CCObject *sender)
{
    log::info("Settings button clicked for command: {}", m_command.name);
    auto commandManager = TwitchCommandManager::getInstance();

    for (const auto &cmd : commandManager->getCommands())
    {
        if (cmd.name == m_command.name)
        {
            m_command = cmd;
            break;
        };
    };

    if (auto popup = CommandSettingsPopup::create(m_command))
    {
        popup->m_noElasticity = true;
        popup->show();
    };
};

void CommandActionEventNode::onToggleEnableCommand(cocos2d::CCObject *sender)
{
    m_command.enabled = !m_command.enabled;

    log::info("Command '{}' enabled state set to {}", m_command.name, m_command.enabled);

    auto commandManager = TwitchCommandManager::getInstance();
    commandManager->enableCommand(m_command.name, m_command.enabled);
};

cocos2d::CCMenuItem *CommandActionEventNode::createSettingsButton()
{
    auto settingsBtnSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingsBtnSprite->setScale(0.65f);

    // create the settings button
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsBtnSprite,
        this,
        menu_selector(CommandActionEventNode::onSettingsCommand));
    settingsBtn->setID("settings-btn");
    settingsBtn->ignoreAnchorPointForPosition(true);
    settingsBtn->setContentSize({40.0f, 40.0f});

    auto btnSprite = settingsBtn->getNormalImage();
    if (btnSprite)
        btnSprite->setPosition(20.0f, 20.0f);

    return settingsBtn;
};

cocos2d::CCMenuItem *CommandActionEventNode::createEditButton()
{
    auto editBtnSprite = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
    editBtnSprite->setScale(0.4f);

    // create the edit button
    auto editBtn = CCMenuItemSpriteExtra::create(
        editBtnSprite,
        this,
        menu_selector(CommandActionEventNode::onEditCommand));
    editBtn->setID("edit-btn");
    editBtn->ignoreAnchorPointForPosition(true);
    editBtn->setContentSize({40.0f, 40.0f});

    auto btnSprite = editBtn->getNormalImage();
    if (btnSprite)
        btnSprite->setPosition(20.0f, 20.0f);

    return editBtn;
};

cocos2d::CCMenuItem *CommandActionEventNode::createDeleteButton()
{
    auto deleteBtnSprite = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
    deleteBtnSprite->setScale(0.7f);

    // create the delete button
    auto deleteBtn = CCMenuItemSpriteExtra::create(
        deleteBtnSprite,
        this,
        menu_selector(CommandActionEventNode::onDeleteCommand));
    deleteBtn->setID("delete-btn");
    deleteBtn->ignoreAnchorPointForPosition(true);
    deleteBtn->setContentSize({40.0f, 40.0f});

    auto btnSprite = deleteBtn->getNormalImage();
    if (btnSprite)
        btnSprite->setPosition(20.0f, 20.0f);

    return deleteBtn;
};

// Action Node logic
bool CommandActionEventNode::initActionNode(const std::string &labelText, CCObject *target, SEL_MenuHandler selector, float checkboxScale,
                                            CCObject *moveTarget, SEL_MenuHandler moveUpSelector, SEL_MenuHandler moveDownSelector, int actionIndex, bool canMoveUp, bool canMoveDown)
{
    if (!CCNode::init())
        return false;

    setContentSize(CCSize(370.f, 32.f));

    m_upBtn = nullptr;
    m_downBtn = nullptr;

    if (moveTarget && (moveUpSelector || moveDownSelector))
    {
        auto upSprite = CCSprite::createWithSpriteFrameName("edit_upBtn_001.png");
        upSprite->setScale(0.5f);
        upSprite->setRotation(0.f);

        // Create move up button
        m_upBtn = ::CCMenuItemSpriteExtra::create(
            upSprite,
            moveTarget,
            moveUpSelector);
        m_upBtn->setID("move-up");
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
            moveDownSelector);
        m_downBtn->setID("move-down");
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
    m_label->setAnchorPoint({0, 0.5f});
    m_label->setAlignment(kCCTextAlignmentLeft);
    m_label->setPosition(50.f, 16.f);

    addChild(m_label);

    return true;
};

CommandActionEventNode *CommandActionEventNode::createActionNode(const std::string &labelText, CCObject *target, SEL_MenuHandler selector, float checkboxScale,
                                                                 CCObject *moveTarget, SEL_MenuHandler moveUpSelector, SEL_MenuHandler moveDownSelector, int actionIndex, bool canMoveUp, bool canMoveDown)
{
    auto ret = new CommandActionEventNode();

    if (ret && ret->initActionNode(labelText, target, selector, checkboxScale, moveTarget, moveUpSelector, moveDownSelector, actionIndex, canMoveUp, canMoveDown))
    {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};

// Event Node logic
bool CommandActionEventNode::initEventNode(const std::string &labelText, CCObject *target, SEL_MenuHandler selector, float checkboxScale)
{
    if (!CCNode::init())
        return false;

    setContentSize(CCSize(370.f, 32.f));

    m_checkbox = CCMenuItemToggler::createWithStandardSprites(target, selector, checkboxScale);
    m_checkbox->setPosition(20.f, 16.f);

    m_label = CCLabelBMFont::create(labelText.c_str(), "bigFont.fnt");
    m_label->setScale(0.5f);
    m_label->setAnchorPoint({0, 0.5f});
    m_label->setAlignment(kCCTextAlignmentLeft);
    m_label->setPosition(50.f, 16.f);

    // Store description for FLAlertLayer
    m_eventDescription = "";

    for (const auto &node : getAllEventNodes())
    {
        if (node.label == labelText)
        {
            m_eventDescription = node.description;
            break;
        }
    }

    auto labelNode = CCNode::create();
    labelNode->setPosition(0, 0);

    labelNode->addChild(m_label);

    addChild(labelNode);

    auto eventMenu = CCMenu::create();
    eventMenu->setPosition(0, 0);

    eventMenu->addChild(m_checkbox);

    addChild(eventMenu);

    return true;
};

CommandActionEventNode *CommandActionEventNode::createEventNode(const std::string &labelText, CCObject *target, SEL_MenuHandler selector, float checkboxScale)
{
    auto ret = new CommandActionEventNode();

    if (ret && ret->initEventNode(labelText, target, selector, checkboxScale))
    {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};

std::vector<EventNodeInfo> CommandActionEventNode::getAllEventNodes()
{
    std::vector<EventNodeInfo> nodes = {
        {"reverse_player", "Reverse Player", "Reverses the player direction. <cg>Only works well on classic level.</c>."},
        {"kill_player", "Destroy Player", "Destroy player. Self-explanatory. <cr>Don't use this while beating extremes!</c>"},
        {"player_effect", "Player Effect", "Play a player visual effect such as <cg>Spawn</c> or <cr>Death</c>"},
        {"jump", "Jump", "Force the player to jump. You can set it to also hold jump."},
        {"move", "Move Player", "Move the player left or right. Lets you pick the player, direction and the distance to move. <cg>Works only on Platformers.</c>"},
        {"color_player", "Color Player", "Set the player's color based on the RGB value. <cr>Broken on Android users at this moment.</c>"},
        {"wait", "Wait", "Pauses the command sequence for a set amount of time (in seconds). <cg>Use as a delay between actions.</c>"},
        {"notification", "Notification", "Shows a notification message on the screen. <cg>Supports the use of identifiers.</c>"},
        {"alert_popup", "Alert Popup", "Shows an alert popup like this one you reading. <cg>Supports the use of identifiers.</c>"},
        {"open_level", "Level Info", "Opens Level Info for a level by ID or force play a level. <cg>Supports identifiers.</c>"},
        {"profile", "Profile", "Opens the Player Profile in-game. <cg>Supports use of identifiers.</c>"},
        {"keycode", "Key Code", "Simulates a key press or release. <cr>Does not work on mobile users.</c>"},
        {"scale_player", "Scale Player", "Scales the player in-game. <cr>Does not affect the player hitbox.</c>"},
        {"sound_effect", "Sound Effect", "Plays a sound effect. <cg>Supports Custom SFX and & GD default SFX.</c>"},
        {"stop_all_sounds", "Stop All Sounds", "Stops all currently playing sound effects immediately"},
        {"gravity", "Gravity Player", "Sets the player's gravity to a specified value for a duration."},
        {"speed_player", "Speed Player", "Sets the player's speed to a specified value for a duration."},
        {"restart_level", "Restart Level", "Restarts the entire level. <cy>For clarification, it does not go back to previous checkpoints.</c>"},
        {"jumpscare", "Jumpscare", "Shows a custom jumpscare image to scare the streamer. <cy>boo!</c>"},
        {"noclip", "Noclip", "Enables or disables noclip mode for the player. <cr>This does not have Safe Mode, use with caution!</c> <cy>Disables upon exiting the level.</c>"},
        {"nothing", "Nothing", "Does nothing at all. <cy>Dev note: this is added so i can just copy paste new events easily.</c>"}};

    auto mod = Mod::get();
    if (mod && mod->getSettingValue<bool>("experimental"))
    {
        nodes.push_back({"edit_camera", "Edit Camera", "Edit Camera's Skew, Rotation, and Scale. <cy>Experimental Feature. May crash your game.</c>"});
    }
    return nodes;
}

// Unified interface
bool CommandActionEventNode::init(TwitchCommandAction action, CCSize scrollSize)
{
    m_action = action;

    if (!CCNode::create())
        return false;

    setContentSize(CCSize(scrollSize.width, 32.f));

    return true;
};

// Update the role label text to reflect current m_command
void CommandActionEventNode::updateRoleLabel()
{
    if (!m_roleLabel)
        return;

    std::string roleText;
    bool anyRole = false;

    if (!m_command.allowedUser.empty())
    {
        roleText += "User: " + m_command.allowedUser;
        anyRole = true;
    };

    if (m_command.allowVip)
    {
        if (anyRole)
            roleText += " | ";

        roleText += "VIP";
        anyRole = true;
    };

    if (m_command.allowMod)
    {
        if (anyRole)
            roleText += " | ";

        roleText += "Mod";
        anyRole = true;
    };

    if (m_command.allowSubscriber)
    {
        if (anyRole)
            roleText += " | ";

        roleText += "Subscriber";
        anyRole = true;
    };

    if (m_command.allowStreamer)
    {
        if (anyRole)
            roleText += " | ";

        roleText += "Streamer";
        anyRole = true;
    };

    if (!anyRole)
        roleText = "Everyone";
    m_roleLabel->setString(roleText.c_str());
};

CommandActionEventNode *CommandActionEventNode::create(TwitchCommandAction action, CCSize scrollSize)
{
    auto ret = new CommandActionEventNode();

    if (ret && ret->init(action, scrollSize))
    {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};