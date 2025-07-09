#pragma once

#include "TwitchCommandManager.hpp"
#include "TwitchDashboard.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CommandNode : public CCNode {
protected:
    TwitchDashboard* m_parent = nullptr;
    TwitchCommand m_command = TwitchCommand("hello", "Greets the user", "Hello {username}!", 0);

    // Cooldown state
    int m_cooldownRemaining = 0;
    CCLabelBMFont* m_cooldownLabel = nullptr;
    CCScale9Sprite* m_commandBg = nullptr;
    bool m_isOnCooldown = false;

    CCMenuItem* createEditButton();
    CCMenuItem* createDeleteButton();

    void onEditCommand(CCObject* sender);
    void onDeleteCommand(CCObject* sender);

    bool init(TwitchDashboard* parent, TwitchCommand command, float width);
    void startCooldown();
    void updateCooldown(float dt);
    void resetCooldown();
public:
    static CommandNode* create(TwitchDashboard* parent, TwitchCommand command, float width);
    std::string getCommandName() const { return m_command.name; }
    void triggerCommand(); // Call this when the command is triggered
};