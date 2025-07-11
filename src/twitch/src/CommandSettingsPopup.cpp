
#include "../CommandSettingsPopup.hpp"
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include "events/PlayLayerEvent.hpp"
#include "../handler/EventNode.hpp"

using namespace geode::prelude;

bool CommandSettingsPopup::setup(TwitchCommand command) {
    this->setTitle(fmt::format("!{} settings", command.name));
    this->setID("command-settings-popup");
    m_command = command;

    auto layerSize = m_mainLayer->getContentSize();

    // Create TextInput for custom notification
    m_notificationInput = TextInput::create(400, "Custom notification (leave empty to disable)", "bigFont.fnt");
    m_notificationInput->setPosition(layerSize.width / 2, layerSize.height - 50);
    m_notificationInput->setScale(0.8f);
    m_notificationInput->setID("command-settings-notification-input");
    // If the command has a notification action, prefill it with the value if set, otherwise leave blank
    bool foundCustomNotif = false;
    for (const auto& action : command.actions) {
        if (action.type == CommandActionType::Notification) {
            if (!action.arg.empty()) {
                m_notificationInput->setString(action.arg.c_str());
            } else {
                m_notificationInput->setString("");
            }
            foundCustomNotif = true;
            break;
        }
    }
    if (!foundCustomNotif) {
        m_notificationInput->setString("");
    }
    m_mainLayer->addChild(m_notificationInput);



    // --- Event & Action Section ---
    // Set fixed size for event and action scroll layers
    float sectionWidth = 250.f;
    float sectionHeight = 200.f;
    float popupWidth = layerSize.width;
    float popupHeight = layerSize.height;
    float sectionY = (popupHeight - sectionHeight) / 2.0f;
    // Center both sections horizontally
    float gap = 20.f; // Gap between event and action layers
    float totalSectionsWidth = sectionWidth * 2 + gap;
    float startX = (popupWidth - totalSectionsWidth) / 2.0f;
    float eventSectionX = startX;
    float actionSectionX = startX + sectionWidth + gap;

    CCSize eventScrollSize = CCSize(sectionWidth, sectionHeight);
    CCSize actionScrollSize = CCSize(sectionWidth, sectionHeight);

    // Background for the event scroll layer
    auto eventScrollBg = CCScale9Sprite::create("square02_001.png");
    eventScrollBg->setContentSize(eventScrollSize);
    eventScrollBg->setOpacity(80);
    eventScrollBg->setID("events-scroll-background");
    float scrollX = eventSectionX;
    float scrollY = sectionY;
    eventScrollBg->setPosition(scrollX + eventScrollSize.width / 2, scrollY + eventScrollSize.height / 2);
    m_mainLayer->addChild(eventScrollBg);

    // Background for the actions scroll layer
    auto actionScrollBg = CCScale9Sprite::create("square02_001.png");
    actionScrollBg->setContentSize(actionScrollSize);
    actionScrollBg->setOpacity(80);
    actionScrollBg->setID("actions-scroll-background");
    actionScrollBg->setPosition(actionSectionX + actionScrollSize.width / 2, scrollY + actionScrollSize.height / 2);
    m_mainLayer->addChild(actionScrollBg);

    // Scroll layer for events
    auto eventScrollLayer = ScrollLayer::create(eventScrollSize);
    eventScrollLayer->setID("events-scroll");
    eventScrollLayer->setPosition(scrollX, scrollY);

    // Scroll layer for actions
    auto actionScrollLayer = ScrollLayer::create(actionScrollSize);
    actionScrollLayer->setID("actions-scroll");
    actionScrollLayer->setPosition(actionSectionX, scrollY);


    // Content layer for event nodes
    auto eventContent = eventScrollLayer->m_contentLayer;
    eventContent->setID("events-content");
    eventContent->setContentSize(eventScrollSize);
    auto eventLayout = ColumnLayout::create()
        ->setAxisReverse(true) // Make items stack from the top
        ->setAxisAlignment(AxisAlignment::Start)
        ->setCrossAxisAlignment(AxisAlignment::Start)
        ->setAutoGrowAxis(eventScrollSize.height)
        ->setGap(8.0f);
    eventContent->setLayout(eventLayout);

    // Content layer for actions
    auto actionContent = actionScrollLayer->m_contentLayer;
    actionContent->setID("actions-content");
    actionContent->setContentSize(actionScrollSize);
    auto actionLayout = ColumnLayout::create()
        ->setAxisReverse(false)
        ->setAxisAlignment(AxisAlignment::Start)
        ->setCrossAxisAlignment(AxisAlignment::Start)
        ->setAutoGrowAxis(actionScrollSize.height)
        ->setGap(8.0f);
    actionContent->setLayout(actionLayout);


    // Store actions for this command as a member
    m_commandActions.clear();
    m_actionContent = actionContent;
    m_actionSectionHeight = sectionHeight;
    refreshActionsList();

    // Dynamically add all event nodes from EventNodeFactory, each with an add button (manual layout)
    float eventNodeY = eventScrollSize.height - 16.f; // Start from top, 16px for half node height
    float eventNodeGap = 8.0f;
    for (const auto& info : EventNodeFactory::getAllEventNodes()) {
        auto node = CCNode::create();
        node->setContentSize(CCSize(eventScrollSize.width, 32.f));
        // Label
        auto label = CCLabelBMFont::create(info.label.c_str(), "bigFont.fnt");
        label->setScale(0.5f);
        label->setAnchorPoint({0, 0.5f});
        label->setAlignment(kCCTextAlignmentLeft);
        label->setPosition(20.f, 16.f);
        label->setID("event-" + info.id + "-label");
        
        // Add button (always use GJ_plusBtn_001.png)
        auto addSprite = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        addSprite->setScale(0.5f);
        auto addBtn = CCMenuItemSpriteExtra::create(
            addSprite,
            this,
            menu_selector(CommandSettingsPopup::onAddEventAction)
        );
        addBtn->setID("event-" + info.id + "-add-btn");
        addBtn->setPosition(eventScrollSize.width - 60.f, 16.f);
        addBtn->setUserObject(CCString::create(info.id));
        // Menu for button
        auto menu = CCMenu::create();
        menu->addChild(addBtn);
        menu->setPosition(0, 0);
        node->addChild(label);
        node->addChild(menu);
        // Add a background to the event node
        auto nodeBg = CCScale9Sprite::create("square02_001.png");
        nodeBg->setContentSize(node->getContentSize());
        nodeBg->setOpacity(60);
        nodeBg->setAnchorPoint({0, 0});
        nodeBg->setPosition(0, 0);
        node->addChild(nodeBg, -1);
        node->setPosition(0, eventNodeY - 16.f); // 16.f is half node height
        eventContent->addChild(node);
        eventNodeY -= (32.f + eventNodeGap);
    }

    m_mainLayer->addChild(actionScrollLayer);
    m_mainLayer->addChild(eventScrollLayer);
    // Set checkbox state from command actions
    bool killChecked = false;
    for (const auto& action : command.actions) {
        if (action.type == CommandActionType::Event && action.arg == "kill_player") {
            killChecked = true;
            break;
        }
    }
    if (m_killPlayerCheckbox) m_killPlayerCheckbox->toggle(killChecked);

    // Save button
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(CommandSettingsPopup::onSave)
    );
    saveBtn->setID("command-settings-save-btn");

    // Close button
    auto closeBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Close", "bigFont.fnt", "GJ_button_06.png", 0.6f),
        this,
        menu_selector(CommandSettingsPopup::onCloseBtn)
    );
    closeBtn->setID("command-settings-close-btn");

    // Menu for buttons
    auto commandBtnMenu = CCMenu::create();
    commandBtnMenu->setID("command-settings-button-menu");
    commandBtnMenu->addChild(saveBtn);
    commandBtnMenu->addChild(closeBtn);
    commandBtnMenu->setContentSize({570.f, 25.f});
    auto menuSize = commandBtnMenu->getContentSize();
    float menuWidth = menuSize.width;
    float menuHeight = menuSize.height;
    float centerY = menuHeight / 2;
    float spacing = 120.0f;
    saveBtn->setPosition(menuWidth / 2 - spacing / 2, centerY);
    closeBtn->setPosition(menuWidth / 2 + spacing / 2, centerY);
    commandBtnMenu->setPosition(25.f, 15.f);
    m_mainLayer->addChild(commandBtnMenu);

    return true;
}

void CommandSettingsPopup::onAddEventAction(cocos2d::CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    std::string eventId;
    if (btn->getUserObject()) {
        eventId = static_cast<CCString*>(btn->getUserObject())->getCString();
    }
    if (!eventId.empty()) {
        m_commandActions.push_back(eventId);
        refreshActionsList();
    }
}

void CommandSettingsPopup::refreshActionsList() {
    if (!m_actionContent) return;
    m_actionContent->removeAllChildren();
    float actionNodeY = m_actionSectionHeight - 16.f; // Start from top, 16px for half node height
    float actionNodeGap = 8.0f;
    for (const auto& actionId : m_commandActions) {
        auto node = CCNode::create();
        node->setContentSize(CCSize(m_actionContent->getContentSize().width, 32.f));
        // Find the event label/title for this actionId
        std::string eventLabel = actionId;
        for (const auto& info : EventNodeFactory::getAllEventNodes()) {
            if (info.id == actionId) {
                eventLabel = info.label;
                break;
            }
        }
        // Label
        auto label = CCLabelBMFont::create(eventLabel.c_str(), "bigFont.fnt");
        label->setScale(0.5f);
        label->setAnchorPoint({0, 0.5f});
        label->setAlignment(kCCTextAlignmentLeft);
        label->setPosition(20.f, 16.f);
        label->setID("action-" + actionId + "-label");
        // Remove button (always use GJ_trashBtn_001.png)
        auto removeSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
        removeSprite->setScale(0.5f);
        auto removeBtn = CCMenuItemSpriteExtra::create(
            removeSprite,
            this,
            menu_selector(CommandSettingsPopup::onRemoveAction)
        );
        removeBtn->setID("action-" + actionId + "-remove-btn");
        removeBtn->setPosition(m_actionContent->getContentSize().width - 60.f, 16.f);
        removeBtn->setUserObject(CCString::create(actionId));
        // Menu for button
        auto menu = CCMenu::create();
        menu->addChild(removeBtn);
        menu->setPosition(0, 0);
        node->addChild(label);
        node->addChild(menu);
        // Add a background to the action node
        auto nodeBg = CCScale9Sprite::create("square02_001.png");
        nodeBg->setContentSize(node->getContentSize());
        nodeBg->setOpacity(60);
        nodeBg->setAnchorPoint({0, 0});
        nodeBg->setPosition(0, 0);
        node->addChild(nodeBg, -1);
        node->setPosition(0, actionNodeY - 16.f); // 16.f is half node height
        m_actionContent->addChild(node);
        actionNodeY -= (32.f + actionNodeGap);
    }
}

void CommandSettingsPopup::onRemoveAction(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    std::string actionId;
    if (btn->getUserObject()) {
        actionId = static_cast<CCString*>(btn->getUserObject())->getCString();
    }
    if (!actionId.empty()) {
        auto it = std::find(m_commandActions.begin(), m_commandActions.end(), actionId);
        if (it != m_commandActions.end()) {
            m_commandActions.erase(it);
            refreshActionsList();
        }
    }
}

void CommandSettingsPopup::onCloseBtn(CCObject* sender) {
    this->onClose(sender);
}

void CommandSettingsPopup::onKillPlayerToggled(CCObject* sender) {
    // This just toggles the checkbox, actual logic is handled on save
}


CommandSettingsPopup* CommandSettingsPopup::create(TwitchCommand command) {
    auto ret = new CommandSettingsPopup();
    if (ret && ret->initAnchored(620.f, 325.f, command)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

std::string CommandSettingsPopup::getNotificationText() const {
    if (m_notificationInput) {
        std::string text = m_notificationInput->getString();
        // Trim whitespace
        text.erase(0, text.find_first_not_of(" \t\n\r"));
        text.erase(text.find_last_not_of(" \t\n\r") + 1);
        return text;
    }
    return "";
}

void CommandSettingsPopup::onSave(CCObject* sender) {
    std::string notifText = getNotificationText();
    // Update the command's notification action
    bool found = false;
    for (auto& action : m_command.actions) {
        if (action.type == CommandActionType::Notification) {
            action.arg = notifText;
            found = true;
            break;
        }
    }
    if (!found && !notifText.empty()) {
        // Add a notification action if not present
        for (auto& action : m_command.actions) {
            if (action.type == CommandActionType::Notification || action.type == CommandActionType::Chat || action.type == CommandActionType::Keybind) {
                action = TwitchCommandAction(CommandActionType::Notification, notifText, 0);
                found = true;
                break;
            }
        }
    }

    // Save kill player setting as a custom action (persisted in command actions)
    // Always clear any previous kill_player action
    for (auto& action : m_command.actions) {
        if (action.type == CommandActionType::Event && action.arg == "kill_player") {
            action = TwitchCommandAction(); // Reset to default
        }
    }
    // If checked, add or update a kill_player action in the first available slot
    if (m_killPlayerCheckbox && m_killPlayerCheckbox->isToggled()) {
        for (auto& action : m_command.actions) {
            if (action.type == CommandActionType::Notification && action.arg.empty()) {
                action = TwitchCommandAction(CommandActionType::Event, "kill_player", 0);
                break;
            }
        }
    }

    Notification::create("Command Settings Saved!", NotificationIcon::Success)->show();

    // Save changes to the command manager
    auto commandManager = TwitchCommandManager::getInstance();
    for (auto& cmd : commandManager->getCommands()) {
        if (cmd.name == m_command.name) {
            cmd.actions = m_command.actions;
            break;
        }
    }
    commandManager->saveCommands();
    this->onClose(nullptr);
}