#include "../handler/KeyCodesSettingsPopup.hpp"
#include "../handler/ProfileSettingsPopup.hpp"
#include "../handler/MoveSettingsPopup.hpp"
#include "../handler/JumpSettingsPopup.hpp"

#include "CommandSettingsPopup.hpp"
#include "CommandActionEventNode.hpp"

#include <algorithm>
#include <cocos2d.h>

using namespace cocos2d;
using namespace geode::prelude;

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
    // Align scroll backgrounds and scroll layers at the same Y position
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

    setTitle(fmt::format("!{} settings", command.name));
    setID("command-settings-popup");

    // Background for the event scroll layer
    auto eventScrollBg = CCScale9Sprite::create("square02_001.png");
    eventScrollBg->setID("events-scroll-background");
    eventScrollBg->setContentSize(scrollSize);
    eventScrollBg->setOpacity(80);
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
    actionScrollBg->setOpacity(80);
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
    commandBtnMenu->setPosition(25.f, 15.f);

    m_mainLayer->addChild(commandBtnMenu);
    return true;
}

void CommandSettingsPopup::onJumpSettings(cocos2d::CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= static_cast<int>(m_commandActions.size())) return;
    std::string actionIdRaw = m_commandActions[idx];
    // Extract current player value as int and hold state
    int jumpPlayerValue = 1;
    bool isHold = false;
    size_t colonPos = actionIdRaw.find(":");
    if (colonPos != std::string::npos && colonPos + 1 < actionIdRaw.size()) {
        std::string val = actionIdRaw.substr(colonPos + 1);
        size_t holdPos = val.find(":hold");
        if (holdPos != std::string::npos) {
            isHold = true;
            val = val.substr(0, holdPos);
        }
        if (!val.empty() && val.find_first_not_of("-0123456789") == std::string::npos) {
            jumpPlayerValue = std::stoi(val);
        }
    }

    JumpSettingsPopup::create(
        jumpPlayerValue,
        isHold,
        [this, idx](int newPlayer, bool hold) {
            if (idx >= 0 && idx < static_cast<int>(m_commandActions.size())) {
                std::string action = "jump:" + std::to_string(newPlayer);
                if (hold) action += ":hold";
                m_commandActions[idx] = action;
                refreshActionsList();
            }
        }
    )->show();
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

// Move Player settings handler
void CommandSettingsPopup::onMoveSettings(cocos2d::CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= static_cast<int>(m_commandActions.size())) return;
    std::string& actionStr = m_commandActions[idx];
    std::string actionStrLower = actionStr;
    std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);
    if (actionStrLower.rfind("move", 0) != 0) return;
    // Parse player and direction
    int player = 1;
    bool moveRight = true;
    float distance = 0.f;
    size_t firstColon = actionStr.find(":");
    size_t secondColon = actionStr.find(":", firstColon + 1);
    size_t thirdColon = actionStr.find(":", secondColon + 1);
    if (firstColon != std::string::npos && secondColon != std::string::npos) {
        std::string playerStr = actionStr.substr(firstColon + 1, secondColon - firstColon - 1);
        std::string dirStr;
        std::string distStr;
        if (thirdColon != std::string::npos) {
            dirStr = actionStr.substr(secondColon + 1, thirdColon - secondColon - 1);
            distStr = actionStr.substr(thirdColon + 1);
        } else {
            dirStr = actionStr.substr(secondColon + 1);
        }
        if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos) {
            player = std::stoi(playerStr);
        }
        moveRight = (dirStr == "right");
        if (!distStr.empty() && distStr.find_first_not_of("-0123456789.") == std::string::npos) {
            distance = std::stof(distStr);
        }
    }
    auto popup = MoveSettingsPopup::create(
        player,
        moveRight,
        [this, idx](int newPlayer, bool newMoveRight, float newDistance) {
            if (idx >= 0 && idx < static_cast<int>(m_commandActions.size())) {
                std::string dirStr = newMoveRight ? "right" : "left";
                m_commandActions[idx] = "move:" + std::to_string(newPlayer) + ":" + dirStr + ":" + std::to_string(newDistance);
                refreshActionsList();
            }
        }
    );
    if (popup) {
        popup->setDistance(distance);
        if (auto input = popup->getDistanceInput()) {
            char distBuf[32];
            snprintf(distBuf, sizeof(distBuf), "%.5f", distance);
            input->setString(distBuf);
        }
        popup->show();
    }
}

// Notification settings handler
void CommandSettingsPopup::onNotificationSettings(cocos2d::CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= as<int>(m_commandActions.size())) return;
    std::string& actionStr = m_commandActions[idx];
    std::string actionStrLower = actionStr;
    std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);
    if (actionStrLower.rfind("notification", 0) != 0) return;
    int iconTypeInt = 1;
    std::string notifText;
    size_t firstColon = actionStr.find(":");
    size_t secondColon = actionStr.find(":", firstColon + 1);
    if (firstColon != std::string::npos && secondColon != std::string::npos) {
        iconTypeInt = std::stoi(actionStr.substr(firstColon + 1, secondColon - firstColon - 1));
        notifText = actionStr.substr(secondColon + 1);
    } else if (actionStr.length() > 13) {
        notifText = actionStr.substr(13);
    } else {
        notifText = "";
    }
    NotificationSettingsPopup::create(
        notifText,
        [this, idx](const std::string& newText, NotificationIconType newIconType) {
            updateNotificationNextTextLabel(idx, newText, newIconType);
        },
        static_cast<NotificationIconType>(iconTypeInt)
    )->show();
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

// Player Profile settings handler
void CommandSettingsPopup::onProfileSettings(cocos2d::CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= static_cast<int>(m_commandActions.size())) return;
    std::string& actionStr = m_commandActions[idx];
    std::string actionStrLower = actionStr;
    std::transform(actionStrLower.begin(), actionStrLower.end(), actionStrLower.begin(), ::tolower);
    if (actionStrLower.rfind("profile", 0) != 0) return;
    // Extract current accountId
    std::string accountId = "7689052";
    size_t colonPos = actionStr.find(":");
    if (colonPos != std::string::npos && colonPos + 1 < actionStr.size()) {
        accountId = actionStr.substr(colonPos + 1);
        if (accountId.empty()) accountId = "7689052";
    }
    ProfileSettingsPopup::create(
        accountId,
        [this, idx](const std::string& newAccountId) {
            if (idx >= 0 && idx < static_cast<int>(m_commandActions.size())) {
                m_commandActions[idx] = "profile:" + newAccountId;
                refreshActionsList();
            }
        }
    )->show();
}
// KeyCode settings handler
void CommandSettingsPopup::onKeyCodeSettings(cocos2d::CCObject* sender) {
    auto btn = as<CCMenuItemSpriteExtra*>(sender);
    int idx = 0;
    if (btn->getUserObject()) idx = as<CCInteger*>(btn->getUserObject())->getValue();
    if (idx < 0 || idx >= as<int>(m_commandActions.size())) return;
    std::string& actionStr = m_commandActions[idx];
    if (actionStr.rfind("keycode", 0) != 0) return;
    std::string keyValue;
    size_t colonPos = actionStr.find(":");
    if (colonPos != std::string::npos && colonPos + 1 < actionStr.size()) {
        keyValue = actionStr.substr(colonPos + 1);
    } else {
        keyValue = "";
    }
    KeyCodesSettingsPopup::create(
        keyValue,
        [this, idx](const std::string& newKeyWithDuration) {
            updateKeyCodeNextTextLabel(idx, newKeyWithDuration);
        }
    )->show();
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

    float actionNodeY = contentHeight - 16.f;

    int actionIndex = 0;
    for (size_t i = 0; i < m_commandActions.size(); ++i) {
        const auto& actionIdRaw = m_commandActions[i];

        std::string actionId = actionIdRaw;
        std::string waitValue;
        std::string jumpPlayerValue;

        if (actionIdRaw.rfind("wait:", 0) == 0) {
            actionId = "wait";
            waitValue = actionIdRaw.substr(5);
        } else if (actionIdRaw.rfind("jump:", 0) == 0) {
            actionId = "jump";
            jumpPlayerValue = actionIdRaw.substr(5); // "1" or "2"
        };

        // Find the event label/title for this actionId
        std::string eventLabel = actionId;
        for (const auto& info : CommandActionEventNode::getAllEventNodes()) {
            if (info.id == actionId) {
                eventLabel = info.label;
                break;
            };
        };


        // mainLabel is always based on the event name label.
        std::string nodeLabel;
        std::string actionIdLower = actionId;
        std::transform(actionIdLower.begin(), actionIdLower.end(), actionIdLower.begin(), ::tolower);
        if (actionIdLower.rfind("notification", 0) == 0) {
            nodeLabel = "Notification";
        } else if (actionIdLower.rfind("keycode", 0) == 0) {
            nodeLabel = "Key Code";
        } else if (actionIdLower.rfind("profile", 0) == 0) {
            nodeLabel = "Profile";
        } else if (actionIdLower.rfind("move", 0) == 0) {
            nodeLabel = "Move Player";
        } else {
            nodeLabel = eventLabel;
        }

        // Add action index label (1-based)
        auto indexLabel = CCLabelBMFont::create(std::to_string(actionIndex + 1).c_str(), "goldFont.fnt");
        indexLabel->setScale(0.5f);
        indexLabel->setAnchorPoint({ 0, 0.5f });
        indexLabel->setAlignment(kCCTextAlignmentLeft);
        indexLabel->setPosition(8.f, 16.f);
        indexLabel->setID("action-index-label-" + std::to_string(actionIndex));

        // Only show up arrow if not the first action, down arrow if not the last
        bool showUp = (i > 0);
        bool showDown = (i < m_commandActions.size() - 1);

        // Create ActionNode with move up/down
        auto actionNode = CommandActionEventNode::createActionNode(
            nodeLabel,
            nullptr, nullptr, 0.0f, // no checkbox for now
            this,
            menu_selector(CommandSettingsPopup::onMoveActionUp),
            menu_selector(CommandSettingsPopup::onMoveActionDown),
            as<int>(i),
            showUp,
            showDown
        );
        actionNode->setPosition(0, actionNodeY - 16.f);
        // Set a unique ID for the main action node text label
        if (auto mainLabel = actionNode->getLabel()) {
            mainLabel->setID("action-main-label-" + std::to_string(actionIndex));
        }
        actionNode->addChild(indexLabel);
        m_actionContent->addChild(actionNode);

        // Always move the main action node text label up by 4.f for notification, keycode, jump, profile, and move actions
        if (
            actionIdLower.rfind("notification", 0) == 0 ||
            actionIdLower.rfind("keycode", 0) == 0 ||
            actionIdLower.rfind("jump", 0) == 0 ||
            actionIdLower.rfind("profile", 0) == 0 ||
            actionIdLower.rfind("move", 0) == 0
            ) {
            if (auto mainLabel = actionNode->getLabel()) mainLabel->setPositionY(mainLabel->getPositionY() + 4.f);
        }

        // Notification action node (case-insensitive)
        if (actionIdLower.rfind("notification", 0) == 0) {
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
            }
            std::string iconName = "Info";
            switch (iconTypeInt) {
            case 0: iconName = "None"; break;
            case 1: iconName = "Info"; break;
            case 2: iconName = "Success"; break;
            case 3: iconName = "Warning"; break;
            case 4: iconName = "Error"; break;
            case 5: iconName = "Loading"; break;
            default: iconName = "Info"; break;
            }
            std::string notifLabelText = iconName;
            if (!notifText.empty()) {
                notifLabelText += ": ";
                notifLabelText += notifText;
            }
            auto notifLabelId = "notification-action-text-label-" + std::to_string(actionIndex);
            if (!actionNode->getChildByID(notifLabelId)) {
                auto notifLabel = CCLabelBMFont::create(notifLabelText.c_str(), "chatFont.fnt");
                notifLabel->setScale(0.5f);
                notifLabel->setAnchorPoint({ 0, 0.5f });
                notifLabel->setAlignment(kCCTextAlignmentLeft);
                float labelX = 0.f;
                float labelY = 6.f;
                if (auto label = actionNode->getLabel()) {
                    labelX = label->getPositionX();
                } else {
                    labelX = 8.f;
                }
                notifLabel->setPosition(labelX, labelY);
                notifLabel->setID(notifLabelId);
                actionNode->addChild(notifLabel);
            } else {
                if (auto notifLabel = dynamic_cast<CCLabelBMFont*>(actionNode->getChildByID(notifLabelId))) notifLabel->setString(notifLabelText.c_str());
            }
            // Always create a new menu for the settings button to ensure it's clickable and not overlapped
            auto settingsSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
            settingsSprite->setScale(0.5);
            auto settingsBtn = CCMenuItemSpriteExtra::create(
                settingsSprite,
                this,
                menu_selector(CommandSettingsPopup::onNotificationSettings)
            );
            settingsBtn->setID("notification-settings-btn-" + std::to_string(actionIndex));
            settingsBtn->setUserObject(CCInteger::create(static_cast<int>(i)));
            auto settingsMenu = CCMenu::create();
            settingsMenu->addChild(settingsBtn);
            settingsMenu->setPosition(0, 0);
            actionNode->addChild(settingsMenu);
            float btnX = m_actionContent->getContentSize().width - 24.f;
            settingsBtn->setPosition(btnX - 40.f, 16.f);
        }

        // Player Profile action node (unified UI)
        if (actionIdLower.rfind("profile", 0) == 0) {
            // Format: profile:<accountId>
            std::string accountId = "7689052"; // Default account ID (aka me)
            size_t colonPos = actionIdRaw.find(":");
            if (colonPos != std::string::npos && colonPos + 1 < actionIdRaw.size()) {
                accountId = actionIdRaw.substr(colonPos + 1);
                if (accountId.empty()) accountId = "0";
            }
            // The main label is already set to eventLabel ("Player Profile") above, matching other nodes
            // Place the accountId label below the main label, using chatFont
            std::string profileLabelId = "profile-action-text-label-" + std::to_string(actionIndex);
            float labelX = 0.f;
            float labelY = 6.f;
            if (auto mainLabel = actionNode->getLabel()) {
                labelX = mainLabel->getPositionX();
            } else {
                labelX = 8.f;
            }
            std::string labelText = "Account ID: " + accountId;
            if (!actionNode->getChildByID(profileLabelId)) {
                auto profileLabel = CCLabelBMFont::create(labelText.c_str(), "chatFont.fnt");
                profileLabel->setScale(0.5f);
                profileLabel->setAnchorPoint({ 0, 0.5f });
                profileLabel->setAlignment(kCCTextAlignmentLeft);
                profileLabel->setPosition(labelX, labelY);
                profileLabel->setID(profileLabelId);
                actionNode->addChild(profileLabel);
            } else {
                if (auto profileLabel = dynamic_cast<CCLabelBMFont*>(actionNode->getChildByID(profileLabelId))) profileLabel->setString(labelText.c_str());
            }
            // Settings button for profile (same style as others)
            auto settingsSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
            settingsSprite->setScale(0.5);
            auto settingsBtn = CCMenuItemSpriteExtra::create(
                settingsSprite,
                this,
                menu_selector(CommandSettingsPopup::onProfileSettings)
            );
            settingsBtn->setID("profile-settings-btn-" + std::to_string(actionIndex));
            settingsBtn->setUserObject(CCInteger::create(static_cast<int>(i)));
            auto settingsMenu = CCMenu::create();
            settingsMenu->addChild(settingsBtn);
            settingsMenu->setPosition(0, 0);
            actionNode->addChild(settingsMenu);
            float btnX = m_actionContent->getContentSize().width - 24.f;
            settingsBtn->setPosition(btnX - 40.f, 16.f);
        }

        // KeyCode action node (unified UI with notification)
        if (actionId.rfind("keycode", 0) == 0) {
            std::string keyValue = "";
            size_t colonPos = actionIdRaw.find(":");
            if (colonPos != std::string::npos && colonPos + 1 < actionIdRaw.size()) {
                keyValue = actionIdRaw.substr(colonPos + 1);
            }
            // Parse key and hold duration (format: key|duration)
            std::string keyLabelText = "-";
            if (!keyValue.empty()) {
                size_t pipePos = keyValue.find("|");
                std::string keyPart = keyValue;
                std::string durPart;
                if (pipePos != std::string::npos) {
                    keyPart = keyValue.substr(0, pipePos);
                    durPart = keyValue.substr(pipePos + 1);
                }
                keyLabelText = keyPart.empty() ? "-" : keyPart;
                if (!durPart.empty()) {
                    keyLabelText += " (" + durPart + "s)";
                }
            }
            auto keyLabelId = "keycode-action-text-label-" + std::to_string(actionIndex);
            // Place the keycode label below the main label, like notification
            float labelX = 0.f;
            float labelY = 6.f;
            if (auto mainLabel = actionNode->getLabel()) {
                labelX = mainLabel->getPositionX();
            } else {
                labelX = 8.f;
            }
            if (!actionNode->getChildByID(keyLabelId)) {
                auto keyLabel = CCLabelBMFont::create(keyLabelText.c_str(), "chatFont.fnt");
                keyLabel->setScale(0.5f);
                keyLabel->setAnchorPoint({ 0, 0.5f });
                keyLabel->setAlignment(kCCTextAlignmentLeft);
                keyLabel->setPosition(labelX, labelY);
                keyLabel->setID(keyLabelId);
                actionNode->addChild(keyLabel);
            } else {
                if (auto keyLabel = dynamic_cast<CCLabelBMFont*>(actionNode->getChildByID(keyLabelId))) keyLabel->setString(keyLabelText.c_str());
            }
            // Always create a new menu for the settings button to ensure it's clickable and not overlapped
            auto settingsSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
            settingsSprite->setScale(0.5);
            auto settingsBtn = CCMenuItemSpriteExtra::create(
                settingsSprite,
                this,
                menu_selector(CommandSettingsPopup::onKeyCodeSettings)
            );
            settingsBtn->setID("keycode-settings-btn-" + std::to_string(actionIndex));
            settingsBtn->setUserObject(CCInteger::create(as<int>(i)));
            auto settingsMenu = CCMenu::create();
            settingsMenu->addChild(settingsBtn);
            settingsMenu->setPosition(0, 0);
            actionNode->addChild(settingsMenu);
            float btnX = m_actionContent->getContentSize().width - 24.f;
            settingsBtn->setPosition(btnX - 40.f, 16.f);
        };

        // Jump action node (unified UI with notification)
        if (actionId == "jump" && !jumpPlayerValue.empty()) {
            // Remove any ":hold" from jumpPlayerValue for display
            std::string playerValueClean = jumpPlayerValue;

            size_t holdPos = playerValueClean.find(":hold");
            if (holdPos != std::string::npos) {
                playerValueClean = playerValueClean.substr(0, holdPos);
            };

            std::string playerInfo;
            if (playerValueClean == "3") {
                playerInfo = "Both Players";
            } else {
                playerInfo = "Player " + playerValueClean;
            };

            // Check if this jump action is a hold
            bool isHold = false;
            if (m_commandActions[i].find(":hold") != std::string::npos) isHold = true;
            if (isHold) playerInfo += " (Hold)";

            auto playerLabelId = "jump-action-text-label-" + std::to_string(actionIndex);

            float labelX = 0.f;
            float labelY = 6.f;

            if (auto mainLabel = actionNode->getLabel()) {
                labelX = mainLabel->getPositionX();
            } else {
                labelX = 8.f;
            };

            if (!actionNode->getChildByID(playerLabelId)) {
                auto playerLabel = CCLabelBMFont::create(playerInfo.c_str(), "chatFont.fnt");
                playerLabel->setScale(0.5f);
                playerLabel->setAnchorPoint({ 0, 0.5f });
                playerLabel->setAlignment(kCCTextAlignmentLeft);
                playerLabel->setPosition(labelX, labelY);
                playerLabel->setID(playerLabelId);
                actionNode->addChild(playerLabel);
            } else {
                if (auto playerLabel = dynamic_cast<CCLabelBMFont*>(actionNode->getChildByID(playerLabelId))) playerLabel->setString(playerInfo.c_str());
            };

            auto settingsMenu = CCMenu::create();
            settingsMenu->setPosition(0, 0);

            auto settingsSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
            settingsSprite->setScale(0.5f);

            float btnX = m_actionContent->getContentSize().width - 24.f;

            // Always create a new menu for the settings button to ensure it's clickable and not overlapped
            auto settingsBtn = CCMenuItemSpriteExtra::create(
                settingsSprite,
                this,
                menu_selector(CommandSettingsPopup::onJumpSettings)
            );
            settingsBtn->setID("jump-settings-btn-" + std::to_string(i));
            settingsBtn->setUserObject(CCInteger::create(static_cast<int>(i)));
            settingsBtn->setPosition(btnX - 40.f, 16.f);

            settingsMenu->addChild(settingsBtn);
            actionNode->addChild(settingsMenu);
        };

        // Move Player action node (unified UI)
        if (actionIdLower.rfind("move", 0) == 0) {
            // Format: move:<player>:<direction>:<distance>
            int player = 1;
            std::string direction = "right";
            float distance = 0.f;

            size_t firstColon = actionIdRaw.find(":");
            size_t secondColon = actionIdRaw.find(":", firstColon + 1);
            size_t thirdColon = actionIdRaw.find(":", secondColon + 1);

            if (firstColon != std::string::npos && secondColon != std::string::npos) {
                std::string playerStr = actionIdRaw.substr(firstColon + 1, secondColon - firstColon - 1);
                std::string dirStr;
                std::string distStr;

                if (thirdColon != std::string::npos) {
                    dirStr = actionIdRaw.substr(secondColon + 1, thirdColon - secondColon - 1);
                    distStr = actionIdRaw.substr(thirdColon + 1);
                } else {
                    dirStr = actionIdRaw.substr(secondColon + 1);
                };

                if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos) player = std::stoi(playerStr);

                direction = dirStr;
                if (!distStr.empty() && distStr.find_first_not_of("-0123456789.") == std::string::npos) distance = std::stof(distStr);
            }
            std::string moveLabelId = "move-action-text-label-" + std::to_string(actionIndex);

            float labelX = 0.f;
            float labelY = 6.f;

            if (auto mainLabel = actionNode->getLabel()) {
                labelX = mainLabel->getPositionX();
            } else {
                labelX = 8.f;
            };

            char distBuf[32];
            snprintf(distBuf, sizeof(distBuf), "%.5f", distance);

            std::string labelText = "Player " + std::to_string(player) + ": " + (direction == "right" ? "Right" : "Left") + " (" + distBuf + ")";

            if (!actionNode->getChildByID(moveLabelId)) {
                auto moveLabel = CCLabelBMFont::create(labelText.c_str(), "chatFont.fnt");
                moveLabel->setScale(0.5f);
                moveLabel->setAnchorPoint({ 0, 0.5f });
                moveLabel->setAlignment(kCCTextAlignmentLeft);
                moveLabel->setPosition(labelX, labelY);
                moveLabel->setID(moveLabelId);

                actionNode->addChild(moveLabel);
            } else {
                if (auto moveLabel = dynamic_cast<CCLabelBMFont*>(actionNode->getChildByID(moveLabelId))) moveLabel->setString(labelText.c_str());
            };

            auto settingsMenu = CCMenu::create();
            settingsMenu->setPosition(0, 0);

            auto settingsSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
            settingsSprite->setScale(0.5);

            float btnX = m_actionContent->getContentSize().width - 24.f;

            // Settings button for move (same style as others)
            auto settingsBtn = CCMenuItemSpriteExtra::create(
                settingsSprite,
                this,
                menu_selector(CommandSettingsPopup::onMoveSettings)
            );
            settingsBtn->setID("move-settings-btn-" + std::to_string(actionIndex));
            settingsBtn->setUserObject(CCInteger::create(static_cast<int>(i)));
            settingsBtn->setPosition(btnX - 40.f, 16.f);

            settingsMenu->addChild(settingsBtn);
            actionNode->addChild(settingsMenu);
        };

        // Remove button
        auto removeSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
        removeSprite->setScale(0.5f);

        // x pos for remove btn
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
        if (actionId == "wait") {
            // Use a unique ID for each wait input based on the index
            std::string waitInputId = "wait-delay-input-" + std::to_string(actionIndex);

            auto waitInput = TextInput::create(50, "Sec", "chatFont.fnt");
            waitInput->setPosition(btnX - 40.f, 16.f);
            waitInput->setScale(0.5f);
            waitInput->setID(waitInputId);

            if (!waitValue.empty()) waitInput->setString(waitValue.c_str());

            actionNode->addChild(waitInput);
        };

        // Menu for button
        auto menu = CCMenu::create();
        menu->setPosition(0, 0);

        menu->addChild(removeBtn);
        actionNode->addChild(menu);

        // Add a background to the action node with width matching the actionScrollLayer
        float nodeBgWidth = m_actionContent ? m_actionContent->getContentSize().width : actionNode->getContentSize().width;

        auto nodeBg = CCScale9Sprite::create("square02_001.png");
        nodeBg->setContentSize(CCSize(nodeBgWidth, actionNode->getContentSize().height));
        nodeBg->setOpacity(60);
        nodeBg->setAnchorPoint({ 0, 0 });
        nodeBg->setPosition(0, 0);

        actionNode->addChild(nodeBg, -1);

        actionNodeY -= (nodeHeight + actionNodeGap);
        actionIndex++;
    };
};

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
};

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
        };

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
        } else if (action.type == CommandActionType::Event) {
            m_commandActions.push_back(action.arg);
        };
    };

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
    onClose(nullptr);
};

void CommandSettingsPopup::updateNotificationNextTextLabel(int actionIdx, const std::string& nextText) {
    // Overload: default to Info if not provided
    updateNotificationNextTextLabel(actionIdx, nextText, NotificationIconType::Info);
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

CommandSettingsPopup* CommandSettingsPopup::create(TwitchCommand command) {
    auto ret = new CommandSettingsPopup();

    if (ret && ret->initAnchored(620.f, 325.f, command)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};