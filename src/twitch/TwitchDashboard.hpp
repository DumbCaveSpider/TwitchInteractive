#pragma once
#include "TwitchCommandManager.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class TwitchDashboard : public Popup<>
{
    friend class CommandActionEventNode;

protected:
    CCLabelBMFont *m_welcomeLabel = nullptr;

    // Commands scroll layer
    ScrollLayer *m_commandScrollLayer = nullptr;
    CCLayer *m_commandLayer = nullptr;

    // Command input elements
    CCMenu *m_commandControlsMenu = nullptr;

    // Command management state
    std::string m_commandToDelete;

    bool setup() override;
    void onClose(CCObject *sender) override;

    // Destructor for cleanup
    ~TwitchDashboard();

    // Command management
    void setupCommandsList();
    void setupCommandInput();
    void setupCommandListening();
    void showTutorialPrompt(float dt);

public:
    void delayedRefreshCommandsList(float dt);
    void onAddCustomCommand(CCObject *sender);
    void onToggleCommandListen(CCObject *sender);
    void onEditCommand(CCObject *sender);
    void handleCommandEdit(const std::string &originalName, const std::string &newName, const std::string &newDesc);
    void handleCommandDelete(const std::string &commandName);
    void refreshCommandsList();
    static TwitchDashboard *create();
    void triggerCommandCooldown(const std::string &commandName);
    static bool isListening();
    void onHandbook(CCObject *sender);
};