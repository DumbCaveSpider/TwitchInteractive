#pragma once

#include "TwitchCommandManager.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CommandNode;

class TwitchDashboard : public Popup<> {
    friend class CommandNode;
protected:
    CCLabelBMFont* m_welcomeLabel = nullptr;

    // Commands scroll layer
    ScrollLayer* m_commandScrollLayer = nullptr;
    CCLayer* m_commandLayer = nullptr;

    // Command input elements
    CCMenu* m_commandControlsMenu = nullptr;

    // Command management state
    std::string m_commandToDelete;

    bool setup() override;
    void onClose(CCObject* sender) override;

    // Destructor for cleanup
    ~TwitchDashboard();

    // Command management
    void setupCommandsList();
    void setupCommandInput();
    void setupCommandListening();
    void refreshCommandsList();
    void delayedRefreshCommandsList(float dt);
    void onAddCustomCommand(CCObject* sender);
    void onEditCommand(CCObject* sender);
    void handleCommandEdit(const std::string& originalName, const std::string& newName, const std::string& newDesc);
    void handleCommandDelete(const std::string& commandName);
public:
    static TwitchDashboard* create();
    void triggerCommandCooldown(const std::string& commandName);
};