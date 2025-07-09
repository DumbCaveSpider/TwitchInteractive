#include "CommandNode.hpp"
#include "TwitchDashboard.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

bool CommandNode::init(TwitchDashboard* parent, TwitchCommand command, float width) {
    m_parent = parent;
    m_command = command;

    if (!CCNode::init()) return false;

    // Set standard item height
    const float itemHeight = 40.0f;

    // Calculate width to center within scroll layer
    float itemWidth = width - 10; // Less margin for better filling

    // Set item size for layout to work properly - use full width of scroll layer
    setContentSize(CCSize(width, itemHeight));

    // Background - fill the entire command item space
    auto commandBg = CCScale9Sprite::create("square02_small.png");
    commandBg->setContentSize(CCSize(itemWidth, itemHeight - 1));
    commandBg->setPosition(width / 2, itemHeight / 2); // Center in the command item
    commandBg->setOpacity(100);

    addChild(commandBg);

    // Left side padding
    float leftPadding = 15.f;

    // Command name label - positioned on the left side
    auto nameLabel = CCLabelBMFont::create(("!" + m_command.name).c_str(), "bigFont.fnt");
    nameLabel->setScale(0.4f);
    nameLabel->setAnchorPoint({ 0.0f, 0.5f }); // Left-aligned
    nameLabel->setPosition(leftPadding, itemHeight / 2 + 5); // Top half of container

    addChild(nameLabel);

    // Command description label - positioned on the left side below the name
    auto descLabel = CCLabelBMFont::create(m_command.description.c_str(), "chatFont.fnt");
    descLabel->setScale(0.375f);
    descLabel->setAnchorPoint({ 0.0f, 0.5f }); // Left-aligned
    descLabel->setPosition(leftPadding, itemHeight / 2 - 8); // Bottom half of container

    addChild(descLabel);

    // Create menu with sufficient padding for better touch detection
    auto editMenu = CCMenu::create();
    editMenu->setID("delete-menu");
    editMenu->ignoreAnchorPointForPosition(false);

    // Make sure menu has consistent size for hit detection
    editMenu->setContentSize({ 40, 40 });

    // Create delete button exactly matching the menu's size
    auto deleteBtn = createDeleteButton();

    // Add button to menu
    editMenu->addChild(deleteBtn);

    // Position menu at right side of the item, center vertically
    editMenu->setPosition(width - 30, itemHeight / 2);

    // Set high touch priority to ensure buttons are clickable
    editMenu->setTouchPriority(-130); // Higher priority than default
    // CCMenu doesn't use ccTouchesMode, it has its own handling

    addChild(editMenu);

    return true;
};

CCMenuItem* CommandNode::createDeleteButton() {
    // Create edit button sprite with proper scaling
    auto editBtnSprite = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
    editBtnSprite->setScale(0.5f); // Slightly larger icon

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

void CommandNode::onDeleteCommand(CCObject* sender) {
    // Get the menu item that was clicked
    auto menuItem = static_cast<CCMenuItem*>(sender);

    // Find and visually disable the parent command item as well
    runAction(CCFadeTo::create(0.2f, 120)); // Add a slight fade effect to indicate deletion

    log::info("Deleting command: {}", m_command.name);

    // Delete the command
    m_parent->handleCommandDelete(m_command.name);


};

void CommandNode::onEditCommand(CCObject* sender) {
    // Get the menu item that was clicked
    auto menuItem = static_cast<CCMenuItem*>(sender);

    log::info("Editing command: {}", m_command.name);

    // Call the parent's edit handler
    m_parent->onEditCommand(sender);
    m_parent->handleCommandEdit(m_command.name, m_command.name, m_command.description);
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