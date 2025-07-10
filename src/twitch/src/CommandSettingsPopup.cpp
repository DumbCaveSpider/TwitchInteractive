
#include "../CommandSettingsPopup.hpp"
#include <Geode/Geode.hpp>
#include "events/PlayLayerEvent.hpp"

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



    // Kill Player Checkbox
    m_killPlayerCheckbox = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(CommandSettingsPopup::onKillPlayerToggled), 0.6f);
    m_killPlayerCheckbox->setID("command-settings-killplayer-checkbox");
    m_killPlayerCheckbox->setPosition(0, 0);
    auto killPlayerLabel = CCLabelBMFont::create("Kill Player", "bigFont.fnt");
    killPlayerLabel->setScale(0.5f);
    killPlayerLabel->setAnchorPoint({0, 0.5f});
    killPlayerLabel->setPosition(30, 0);

    // Create a container node for the event menu
    auto eventPlayerMenu = CCMenu::create();
    eventPlayerMenu->setID("command-settings-event-player-menu");
    eventPlayerMenu->setContentSize({400.f, 170.f});
    // Center the eventPlayerMenu in the popup (anchor point at center)
    eventPlayerMenu->setAnchorPoint({0.5f, 0.5f});
    eventPlayerMenu->setPosition(110, 85);

    // Add background to the event menu
    auto eventMenuBg = CCScale9Sprite::create("square02_001.png");
    eventMenuBg->setContentSize({400.f, 170.f});
    eventMenuBg->setPosition(eventPlayerMenu->getContentSize().width / 2, eventPlayerMenu->getContentSize().height / 2);
    eventMenuBg->setOpacity(80);
    eventPlayerMenu->addChild(eventMenuBg, -1);

    // Add the checkbox and label to the event menu, centered
    m_killPlayerCheckbox->setPosition(eventPlayerMenu->getContentSize().width / 2 - 60, eventPlayerMenu->getContentSize().height / 2);
    killPlayerLabel->setPosition(eventPlayerMenu->getContentSize().width / 2, eventPlayerMenu->getContentSize().height / 2);
    eventPlayerMenu->addChild(m_killPlayerCheckbox);
    eventPlayerMenu->addChild(killPlayerLabel);
    m_mainLayer->addChild(eventPlayerMenu);
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
    this->onClose(nullptr);
}