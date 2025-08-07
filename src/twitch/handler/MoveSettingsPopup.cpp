#include "MoveSettingsPopup.hpp"

#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

// Define static member variables
CCMenuItemSpriteExtra* MoveSettingsPopup::playerLeftBtn = nullptr;
CCMenuItemSpriteExtra* MoveSettingsPopup::playerRightBtn = nullptr;
CCMenuItemSpriteExtra* MoveSettingsPopup::dirLeftBtn = nullptr;
CCMenuItemSpriteExtra* MoveSettingsPopup::dirRightBtn = nullptr;

TextInput* MoveSettingsPopup::distanceInput = nullptr;

bool MoveSettingsPopup::setup() {
    setTitle("Edit Move Settings");
    setID("move-settings-popup");

    float centerX = m_mainLayer->getContentSize().width / 2;
    float startY = m_mainLayer->getContentSize().height / 2 + 30.f;

    float btnGapY = 36.f;

    float rowSpacing = 24.f; // Extra spacing between player and direction rows

    m_noElasticity = true;

    // Player selection (top row, centered)
    playerLeftBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Player 1", "bigFont.fnt", m_player == 1 ? "GJ_button_01.png" : "GJ_button_04.png", 0.5f),
        this,
        menu_selector(MoveSettingsPopup::onPlayerLeft));

    playerRightBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Player 2", "bigFont.fnt", m_player == 2 ? "GJ_button_01.png" : "GJ_button_04.png", 0.5f),
        this,
        menu_selector(MoveSettingsPopup::onPlayerRight));

    float playerBtnGap = 60.f;

    playerLeftBtn->setPosition(centerX - playerBtnGap, startY);
    playerRightBtn->setPosition(centerX + playerBtnGap, startY);

    playerLeftBtn->setID("move-player-1-btn");
    playerRightBtn->setID("move-player-2-btn");

    // Place distance textbox between player and direction rows
    float distanceBoxY = startY - (btnGapY + rowSpacing) / 2.0f;

    distanceInput = TextInput::create(80, "Distance", "chatFont.fnt");
    distanceInput->setCommonFilter(CommonFilter::Float);
    distanceInput->setID("move-distance-input");
    distanceInput->setPosition(centerX, distanceBoxY);
    distanceInput->setScale(0.6f);
    distanceInput->setString(std::to_string(m_distance).c_str());

    // Add spacing between player and direction rows
    float dirRowY = startY - btnGapY - rowSpacing;

    // Direction selection (middle row, centered)
    dirLeftBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Left", "bigFont.fnt", !m_moveRight ? "GJ_button_01.png" : "GJ_button_04.png", 0.5f),
        this,
        menu_selector(MoveSettingsPopup::onDirectionLeft));

    dirRightBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Right", "bigFont.fnt", m_moveRight ? "GJ_button_01.png" : "GJ_button_04.png", 0.5f),
        this,
        menu_selector(MoveSettingsPopup::onDirectionRight));

    float dirBtnGap = 60.f;

    dirLeftBtn->setPosition(centerX - dirBtnGap, dirRowY);
    dirRightBtn->setPosition(centerX + dirBtnGap, dirRowY);

    dirLeftBtn->setID("move-dir-left-btn");
    dirRightBtn->setID("move-dir-right-btn");

    // Save button (bottom row, centered)
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(MoveSettingsPopup::onSave));
    saveBtn->setID("move-save-btn");
    saveBtn->setPosition(centerX, dirRowY - btnGapY);

    auto menu = CCMenu::create();
    menu->setPosition(0, 0);

    menu->addChild(playerLeftBtn);
    menu->addChild(playerRightBtn);
    menu->addChild(dirLeftBtn);
    menu->addChild(dirRightBtn);
    menu->addChild(saveBtn);

    m_mainLayer->addChild(menu);
    m_mainLayer->addChild(distanceInput);

    return true;
};

void MoveSettingsPopup::onPlayerLeft(CCObject* sender) {
    m_player = 1;

    // Update button states
    playerLeftBtn->setSprite(ButtonSprite::create("Player 1", "bigFont.fnt", "GJ_button_01.png", 0.5f));
    playerRightBtn->setSprite(ButtonSprite::create("Player 2", "bigFont.fnt", "GJ_button_04.png", 0.5f));
};

void MoveSettingsPopup::onPlayerRight(CCObject* sender) {
    m_player = 2;

    // Update button states
    playerLeftBtn->setSprite(ButtonSprite::create("Player 1", "bigFont.fnt", "GJ_button_04.png", 0.5f));
    playerRightBtn->setSprite(ButtonSprite::create("Player 2", "bigFont.fnt", "GJ_button_01.png", 0.5f));
};

void MoveSettingsPopup::onDirectionLeft(CCObject* sender) {
    m_moveRight = false;

    // Update button states
    dirLeftBtn->setSprite(ButtonSprite::create("Left", "bigFont.fnt", "GJ_button_01.png", 0.5f));
    dirRightBtn->setSprite(ButtonSprite::create("Right", "bigFont.fnt", "GJ_button_04.png", 0.5f));
};

void MoveSettingsPopup::onDirectionRight(CCObject* sender) {
    m_moveRight = true;

    // Update button states
    dirLeftBtn->setSprite(ButtonSprite::create("Left", "bigFont.fnt", "GJ_button_04.png", 0.5f));
    dirRightBtn->setSprite(ButtonSprite::create("Right", "bigFont.fnt", "GJ_button_01.png", 0.5f));
};

void MoveSettingsPopup::onSave(CCObject* sender) {
    // Get distance from input
    if (distanceInput) {
        std::string distStr = distanceInput->getString();

        // Remove whitespace
        distStr.erase(0, distStr.find_first_not_of(" \t\n\r"));
        distStr.erase(distStr.find_last_not_of(" \t\n\r") + 1);

        if (distStr == "${arg}") {
            // Accept as-is, but do not set m_distance (let backend handle it)
        } else if (!distStr.empty() && distStr.find_first_not_of("-0123456789.") == std::string::npos) {
            m_distance = numFromString<float>(distStr).unwrapOrDefault();
        };

        // If not valid, ignore and keep previous m_distance
    };

    if (m_callback)
        m_callback(m_player, m_moveRight, m_distance);
    onClose(sender);
};

MoveSettingsPopup* MoveSettingsPopup::create(int player, bool moveRight, std::function<void(int, bool, float)> callback) {
    auto ret = new MoveSettingsPopup();

    ret->m_player = player;
    ret->m_moveRight = moveRight;
    ret->m_callback = callback;

    if (ret && ret->initAnchored(250.f, 200.f)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};