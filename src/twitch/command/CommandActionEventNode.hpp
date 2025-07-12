#pragma once

#include "../TwitchCommandManager.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

/*
 * Use this instead of manually creating nodes
 */
class CommandActionEventNode : public CCNode {
protected:
    TwitchCommandAction m_action;

    bool init(TwitchCommandAction action, CCSize scrollSize);
public:
    static CommandActionEventNode* create(TwitchCommandAction action, CCSize scrollSize);
};