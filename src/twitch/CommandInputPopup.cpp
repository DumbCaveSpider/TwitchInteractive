#include "CommandInputPopup.hpp"
#include "TwitchCommandManager.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

bool CommandInputPopup::setup() {
    setTitle("Add Command");
    
    auto layerSize = m_mainLayer->getContentSize();
    
    // Set ID for the popup
    this->setID("command-input-popup");
    m_mainLayer->setID("command-input-main-layer");
    
    // Create title label for command name
    m_titleLabel = CCLabelBMFont::create("Command name:", "bigFont.fnt");
    m_titleLabel->setPosition(layerSize.width / 2, layerSize.height - 50);
    m_titleLabel->setScale(0.6f);
    m_titleLabel->setID("command-input-title-label");
    m_mainLayer->addChild(m_titleLabel);
    
    // Create text input for command name
    m_nameInput = TextInput::create(200, "Command name", "bigFont.fnt");
    m_nameInput->setPosition(layerSize.width / 2, layerSize.height - 75);
    m_nameInput->setScale(0.8f);
    m_nameInput->setID("command-input-name-field");
    m_mainLayer->addChild(m_nameInput);
    
    // Create label for description
    m_descLabel = CCLabelBMFont::create("Command description:", "bigFont.fnt");
    m_descLabel->setPosition(layerSize.width / 2, layerSize.height - 105);
    m_descLabel->setScale(0.6f);
    m_descLabel->setID("command-input-desc-label");
    m_mainLayer->addChild(m_descLabel);
    
    // Create text input for description
    m_descInput = TextInput::create(200, "Command description", "bigFont.fnt");
    m_descInput->setPosition(layerSize.width / 2, layerSize.height - 130);
    m_descInput->setScale(0.8f);
    m_descInput->setID("command-input-desc-field");
    m_mainLayer->addChild(m_descInput);
    
    // Create button menu
    auto buttonMenu = CCMenu::create();
    
    // Add button
    auto addBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Add", "bigFont.fnt", "GJ_button_04.png", 0.6f),
        this,
        menu_selector(CommandInputPopup::onAdd)
    );
    addBtn->setPosition(-50, 0);
    addBtn->setID("command-input-add-btn");
    
    // Cancel button
    auto cancelBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Cancel", "bigFont.fnt", "GJ_button_06.png", 0.6f),
        this,
        menu_selector(CommandInputPopup::onCancel)
    );
    cancelBtn->setPosition(50, 0);
    cancelBtn->setID("command-input-cancel-btn");
    
    buttonMenu->addChild(addBtn);
    buttonMenu->addChild(cancelBtn);
    buttonMenu->setPosition(layerSize.width / 2, layerSize.height / 2 - 60);
    buttonMenu->setID("command-input-button-menu");
    m_mainLayer->addChild(buttonMenu);
    
    return true;
}

void CommandInputPopup::setCallback(std::function<void(const std::string&, const std::string&)> callback) {
    m_callback = callback;
}

void CommandInputPopup::onAdd(CCObject* sender) {
    // Get the command name from the text input
    std::string commandName = m_nameInput->getString();
    std::string commandDesc = m_descInput->getString();
    
    // Trim whitespace for command name
    commandName.erase(0, commandName.find_first_not_of(" \t\n\r"));
    commandName.erase(commandName.find_last_not_of(" \t\n\r") + 1);
    
    // Trim whitespace for description
    commandDesc.erase(0, commandDesc.find_first_not_of(" \t\n\r"));
    commandDesc.erase(commandDesc.find_last_not_of(" \t\n\r") + 1);
    
    // Check if command name is empty
    if (commandName.empty()) {
        FLAlertLayer::create(
            "Error",
            "Please enter a command name",
            "OK"
        )->show();
        return;
    }
    
    // Remove any '!' prefix if user added it
    if (commandName.front() == '!') {
        commandName = commandName.substr(1);
    }
    
    // Check if still empty after removing '!'
    if (commandName.empty()) {
        FLAlertLayer::create(
            "Error",
            "Please enter a valid command name",
            "OK"
        )->show();
        return;
    }
    
    // Check for illegal characters in command name
    const std::string illegalChars = "!@#$%^&*()+={}[]|\\:;\"'<>,?/~`";
    if (commandName.find_first_of(illegalChars) != std::string::npos) {
        FLAlertLayer::create(
            "Error",
            "Command name contains illegal characters.\nPlease use only letters, numbers, and underscores.",
            "OK"
        )->show();
        return;
    }
    
    // Check if command already exists
    auto commandManager = TwitchCommandManager::getInstance();
    for (const auto& cmd : commandManager->getCommands()) {
        if (cmd.name == commandName) {
            FLAlertLayer::create(
                "Error",
                "A command with this name already exists.\nPlease choose a different name.",
                "OK"
            )->show();
            return;
        }
    }
    
    // Use default description if none provided
    if (commandDesc.empty()) {
        commandDesc = "No description provided";
    }
    
    // Call the callback with the command name and description
    if (m_callback) {
        m_callback(commandName, commandDesc);
    }
    
    // Close the popup
    this->onClose(nullptr);
}

void CommandInputPopup::onCancel(CCObject* sender) {
    this->onClose(nullptr);
}

CommandInputPopup* CommandInputPopup::create(std::function<void(const std::string&, const std::string&)> callback) {
    auto ret = new CommandInputPopup();
    if (ret && ret->initAnchored(300.f, 240.f)) { // Increased height for extra field
        ret->autorelease();
        ret->setCallback(callback);
        
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
