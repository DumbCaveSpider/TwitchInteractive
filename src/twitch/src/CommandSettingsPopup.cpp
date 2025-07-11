
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
    float eventSectionY = (popupHeight - eventSectionHeight) / 2 + 20.f;

    // Define a consistent size for both background and scroll layer
    CCSize eventScrollSize = CCSize(eventSectionWidth - 10.f, eventSectionHeight + 90.f);

    // Background for the scroll layer
    auto eventScrollBg = CCScale9Sprite::create("square02_001.png");
    eventScrollBg->setContentSize(eventScrollSize);
    eventScrollBg->setOpacity(80);
    eventScrollBg->setID("events-scroll-background");
    // Position background and scroll layer to be perfectly aligned
    float scrollX = eventSectionX + 5.f;
    float scrollY = eventSectionY - 65.f;
    eventScrollBg->setPosition(scrollX + eventScrollSize.width / 2, scrollY + eventScrollSize.height / 2);
    m_mainLayer->addChild(eventScrollBg);

    // Scroll layer for events (now matches background size exactly)
    auto eventScrollLayer = ScrollLayer::create(eventScrollSize);
    eventScrollLayer->setID("events-scroll");
    eventScrollLayer->setPosition(scrollX, scrollY);

    // Content layer for event nodes
    auto eventContent = eventScrollLayer->m_contentLayer;
    eventContent->setID("events-content");
    // Fill the entire scroll background
    eventContent->setContentSize(eventScrollSize - 10.f);
    // Use a vertical column layout for event nodes, aligned to the top
    auto eventLayout = ColumnLayout::create()
        ->setAutoGrowAxis(eventScrollSize.height)
        ->setGap(8.0f);
    eventContent->setLayout(eventLayout);


    // Dynamically add all event nodes from EventNodeFactory
    for (const auto& info : EventNodeFactory::getAllEventNodes()) {
        auto node = EventNode::create(info.label, this, menu_selector(CommandSettingsPopup::onKillPlayerToggled), 0.6f);
        node->setContentSize(CCSize(eventScrollSize.width, 32.f));
        node->m_checkbox->setID("command-settings-" + info.id + "-checkbox");
        node->m_label->setID("event-" + info.id + "-label");
        // Add a background to the event node
        auto nodeBg = CCScale9Sprite::create("square02_001.png");
        nodeBg->setContentSize(node->getContentSize());
        nodeBg->setOpacity(60);
        nodeBg->setAnchorPoint({0, 0});
        nodeBg->setPosition(0, 0);
        node->addChild(nodeBg, -1);
        eventContent->addChild(node);

        if (info.id == "kill_player") m_killPlayerCheckbox = node->m_checkbox;
    }

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