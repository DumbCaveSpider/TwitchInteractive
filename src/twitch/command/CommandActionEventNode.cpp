#include "CommandActionEventNode.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

bool CommandActionEventNode::init(TwitchCommandAction action, CCSize scrollSize) {
    m_action = action;

    if (!CCNode::create()) return false;

    // gotta sleep just move the code here
    //
    // enjoy :)

    setContentSize(CCSize(scrollSize.width, 32.f));

    return true;
};

CommandActionEventNode* CommandActionEventNode::create(TwitchCommandAction action, CCSize scrollSize) {
    auto ret = new CommandActionEventNode();

    if (ret && ret->init(action, scrollSize)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};