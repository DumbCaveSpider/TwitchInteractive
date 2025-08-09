#include <string>

#include <cocos2d.h>
#include <cocos-ext.h>

#include <algorithm>

#include "CommandSettingsPopup.hpp"
#include "CommandActionEventNode.hpp"
#include "CommandUserSettingsPopup.hpp"

#include "../TwitchCommandManager.hpp"
#include "../handler/KeyCodesSettingsPopup.hpp"
#include "../handler/ProfileSettingsPopup.hpp"
#include "../handler/MoveSettingsPopup.hpp"
#include "../handler/JumpSettingsPopup.hpp"
#include "../handler/ColorPlayerSettingsPopup.hpp"
#include "../handler/SettingsHandler.hpp"
#include "../handler/AlertSettingsPopup.hpp"
#include "../handler/CameraSettingsPopup.hpp"

using namespace cocos2d;
using namespace geode::prelude;

// Free function to add or update a label for an action node with uniform style
void addOrUpdateActionLabel(CCNode *actionNode, const std::string &labelId, const std::string &text, float x, float y)
{
    if (!actionNode)
        return;

    if (auto label = actionNode->getChildByID(labelId))
    {
        if (auto bmLabel = typeinfo_cast<CCLabelBMFont *>(label))
            bmLabel->setString(text.c_str());
    }
    else
    {
        auto newLabel = CCLabelBMFont::create(text.c_str(), "chatFont.fnt");
        newLabel->setID(labelId);
        newLabel->setScale(0.5f);
        newLabel->setAnchorPoint({0, 0.5f});
        newLabel->setAlignment(kCCTextAlignmentLeft);
        newLabel->setPosition(x, y);

        actionNode->addChild(newLabel);
    };
};

bool CommandSettingsPopup::setup(TwitchCommand command)
{
    m_command = command;

    setID("command-settings-popup");
    setTitle(fmt::format("!{} settings", m_command.name));

    auto layerSize = m_mainLayer->getContentSize();
    float sectionWidth = 250.f;
    float sectionHeight = 200.f;

    float popupWidth = layerSize.width - 7.5f;
    float popupHeight = layerSize.height - 7.5f;
    // Scroll layer size
    CCSize scrollSize = CCSize(sectionWidth, sectionHeight);
    CCSize scrollContentSize = scrollSize;

    float sectionY = (popupHeight - sectionHeight) / 2.0f;
    float scrollY = sectionY;

    float scrollBgY = scrollY;
    float scrollLayerY = scrollY;

    // Center both sections horizontally
    float gap = 20.f; // Gap between event and action layers
    float totalSectionsWidth = sectionWidth * 2 + gap;

    float startX = (popupWidth - totalSectionsWidth) / 2.0f;

    float eventSectionX = startX;
    float actionSectionX = startX + sectionWidth + gap;

    // label :)
    auto eventLabel = CCLabelBMFont::create("Events", "bigFont.fnt");
    eventLabel->setScale(0.6f);
    eventLabel->setAnchorPoint({0.5f, 0.5f});
    eventLabel->setPosition(eventSectionX + scrollSize.width / 2, scrollBgY + scrollSize.height + 38.f);
    eventLabel->setID("events-section-label");
    m_mainLayer->addChild(eventLabel);

    // Search box for event nodes (centered underneath the label)
    float searchBoxWidth = sectionWidth - 10.f;
    auto eventSearchInput = TextInput::create(static_cast<int>(searchBoxWidth), "Search events", "bigFont.fnt");
    eventSearchInput->setID("event-search-input");
    // Center horizontally under the label
    eventSearchInput->setPosition(eventSectionX + scrollSize.width / 2, scrollBgY + scrollSize.height + 18.f);
    eventSearchInput->setScale(0.5f);
    eventSearchInput->setAnchorPoint({0.5f, 0.5f});
    eventSearchInput->setString("");
    m_mainLayer->addChild(eventSearchInput);

    auto actionLabel = CCLabelBMFont::create("Actions", "bigFont.fnt");
    actionLabel->setScale(0.6f);
    actionLabel->setAnchorPoint({0.5f, 0.5f});
    actionLabel->setPosition(actionSectionX + scrollSize.width / 2, scrollBgY + scrollSize.height + 22.f);
    actionLabel->setID("actions-section-label");

    m_mainLayer->addChild(actionLabel);

    // Profile button (top right corner)
    auto profileMenu = CCMenu::create();
    profileMenu->setID("command-profile-menu");
    float profileBtnMarginX = 10.f;
    float profileBtnMarginY = 10.f;
    float profileBtnWidth = 0.f;
    float profileBtnHeight = 0.f;

    auto profileBtnSprite = CCSprite::createWithSpriteFrameName("GJ_profileButton_001.png");
    profileBtnSprite->setScale(0.7f);
    profileBtnWidth = profileBtnSprite->getContentSize().width * profileBtnSprite->getScale();
    profileBtnHeight = profileBtnSprite->getContentSize().height * profileBtnSprite->getScale();

    auto profileBtn = CCMenuItemSpriteExtra::create(
        profileBtnSprite,
        this,
        menu_selector(CommandSettingsPopup::onProfileUserSettings));

    profileBtn->setID("command-profile-user-btn");
    profileMenu->addChild(profileBtn);

    // Position at top right corner (inside the popup, with margin)
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    float menuX = winSize.width - profileBtnWidth / 2 + profileBtnMarginX;
    float menuY = winSize.height - profileBtnHeight / 2 - profileBtnMarginY;

    profileMenu->setPosition(menuX, menuY);
    m_mainLayer->addChild(profileMenu);

    // Background for the event scroll layer
    auto eventScrollBg = CCScale9Sprite::create("square02_small.png");
    eventScrollBg->setID("events-scroll-background");
    eventScrollBg->setContentSize(scrollSize);
    eventScrollBg->setOpacity(50);
    eventScrollBg->setAnchorPoint({0.5f, 0.5f});
    eventScrollBg->setScale(1.05f);
    eventScrollBg->setPosition(eventSectionX + scrollSize.width / 2, scrollBgY + scrollSize.height / 2);

    m_mainLayer->addChild(eventScrollBg);

    // Scroll layer for events
    auto eventScrollLayer = ScrollLayer::create(scrollSize);
    eventScrollLayer->setID("events-scroll");
    eventScrollLayer->setPosition(eventSectionX, scrollLayerY);

    // Background for the actions scroll layer
    auto actionScrollBg = CCScale9Sprite::create("square02_small.png");
    actionScrollBg->setID("actions-scroll-background");
    actionScrollBg->setContentSize(scrollSize);
    actionScrollBg->setOpacity(50);
    actionScrollBg->setAnchorPoint({0.5f, 0.5f});
    actionScrollBg->setScale(1.05f);
    actionScrollBg->setPosition(actionSectionX + scrollSize.width / 2, scrollBgY + scrollSize.height / 2);

    m_mainLayer->addChild(actionScrollBg);

    // Scroll layer for actions
    auto actionScrollLayer = ScrollLayer::create(scrollSize);
    actionScrollLayer->setID("actions-scroll");
    actionScrollLayer->setPosition(actionSectionX, scrollLayerY);

    // Ensure scroll layers start at the top
    eventScrollLayer->scrollToTop();
    actionScrollLayer->scrollToTop();

    auto eventLayout = ColumnLayout::create()
                           ->setAxisReverse(true) // Make items stack from the top
                           ->setAxisAlignment(AxisAlignment::Start)
                           ->setCrossAxisAlignment(AxisAlignment::Start)
                           ->setAutoGrowAxis(scrollSize.height)
                           ->setGap(8.0f);

    // Content layer for event nodes
    auto eventContent = eventScrollLayer->m_contentLayer;
    eventContent->setID("events-content");
    eventContent->setContentSize(scrollSize);
    eventContent->setLayout(eventLayout);

    auto actionLayout = ColumnLayout::create()
                            ->setAxisReverse(false)
                            ->setAxisAlignment(AxisAlignment::Start)
                            ->setCrossAxisAlignment(AxisAlignment::Start)
                            ->setAutoGrowAxis(scrollSize.height)
                            ->setGap(8.0f);

    // Content layer for actions
    auto actionContent = actionScrollLayer->m_contentLayer;
    actionContent->setID("actions-content");
    actionContent->setContentSize(scrollSize);
    actionContent->setLayout(actionLayout);

    // Store actions for this command as a member
    m_commandActions.clear();

    // Initialize m_commandActions from command.actions
    for (const auto &action : command.actions)
    {
        if (action.type == CommandActionType::Notification)
        {
            m_commandActions.push_back("notification:" + action.arg);
        }
        else if (action.type == CommandActionType::Wait)
        {
            m_commandActions.push_back("wait:" + std::to_string(action.index));
        }
        else if (action.type == CommandActionType::Event)
        {
            if (action.arg.rfind("jump:", 0) == 0)
            {
                m_commandActions.push_back(action.arg);
            }
            else
            {
                m_commandActions.push_back(action.arg);
            };
        };
    };

    m_actionContent = actionContent;
    m_actionSectionHeight = sectionHeight;

    refreshActionsList();

    // Store search string as a member variable
    m_eventSearchString = "";

    m_lastEventSearchString = "";
    // Helper lambda to refresh event node list based on search
    auto refreshEventNodeList = [eventContent, scrollSize, this](const std::string &searchStr)
    {
        eventContent->removeAllChildren();
        std::vector<EventNodeInfo> sortedEventNodes = CommandActionEventNode::getAllEventNodes();
        std::sort(sortedEventNodes.begin(), sortedEventNodes.end(), [](const EventNodeInfo &a, const EventNodeInfo &b)
                  { return a.label < b.label; });
        std::vector<EventNodeInfo> filteredNodes;
        for (const auto &info : sortedEventNodes)
        {
            std::string labelLower = info.label;
            std::string idLower = info.id;
            std::string searchLower = searchStr;
            std::transform(labelLower.begin(), labelLower.end(), labelLower.begin(), ::tolower);
            std::transform(idLower.begin(), idLower.end(), idLower.begin(), ::tolower);
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
            if (searchLower.empty() || labelLower.find(searchLower) != std::string::npos || idLower.find(searchLower) != std::string::npos)
            {
                filteredNodes.push_back(info);
            }
        }
        float eventNodeGap = 8.0f;
        float nodeHeight = 32.f;
        int eventCount = static_cast<int>(filteredNodes.size());
        float minContentHeight = scrollSize.height;
        float neededHeight = eventCount * (nodeHeight + eventNodeGap);
        float contentHeight = std::max(minContentHeight, neededHeight);
        eventContent->setContentSize(CCSize(scrollSize.width, contentHeight));
        float eventNodeY = contentHeight - 16.f;
        for (const auto &info : filteredNodes)
        {
            auto node = CCNode::create();
            node->setContentSize(CCSize(scrollSize.width, nodeHeight));
            auto label = CCLabelBMFont::create(info.label.c_str(), "bigFont.fnt");
            label->setID("event-" + info.id + "-label");
            label->setScale(0.5f);
            label->setAnchorPoint({0, 0.5f});
            label->setAlignment(kCCTextAlignmentLeft);
            label->setPosition(20.f, 16.f);
            auto infoBtnSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
            infoBtnSprite->setScale(0.5f);
            float infoBtnX = 20.f + label->getContentSize().width * label->getScale() + 12.f;
            auto infoBtn = CCMenuItemSpriteExtra::create(
                infoBtnSprite,
                this,
                menu_selector(CommandSettingsPopup::onEventInfoBtn));
            infoBtn->setID("event-" + info.id + "-info-btn");
            infoBtn->setUserObject(CCString::create(info.description));
            infoBtn->setPosition(infoBtnX, 16.f);
            auto menu = CCMenu::create();
            menu->setPosition(0, 0);
            auto addSprite = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
            addSprite->setScale(0.5);
            auto addBtn = CCMenuItemSpriteExtra::create(
                addSprite,
                this,
                menu_selector(CommandSettingsPopup::onAddEventAction));
            addBtn->setID("event-" + info.id + "-add-btn");
            addBtn->setPosition(scrollSize.width - 24.f, 16.f);
            addBtn->setUserObject(CCString::create(info.id));
            menu->addChild(addBtn);
            menu->addChild(infoBtn);
            node->addChild(label);
            node->addChild(menu);
            auto nodeBg = CCScale9Sprite::create("square02_small.png");
            nodeBg->setContentSize(node->getContentSize());
            nodeBg->setOpacity(60);
            nodeBg->setAnchorPoint({0, 0});
            nodeBg->setPosition(0, 0);
            node->addChild(nodeBg, -1);
            node->setPosition(0, eventNodeY - 16.f);
            eventContent->addChild(node);
            eventNodeY -= (nodeHeight + eventNodeGap);
        }
    };

    // Store search input and refresh lambda for use in scheduled function
    m_eventSearchInput = eventSearchInput;
    m_refreshEventNodeList = refreshEventNodeList;

    // Polling approach for search input (since setTextChangedCallback is not available)
    this->schedule(schedule_selector(CommandSettingsPopup::onEventSearchPoll), 0.1f);
    refreshEventNodeList(m_eventSearchString);
    m_lastEventSearchString = m_eventSearchString;

    // After adding all event nodes, scroll to top
    eventScrollLayer->scrollToTop();

    // Add action nodes for existing actions
    m_mainLayer->addChild(actionScrollLayer);
    m_mainLayer->addChild(eventScrollLayer);

    // Set checkbox state from command actions
    bool killChecked = false;
    for (const auto &action : command.actions)
    {
        if (action.type == CommandActionType::Event && action.arg == "kill_player")
        {
            killChecked = true;
            break;
        };
    };

    if (m_killPlayerCheckbox)
        m_killPlayerCheckbox->toggle(killChecked);

    // Save button
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(CommandSettingsPopup::onSave));
    saveBtn->setID("command-settings-save-btn");

    // Handbook button
    auto handbookBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Handbook", "bigFont.fnt", "GJ_button_05.png", 0.6f),
        this,
        menu_selector(CommandSettingsPopup::onHandbookBtn));
    handbookBtn->setID("command-settings-handbook-btn");

    // Close button
    auto closeBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Close", "bigFont.fnt", "GJ_button_06.png", 0.6f),
        this,
        menu_selector(CommandSettingsPopup::onCloseBtn));
    closeBtn->setID("command-settings-close-btn");

    // Menu for buttons
    auto commandBtnMenu = CCMenu::create();
    commandBtnMenu->setID("command-settings-button-menu");
    commandBtnMenu->addChild(saveBtn);
    commandBtnMenu->addChild(handbookBtn);
    commandBtnMenu->addChild(closeBtn);
    commandBtnMenu->setContentSize({570.f, 25.f});

    auto menuSize = commandBtnMenu->getContentSize();

    float menuWidth = menuSize.width;
    float menuHeight = menuSize.height;

    float centerY = menuHeight / 2;
    float spacing = 120.0f;

    saveBtn->setPosition(menuWidth / 2 - spacing, centerY);
    handbookBtn->setPosition(menuWidth / 2, centerY);
    closeBtn->setPosition(menuWidth / 2 + spacing, centerY);

    // Center the button menu in the popup/main layer
    float mainW = m_mainLayer->getContentSize().width;
    float mainH = m_mainLayer->getContentSize().height;

    commandBtnMenu->setPosition(mainW / 2 - menuWidth / 2, 15.f);

    m_mainLayer->addChild(commandBtnMenu);

    return true;
};

void CommandSettingsPopup::onProfileUserSettings(CCObject *sender)
{
    std::string allowedUser = m_command.allowedUser;

    bool allowVip = m_command.allowVip;
    bool allowMod = m_command.allowMod;
    bool allowSubscriber = m_command.allowSubscriber;
    bool allowStreamer = m_command.allowStreamer;

    auto popup = CommandUserSettingsPopup::create(
        allowedUser,
        allowVip,
        allowMod,
        allowSubscriber,
        allowStreamer,
        [this](const std::string &user, bool vip, bool mod, bool subscriber, bool streamer)
        {
            m_command.allowedUser = user;
            m_command.allowVip = vip;
            m_command.allowMod = mod;
            m_command.allowSubscriber = subscriber;
            m_command.allowStreamer = streamer;

            // Update all CommandActionEventNode role labels in the UI
            if (m_actionContent)
            {
                auto children = m_actionContent->getChildren();
                if (children)
                {
                    for (int i = 0; i < children->count(); ++i)
                    {
                        if (auto node = static_cast<CCNode *>(children->objectAtIndex(i)))
                        {
                            if (auto eventNode = typeinfo_cast<CommandActionEventNode *>(node))
                                eventNode->updateRoleLabel();
                        };
                    };
                };
            };
        });

    if (popup)
        popup->show();
    return;
};

void CommandSettingsPopup::onMoveActionUp(cocos2d::CCObject *sender)
{
    auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);
    int idx = 0;
    if (btn->getUserObject())
        idx = static_cast<CCInteger *>(btn->getUserObject())->getValue();
    if (idx <= 0 || idx >= static_cast<int>(m_commandActions.size()))
        return;
    // Keep the wait input when adding new event
    if (m_actionContent)
    {
        auto children = m_actionContent->getChildren();
        if (children)
        {
            int maxCount = std::min(static_cast<int>(children->count()), static_cast<int>(m_commandActions.size()));
            for (int i = 0; i < maxCount; ++i)
            {
                std::string lower = m_commandActions[i];
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.rfind("wait:", 0) == 0)
                {
                    if (auto node = static_cast<CCNode *>(children->objectAtIndex(i)))
                    {
                        std::string waitInputId = "wait-delay-input-" + std::to_string(i);
                        if (auto waitInput = typeinfo_cast<TextInput *>(node->getChildByID(waitInputId)))
                        {
                            std::string val = waitInput->getString();
                            // Trim
                            val.erase(0, val.find_first_not_of(" \t\n\r"));
                            size_t last = val.find_last_not_of(" \t\n\r");
                            if (last != std::string::npos)
                                val.erase(last + 1);
                            if (!val.empty())
                            {
                                auto parsed = numFromString<float>(val);
                                if (parsed)
                                {
                                    float f = parsed.unwrap();
                                    f = std::round(f * 1000.0f) / 1000.0f;
                                    m_commandActions[i] = "wait:" + fmt::format("{:.2f}", f);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    std::swap(m_commandActions[idx], m_commandActions[idx - 1]);
    refreshActionsList();
};

void CommandSettingsPopup::onMoveActionDown(cocos2d::CCObject *sender)
{
    auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);
    int idx = 0;
    if (btn->getUserObject())
        idx = static_cast<CCInteger *>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= static_cast<int>(m_commandActions.size()) - 1)
        return;
    // Keep the wait input when moving the index positions
    if (m_actionContent)
    {
        auto children = m_actionContent->getChildren();
        if (children)
        {
            int maxCount = std::min(static_cast<int>(children->count()), static_cast<int>(m_commandActions.size()));
            for (int i = 0; i < maxCount; ++i)
            {
                std::string lower = m_commandActions[i];
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.rfind("wait:", 0) == 0)
                {
                    if (auto node = static_cast<CCNode *>(children->objectAtIndex(i)))
                    {
                        std::string waitInputId = "wait-delay-input-" + std::to_string(i);
                        if (auto waitInput = typeinfo_cast<TextInput *>(node->getChildByID(waitInputId)))
                        {
                            std::string val = waitInput->getString();
                            // Trim
                            val.erase(0, val.find_first_not_of(" \t\n\r"));
                            size_t last = val.find_last_not_of(" \t\n\r");
                            if (last != std::string::npos)
                                val.erase(last + 1);
                            if (!val.empty())
                            {
                                auto parsed = numFromString<float>(val);
                                if (parsed)
                                {
                                    float f = parsed.unwrap();
                                    f = std::round(f * 1000.0f) / 1000.0f;
                                    m_commandActions[i] = "wait:" + fmt::format("{:.2f}", f);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    std::swap(m_commandActions[idx], m_commandActions[idx + 1]);
    refreshActionsList();
};

// Gravity settings handler
void CommandSettingsPopup::onGravitySettings(CCObject *sender)
{
    SettingsHandler::handleGravitySettings(this, sender);
};

void CommandSettingsPopup::onSpeedSettings(CCObject *sender)
{
    SettingsHandler::handleSpeedSettings(this, sender);
};

// Handler for color player settings button
void CommandSettingsPopup::onColorPlayerSettings(CCObject *sender)
{
    SettingsHandler::handleColorPlayerSettings(this, sender);
};

// Jump settings handler
void CommandSettingsPopup::onJumpSettings(cocos2d::CCObject *sender)
{
    SettingsHandler::handleJumpSettings(this, sender);
};

// Move Player settings handler
void CommandSettingsPopup::onMoveSettings(cocos2d::CCObject *sender)
{
    SettingsHandler::handleMoveSettings(this, sender);
};

// Edit Camera settings handler
void CommandSettingsPopup::onEditCameraSettings(cocos2d::CCObject *sender)
{
    SettingsHandler::handleEditCameraSettings(this, sender);
};

// Notification settings handler
void CommandSettingsPopup::onNotificationSettings(cocos2d::CCObject *sender)
{
    SettingsHandler::handleNotificationSettings(this, sender);
};

void CommandSettingsPopup::onAlertSettings(cocos2d::CCObject *sender)
{
    SettingsHandler::handleAlertSettings(this, sender);
};

// Player Profile settings handler
void CommandSettingsPopup::onProfileSettings(cocos2d::CCObject *sender)
{
    SettingsHandler::handleProfileSettings(this, sender);
};

// KeyCode settings handler
void CommandSettingsPopup::onKeyCodeSettings(cocos2d::CCObject *sender)
{
    SettingsHandler::handleKeyCodeSettings(this, sender);
};

// Handbook button handler
void CommandSettingsPopup::onHandbookBtn(cocos2d::CCObject *sender)
{
    auto popup = HandbookPopup::create();
    if (popup)
        popup->show();
};

void CommandSettingsPopup::onAddEventAction(cocos2d::CCObject *sender)
{
    auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);

    std::string eventId;
    if (btn->getUserObject())
        eventId = static_cast<CCString *>(btn->getUserObject())->getCString();

    // Persist current wait input values into m_commandActions before modifying the list
    if (m_actionContent)
    {
        auto children = m_actionContent->getChildren();
        if (children)
        {
            int maxCount = std::min(static_cast<int>(children->count()), static_cast<int>(m_commandActions.size()));
            for (int i = 0; i < maxCount; ++i)
            {
                std::string lower = m_commandActions[i];
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.rfind("wait:", 0) == 0)
                {
                    if (auto node = static_cast<CCNode *>(children->objectAtIndex(i)))
                    {
                        std::string waitInputId = "wait-delay-input-" + std::to_string(i);
                        if (auto waitInput = typeinfo_cast<TextInput *>(node->getChildByID(waitInputId)))
                        {
                            std::string val = waitInput->getString();
                            // Trim whitespace
                            val.erase(0, val.find_first_not_of(" \t\n\r"));
                            size_t last = val.find_last_not_of(" \t\n\r");
                            if (last != std::string::npos)
                                val.erase(last + 1);
                            if (!val.empty())
                            {
                                auto parsed = numFromString<float>(val);
                                if (parsed)
                                {
                                    float f = parsed.unwrap();
                                    f = std::round(f * 1000.0f) / 1000.0f;
                                    m_commandActions[i] = "wait:" + fmt::format("{:.2f}", f);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (!eventId.empty())
    {
        if (eventId == "jump")
        {
            m_commandActions.push_back("jump:1");
            refreshActionsList();
        }
        else if (eventId == "notification")
        {
            m_commandActions.push_back("notification:");
            refreshActionsList();
        }
        else if (eventId == "keycode")
        {
            m_commandActions.push_back("keycode:");
            refreshActionsList();
        }
        else if (eventId == "profile")
        {
            m_commandActions.push_back("profile:");
            refreshActionsList();
        }
        else if (eventId == "move")
        {
            m_commandActions.push_back("move:1:right");
            refreshActionsList();
        }
        else if (eventId == "alert_popup")
        {
            m_commandActions.push_back("alert_popup:-:-");
            refreshActionsList();
        }
        else if (eventId == "wait")
        {
            m_commandActions.push_back("wait:");
            refreshActionsList();
        }
        else if (eventId == "reverse_player")
        {
            m_commandActions.push_back("reverse_player");
            refreshActionsList();
        }
        else if (eventId == "scale_player")
        {
            m_commandActions.push_back("scale_player:1.00");
            refreshActionsList();
        }
        else if (eventId == "speed_player")
        {
            m_commandActions.push_back("speed_player:1.00:0.50");
            refreshActionsList();
        }
        else
        {
            m_commandActions.push_back(eventId);
            refreshActionsList();

            if (m_mainLayer)
            {
                auto eventScrollLayer = typeinfo_cast<ScrollLayer *>(m_mainLayer->getChildByID("events-scroll"));

                if (eventScrollLayer && eventScrollLayer->m_contentLayer)
                {
                    auto eventContent = eventScrollLayer->m_contentLayer;

                    float eventNodeGap = 8.0f;
                    float nodeHeight = 32.f;

                    int eventCount = static_cast<int>(CommandActionEventNode::getAllEventNodes().size());

                    float minContentHeight = eventScrollLayer->getContentSize().height;
                    float neededHeight = eventCount * (nodeHeight + eventNodeGap);
                    float contentHeight = std::max(minContentHeight, neededHeight);

                    eventContent->setContentSize(CCSize(eventScrollLayer->getContentSize().width, contentHeight));
                };
            };
        };
    };
};

void CommandSettingsPopup::updateKeyCodeNextTextLabel(int actionIdx, const std::string &nextKey)
{
    if (actionIdx >= 0 && actionIdx < static_cast<int>(m_commandActions.size()))
        m_commandActions[actionIdx] = "keycode:" + nextKey;

    // Update the label in the action node
    auto children = m_actionContent->getChildren();
    if (!children || actionIdx < 0 || actionIdx >= children->count())
        return;

    auto actionNode = static_cast<CCNode *>(children->objectAtIndex(actionIdx));
    if (!actionNode)
        return;

    std::string keyLabelId = "keycode-action-text-label-" + std::to_string(actionIdx);

    // Separate the duration and key in the label
    std::string keyPart = nextKey;
    std::string durationPart;

    size_t pipePos = keyPart.find(":");
    if (pipePos != std::string::npos)
    {
        durationPart = keyPart.substr(pipePos + 1);
        keyPart = keyPart.substr(0, pipePos);
    }

    // Update the label text with the new format
    std::string labelText = "Key: " + (keyPart.empty() ? "-" : keyPart);
    if (!durationPart.empty())
    {
        labelText += " (" + durationPart + ")";
    }

    if (auto label = actionNode->getChildByID(keyLabelId))
    {
        if (auto bmLabel = typeinfo_cast<CCLabelBMFont *>(label))
        {
            bmLabel->setString(labelText.c_str());
        }
    }
};

void CommandSettingsPopup::refreshActionsList()
{
    if (!m_actionContent)
        return;

    m_actionContent->removeAllChildren();

    float actionNodeGap = 8.0f;
    float nodeHeight = 32.f;

    int actionCount = static_cast<int>(m_commandActions.size());

    // Dynamically expand content layer height if needed
    float minContentHeight = m_actionSectionHeight;
    float neededHeight = actionCount * (nodeHeight + actionNodeGap);
    float contentHeight = std::max(minContentHeight, neededHeight);

    m_actionContent->setContentSize(CCSize(m_actionContent->getContentSize().width, contentHeight));

    float actionNodeY = contentHeight - nodeHeight / 2;
    int actionIndex = 0;

    for (int i = 0; i < actionCount; ++i)
    {
        std::string &actionIdRaw = m_commandActions[i];
        std::string actionIdLower = actionIdRaw;
        std::transform(actionIdLower.begin(), actionIdLower.end(), actionIdLower.begin(), ::tolower);

        // Create the action node and align to left edge of scroll content
        TwitchCommandAction actionObj(CommandActionType::Event, actionIdRaw, 0);

        auto actionNode = CommandActionEventNode::create(actionObj, CCSize(m_actionContent->getContentSize().width, nodeHeight));
        actionNode->setAnchorPoint({0, 0.5f});
        actionNode->setPosition(0, actionNodeY); // align left, vertical stack

        m_actionContent->addChild(actionNode);

        // Action index order label (left side)
        std::string indexLabelId = "action-index-label-" + std::to_string(i);
        auto indexLabel = CCLabelBMFont::create(std::to_string(i + 1).c_str(), "goldFont.fnt");
        indexLabel->setScale(0.4f);
        indexLabel->setAnchorPoint({0, 0.5f});
        indexLabel->setAlignment(kCCTextAlignmentLeft);
        indexLabel->setPosition(4.f, 16.f);
        indexLabel->setID(indexLabelId);

        actionNode->addChild(indexLabel);

        // Main label for the action node (always use Event Label if available)
        std::string mainLabelId = "action-main-label-" + std::to_string(i);
        std::string mainLabelText = actionIdRaw;
        // Always use the event label from CommandActionEventNode::getAllEventNodes if available
        for (const auto &info : CommandActionEventNode::getAllEventNodes())
        {
            if (actionIdLower.rfind(info.id, 0) == 0)
            {
                mainLabelText = info.label;
                break;
            }
        }

        //
        std::string btnId;
        bool hasSettingsHandler = false;

        if (actionIdLower.rfind("alert_popup", 0) == 0)
        {
            btnId = "alert-popup-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("notification", 0) == 0)
        {
            btnId = "notification-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("keycode", 0) == 0)
        {
            btnId = "keycode-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("profile", 0) == 0)
        {
            btnId = "profile-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("move", 0) == 0)
        {
            btnId = "move-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("jump", 0) == 0)
        {
            btnId = "jump-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("color_player", 0) == 0)
        {
            btnId = "color-player-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("edit_camera", 0) == 0)
        {
            btnId = "edit-camera-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("scale_player", 0) == 0)
        {
            btnId = "scale-player-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("speed_player", 0) == 0)
        {
            btnId = "speed-player-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("scale_player", 0) == 0)
        {
            btnId = "scale-player-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("sound", 0) == 0)
        {
            btnId = "sound-effect-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }
        else if (actionIdLower.rfind("gravity", 0) == 0)
        {
            btnId = "gravity-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }

        // Always align main label to the same x/y for all nodes
        float mainLabelX = 25.f;
        float mainLabelY = hasSettingsHandler ? 21.f : 16.f;

        auto mainLabel = CCLabelBMFont::create(mainLabelText.c_str(), "bigFont.fnt");
        mainLabel->setID(mainLabelId);
        mainLabel->setScale(0.5f);
        mainLabel->setAnchorPoint({0, 0.5f});
        mainLabel->setAlignment(kCCTextAlignmentLeft);
        mainLabel->setPosition(mainLabelX, mainLabelY);

        actionNode->addChild(mainLabel);

        // Add a secondary text label for extra info (if needed)
        std::string textLabelId = "action-text-label-" + std::to_string(i);
        std::string textLabelText = "";

        // Add a settings text label for the current settings values (if any)
        std::string settingsLabelId = "action-settings-label-" + std::to_string(i);
        std::string settingsLabelText = "";

        if (hasSettingsHandler)
        {
            if (actionIdLower.rfind("alert_popup", 0) == 0)
            {
                size_t firstColon = actionIdRaw.find(":");
                size_t secondColon = actionIdRaw.find(":", firstColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string title = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string desc = actionIdRaw.substr(secondColon + 1);
                    settingsLabelText = "Title: " + title + " | Content: " + desc;
                };
            }
            else if (actionIdLower.rfind("notification", 0) == 0)
            {
                std::string notifText;
                size_t firstColon = actionIdRaw.find(":");
                size_t secondColon = actionIdRaw.find(":", firstColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string iconTypeStr = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                    notifText = actionIdRaw.substr(secondColon + 1);
                    std::string iconName = "Info";
                    int iconTypeInt = 1;
                    bool validInt = true;
                    for (char c : iconTypeStr)
                    {
                        if (!isdigit(c) && !(c == '-' && &c == &iconTypeStr[0]))
                        {
                            validInt = false;
                            break;
                        };
                    };
                    if (validInt && !iconTypeStr.empty())
                        iconTypeInt = std::atoi(iconTypeStr.c_str());
                    switch (iconTypeInt)
                    {
                    case 0:
                        iconName = "None";
                        break;
                    case 1:
                        iconName = "Info";
                        break;
                    case 2:
                        iconName = "Success";
                        break;
                    case 3:
                        iconName = "Warning";
                        break;
                    case 4:
                        iconName = "Error";
                        break;
                    case 5:
                        iconName = "Loading";
                        break;
                    default:
                        iconName = "Info";
                        break;
                    };
                    settingsLabelText = "Icon: " + iconName + " | Text: " + notifText;
                };
                if (notifText.empty())
                {
                    settingsLabelText = "Icon: Info | Text: -";
                }
            }
            else if (actionIdLower.rfind("keycode", 0) == 0)
            {
                size_t colon = actionIdRaw.find(":");
                std::string key;
                if (colon != std::string::npos && colon + 1 < actionIdRaw.size())
                {
                    key = actionIdRaw.substr(colon + 1);
                    settingsLabelText = "Key: " + key;
                };
                if (key.empty())
                {
                    settingsLabelText = "Key: None";
                }
            }
            else if (actionIdLower.rfind("profile", 0) == 0)
            {
                size_t colon = actionIdRaw.find(":");
                std::string user;
                if (colon != std::string::npos && colon + 1 < actionIdRaw.size())
                {
                    user = actionIdRaw.substr(colon + 1);
                    settingsLabelText = "User: " + user;
                };
                if (user.empty())
                {
                    settingsLabelText = "No User selected";
                }
            }
            else if (actionIdLower.rfind("move", 0) == 0)
            {
                size_t firstColon = actionIdRaw.find(":");
                size_t secondColon = actionIdRaw.find(":", firstColon + 1);
                size_t thirdColon = std::string::npos;
                if (secondColon != std::string::npos)
                    thirdColon = actionIdRaw.find(":", secondColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string player = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string dir, amount;
                    if (thirdColon != std::string::npos)
                    {
                        dir = actionIdRaw.substr(secondColon + 1, thirdColon - secondColon - 1);
                        amount = actionIdRaw.substr(thirdColon + 1);
                    }
                    else
                    {
                        dir = actionIdRaw.substr(secondColon + 1);
                        amount = "";
                    };
                    if (!dir.empty())
                        dir[0] = toupper(dir[0]);
                    std::string amountStr = amount;
                    if (!amount.empty())
                    {
                        char *endptr = nullptr;
                        float amt = strtof(amount.c_str(), &endptr);
                        if (endptr != amount.c_str() && *endptr == '\0')
                        {
                            auto amountStr = fmt::format("{:.4f}", amt);
                        };
                    };
                    settingsLabelText = "Player " + player + " | Dir: " + dir;
                    if (!amountStr.empty())
                        settingsLabelText += " (" + amountStr + ")";
                };
            }
            else if (actionIdLower.rfind("jump", 0) == 0)
            {
                size_t colon = actionIdRaw.find(":");
                if (colon != std::string::npos && colon + 1 < actionIdRaw.size())
                {
                    std::string rest = actionIdRaw.substr(colon + 1);
                    if (rest == "3")
                    {
                        settingsLabelText = "Both Players";
                    }
                    else if (rest.rfind("3:hold", 0) == 0)
                    {
                        settingsLabelText = "Both Players (Hold)";
                    }
                    else
                    {
                        size_t holdPos = rest.find(":hold");
                        if (holdPos != std::string::npos)
                        {
                            std::string playerNum = rest.substr(0, holdPos);
                            settingsLabelText = "Player " + playerNum + " (Hold)";
                        }
                        else
                        {
                            settingsLabelText = "Player " + rest;
                        }
                    }
                }
            }
            else if (actionIdLower.rfind("color_player", 0) == 0)
            {
                size_t colon = actionIdRaw.find(":");
                if (colon != std::string::npos && colon + 1 < actionIdRaw.size())
                {
                    std::string rgb = actionIdRaw.substr(colon + 1);
                    settingsLabelText = "RGB: " + rgb;
                }
                else
                {
                    settingsLabelText = "No color selected";
                }
            }
            else if (actionIdLower.rfind("edit_camera", 0) == 0)
            {
                size_t firstColon = actionIdRaw.find(":");
                size_t secondColon = actionIdRaw.find(":", firstColon + 1);
                size_t thirdColon = actionIdRaw.find(":", secondColon + 1);
                size_t fourthColon = actionIdRaw.find(":", thirdColon + 1);
                float skew = 0.f, rot = 0.f, scale = 0.f, time = 0.f;
                if (firstColon != std::string::npos && secondColon != std::string::npos && thirdColon != std::string::npos && fourthColon != std::string::npos)
                {
                    std::string skewStr = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string rotStr = actionIdRaw.substr(secondColon + 1, thirdColon - secondColon - 1);
                    std::string scaleStr = actionIdRaw.substr(thirdColon + 1, fourthColon - thirdColon - 1);
                    std::string timeStr = actionIdRaw.substr(fourthColon + 1);
                    if (!skewStr.empty())
                        skew = strtof(skewStr.c_str(), nullptr);
                    if (!rotStr.empty())
                        rot = strtof(rotStr.c_str(), nullptr);
                    if (!scaleStr.empty())
                        scale = strtof(scaleStr.c_str(), nullptr);
                    if (!timeStr.empty())
                        time = strtof(timeStr.c_str(), nullptr);
                }
                auto buf = fmt::format("Skew: {:.2f}, Rot: {:.2f}, Scale: {:.2f}, Time: {:.2f}", skew, rot, scale, time);
                settingsLabelText = buf;
            }
            else if (actionIdLower.rfind("scale_player", 0) == 0)
            {
                float scale = 1.f;
                float time = 0.5f;
                size_t firstColon = actionIdRaw.find(":");
                size_t secondColon = actionIdRaw.find(":", firstColon + 1);
                if (firstColon != std::string::npos)
                {
                    std::string scaleStr = actionIdRaw.substr(firstColon + 1, (secondColon != std::string::npos ? secondColon - firstColon - 1 : std::string::npos));
                    if (!scaleStr.empty())
                        scale = strtof(scaleStr.c_str(), nullptr);
                    if (secondColon != std::string::npos)
                    {
                        std::string timeStr = actionIdRaw.substr(secondColon + 1);
                        if (!timeStr.empty())
                            time = strtof(timeStr.c_str(), nullptr);
                    }
                }
                auto buf = fmt::format("Scale: {:.2f}, Time: {:.2f}", scale, time);
                settingsLabelText = buf;
            }
            else if (actionIdLower.rfind("sound", 0) == 0)
            {
                // sound_effect:<sound>:<speed>:<volume>:<pitch>:<start>:<end>
                size_t firstColon = actionIdRaw.find(":");
                if (firstColon == std::string::npos || firstColon + 1 >= actionIdRaw.size()) {
                    settingsLabelText = "No sound selected";
                } else {
                    std::string rest = actionIdRaw.substr(firstColon + 1);
                    std::vector<std::string> parts;
                    size_t start = 0;
                    while (true) {
                        size_t pos = rest.find(":", start);
                        if (pos == std::string::npos) { parts.push_back(rest.substr(start)); break; }
                        parts.push_back(rest.substr(start, pos - start));
                        start = pos + 1;
                    }
                    if (parts.empty() || parts[0].empty()) {
                        settingsLabelText = "No sound selected";
                    } else if (parts.size() == 1) {
                        settingsLabelText = parts[0];
                    } else {
                        // Try parse numbers for nicer formatting
                        float spd = (parts.size() > 1) ? strtof(parts[1].c_str(), nullptr) : 1.f;
                        float vol = (parts.size() > 2) ? strtof(parts[2].c_str(), nullptr) : 1.f;
                        float pit = (parts.size() > 3) ? strtof(parts[3].c_str(), nullptr) : 0.f;
                        int st = (parts.size() > 4) ? atoi(parts[4].c_str()) : 0;
                        int en = (parts.size() > 5) ? atoi(parts[5].c_str()) : 0;
                        settingsLabelText = fmt::format("{} | Speed: {:.2f}, Vol: {:.2f}, Pit: {:.2f}, ({}-{} ms)", parts[0], spd, vol, pit, st, en);
                    }
                }
            }
            else if (actionIdLower.rfind("gravity", 0) == 0)
            {
                // Format: gravity:<gravity>:<duration>
                size_t firstColon = actionIdRaw.find(":");
                size_t secondColon = actionIdRaw.find(":", firstColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string gravityStr = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string durationStr = actionIdRaw.substr(secondColon + 1);
                    settingsLabelText = "Gravity: " + gravityStr + " | Duration: " + durationStr;
                }
                else
                {
                    settingsLabelText = "Gravity: 1.00 | Duration: 2.00";
                }
            }
            else if (actionIdLower.rfind("speed_player", 0) == 0)
            {
                // Format: speed_player:<speed>:<duration>
                size_t firstColon = actionIdRaw.find(":");
                size_t secondColon = actionIdRaw.find(":", firstColon + 1);
                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string speedStr = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string durationStr = actionIdRaw.substr(secondColon + 1);
                    settingsLabelText = "Speed: " + speedStr + " | Duration: " + durationStr;
                }
                else
                {
                    settingsLabelText = "Speed: - | Duration: -";
                }
            }
        };

        if (!textLabelText.empty())
        {
            auto textLabel = CCLabelBMFont::create(textLabelText.c_str(), "chatFont.fnt");
            textLabel->setID(textLabelId);
            textLabel->setScale(0.5f);
            textLabel->setAnchorPoint({0, 0.5f});
            textLabel->setAlignment(kCCTextAlignmentLeft);
            textLabel->setPosition(140.f, 16.f);

            actionNode->addChild(textLabel);
        };

        // Always create the settings label for these actions, even if the text is empty
        if (hasSettingsHandler)
        {
            auto settingsLabel = CCLabelBMFont::create(settingsLabelText.c_str(), "chatFont.fnt");
            settingsLabel->setScale(0.5f);
            settingsLabel->setAnchorPoint({0, 0.5f});
            settingsLabel->setAlignment(kCCTextAlignmentLeft);

            // Place settings label directly underneath the main label
            float settingsLabelX = mainLabel->getPositionX();
            float settingsLabelY = mainLabel->getPositionY() - (mainLabel->getContentSize().height * mainLabel->getScale() / 2) - (settingsLabel->getContentSize().height * settingsLabel->getScale() / 2) - 2.f;

            settingsLabel->setID(settingsLabelId);
            settingsLabel->setPosition(settingsLabelX, settingsLabelY);

            actionNode->addChild(settingsLabel);
        };

        // Right-aligned button layout
        float btnMenuRight = m_actionContent->getContentSize().width - 16.f; // right edge of the action node
        float btnMenuY = 16.f;
        float btnSpacing = 28.f; // horizontal spacing between buttons

        // up sprite
        auto upSprite = CCSprite::createWithSpriteFrameName("edit_upBtn_001.png");
        upSprite->setScale(0.7f);

        // Up arrow
        auto upBtn = CCMenuItemSpriteExtra::create(upSprite, this, menu_selector(CommandSettingsPopup::onMoveActionUp));
        upBtn->setID("action-" + std::to_string(i) + "-up-btn");
        upBtn->setUserObject(CCInteger::create(static_cast<int>(i)));
        upBtn->setPosition(0, 8.f);

        // down sprite
        auto downSprite = CCSprite::createWithSpriteFrameName("edit_downBtn_001.png");
        downSprite->setScale(0.7f);

        // Down arrow
        auto downBtn = CCMenuItemSpriteExtra::create(downSprite, this, menu_selector(CommandSettingsPopup::onMoveActionDown));
        downBtn->setID("action-" + std::to_string(i) + "-down-btn");
        downBtn->setUserObject(CCInteger::create(static_cast<int>(i)));
        downBtn->setPosition(0, -8.f);

        // Settings button (if applicable)
        CCMenuItemSpriteExtra *settingsBtn = nullptr;

        if (hasSettingsHandler)
        {
            auto settingsSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
            settingsSprite->setScale(0.5f);
            settingsBtn = CCMenuItemSpriteExtra::create(settingsSprite, this, menu_selector(CommandSettingsPopup::onSettingsButtonUnified));
            settingsBtn->setID(btnId);
            settingsBtn->setUserObject(CCInteger::create(static_cast<int>(i)));
            settingsBtn->setPosition(btnSpacing, 0);
        }

        // handle checkboxes for specific actions
        CCMenuItemToggler *noclipCheckbox = nullptr;
        if (actionIdLower.rfind("noclip", 0) == 0)
        {
            auto noclipOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
            auto noclipOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
            noclipCheckbox = CCMenuItemToggler::create(noclipOff, noclipOn, nullptr, nullptr);
            noclipCheckbox->setID("noclip-checkbox-" + std::to_string(i));
            noclipCheckbox->setPosition(btnSpacing, 0);
            noclipCheckbox->setScale(0.75f);
            // Set initial state from actionIdRaw ("noclip:true" or "noclip:false")
            bool checked = false;
            size_t colonPos = actionIdRaw.find(":");
            if (colonPos != std::string::npos && colonPos + 1 < actionIdRaw.size())
            {
                std::string val = actionIdRaw.substr(colonPos + 1);
                checked = (val == "true");
            }
            noclipCheckbox->toggle(checked);
        }

        // Remove button
        auto removeSprite = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
        removeSprite->setScale(0.55f);

        auto removeBtn = CCMenuItemSpriteExtra::create(
            removeSprite,
            this,
            menu_selector(CommandSettingsPopup::onRemoveAction));
        removeBtn->setID("action-" + std::to_string(i) + "-remove-btn");
        removeBtn->setPosition(btnSpacing * 2, 0);
        removeBtn->setUserObject(CCInteger::create(static_cast<int>(i)));
        removeBtn->setScale(1.0f);

        // If this is a wait action, add a TextInput at the settings button
        if (actionIdLower.rfind("wait:", 0) == 0) // Match any wait action with value
        {
            std::string waitInputId = "wait-delay-input-" + std::to_string(i); // Use i for consistent indexing
            auto waitInput = TextInput::create(50, "Sec", "bigFont.fnt");
            waitInput->setCommonFilter(CommonFilter::Float);

            // If there is a value in the action string, set it
            std::string waitValue = "";
            size_t colon = actionIdRaw.find(":");

            if (colon != std::string::npos && colon + 1 < actionIdRaw.size())
                waitValue = actionIdRaw.substr(colon + 1);
            if (!waitValue.empty())
            {
                waitValue.erase(0, waitValue.find_first_not_of(" \t\n\r"));
                waitValue.erase(waitValue.find_last_not_of(" \t\n\r") + 1);
                auto parsedWait = numFromString<float>(waitValue);
                float waitFloat = 0.0f;
                if (parsedWait)
                    waitFloat = parsedWait.unwrap();
                waitFloat = std::round(waitFloat * 1000.0f) / 1000.0f;
                waitInput->setString(fmt::format("{:.2f}", waitFloat));
            }

            waitInput->setID(waitInputId);
            waitInput->setPosition(btnMenuRight - btnSpacing, 16.f);
            waitInput->setScale(0.5f);

            actionNode->addChild(waitInput);
        }

        // Menu for all buttons, positioned at the right side of the node
        auto menu = CCMenu::create();
        menu->setPosition(btnMenuRight - btnSpacing * 2, btnMenuY);

        menu->addChild(upBtn);
        menu->addChild(downBtn);
        if (settingsBtn)
            menu->addChild(settingsBtn);
        if (noclipCheckbox)
            menu->addChild(noclipCheckbox);
        menu->addChild(removeBtn);

        actionNode->addChild(menu);

        float nodeBgWidth = m_actionContent ? m_actionContent->getContentSize().width : actionNode->getContentSize().width;
        auto nodeBg = CCScale9Sprite::create("square02_small.png");
        nodeBg->setContentSize(CCSize(nodeBgWidth, actionNode->getContentSize().height));
        nodeBg->setOpacity(60);
        nodeBg->setAnchorPoint({0, 0});
        nodeBg->setPosition(0, 0);
        actionNode->addChild(nodeBg, -1);

        actionNodeY -= (nodeHeight + actionNodeGap);
        actionIndex++;
    };
};

void CommandSettingsPopup::onRemoveAction(CCObject *sender)
{
    auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);
    int idx = 0;

    if (btn->getUserObject())
        idx = static_cast<CCInteger *>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= static_cast<int>(m_commandActions.size()))
        return;

    m_commandActions.erase(m_commandActions.begin() + idx);

    refreshActionsList();
};

void CommandSettingsPopup::onCloseBtn(CCObject *sender)
{
    this->onClose(nullptr);
}

void CommandSettingsPopup::onClose(CCObject *sender)
{
    geode::createQuickPopup(
        "Close Without Saving?",
        "Are you sure you want to close the settings without saving? <cr>Any unsaved changes will be lost.</c>",
        "Cancel", "Close",
        [this](auto, bool btn2) {
            if (btn2) {
                this->removeFromParent();
            }
        });
}

std::string CommandSettingsPopup::getNotificationText() const
{
    if (m_notificationInput)
    {
        std::string text = m_notificationInput->getString();

        // Trim whitespace
        text.erase(0, text.find_first_not_of(" \t\n\r"));
        text.erase(text.find_last_not_of(" \t\n\r") + 1);

        return text;
    };

    return "";
};

// Helper to update the color player RGB label for a given action index
void CommandSettingsPopup::updateColorPlayerLabel(int actionIdx)
{
    if (actionIdx >= 0 && actionIdx < static_cast<int>(m_commandActions.size()))
    {
        std::string &actionStr = m_commandActions[actionIdx];
        std::string actionStrLower = actionStr;
        std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);

        if (actionStrLower.rfind("color_player", 0) == 0)
        {
            auto children = m_actionContent->getChildren();

            if (children && actionIdx < children->count())
            {
                if (auto actionNode = static_cast<CCNode *>(children->objectAtIndex(actionIdx)))
                {
                    std::string colorLabelId = "color-player-action-text-label-" + std::to_string(actionIdx);
                    std::string rgbText = "255,255,255";

                    size_t colonPos = actionStr.find(":");
                    if (colonPos != std::string::npos && colonPos + 1 < actionStr.size())
                        rgbText = actionStr.substr(colonPos + 1);

                    std::string labelText = "RGB: " + rgbText;
                    if (auto colorLabel = typeinfo_cast<CCLabelBMFont *>(actionNode->getChildByID(colorLabelId)))
                        colorLabel->setString(labelText.c_str());
                };
            };
        };
    };
};

// Info button handler for event list
void CommandSettingsPopup::onEventInfoBtn(cocos2d::CCObject *sender)
{
    auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);

    std::string desc;
    std::string eventName = "Event";
    std::string btnId = btn ? btn->getID() : "";
    std::string eventId;

    // ID format: event-<id>-info-btn
    if (btnId.rfind("event-", 0) == 0)
    {
        size_t start = 6;
        size_t end = btnId.find("-info-btn");

        if (end != std::string::npos && end > start)
            eventId = btnId.substr(start, end - start);
    };

    // Find the event name from EventNodeInfo
    for (const auto &info : CommandActionEventNode::getAllEventNodes())
    {
        if (info.id == eventId)
        {
            eventName = info.label;
            break;
        };
    };

    if (btn && btn->getUserObject())
        desc = static_cast<CCString *>(btn->getUserObject())->getCString();

    if (!desc.empty())
    {
        FLAlertLayer::create(eventName.c_str(), desc, "OK")->show();
    }
    else
    {
        FLAlertLayer::create(eventName.c_str(), "No description available.", "OK")->show();
    };
};

void CommandSettingsPopup::onSave(CCObject *sender)
{
    // Build up to 10 actions in order, validate all wait inputs
    std::vector<TwitchCommandAction> actionsVec;

    for (size_t idx = 0; idx < m_commandActions.size(); ++idx)
    {
        std::string &actionIdRaw = m_commandActions[idx];

        std::string actionId = actionIdRaw;
        std::string waitValue;
        std::string jumpPlayerValue;
        std::string alertTitle, alertDesc;

        bool isHold = false;

        if (actionIdRaw.rfind("wait:", 0) == 0)
        {
            actionId = "wait";
            waitValue = actionIdRaw.substr(5);
        }
        else if (actionIdRaw.rfind("jump:", 0) == 0)
        {
            actionId = "jump";

            // Parse jumpPlayerValue and hold
            std::string val = actionIdRaw.substr(5); // could be "1", "1:hold", etc.

            size_t holdPos = val.find(":hold");
            if (holdPos != std::string::npos)
            {
                isHold = true;
                jumpPlayerValue = val.substr(0, holdPos);
            }
            else
            {
                jumpPlayerValue = val;
            };
        }
        else if (actionIdRaw.rfind("alert_popup:", 0) == 0)
        {
            actionId = "alert_popup";

            size_t firstColon = actionIdRaw.find(":");
            size_t secondColon = actionIdRaw.find(":", firstColon + 1);

            if (firstColon != std::string::npos && secondColon != std::string::npos)
            {
                alertTitle = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                alertDesc = actionIdRaw.substr(secondColon + 1);
            };
        };

        if (actionId == "wait")
        {
            std::string inputId = "wait-delay-input-" + std::to_string(idx);
            TextInput *waitInput = nullptr;
            auto children = m_actionContent->getChildren();
            if (children && idx < children->count())
            {
                if (auto node = static_cast<CCNode *>(children->objectAtIndex(idx)))
                {
                    auto inputNode = node->getChildByID(inputId);
                    if (inputNode)
                        waitInput = typeinfo_cast<TextInput *>(inputNode);
                }
            }
            std::string delayStr = waitValue;
            if (waitInput)
                delayStr = waitInput->getString();
            // Trim whitespace
            delayStr.erase(0, delayStr.find_first_not_of(" \t\n\r"));
            delayStr.erase(delayStr.find_last_not_of(" \t\n\r") + 1);
            if (delayStr.empty())
            {
                Notification::create("Please fill in all wait delay fields!", NotificationIcon::Error)->show();
                return;
            }
            auto parsedDelay = numFromString<float>(delayStr);
            if (!parsedDelay)
            {
                Notification::create("Wait delay must be a valid number!", NotificationIcon::Error)->show();
                return;
            }
            float delay = parsedDelay.unwrap();
            delay = std::round(delay * 1000.0f) / 1000.0f;
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Wait, "wait", delay));
            actionIdRaw = fmt::format("wait:{:.3f}", delay);
        }
        else if (actionId == "jump")
        {
            int playerIdx = 1;
            jumpPlayerValue.erase(0, jumpPlayerValue.find_first_not_of(" \t\n\r"));
            jumpPlayerValue.erase(jumpPlayerValue.find_last_not_of(" \t\n\r") + 1);
            if (!jumpPlayerValue.empty())
            {
                auto parsedJump = numFromString<int>(jumpPlayerValue);
                if (!parsedJump)
                {
                    Notification::create("Jump player index must be a valid number!", NotificationIcon::Error)->show();
                    return;
                }
                playerIdx = parsedJump.unwrap();
            }
            std::string jumpActionStr = "jump:" + std::to_string(playerIdx);
            if (isHold)
                jumpActionStr += ":hold";
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, jumpActionStr, 0));
            actionIdRaw = jumpActionStr;
        }
        else if (actionId == "alert_popup")
        {
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, "alert_popup:" + alertTitle + ":" + alertDesc, 0));
        }
        else if (actionId == "kill_player")
        {
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, "kill_player", 0));
        }
        else if (actionIdRaw.rfind("noclip", 0) == 0)
        {
            // Always get the current state from the visible checkbox, not from actionIdRaw
            bool checked = false;
            auto children = m_actionContent->getChildren();

            if (children && idx < children->count())
            {
                if (auto node = static_cast<CCNode *>(children->objectAtIndex(idx)))
                {
                    // Find the noclip checkbox in the menu
                    CCMenu *menu = nullptr;

                    for (auto child : CCArrayExt<CCNode *>(node->getChildren()))
                    {
                        if ((menu = typeinfo_cast<CCMenu *>(child)))
                            break;
                    };

                    if (menu)
                    {
                        for (auto btn : CCArrayExt<CCNode *>(menu->getChildren()))
                        {
                            if (auto toggler = typeinfo_cast<CCMenuItemToggler *>(btn))
                            {
                                if (std::string(toggler->getID()).find("noclip-checkbox-") == 0)
                                {
                                    checked = toggler->isToggled();
                                    // Update actionIdRaw to reflect the current checkbox state
                                    actionIdRaw = "noclip:" + std::string(checked ? "true" : "false");
                                    break;
                                };
                            };
                        };
                    };
                };
            };
            // Always push the updated value
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, actionIdRaw, 0));
        }
        else if (actionIdRaw.rfind("keycode:", 0) == 0)
        {
            // Save as event with arg 'keycode:<key>'
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, actionIdRaw, 0));
        }
        else if (actionIdRaw.rfind("notification:", 0) == 0)
        {
            // Parse icon type and text: notification:<iconInt>:<text>
            int iconTypeInt = 1;
            std::string notifText;

            size_t firstColon = actionIdRaw.find(":");
            size_t secondColon = actionIdRaw.find(":", firstColon + 1);

            if (firstColon != std::string::npos && secondColon != std::string::npos)
            {
                {
                    std::string iconTypeStr = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                    iconTypeStr.erase(0, iconTypeStr.find_first_not_of(" \t\n\r"));
                    iconTypeStr.erase(iconTypeStr.find_last_not_of(" \t\n\r") + 1);
                    auto parsedIcon = numFromString<int>(iconTypeStr);
                    if (parsedIcon)
                        iconTypeInt = parsedIcon.unwrap();
                }
                notifText = actionIdRaw.substr(secondColon + 1);
            }
            else if (actionIdRaw.length() > 13)
            {
                notifText = actionIdRaw.substr(13);
            }
            else
            {
                notifText = "";
            };

            actionsVec.push_back(TwitchCommandAction(CommandActionType::Notification, std::to_string(iconTypeInt) + ":" + notifText, 0));
        }
        else if (actionIdRaw.rfind("move:", 0) == 0)
        {
            // Save as event with arg 'move:<player>:<direction>'
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, actionIdRaw, 0));
        }
        else if (actionIdRaw.rfind("scale_player:", 0) == 0)
        {
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, actionIdRaw, 0));
        }
        else
        {
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, actionId, 0));
        };
    };

    // Replace m_command.actions with actionsVec (preserve order, no size limit)
    m_command.actions = actionsVec;

    // Rebuild m_commandActions from m_command.actions to ensure UI and data are in sync
    m_commandActions.clear();

    for (const auto &action : m_command.actions)
    {
        if (action.type == CommandActionType::Notification)
        {
            m_commandActions.push_back("notification:" + action.arg);
        }
        else if (action.type == CommandActionType::Wait)
        {
            auto buf = fmt::format("{:.2f}", action.index);
            m_commandActions.push_back("wait:" + std::string(buf));
        }
        else if (action.type == CommandActionType::Event && action.arg.rfind("alert_popup:", 0) == 0)
        {
            m_commandActions.push_back(action.arg);
        }
        else if (action.type == CommandActionType::Event)
        {
            m_commandActions.push_back(action.arg);
        };
    };

    // Refresh the actions list to ensure notification node is visible
    refreshActionsList();

    Notification::create("Command Settings Saved!", NotificationIcon::Success)->show();

    // Save changes to the command manager
    auto commandManager = TwitchCommandManager::getInstance();
    for (auto &cmd : commandManager->getCommands())
    {
        if (cmd.name == m_command.name)
        {
            cmd = m_command; // Replace the entire command object
            break;
        };
    };

    commandManager->saveCommands();

    // Refresh the dashboard command list if the dashboard is open
    if (auto scene = cocos2d::CCDirector::sharedDirector()->getRunningScene())
    {
        if (auto dashboard = typeinfo_cast<TwitchDashboard *>(scene->getChildByID("twitch-dashboard-popup")))
            dashboard->refreshCommandsList();
    };

    this->removeFromParent();
};

void CommandSettingsPopup::updateNotificationNextTextLabel(int actionIdx, const std::string &nextText, NotificationIconType iconType)
{
    if (actionIdx >= 0 && actionIdx < static_cast<int>(m_commandActions.size()))
    {
        int iconTypeInt = static_cast<int>(iconType);
        m_commandActions[actionIdx] = "notification:" + std::to_string(iconTypeInt) + ":" + nextText;

        // Find the Nth notification action node (among all notification nodes)
        int notifNodeIdx = -1;
        int notifCount = 0;

        for (int i = 0; i <= actionIdx; ++i)
        {
            if (m_commandActions[i].rfind("notification:", 0) == 0)
            {
                notifNodeIdx = notifCount;
                notifCount++;
            };
        };

        // Now update the notifNodeIdx-th notification action in m_command.actions
        if (notifNodeIdx >= 0)
        {
            int notifIdx = 0;

            for (auto &action : m_command.actions)
            {
                if (action.type == CommandActionType::Notification)
                {
                    if (notifIdx == notifNodeIdx)
                    {
                        action.arg = std::to_string(iconTypeInt) + ":" + nextText;
                        break;
                    };

                    notifIdx++;
                };
            };
        };
    };

    // Update the label in the action node to show the new custom notification text (or 'Notification' if empty)
    auto children = m_actionContent->getChildren();
    if (!children || actionIdx < 0 || actionIdx >= children->count())
        return;

    auto actionNode = static_cast<CCNode *>(children->objectAtIndex(actionIdx));
    if (!actionNode)
        return;

    std::string notifLabelId = "notification-action-text-label-" + std::to_string(actionIdx);
    // Show icon name and text
    std::string iconName = "Info";

    switch (iconType)
    {
    case NotificationIconType::None:
        iconName = "None";
        break;

    case NotificationIconType::Info:
        iconName = "Info";
        break;

    case NotificationIconType::Success:
        iconName = "Success";
        break;

    case NotificationIconType::Warning:
        iconName = "Warning";
        break;

    case NotificationIconType::Error:
        iconName = "Error";
        break;

    case NotificationIconType::Loading:
        iconName = "Loading";
        break;

    default:
        iconName = "Info";
        break;
    };

    std::string labelText = iconName;
    if (!nextText.empty())
    {
        labelText += ": ";
        labelText += nextText;
    };

    if (auto notifLabel = typeinfo_cast<CCLabelBMFont *>(actionNode->getChildByID(notifLabelId)))
        notifLabel->setString(labelText.c_str());
};

// Unified settings button handler
void CommandSettingsPopup::onSettingsButtonUnified(cocos2d::CCObject *sender)
{
    auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);
    int actionIdx = 0;

    if (btn && btn->getUserObject())
        actionIdx = static_cast<CCInteger *>(btn->getUserObject())->getValue();
    if (actionIdx < 0 || actionIdx >= static_cast<int>(m_commandActions.size()))
        return;

    std::string &actionStr = m_commandActions[actionIdx];
    std::string actionStrLower = actionStr;

    std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);

    // Only one handler should be called per action
    if (actionStrLower.rfind("alert_popup", 0) == 0)
        SettingsHandler::handleAlertSettings(this, sender);
    else if (actionStrLower.rfind("notification", 0) == 0)
        SettingsHandler::handleNotificationSettings(this, sender);
    else if (actionStrLower.rfind("keycode", 0) == 0)
        SettingsHandler::handleKeyCodeSettings(this, sender);
    else if (actionStrLower.rfind("profile", 0) == 0)
        SettingsHandler::handleProfileSettings(this, sender);
    else if (actionStrLower.rfind("move", 0) == 0)
        SettingsHandler::handleMoveSettings(this, sender);
    else if (actionStrLower.rfind("jump", 0) == 0)
        SettingsHandler::handleJumpSettings(this, sender);
    else if (actionStrLower.rfind("color_player", 0) == 0)
        SettingsHandler::handleColorPlayerSettings(this, sender);
    else if (actionStrLower.rfind("edit_camera", 0) == 0)
        SettingsHandler::handleEditCameraSettings(this, sender);
    else if (actionStrLower.rfind("scale_player", 0) == 0)
        SettingsHandler::handleScalePlayerSettings(this, sender);
    else if (actionStrLower.rfind("speed_player", 0) == 0)
        SettingsHandler::handleSpeedSettings(this, sender);
    else if (actionStrLower.rfind("sound", 0) == 0)
        SettingsHandler::handleSoundEffectSettings(this, sender);
    else if (actionStrLower.rfind("gravity", 0) == 0)
        SettingsHandler::handleGravitySettings(this, sender);
}
// Polling function for event search input
void CommandSettingsPopup::onEventSearchPoll(float)
{
    if (!m_eventSearchInput || !m_refreshEventNodeList)
        return;

    std::string text = m_eventSearchInput->getString();
    if (text != m_lastEventSearchString)
    {
        m_eventSearchString = text;
        m_refreshEventNodeList(m_eventSearchString);
        m_lastEventSearchString = text;

        if (m_mainLayer)
        {
            if (auto eventScrollLayer = typeinfo_cast<ScrollLayer *>(m_mainLayer->getChildByID("events-scroll")))
                eventScrollLayer->scrollToTop();
        };
    };
};

// Static create function for CommandSettingsPopup with only TwitchCommand argument
CommandSettingsPopup *CommandSettingsPopup::create(TwitchCommand command)
{
    auto ret = new CommandSettingsPopup();
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    if (ret && ret->initAnchored(winSize.width + 50.f, winSize.height, command))
    {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};