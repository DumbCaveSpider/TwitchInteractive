#pragma once

#include "TwitchCommandManager.hpp"
#include "TwitchDashboard.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CommandNode : public CCNode {
protected:
    TwitchDashboard* m_parent = nullptr;
    TwitchCommand m_command = TwitchCommand("hello", "Greets the user", "Hello {username}!");

    CCMenuItem* createEditButton();
    CCMenuItem* createDeleteButton();

    void onEditCommand(CCObject* sender);
    void onDeleteCommand(CCObject* sender);

    bool init(TwitchDashboard* parent, TwitchCommand command, float width);
public:
    static CommandNode* create(TwitchDashboard* parent, TwitchCommand command, float width);
    std::string getCommandName() const { return m_command.name; }
};