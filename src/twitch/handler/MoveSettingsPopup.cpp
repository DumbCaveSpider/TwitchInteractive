#include "MoveSettingsPopup.hpp"

bool MoveSettingsPopup::setup() {
    setTitle("Move Settings");
    setID("move-settings-popup");

    float y = 60.f;
    float x = m_mainLayer->getContentSize().width / 2;

    // Player selection (left/right)
    auto playerLeftBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Player 1", "bigFont.fnt", "GJ_button_01.png", 0.5f),
        this,
        menu_selector(MoveSettingsPopup::onPlayerLeft)
    );
    auto playerRightBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Player 2", "bigFont.fnt", "GJ_button_01.png", 0.5f),
        this,
        menu_selector(MoveSettingsPopup::onPlayerRight)
    );
    playerLeftBtn->setPosition(x - 40, y + 20);
    playerRightBtn->setPosition(x + 40, y + 20);
    playerLeftBtn->setID("move-player-1-btn");
    playerRightBtn->setID("move-player-2-btn");

    // Direction selection (left/right)
    auto dirLeftBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Left", "bigFont.fnt", "GJ_button_05.png", 0.5f),
        this,
        menu_selector(MoveSettingsPopup::onDirectionLeft)
    );
    auto dirRightBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Right", "bigFont.fnt", "GJ_button_05.png", 0.5f),
        this,
        menu_selector(MoveSettingsPopup::onDirectionRight)
    );
    dirLeftBtn->setPosition(x - 40, y - 10);
    dirRightBtn->setPosition(x + 40, y - 10);
    dirLeftBtn->setID("move-dir-left-btn");
    dirRightBtn->setID("move-dir-right-btn");

    // Save button
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(MoveSettingsPopup::onSave)
    );
    saveBtn->setPosition(x, y - 40);
    saveBtn->setID("move-save-btn");

    auto menu = CCMenu::create();
    menu->addChild(playerLeftBtn);
    menu->addChild(playerRightBtn);
    menu->addChild(dirLeftBtn);
    menu->addChild(dirRightBtn);
    menu->addChild(saveBtn);
    menu->setPosition(0, 0);
    m_mainLayer->addChild(menu);
    return true;
}

void MoveSettingsPopup::onPlayerLeft(CCObject* sender) {
    m_player = 1;
}
void MoveSettingsPopup::onPlayerRight(CCObject* sender) {
    m_player = 2;
}
void MoveSettingsPopup::onDirectionLeft(CCObject* sender) {
    m_moveRight = false;
}
void MoveSettingsPopup::onDirectionRight(CCObject* sender) {
    m_moveRight = true;
}
void MoveSettingsPopup::onSave(CCObject* sender) {
    if (m_callback) m_callback(m_player, m_moveRight);
    onClose(sender);
}

MoveSettingsPopup* MoveSettingsPopup::create(int player, bool moveRight, std::function<void(int, bool)> callback) {
    auto ret = new MoveSettingsPopup();
    ret->m_player = player;
    ret->m_moveRight = moveRight;
    ret->m_callback = callback;
    if (ret && ret->initAnchored(250.f, 170.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
