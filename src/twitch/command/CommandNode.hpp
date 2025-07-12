#pragma once

#include "../TwitchCommandManager.hpp"
#include "../TwitchDashboard.hpp"

#include <Geode/Geode.hpp>

namespace cocos2d { class CCMenuItem; class CCLabelBMFont; class CCObject; }
namespace cocos2d { namespace extension { class CCScale9Sprite; } }

class CommandNode : public cocos2d::CCNode {
    protected:
    TwitchDashboard* m_parent = nullptr;
    TwitchCommand m_command = TwitchCommand("Name", "Description", "This argument has no use but the int is the cooldown", 0, {});
    
    int m_cooldownRemaining = 0;
    CCLabelBMFont* m_cooldownLabel = nullptr;
    CCScale9Sprite* m_commandBg = nullptr;
    bool m_isOnCooldown = false;
    
    cocos2d::CCMenuItem* createEditButton();
    cocos2d::CCMenuItem* createDeleteButton();
    cocos2d::CCMenuItem* createSettingsButton();
    
    void onEditCommand(cocos2d::CCObject* sender);
    void onDeleteCommand(cocos2d::CCObject* sender);
    void onCopyCommandName(cocos2d::CCObject* sender);
    void onSettingsCommand(cocos2d::CCObject* sender);
    
    bool init(TwitchDashboard* parent, TwitchCommand command, float width);
    void startCooldown();
    void updateCooldown(float dt);
    void resetCooldown();
    
public:
    static CommandNode* create(TwitchDashboard* parent, TwitchCommand command, float width);
    std::string getCommandName() const { return m_command.name; }
    void onToggleEnableCommand(cocos2d::CCObject* sender);
    void triggerCommand();
};
