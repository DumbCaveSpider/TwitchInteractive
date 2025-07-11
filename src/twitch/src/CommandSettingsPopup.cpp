
#include "../CommandSettingsPopup.hpp"
#include <Geode/Geode.hpp>
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



    // --- Event Section ---
    // Center the event section in the popup
    float eventSectionWidth = 400.f;
    float eventSectionHeight = 110.f;
    float popupWidth = layerSize.width;
    float popupHeight = layerSize.height;
    float eventSectionX = (popupWidth - eventSectionWidth) / 2;
    float eventSectionY = (popupHeight - eventSectionHeight) / 2 + 20.f; // +20 for slight upward bias



    // Scroll layer for events
    auto eventScrollLayer = ScrollLayer::create(CCSize(eventSectionWidth - 10.f, eventSectionHeight + 40.f));
    eventScrollLayer->setID("events-scroll");
    // Position to match the background
    eventScrollLayer->setPosition(eventSectionX + 5.f, eventSectionY - 35.f);

    // Background for the scroll layer (now matches scroll layer size exactly)
    auto eventScrollBg = CCScale9Sprite::create("square02_001.png");
    eventScrollBg->setContentSize(eventScrollLayer->getContentSize());
    eventScrollBg->setOpacity(80);
    eventScrollBg->setID("events-scroll-background");
    // Position to match the scroll layer
    eventScrollBg->setPosition(eventSectionX + 5.f + eventScrollLayer->getContentSize().width / 2,
                               eventSectionY - 35.f + eventScrollLayer->getContentSize().height / 2);
    m_mainLayer->addChild(eventScrollBg);

    // Content layer for event nodes
    auto eventContent = eventScrollLayer->m_contentLayer;
    eventContent->setID("events-content");
    // Use a vertical column layout for event nodes, aligned to the top
    auto eventLayout = ColumnLayout::create()
        ->setAxisReverse(false) // Top to bottom
        ->setAxisAlignment(AxisAlignment::Start) // Top alignment
        ->setCrossAxisAlignment(AxisAlignment::Start)
        ->setAutoGrowAxis(eventSectionHeight - 10.f)
        ->setGap(8.0f);
    eventContent->setLayout(eventLayout);
    eventContent->setContentSize(CCSize(eventSectionWidth - 10.f, eventSectionHeight - 10.f));


    // Use EventNode for event nodes
    auto killPlayerNode = EventNode::create("Kill Player", this, menu_selector(CommandSettingsPopup::onKillPlayerToggled), 0.6f);
    killPlayerNode->setContentSize(CCSize(eventSectionWidth - 30.f, 32.f));
    killPlayerNode->m_checkbox->setID("command-settings-killplayer-checkbox");
    killPlayerNode->m_label->setID("event-killplayer-label");
    m_killPlayerCheckbox = killPlayerNode->m_checkbox;
    eventContent->addChild(killPlayerNode);

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
    commandBtnMenu->setPosition(25.f, 40);
    m_mainLayer->addChild(commandBtnMenu);

    return true;
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