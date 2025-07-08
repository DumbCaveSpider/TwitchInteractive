#pragma once

#include <Geode/Geode.hpp>
#include "TwitchCommandManager.hpp"

using namespace geode::prelude;

class TwitchDashboard : public Popup<> {
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
    void onDeleteCommand(CCObject* sender);
    void processDeleteCommand();
    void ensureMenusRegistered();
    CCMenuItem* createDeleteButton(const std::string& commandName);
    
public:
    static TwitchDashboard* create();
};