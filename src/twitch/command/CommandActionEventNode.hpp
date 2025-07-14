#pragma once

#include "../TwitchCommandManager.hpp"
#include "../TwitchDashboard.hpp"
#include <vector>
#include <string>
#include <Geode/Geode.hpp>
#include <cocos2d.h>

using namespace geode::prelude;

namespace cocos2d { class CCMenuItem; class CCLabelBMFont; class CCObject; class CCNode; class CCSprite; class CCInteger; }
namespace cocos2d { namespace extension { class CCScale9Sprite; } }
class CCMenuItemToggler;
typedef void (cocos2d::CCObject::* SEL_MenuHandler)(cocos2d::CCObject*);

struct EventNodeInfo {
    std::string id;
    std::string label;
    std::string description;
};

class CommandActionEventNode : public cocos2d::CCNode {
protected:
    // Command node members
    TwitchDashboard* m_parent = nullptr;
    TwitchCommand m_command = TwitchCommand("Name", "Description", "This argument has no use but the int is the cooldown", 0, {});
    int m_cooldownRemaining = 0;
    cocos2d::CCLabelBMFont* m_cooldownLabel = nullptr;
    cocos2d::extension::CCScale9Sprite* m_commandBg = nullptr;
    bool m_isOnCooldown = false;

    // Store pointer to role label for live updates
    cocos2d::CCLabelBMFont* m_roleLabel = nullptr;

    // Action node members
    CCMenuItemToggler* m_checkbox = nullptr;
    cocos2d::CCLabelBMFont* m_label = nullptr;
    ::CCMenuItemSpriteExtra* m_upBtn = nullptr;
    ::CCMenuItemSpriteExtra* m_downBtn = nullptr;

    // Event node members
    // (m_label and m_checkbox reused)

    TwitchCommandAction m_action;

    // For event info popup
    std::string m_eventDescription;

public:
    // Command node
    static CommandActionEventNode* createCommandNode(TwitchDashboard* parent, TwitchCommand command, float width);
    std::string getCommandName() const { return m_command.name; }
    void onToggleEnableCommand(cocos2d::CCObject* sender);
    void triggerCommand();

    // Live update for role label
    void updateRoleLabel();
    
    // Action node
    static CommandActionEventNode* createActionNode(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale = 0.7f, cocos2d::CCObject* moveTarget = nullptr, SEL_MenuHandler moveUpSelector = nullptr, SEL_MenuHandler moveDownSelector = nullptr, int actionIndex = 0, bool canMoveUp = false, bool canMoveDown = false);
    CCMenuItemToggler* getCheckbox() const { return m_checkbox; }
    cocos2d::CCLabelBMFont* getLabel() const { return m_label; }
    
    // Event node
    static CommandActionEventNode* createEventNode(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale = 0.6f);
    static std::vector<EventNodeInfo> getAllEventNodes(); // Now includes 'move' event
    
    // Unified
    static CommandActionEventNode* create(TwitchCommandAction action, CCSize scrollSize);

    // Command node methods
    cocos2d::CCMenuItem* createEditButton();
    cocos2d::CCMenuItem* createDeleteButton();
    cocos2d::CCMenuItem* createSettingsButton();
    void onEditCommand(cocos2d::CCObject* sender);
    void onDeleteCommand(cocos2d::CCObject* sender);
    void onCopyCommandName(cocos2d::CCObject* sender);
    void onSettingsCommand(cocos2d::CCObject* sender);
    bool initCommandNode(TwitchDashboard* parent, TwitchCommand command, float width);
    void startCooldown();
    void updateCooldown(float dt);
    void resetCooldown();
    
    // Action node methods
    bool initActionNode(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale, cocos2d::CCObject* moveTarget = nullptr, SEL_MenuHandler moveUpSelector = nullptr, SEL_MenuHandler moveDownSelector = nullptr, int actionIndex = 0, bool canMoveUp = false, bool canMoveDown = false);
    void setMoveEnabled(bool canMoveUp, bool canMoveDown);
    
    // Event node methods
    bool initEventNode(const std::string& labelText, cocos2d::CCObject* target, SEL_MenuHandler selector, float checkboxScale);
    
    bool init(TwitchCommandAction action, CCSize scrollSize);

    // For event info popup
    void onEventInfoBtn(cocos2d::CCObject* sender);
};