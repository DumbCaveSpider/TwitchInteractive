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
#include "../handler/LevelInfoSettingsPopup.hpp"

using namespace cocos2d;
using namespace geode::prelude;

// Free function to add or update a label for an action node with uniform style
void addOrUpdateActionLabel(CCNode* actionNode, const std::string& labelId, const std::string& text, float x, float y) {
    if (!actionNode) return;
    auto label = actionNode->getChildByID(labelId);
    if (!label) {
        auto newLabel = CCLabelBMFont::create(text.c_str(), "chatFont.fnt");
        newLabel->setScale(0.5f);
        newLabel->setAnchorPoint({ 0, 0.5f });
        newLabel->setAlignment(kCCTextAlignmentLeft);
        newLabel->setPosition(x, y);
        newLabel->setID(labelId);
        actionNode->addChild(newLabel);
    } else {
        if (auto bmLabel = dynamic_cast<CCLabelBMFont*>(label)) bmLabel->setString(text.c_str());
    }
}

bool CommandSettingsPopup::setup(TwitchCommand command) {
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
    eventLabel->setAnchorPoint({ 0.5f, 0.5f });
    eventLabel->setPosition(eventSectionX + scrollSize.width / 2, scrollBgY + scrollSize.height + 22.f);
    eventLabel->setID("events-section-label");
    m_mainLayer->addChild(eventLabel);

    auto actionLabel = CCLabelBMFont::create("Actions", "bigFont.fnt");
    actionLabel->setScale(0.6f);
    actionLabel->setAnchorPoint({ 0.5f, 0.5f });
    actionLabel->setPosition(actionSectionX + scrollSize.width / 2, scrollBgY + scrollSize.height + 22.f);
    actionLabel->setID("actions-section-label");
    m_mainLayer->addChild(actionLabel);
    m_command = command;

    // Profile button (top right)
    auto profileBtnSprite = CCSprite::createWithSpriteFrameName("GJ_profileButton_001.png");
    profileBtnSprite->setScale(0.7f);
    auto profileBtn = CCMenuItemSpriteExtra::create(
        profileBtnSprite,
        this,
        menu_selector(CommandSettingsPopup::onProfileUserSettings)
    );
    profileBtn->setID("command-profile-user-btn");
    auto profileMenu = CCMenu::create();
    profileMenu->addChild(profileBtn);
    profileMenu->setPosition(m_mainLayer->getContentSize().width - 150.f, m_mainLayer->getContentSize().height - 30.f);
    m_mainLayer->addChild(profileMenu);

    setTitle(fmt::format("!{} settings", command.name));
    setID("command-settings-popup");

    // Background for the event scroll layer
    auto eventScrollBg = CCScale9Sprite::create("square02_001.png");
    eventScrollBg->setID("events-scroll-background");
    eventScrollBg->setContentSize(scrollSize);
    eventScrollBg->setOpacity(50);
    eventScrollBg->setAnchorPoint({ 0.5f, 0.5f });
    eventScrollBg->setScale(1.05f);
    eventScrollBg->setPosition(eventSectionX + scrollSize.width / 2, scrollBgY + scrollSize.height / 2);
    m_mainLayer->addChild(eventScrollBg);

    // Scroll layer for events
    auto eventScrollLayer = ScrollLayer::create(scrollSize);
    eventScrollLayer->setID("events-scroll");
    eventScrollLayer->setPosition(eventSectionX, scrollLayerY);

    // Background for the actions scroll layer
    auto actionScrollBg = CCScale9Sprite::create("square02_001.png");
    actionScrollBg->setID("actions-scroll-background");
    actionScrollBg->setContentSize(scrollSize);
    actionScrollBg->setOpacity(50);
    actionScrollBg->setAnchorPoint({ 0.5f, 0.5f });
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
    for (const auto& action : command.actions) {
        if (action.type == CommandActionType::Notification) {
            m_commandActions.push_back("notification:" + action.arg);
        } else if (action.type == CommandActionType::Wait) {
            m_commandActions.push_back("wait:" + std::to_string(action.index));
        } else if (action.type == CommandActionType::Event) {
            if (action.arg.rfind("jump:", 0) == 0) {
                m_commandActions.push_back(action.arg);
            } else {
                m_commandActions.push_back(action.arg);
            };
        };
    };

    m_actionContent = actionContent;
    m_actionSectionHeight = sectionHeight;

    refreshActionsList();

    // Dynamically add all event nodes
    float eventNodeGap = 8.0f;
    float nodeHeight = 32.f;
    int eventCount = static_cast<int>(CommandActionEventNode::getAllEventNodes().size());
    float minContentHeight = scrollSize.height;
    float neededHeight = eventCount * (nodeHeight + eventNodeGap);
    float contentHeight = std::max(minContentHeight, neededHeight);
    eventContent->setContentSize(CCSize(scrollSize.width, contentHeight));

    float eventNodeY = contentHeight - 16.f;
    for (const auto& info : CommandActionEventNode::getAllEventNodes()) {
        auto node = CCNode::create();
        node->setContentSize(CCSize(scrollSize.width, nodeHeight));

        // Label
        auto label = CCLabelBMFont::create(info.label.c_str(), "bigFont.fnt");
        label->setID("event-" + info.id + "-label");
        label->setScale(0.5f);
        label->setAnchorPoint({ 0, 0.5f });
        label->setAlignment(kCCTextAlignmentLeft);
        label->setPosition(20.f, 16.f);

        // Info button (FLAlertLayer)
        auto infoBtnSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoBtnSprite->setScale(0.5f);
        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoBtnSprite,
            this,
            menu_selector(CommandSettingsPopup::onEventInfoBtn)
        );
        infoBtn->setID("event-" + info.id + "-info-btn");
        infoBtn->setUserObject(CCString::create(info.description));
        float infoBtnX = 20.f + label->getContentSize().width * label->getScale() + 12.f;
        infoBtn->setPosition(infoBtnX, 16.f);

        // Menu for add/info buttons
        auto menu = CCMenu::create();
        menu->setPosition(0, 0);

        // Add button (always use GJ_plusBtn_001.png) at right side
        auto addSprite = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        addSprite->setScale(0.5f);

        auto addBtn = CCMenuItemSpriteExtra::create(
            addSprite,
            this,
            menu_selector(CommandSettingsPopup::onAddEventAction)
        );
        addBtn->setID("event-" + info.id + "-add-btn");
        addBtn->setPosition(scrollSize.width - 24.f, 16.f); // Place button at far right
        addBtn->setUserObject(CCString::create(info.id));

        menu->addChild(addBtn);
        menu->addChild(infoBtn);

        node->addChild(label);
        node->addChild(menu);

        // Add a background to the event node
        auto nodeBg = CCScale9Sprite::create("square02_001.png");
        nodeBg->setContentSize(node->getContentSize());
        nodeBg->setOpacity(60);
        nodeBg->setAnchorPoint({ 0, 0 });
        nodeBg->setPosition(0, 0);

        node->addChild(nodeBg, -1);
        node->setPosition(0, eventNodeY - 16.f); // 16.f is half node height

        eventContent->addChild(node);

        eventNodeY -= (nodeHeight + eventNodeGap);
    }
    // After adding all event nodes, scroll to top
    eventScrollLayer->scrollToTop();

    // Add action nodes for existing actions
    m_mainLayer->addChild(actionScrollLayer);
    m_mainLayer->addChild(eventScrollLayer);

    // Set checkbox state from command actions
    bool killChecked = false;
    for (const auto& action : command.actions) {
        if (action.type == CommandActionType::Event && action.arg == "kill_player") {
            killChecked = true;
            break;
        };
    };

    if (m_killPlayerCheckbox) m_killPlayerCheckbox->toggle(killChecked);


    // Save button
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(CommandSettingsPopup::onSave)
    );
    saveBtn->setID("command-settings-save-btn");

    // Handbook button
    auto handbookBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Handbook", "bigFont.fnt", "GJ_button_05.png", 0.6f),
        this,
        menu_selector(CommandSettingsPopup::onHandbookBtn)
    );
    handbookBtn->setID("command-settings-handbook-btn");

    // Close button
    auto closeBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Close", "bigFont.fnt", "GJ_button_06.png", 0.6f),
        this,
        menu_selector(CommandSettingsPopup::onCloseBtn)
    );
    closeBtn->setID("command-settings-close-btn");

    // Menu for buttons
    auto commandBtnMenu = CCMenu::create();
    commandBtnMenu->setID("command-settings-button-menu");
    commandBtnMenu->addChild(saveBtn);
    commandBtnMenu->addChild(handbookBtn);
    commandBtnMenu->addChild(closeBtn);
    commandBtnMenu->setContentSize({ 570.f, 25.f });

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
}

void CommandSettingsPopup::onProfileUserSettings(CCObject* sender) {
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
        [this](const std::string& user, bool vip, bool mod, bool subscriber, bool streamer) {
            m_command.allowedUser = user;
            m_command.allowVip = vip;
            m_command.allowMod = mod;
            m_command.allowSubscriber = subscriber;
            m_command.allowStreamer = streamer;

            // Update all CommandActionEventNode role labels in the UI
            if (m_actionContent) {
                auto children = m_actionContent->getChildren();
                if (children) {
                    for (int i = 0; i < children->count(); ++i) {
                        auto node = as<CCNode*>(children->objectAtIndex(i));
                        if (node) {
                            auto eventNode = dynamic_cast<CommandActionEventNode*>(node);
                            if (eventNode) {
                                eventNode->updateRoleLabel();
                            }
                        }
                    }
                }
            }
        }
    );
    if (popup) popup->show();
    return;
}



void CommandSettingsPopup::onMoveActionUp(cocos2d::CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx <= 0 || idx >= as<int>(m_commandActions.size())) return;
    std::swap(m_commandActions[idx], m_commandActions[idx - 1]);
    refreshActionsList();
}

void CommandSettingsPopup::onMoveActionDown(cocos2d::CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= as<int>(m_commandActions.size()) - 1) return;
    std::swap(m_commandActions[idx], m_commandActions[idx + 1]);
    refreshActionsList();
}

// Handler for color player settings button
void CommandSettingsPopup::onColorPlayerSettings(CCObject* sender) {
    SettingsHandler::handleColorPlayerSettings(this, sender);
}

// Jump settings handler
void CommandSettingsPopup::onJumpSettings(cocos2d::CCObject* sender) {
    SettingsHandler::handleJumpSettings(this, sender);
}

// Move Player settings handler
void CommandSettingsPopup::onMoveSettings(cocos2d::CCObject* sender) {
    SettingsHandler::handleMoveSettings(this, sender);
}

// Edit Camera settings handler
void CommandSettingsPopup::onEditCameraSettings(cocos2d::CCObject* sender) {
    SettingsHandler::handleEditCameraSettings(this, sender);
}

// Notification settings handler
void CommandSettingsPopup::onNotificationSettings(cocos2d::CCObject* sender) {
    SettingsHandler::handleNotificationSettings(this, sender);
}

void CommandSettingsPopup::onAlertSettings(cocos2d::CCObject* sender) {
    SettingsHandler::handleAlertSettings(this, sender);
}

// Player Profile settings handler
void CommandSettingsPopup::onProfileSettings(cocos2d::CCObject* sender) {
    SettingsHandler::handleProfileSettings(this, sender);
}
// KeyCode settings handler
void CommandSettingsPopup::onKeyCodeSettings(cocos2d::CCObject* sender) {
    SettingsHandler::handleKeyCodeSettings(this, sender);
}
// Level Info settings handler
void CommandSettingsPopup::onOpenLevelInfoSettings(cocos2d::CCObject* sender) {
    SettingsHandler::handleLevelInfoSettings(this, sender);
}

// Handbook button handler
void CommandSettingsPopup::onHandbookBtn(cocos2d::CCObject* sender) {
    auto popup = HandbookPopup::create();
    if (popup) popup->show();
}

void CommandSettingsPopup::onAddEventAction(cocos2d::CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);

    std::string eventId;
    if (btn->getUserObject()) eventId = as<CCString*>(btn->getUserObject())->getCString();

    if (!eventId.empty()) {
        if (eventId == "jump") {
            m_commandActions.push_back("jump:1");
            refreshActionsList();
        } else if (eventId == "notification") {
            m_commandActions.push_back("notification:");
            refreshActionsList();
        } else if (eventId == "keycode") {
            m_commandActions.push_back("keycode:");
            refreshActionsList();
        } else if (eventId == "profile") {
            m_commandActions.push_back("profile:");
            refreshActionsList();
        } else if (eventId == "move") {
            m_commandActions.push_back("move:1:right");
            refreshActionsList();
        } else if (eventId == "alert_popup") {
            m_commandActions.push_back("alert_popup:-:-");
            refreshActionsList();
        } else {
            m_commandActions.push_back(eventId);
            refreshActionsList();
            if (m_mainLayer) {
                auto eventScrollLayer = dynamic_cast<ScrollLayer*>(m_mainLayer->getChildByID("events-scroll"));
                if (eventScrollLayer && eventScrollLayer->m_contentLayer) {
                    auto eventContent = eventScrollLayer->m_contentLayer;
                    float eventNodeGap = 8.0f;
                    float nodeHeight = 32.f;
                    int eventCount = static_cast<int>(CommandActionEventNode::getAllEventNodes().size());
                    float minContentHeight = eventScrollLayer->getContentSize().height;
                    float neededHeight = eventCount * (nodeHeight + eventNodeGap);
                    float contentHeight = std::max(minContentHeight, neededHeight);
                    eventContent->setContentSize(CCSize(eventScrollLayer->getContentSize().width, contentHeight));
                }
            }
        }
    }
}


void CommandSettingsPopup::updateKeyCodeNextTextLabel(int actionIdx, const std::string& nextKey) {
    if (actionIdx >= 0 && actionIdx < as<int>(m_commandActions.size())) {
        m_commandActions[actionIdx] = "keycode:" + nextKey;
    }
    // Update the label in the action node
    auto children = m_actionContent->getChildren();
    if (!children || actionIdx < 0 || actionIdx >= children->count()) return;
    auto actionNode = as<CCNode*>(children->objectAtIndex(actionIdx));
    if (!actionNode) return;
    std::string keyLabelId = "keycode-action-text-label-" + std::to_string(actionIdx);
    // Only show the key part in the label
    std::string keyPart = nextKey;
    size_t pipePos = keyPart.find("|");
    if (pipePos != std::string::npos) keyPart = keyPart.substr(0, pipePos);
    std::string labelText = keyPart.empty() ? "-" : keyPart;
    if (auto keyLabel = dynamic_cast<CCLabelBMFont*>(actionNode->getChildByID(keyLabelId))) keyLabel->setString(labelText.c_str());
}


void CommandSettingsPopup::refreshActionsList() {
    if (!m_actionContent) return;

    m_actionContent->removeAllChildren();

    float actionNodeGap = 8.0f;
    float nodeHeight = 32.f;

    int actionCount = as<int>(m_commandActions.size());

    // Dynamically expand content layer height if needed
    float minContentHeight = m_actionSectionHeight;
    float neededHeight = actionCount * (nodeHeight + actionNodeGap);
    float contentHeight = std::max(minContentHeight, neededHeight);
    m_actionContent->setContentSize(CCSize(m_actionContent->getContentSize().width, contentHeight));

    float actionNodeY = contentHeight - nodeHeight / 2;
    int actionIndex = 0;
    for (int i = 0; i < actionCount; ++i) {
        std::string& actionIdRaw = m_commandActions[i];
        std::string actionIdLower = actionIdRaw;
        std::transform(actionIdLower.begin(), actionIdLower.end(), actionIdLower.begin(), ::tolower);

        // Create the action node and align to left edge of scroll content
        TwitchCommandAction actionObj(CommandActionType::Event, actionIdRaw, 0);
        auto actionNode = CommandActionEventNode::create(actionObj, CCSize(m_actionContent->getContentSize().width, nodeHeight));
        actionNode->setAnchorPoint({0, 0.5f});
        actionNode->setPosition(0, actionNodeY); // align left, vertical stack
        m_actionContent->addChild(actionNode);

        // Main label for the action node (ensure always present)
        std::string mainLabelId = "action-main-label-" + std::to_string(i);
        std::string mainLabelText = actionIdRaw;
        // Optionally, parse and prettify label text for known types
        if (actionIdLower.rfind("wait", 0) == 0) mainLabelText = "Wait";
        else if (actionIdLower.rfind("jump", 0) == 0) mainLabelText = "Jump";
        else if (actionIdLower.rfind("alert_popup", 0) == 0) mainLabelText = "Alert Popup";
        else if (actionIdLower.rfind("notification", 0) == 0) mainLabelText = "Notification";
        else if (actionIdLower.rfind("keycode", 0) == 0) mainLabelText = "Keycode";
        else if (actionIdLower.rfind("profile", 0) == 0) mainLabelText = "Profile";
        else if (actionIdLower.rfind("move", 0) == 0) mainLabelText = "Move";
        else if (actionIdLower.rfind("color_player", 0) == 0 || actionIdLower.rfind("color player", 0) == 0) mainLabelText = "Color Player";
        else if (actionIdLower.rfind("edit_camera", 0) == 0) mainLabelText = "Edit Camera";
        else if (actionIdLower.rfind("level_info", 0) == 0) mainLabelText = "Level Info";

        auto mainLabel = CCLabelBMFont::create(mainLabelText.c_str(), "bigFont.fnt");
        mainLabel->setScale(0.5f);
        mainLabel->setAnchorPoint({0, 0.5f});
        mainLabel->setAlignment(kCCTextAlignmentLeft);
        mainLabel->setPosition(20.f, 16.f);
        mainLabel->setID(mainLabelId);
        actionNode->addChild(mainLabel);

        // Add a secondary text label for extra info (if needed)
        std::string textLabelId = "action-text-label-" + std::to_string(i);
        std::string textLabelText = "";
        if (actionIdLower.rfind("notification", 0) == 0) {
            // Show notification text if present
            size_t secondColon = actionIdRaw.find(":", 13);
            if (secondColon != std::string::npos && secondColon + 1 < actionIdRaw.size())
                textLabelText = actionIdRaw.substr(secondColon + 1);
        } else if (actionIdLower.rfind("wait", 0) == 0) {
            // Show wait seconds if present
            size_t colon = actionIdRaw.find(":");
            if (colon != std::string::npos && colon + 1 < actionIdRaw.size())
                textLabelText = actionIdRaw.substr(colon + 1) + " sec";
        } else if (actionIdLower.rfind("keycode", 0) == 0) {
            size_t colon = actionIdRaw.find(":");
            if (colon != std::string::npos && colon + 1 < actionIdRaw.size())
                textLabelText = actionIdRaw.substr(colon + 1);
        }
        // Add a settings text label for the current settings values (if any)
        std::string settingsLabelId = "action-settings-label-" + std::to_string(i);
        std::string settingsLabelText = "";
        if (actionIdLower.rfind("alert_popup", 0) == 0) {
            // Format: alert_popup:title:desc
            size_t firstColon = actionIdRaw.find(":");
            size_t secondColon = actionIdRaw.find(":", firstColon + 1);
            if (firstColon != std::string::npos && secondColon != std::string::npos) {
                std::string title = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                std::string desc = actionIdRaw.substr(secondColon + 1);
                settingsLabelText = "Title: " + title + " | Desc: " + desc;
            }
        } else if (actionIdLower.rfind("notification", 0) == 0) {
            // Format: notification:iconType:text
            size_t firstColon = actionIdRaw.find(":");
            size_t secondColon = actionIdRaw.find(":", firstColon + 1);
            if (firstColon != std::string::npos && secondColon != std::string::npos) {
                std::string iconType = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                std::string notifText = actionIdRaw.substr(secondColon + 1);
                settingsLabelText = "Icon: " + iconType + " | Text: " + notifText;
            }
        } else if (actionIdLower.rfind("keycode", 0) == 0) {
            // Format: keycode:key
            size_t colon = actionIdRaw.find(":");
            if (colon != std::string::npos && colon + 1 < actionIdRaw.size()) {
                std::string key = actionIdRaw.substr(colon + 1);
                settingsLabelText = "Key: " + key;
            }
        } else if (actionIdLower.rfind("profile", 0) == 0) {
            // Format: profile:username
            size_t colon = actionIdRaw.find(":");
            if (colon != std::string::npos && colon + 1 < actionIdRaw.size()) {
                std::string user = actionIdRaw.substr(colon + 1);
                settingsLabelText = "User: " + user;
            }
        } else if (actionIdLower.rfind("move", 0) == 0) {
            // Format: move:player:direction
            size_t firstColon = actionIdRaw.find(":");
            size_t secondColon = actionIdRaw.find(":", firstColon + 1);
            if (firstColon != std::string::npos && secondColon != std::string::npos) {
                std::string player = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                std::string dir = actionIdRaw.substr(secondColon + 1);
                settingsLabelText = "Player: " + player + " | Dir: " + dir;
            }
        } else if (actionIdLower.rfind("jump", 0) == 0) {
            // Format: jump:player[:hold]
            size_t colon = actionIdRaw.find(":");
            if (colon != std::string::npos && colon + 1 < actionIdRaw.size()) {
                std::string rest = actionIdRaw.substr(colon + 1);
                settingsLabelText = "Player: " + rest;
            }
        } else if (actionIdLower.rfind("color_player", 0) == 0 || actionIdLower.rfind("color player", 0) == 0) {
            // Format: color_player:r,g,b
            size_t colon = actionIdRaw.find(":");
            if (colon != std::string::npos && colon + 1 < actionIdRaw.size()) {
                std::string rgb = actionIdRaw.substr(colon + 1);
                settingsLabelText = "RGB: " + rgb;
            }
        } else if (actionIdLower.rfind("edit_camera", 0) == 0) {
            // Format: edit_camera:params
            size_t colon = actionIdRaw.find(":");
            if (colon != std::string::npos && colon + 1 < actionIdRaw.size()) {
                std::string params = actionIdRaw.substr(colon + 1);
                settingsLabelText = params;
            }
        } else if (actionIdLower.rfind("level_info", 0) == 0) {
            // Format: level_info:params
            size_t colon = actionIdRaw.find(":");
            if (colon != std::string::npos && colon + 1 < actionIdRaw.size()) {
                std::string params = actionIdRaw.substr(colon + 1);
                settingsLabelText = params;
            }
        }
        if (!textLabelText.empty()) {
            auto textLabel = CCLabelBMFont::create(textLabelText.c_str(), "chatFont.fnt");
            textLabel->setScale(0.5f);
            textLabel->setAnchorPoint({0, 0.5f});
            textLabel->setAlignment(kCCTextAlignmentLeft);
            textLabel->setPosition(140.f, 16.f);
            textLabel->setID(textLabelId);
            actionNode->addChild(textLabel);
        }
        if (!settingsLabelText.empty()) {
            auto settingsLabel = CCLabelBMFont::create(settingsLabelText.c_str(), "chatFont.fnt");
            settingsLabel->setScale(0.5f);
            settingsLabel->setAnchorPoint({0, 0.5f});
            settingsLabel->setAlignment(kCCTextAlignmentLeft);
            settingsLabel->setPosition(140.f, 2.f);
            settingsLabel->setID(settingsLabelId);
            actionNode->addChild(settingsLabel);
        }

        // Unified settings button logic for nodes with a SettingsHandler
        std::string btnId;
        bool hasSettingsHandler = false;
        if (actionIdLower.rfind("alert_popup", 0) == 0) {
            btnId = "alert-popup-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        } else if (actionIdLower.rfind("notification", 0) == 0) {
            btnId = "notification-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        } else if (actionIdLower.rfind("keycode", 0) == 0) {
            btnId = "keycode-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        } else if (actionIdLower.rfind("profile", 0) == 0) {
            btnId = "profile-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        } else if (actionIdLower.rfind("move", 0) == 0) {
            btnId = "move-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        } else if (actionIdLower.rfind("jump", 0) == 0) {
            btnId = "jump-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        } else if (actionIdLower.rfind("color_player", 0) == 0 || actionIdLower.rfind("color player", 0) == 0) {
            btnId = "color-player-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        } else if (actionIdLower.rfind("edit_camera", 0) == 0) {
            btnId = "edit-camera-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        } else if (actionIdLower.rfind("level_info", 0) == 0) {
            btnId = "level_info-settings-btn-" + std::to_string(actionIndex);
            hasSettingsHandler = true;
        }

        if (hasSettingsHandler) {
            auto settingsSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
            settingsSprite->setScale(0.5f);
            auto settingsBtn = CCMenuItemSpriteExtra::create(settingsSprite, this, menu_selector(CommandSettingsPopup::onSettingsButtonUnified));
            settingsBtn->setID(btnId);
            settingsBtn->setUserObject(CCInteger::create(static_cast<int>(i)));
            auto settingsMenu = CCMenu::create();
            settingsMenu->addChild(settingsBtn);
            settingsMenu->setPosition(0, 0);
            float btnX = m_actionContent->getContentSize().width - 24.f;
            settingsBtn->setPosition(btnX - 40.f, 16.f);
            actionNode->addChild(settingsMenu);
        }

        // Remove button
        auto removeSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
        removeSprite->setScale(0.5f);
        float btnX = m_actionContent->getContentSize().width - 24.f;
        auto removeBtn = CCMenuItemSpriteExtra::create(
            removeSprite,
            this,
            menu_selector(CommandSettingsPopup::onRemoveAction)
        );
        removeBtn->setID("action-" + std::to_string(i) + "-remove-btn");
        removeBtn->setPosition(btnX, 16.f);
        removeBtn->setUserObject(CCInteger::create(static_cast<int>(i)));
        removeBtn->setScale(1.2f);

        // If this is a wait action, add a TextInput to the left of the button
        if (actionIdLower == "wait") {
            std::string waitInputId = "wait-delay-input-" + std::to_string(actionIndex);
            auto waitInput = TextInput::create(50, "Sec", "chatFont.fnt");
            waitInput->setPosition(btnX - 40.f, 16.f);
            waitInput->setScale(0.5f);
            waitInput->setID(waitInputId);
            // Optionally set value if available (not shown in original code)
            actionNode->addChild(waitInput);
        }

        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        menu->addChild(removeBtn);
        actionNode->addChild(menu);

        float nodeBgWidth = m_actionContent ? m_actionContent->getContentSize().width : actionNode->getContentSize().width;
        auto nodeBg = CCScale9Sprite::create("square02_001.png");
        nodeBg->setContentSize(CCSize(nodeBgWidth, actionNode->getContentSize().height));
        nodeBg->setOpacity(60);
        nodeBg->setAnchorPoint({ 0, 0 });
        nodeBg->setPosition(0, 0);
        actionNode->addChild(nodeBg, -1);

        actionNodeY -= (nodeHeight + actionNodeGap);
        actionIndex++;
    }
}



void CommandSettingsPopup::onRemoveAction(CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;

    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= static_cast<int>(m_commandActions.size())) return;

    m_commandActions.erase(m_commandActions.begin() + idx);

    refreshActionsList();
};

void CommandSettingsPopup::onCloseBtn(CCObject* sender) {
    onClose(sender);
};

std::string CommandSettingsPopup::getNotificationText() const {
    if (m_notificationInput) {
        std::string text = m_notificationInput->getString();

        // Trim whitespace
        text.erase(0, text.find_first_not_of(" \t\n\r"));
        text.erase(text.find_last_not_of(" \t\n\r") + 1);

        return text;
    };

    return "";
}

// Helper to update the color player RGB label for a given action index
void CommandSettingsPopup::updateColorPlayerLabel(int actionIdx) {
    if (actionIdx >= 0 && actionIdx < as<int>(m_commandActions.size())) {
        std::string& actionStr = m_commandActions[actionIdx];
        std::string actionStrLower = actionStr;
        std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);
        if (actionStrLower.rfind("color_player", 0) == 0 || actionStrLower.rfind("color player", 0) == 0) {
            auto children = m_actionContent->getChildren();
            if (children && actionIdx < children->count()) {
                auto actionNode = as<CCNode*>(children->objectAtIndex(actionIdx));
                if (actionNode) {
                    std::string colorLabelId = "color-player-action-text-label-" + std::to_string(actionIdx);
                    std::string rgbText = "255,255,255";
                    size_t colonPos = actionStr.find(":");
                    if (colonPos != std::string::npos && colonPos + 1 < actionStr.size()) {
                        rgbText = actionStr.substr(colonPos + 1);
                    }
                    std::string labelText = "RGB: " + rgbText;
                    if (auto colorLabel = dynamic_cast<CCLabelBMFont*>(actionNode->getChildByID(colorLabelId))) colorLabel->setString(labelText.c_str());
                }
            }
        }
    }
}

// Info button handler for event list
void CommandSettingsPopup::onEventInfoBtn(cocos2d::CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    std::string desc;
    std::string eventName = "Event";
    std::string btnId = btn ? btn->getID() : "";
    std::string eventId;

    // ID format: event-<id>-info-btn
    if (btnId.rfind("event-", 0) == 0) {
        size_t start = 6;
        size_t end = btnId.find("-info-btn");
        if (end != std::string::npos && end > start) {
            eventId = btnId.substr(start, end - start);
        };
    };

    // Find the event name from EventNodeInfo
    for (const auto& info : CommandActionEventNode::getAllEventNodes()) {
        if (info.id == eventId) {
            eventName = info.label;
            break;
        };
    };

    if (btn && btn->getUserObject()) desc = as<CCString*>(btn->getUserObject())->getCString();

    if (!desc.empty()) {
        FLAlertLayer::create(eventName.c_str(), desc, "OK")->show();
    } else {
        FLAlertLayer::create(eventName.c_str(), "No description available.", "OK")->show();
    };
};

void CommandSettingsPopup::onSave(CCObject* sender) {
    // Build up to 10 actions in order, validate all wait inputs
    std::vector<TwitchCommandAction> actionsVec;

    for (size_t idx = 0; idx < m_commandActions.size(); ++idx) {
        std::string& actionIdRaw = m_commandActions[idx];

        std::string actionId = actionIdRaw;
        std::string waitValue;
        std::string jumpPlayerValue;
        std::string alertTitle, alertDesc;
        bool isHold = false;

        if (actionIdRaw.rfind("wait:", 0) == 0) {
            actionId = "wait";
            waitValue = actionIdRaw.substr(5);
        } else if (actionIdRaw.rfind("jump:", 0) == 0) {
            actionId = "jump";

            // Parse jumpPlayerValue and hold
            std::string val = actionIdRaw.substr(5); // could be "1", "1:hold", etc.

            size_t holdPos = val.find(":hold");
            if (holdPos != std::string::npos) {
                isHold = true;
                jumpPlayerValue = val.substr(0, holdPos);
            } else {
                jumpPlayerValue = val;
            };
        } else if (actionIdRaw.rfind("alert_popup:", 0) == 0) {
            actionId = "alert_popup";
            size_t firstColon = actionIdRaw.find(":");
            size_t secondColon = actionIdRaw.find(":", firstColon + 1);
            if (firstColon != std::string::npos && secondColon != std::string::npos) {
                alertTitle = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                alertDesc = actionIdRaw.substr(secondColon + 1);
            }
        }

        if (actionId == "wait") {
            std::string inputId = "wait-delay-input-" + std::to_string(idx + 1); // match refreshActionsList index label (1-based)
            TextInput* waitInput = nullptr;

            auto children = m_actionContent->getChildren();

            if (children) {
                for (int i = 0; i < children->count(); ++i) {
                    auto node = as<CCNode*>(children->objectAtIndex(i));
                    if (node) {
                        auto inputNode = node->getChildByID("wait-delay-input-" + std::to_string(idx));
                        if (inputNode) {
                            waitInput = dynamic_cast<TextInput*>(inputNode);
                            if (waitInput) break;
                        };
                    };
                };
            };

            std::string delayStr = waitValue;
            if (waitInput) delayStr = waitInput->getString();

            if (delayStr.empty()) {
                Notification::create("Please fill in all wait delay fields!", NotificationIcon::Error)->show();
                return;
            }

            {
                bool valid = !delayStr.empty() && (delayStr.find_first_not_of("-0123456789") == std::string::npos);
                if (valid) {
                    int delay = std::stoi(delayStr);

                    actionsVec.push_back(TwitchCommandAction(CommandActionType::Wait, "wait", delay));
                    actionIdRaw = "wait:" + std::to_string(delay);
                } else {
                    Notification::create("Wait delay must be an integer!", NotificationIcon::Error)->show();
                    return;
                };
            };
        } else if (actionId == "jump") {
            // Always use the value from m_commandActions (jumpPlayerValue and isHold)
            int playerIdx = 1;
            if (!jumpPlayerValue.empty() && (jumpPlayerValue.find_first_not_of("-0123456789") == std::string::npos)) playerIdx = std::stoi(jumpPlayerValue);

            std::string jumpActionStr = "jump:" + std::to_string(playerIdx);
            if (isHold) jumpActionStr += ":hold";

            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, jumpActionStr, 0));

            actionIdRaw = jumpActionStr;
        } else if (actionId == "alert_popup") {
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, "alert_popup:" + alertTitle + ":" + alertDesc, 0));
        } else if (actionId == "kill_player") {
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, "kill_player", 0));
        } else if (actionIdRaw.rfind("keycode:", 0) == 0) {
            // Save as event with arg 'keycode:<key>'
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, actionIdRaw, 0));
        } else if (actionIdRaw.rfind("notification:", 0) == 0) {
            // Parse icon type and text: notification:<iconInt>:<text>
            int iconTypeInt = 1;
            std::string notifText;

            size_t firstColon = actionIdRaw.find(":");
            size_t secondColon = actionIdRaw.find(":", firstColon + 1);

            if (firstColon != std::string::npos && secondColon != std::string::npos) {
                iconTypeInt = std::stoi(actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1));
                notifText = actionIdRaw.substr(secondColon + 1);
            } else if (actionIdRaw.length() > 13) {
                notifText = actionIdRaw.substr(13);
            } else {
                notifText = "";
            };

            actionsVec.push_back(TwitchCommandAction(CommandActionType::Notification, std::to_string(iconTypeInt) + ":" + notifText, 0));
        } else if (actionIdRaw.rfind("move:", 0) == 0) {
            // Save as event with arg 'move:<player>:<direction>'
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, actionIdRaw, 0));
        } else {
            actionsVec.push_back(TwitchCommandAction(CommandActionType::Event, actionId, 0));
        };
    };

    // Replace m_command.actions with actionsVec (preserve order, no size limit)
    m_command.actions = actionsVec;

    // Rebuild m_commandActions from m_command.actions to ensure UI and data are in sync
    m_commandActions.clear();
    for (const auto& action : m_command.actions) {
        if (action.type == CommandActionType::Notification) {
            m_commandActions.push_back("notification:" + action.arg);
        } else if (action.type == CommandActionType::Wait) {
            m_commandActions.push_back("wait:" + std::to_string(action.index));
        } else if (action.type == CommandActionType::Event && action.arg.rfind("alert_popup:", 0) == 0) {
            m_commandActions.push_back(action.arg);
        } else if (action.type == CommandActionType::Event) {
            m_commandActions.push_back(action.arg);
        }
    }

    // Refresh the actions list to ensure notification node is visible
    refreshActionsList();

    Notification::create("Command Settings Saved!", NotificationIcon::Success)->show();

    // Save changes to the command manager
    auto commandManager = TwitchCommandManager::getInstance();
    for (auto& cmd : commandManager->getCommands()) {
        if (cmd.name == m_command.name) {
            cmd = m_command; // Replace the entire command object
            break;
        };
    };

    commandManager->saveCommands();

    // Refresh the dashboard command list if the dashboard is open
    if (auto scene = cocos2d::CCDirector::sharedDirector()->getRunningScene()) {
        if (auto dashboard = dynamic_cast<TwitchDashboard*>(scene->getChildByID("twitch-dashboard-popup"))) {
            dashboard->refreshCommandsList();
        }
    }

    onClose(nullptr);
};


void CommandSettingsPopup::updateNotificationNextTextLabel(int actionIdx, const std::string& nextText, NotificationIconType iconType) {
    if (actionIdx >= 0 && actionIdx < as<int>(m_commandActions.size())) {
        int iconTypeInt = static_cast<int>(iconType);
        m_commandActions[actionIdx] = "notification:" + std::to_string(iconTypeInt) + ":" + nextText;

        // Find the Nth notification action node (among all notification nodes)
        int notifNodeIdx = -1;
        int notifCount = 0;

        for (int i = 0; i <= actionIdx; ++i) {
            if (m_commandActions[i].rfind("notification:", 0) == 0) {
                notifNodeIdx = notifCount;
                notifCount++;
            };
        };

        // Now update the notifNodeIdx-th notification action in m_command.actions
        if (notifNodeIdx >= 0) {
            int notifIdx = 0;
            for (auto& action : m_command.actions) {
                if (action.type == CommandActionType::Notification) {
                    if (notifIdx == notifNodeIdx) {
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
    if (!children || actionIdx < 0 || actionIdx >= children->count()) return;

    auto actionNode = as<CCNode*>(children->objectAtIndex(actionIdx));
    if (!actionNode) return;

    std::string notifLabelId = "notification-action-text-label-" + std::to_string(actionIdx);
    // Show icon name and text
    std::string iconName = "Info";
    switch (iconType) {
    case NotificationIconType::None: iconName = "None"; break;
    case NotificationIconType::Info: iconName = "Info"; break;
    case NotificationIconType::Success: iconName = "Success"; break;
    case NotificationIconType::Warning: iconName = "Warning"; break;
    case NotificationIconType::Error: iconName = "Error"; break;
    case NotificationIconType::Loading: iconName = "Loading"; break;

    default: iconName = "Info"; break;
    };

    std::string labelText = iconName;
    if (!nextText.empty()) {
        labelText += ": ";
        labelText += nextText;
    };

    if (auto notifLabel = dynamic_cast<CCLabelBMFont*>(actionNode->getChildByID(notifLabelId))) notifLabel->setString(labelText.c_str());
};

// Unified settings button handler
void CommandSettingsPopup::onSettingsButtonUnified(cocos2d::CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int actionIdx = 0;
    if (btn && btn->getUserObject()) actionIdx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (actionIdx < 0 || actionIdx >= static_cast<int>(m_commandActions.size())) return;
    std::string& actionStr = m_commandActions[actionIdx];
    std::string actionStrLower = actionStr;
    std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);
    if (actionStrLower.rfind("alert_popup", 0) == 0) {
        SettingsHandler::handleAlertSettings(this, sender);
    } else if (actionStrLower.rfind("notification", 0) == 0) {
        SettingsHandler::handleNotificationSettings(this, sender);
    } else if (actionStrLower.rfind("keycode", 0) == 0) {
        SettingsHandler::handleKeyCodeSettings(this, sender);
    } else if (actionStrLower.rfind("profile", 0) == 0) {
        SettingsHandler::handleProfileSettings(this, sender);
    } else if (actionStrLower.rfind("move", 0) == 0) {
        SettingsHandler::handleMoveSettings(this, sender);
    } else if (actionStrLower.rfind("jump", 0) == 0) {
        SettingsHandler::handleJumpSettings(this, sender);
    } else if (actionStrLower.rfind("color_player", 0) == 0 || actionStrLower.rfind("color player", 0) == 0) {
        SettingsHandler::handleColorPlayerSettings(this, sender);
    } else if (actionStrLower.rfind("edit_camera", 0) == 0) {
        SettingsHandler::handleEditCameraSettings(this, sender);
    } else if (actionStrLower.rfind("level_info", 0) == 0) {
        SettingsHandler::handleLevelInfoSettings(this, sender);
    }
}

// Static create function for CommandSettingsPopup with only TwitchCommand argument
CommandSettingsPopup* CommandSettingsPopup::create(TwitchCommand command) {
    auto ret = new CommandSettingsPopup();
    if (ret && ret->initAnchored(800.f, 325.f, command)) { // 620
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
};