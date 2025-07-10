#include "../CommandSettingsPopup.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;


bool CommandSettingsPopup::setup(TwitchCommand command) {
    this->setTitle(fmt::format("!{} settings", command.name));
    this->setID("command-settings-popup");
    m_command = command;

    auto layerSize = m_mainLayer->getContentSize();

    // Create TextInput for custom notification
    m_notificationInput = TextInput::create(400, "Custom notification (leave empty to disable)", "bigFont.fnt");
    m_notificationInput->setPosition(layerSize.width / 2, layerSize.height - 80);
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

    // Save button
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(CommandSettingsPopup::onSave)
    );
    saveBtn->setID("command-settings-save-btn");
    auto menu = CCMenu::create();
    menu->addChild(saveBtn);
    menu->setPosition(layerSize.width / 2, 40);
    m_mainLayer->addChild(menu);

    return true;
};

CommandSettingsPopup* CommandSettingsPopup::create(TwitchCommand command) {
    auto ret = new CommandSettingsPopup();
    // initAnchored takes width, height, and then the parameters for setup
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
    if (!notifText.empty()) {
        Notification::create("Custom notification saved!", NotificationIcon::Success)->show();
    } else {
        Notification::create("Notification disabled for this command.", NotificationIcon::Info)->show();
    }
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