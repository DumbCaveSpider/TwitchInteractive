#include "CommandInputPopup.hpp"

#include "../TwitchCommandManager.hpp"
#include "../TwitchDashboard.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

CCMenu* CommandInputPopup::createButtonMenu() {
    auto layerSize = m_mainLayer->getContentSize();

    // Create button menu
    auto buttonMenu = CCMenu::create();
    buttonMenu->setID("command-input-button-menu");
    buttonMenu->setContentSize(CCSize(layerSize.width, 5)); // Set a fixed height for the menu
    buttonMenu->setPosition(0, 30);                         // Position the menu at the bottom center

    float centerX = buttonMenu->getContentSize().width / 2.0f;
    float centerY = buttonMenu->getContentSize().height / 2.0f;

    // Only add the Add/Edit button (centered)
    auto addBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create(m_isEditing ? "Edit" : "Add", "bigFont.fnt", "GJ_button_04.png", 0.6f),
        this,
        menu_selector(CommandInputPopup::onAdd));
    addBtn->setID("command-input-add-btn");
    addBtn->setPosition(centerX, centerY);

    buttonMenu->addChild(addBtn);

    return buttonMenu;
};

bool CommandInputPopup::setup() {
    setTitle(m_isEditing ? "Edit Command" : "Add Command");

    auto layerSize = m_mainLayer->getContentSize();

    // Set ID for the popup
    setID("command-input-popup");
    m_mainLayer->setID("command-input-main-layer");

    // Create text input for command name
    m_nameInput = TextInput::create(200, "Command name", "bigFont.fnt");
    m_nameInput->setCommonFilter(CommonFilter::Alphanumeric);
    m_nameInput->setPosition(layerSize.width / 2, layerSize.height - 55);
    m_nameInput->setScale(0.8f);
    m_nameInput->setID("command-input-name-field");

    m_mainLayer->addChild(m_nameInput);

    // Create text input for description
    m_descInput = TextInput::create(200, "Command description", "bigFont.fnt");
    m_descInput->setID("command-input-desc-field");
    m_descInput->setCommonFilter(CommonFilter::Any);
    m_descInput->setPosition(layerSize.width / 2, layerSize.height - 90);
    m_descInput->setScale(0.8f);

    m_mainLayer->addChild(m_descInput);

    // Create text input for cooldown seconds
    m_cooldownInput = TextInput::create(200, "Cooldown (Seconds)", "bigFont.fnt");
    m_cooldownInput->setID("command-input-cooldown-field");
    m_cooldownInput->setCommonFilter(CommonFilter::Int);
    m_cooldownInput->setPosition(layerSize.width / 2, layerSize.height - 125);
    m_cooldownInput->setScale(0.8f);

    m_mainLayer->addChild(m_cooldownInput);

    // Create and add the appropriate button menu
    auto buttonMenu = createButtonMenu();
    m_mainLayer->addChild(buttonMenu);

    return true;
};

void CommandInputPopup::setCallback(std::function<void(const std::string&, const std::string&)> callback) {
    m_callback = callback;
};

void CommandInputPopup::setupForEdit(const std::string& commandName, const std::string& commandDesc) {
    m_isEditing = true;

    m_originalName = commandName;
    m_originalDesc = commandDesc;

    // Parse cooldown if present
    size_t delim = commandDesc.find_last_of('|');
    if (delim != std::string::npos) {
        std::string desc = commandDesc.substr(0, delim);
        std::string cooldownStr = commandDesc.substr(delim + 1);

        m_descInput->setString(desc.c_str());
        m_cooldownInput->setString(cooldownStr.c_str());
    } else {
        m_descInput->setString(commandDesc.c_str());
        m_cooldownInput->setString("");
    };

    // Update the title to reflect we're in edit mode
    setTitle("Edit Command");

    // Change the add button to an edit button
    auto buttonMenu = m_mainLayer->getChildByID("command-input-button-menu");
    if (buttonMenu) {
        auto editBtn = buttonMenu->getChildByID("command-input-add-btn");
        auto editBtnSprite = typeinfo_cast<CCMenuItemSpriteExtra*>(editBtn);

        if (editBtnSprite) {
            // Update the label inside the button sprite
            auto btnSprite = static_cast<ButtonSprite*>(editBtnSprite->getNormalImage());
            if (btnSprite && btnSprite->m_label)
                btnSprite->m_label->setString("Edit");
        };
    };

    if (m_nameInput)
        m_nameInput->setString(commandName.c_str());
};

void CommandInputPopup::onAdd(CCObject* sender) {
    // Get the command name from the text input
    std::string commandName = m_nameInput->getString();
    std::string commandDesc = m_descInput->getString();
    std::string cooldownStr = m_cooldownInput->getString();

    // Trim whitespace for command name
    commandName.erase(0, commandName.find_first_not_of(" \t\n\r"));
    commandName.erase(commandName.find_last_not_of(" \t\n\r") + 1);

    // Trim whitespace for description
    commandDesc.erase(0, commandDesc.find_first_not_of(" \t\n\r"));
    commandDesc.erase(commandDesc.find_last_not_of(" \t\n\r") + 1);

    // Trim whitespace for cooldown
    cooldownStr.erase(0, cooldownStr.find_first_not_of(" \t\n\r"));
    cooldownStr.erase(cooldownStr.find_last_not_of(" \t\n\r") + 1);

    // Convert commandName to lowercase
    std::transform(commandName.begin(), commandName.end(), commandName.begin(), ::tolower);

    // Check if command name is empty
    if (commandName.empty()) {
        FLAlertLayer::create(
            "Error",
            "Please enter a command name",
            "OK"
        )->show();

        return;
    };

    // Remove any '!' prefix if user added it
    if (commandName.front() == '!') commandName = commandName.substr(1);

    // Check if still empty after removing '!'
    if (commandName.empty()) {
        FLAlertLayer::create(
            "Error",
            "Please enter a valid command name",
            "OK"
        )->show();

        return;
    };

    // Check for illegal characters in command name
    const std::string illegalChars = "!@#$%^&*()+={}[]|\\:;\"'<>,?/~`";
    if (commandName.find_first_of(illegalChars) != std::string::npos) {
        FLAlertLayer::create(
            "Error",
            "Command name contains illegal characters.\n<cy>Please use only letters, numbers, and underscores.</c>",
            "OK"
        )->show();

        return;
    };

    // Use default description if none provided
    if (commandDesc.empty()) commandDesc = "No description provided";

    // Validate cooldown input
    int cooldown = 0;
    if (!cooldownStr.empty()) {
        // Only allow digits (no negative cooldowns)
        if (cooldownStr.find_first_not_of("0123456789") == std::string::npos) {
            cooldown = numFromString<int>(cooldownStr).unwrapOr(0);
            if (cooldown < 0) {
                FLAlertLayer::create(
                    "Invalid Cooldown",
                    "Cooldown must be a non-negative integer.",
                    "OK"
                )->show();
                return;
            }
        } else {
            FLAlertLayer::create(
                "Invalid Cooldown",
                "You can only input a number value in the <cg>Cooldown</c> field.",
                "OK"
            )->show();
            return;
        }
    } else {
        FLAlertLayer::create(
            "Required",
            "You must input a number value in the <cg>Cooldown</c> field.",
            "OK"
        )->show();
        return;
    }

    m_cooldownSeconds = cooldown;

    // When editing, check if any field has changed (name, desc, or cooldown)
    if (m_isEditing) {
        bool nameChanged = commandName != m_originalName;
        bool descChanged = commandDesc != m_originalDesc;

        bool cooldownChanged = false;

        // Parse original cooldown from m_originalDesc if present
        int originalCooldown = 0;
        size_t delim = m_originalDesc.find_last_of('|');

        if (delim != std::string::npos) {
            std::string cooldownStrOrig = m_originalDesc.substr(delim + 1);
            originalCooldown = 0;

            if (!cooldownStrOrig.empty() && (cooldownStrOrig.find_first_not_of("-0123456789") == std::string::npos))
                originalCooldown = numFromString<int>(cooldownStrOrig).unwrapOrDefault();
        };

        cooldownChanged = originalCooldown != m_cooldownSeconds;
        if (!nameChanged && !descChanged && !cooldownChanged) {
            FLAlertLayer::create(
                "No Changes",
                "You haven't made any changes to the command.\n<cy>Please modify a field to apply.</c>",
                "OK"
            )->show();

            return;
        };

        // If name changed, check for duplicates
        if (nameChanged) {
            auto commandManager = TwitchCommandManager::getInstance();
            for (const auto& cmd : commandManager->getCommands()) {
                if (cmd.name == commandName && cmd.name != m_originalName) {
                    FLAlertLayer::create(
                        "Error",
                        "A command with this name already exists.\n<cy>Please choose a different name.</c>",
                        "OK"
                    )->show();

                    return;
                };
            };
        };

        // Call the callback with the original name and new details (always include cooldown)
        if (m_callback) {
            m_callback(m_originalName, commandName + "|" + commandDesc + "|" + std::to_string(m_cooldownSeconds));
            // Update m_originalDesc so further edits compare against the new value
            m_originalDesc = commandDesc + "|" + std::to_string(m_cooldownSeconds);
        };

        this->removeFromParent();
        return;
    };

    // Check if command already exists (only if we're not editing the same command)
    if (!m_isEditing || (m_isEditing && commandName != m_originalName)) {
        auto commandManager = TwitchCommandManager::getInstance();
        for (const auto& cmd : commandManager->getCommands()) {
            // Only block if the name matches another command (not the one being edited)
            if (cmd.name == commandName && (!m_isEditing || cmd.name != m_originalName)) {
                FLAlertLayer::create(
                    "Error",
                    "A command with this name already exists.\n<cy>Please choose a different name.</c>",
                    "OK"
                )->show();

                return;
            };
        };
    };

    // Call the callback with the command name and description
    if (m_callback) {
        if (m_isEditing) {
            m_callback(m_originalName, commandName + "|" + commandDesc + "|" + std::to_string(m_cooldownSeconds));
        } else {
            m_callback(commandName, commandDesc + "|" + std::to_string(m_cooldownSeconds));
        };
    };

    // Close the popup
    this->removeFromParent();
};

void CommandInputPopup::onClose(CCObject *sender)
{
    geode::createQuickPopup(
        "Close Without Saving?",
        "Are you sure you want to close the settings without saving? <cr>Any unsaved changes will be lost.</c>",
        "Cancel", "Close",
        [this](auto, bool btn2) {
            if (btn2) {
                this->removeFromParent();
            }
        });
}

CommandInputPopup* CommandInputPopup::create(std::function<void(const std::string&, const std::string&)> callback) {
    auto ret = new CommandInputPopup();

    if (ret && ret->initAnchored(220.f, 200.f)) {
        ret->autorelease();
        ret->setCallback(callback);

        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};

CommandInputPopup* CommandInputPopup::createForEdit(
    const std::string& commandName,
    const std::string& commandDesc,
    std::function<void(const std::string&, const std::string&)> editCallback
) {
    auto ret = new CommandInputPopup();

    if (ret && ret->initAnchored(220.f, 200.f)) {
        ret->autorelease();
        ret->setCallback(editCallback);
        ret->setupForEdit(commandName, commandDesc);

        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};