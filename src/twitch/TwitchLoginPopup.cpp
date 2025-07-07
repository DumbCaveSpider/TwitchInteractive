#include "TwitchLoginPopup.hpp"


#include <Geode/ui/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>


bool TwitchLoginPopup::setup() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    this->setTitle("Twitch Login");
    
    // Create login button
    auto loginBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Login to Twitch", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(TwitchLoginPopup::onLoginPressed)
    );
    
    m_loginMenu = CCMenu::create();
    m_loginMenu->addChild(loginBtn);
    
    // Center the menu within the popup's main layer
    auto layerSize = this->m_mainLayer->getContentSize();
    m_loginMenu->setPosition(layerSize.width / 2, layerSize.height / 2);
    this->m_mainLayer->addChild(m_loginMenu);
    
    // Create status label for showing login status
    m_statusLabel = CCLabelBMFont::create("Not logged in", "bigFont.fnt");
    m_statusLabel->setPosition(layerSize.width / 2, layerSize.height / 2 + 15);
    m_statusLabel->setScale(0.5f);
    m_statusLabel->setVisible(false);
    this->m_mainLayer->addChild(m_statusLabel);
    
    // Create refresh button to check status
    auto refreshBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Refresh Status", "bigFont.fnt", "GJ_button_02.png", 0.5f),
        this,
        menu_selector(TwitchLoginPopup::onRefreshPressed)
    );
    
    m_loggedInMenu = CCMenu::create();
    m_loggedInMenu->addChild(refreshBtn);
    
    m_loggedInMenu->setPosition(layerSize.width / 2, layerSize.height / 2 - 25);
    m_loggedInMenu->setVisible(false);
    this->m_mainLayer->addChild(m_loggedInMenu);
    
    return true;
}

void TwitchLoginPopup::onLoginPressed(CCObject*) {
    TwitchChatAPI::get()->promptLogin();
    
    m_loginMenu->setVisible(false);
    m_statusLabel->setVisible(true);
    m_statusLabel->setString("Logging in...");
    m_loggedInMenu->setVisible(true);
    
    auto delayAction = CCDelayTime::create(2.0f);
    auto updateAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::updateStatusCallback));
    auto sequence = CCSequence::create(delayAction, updateAction, nullptr);
    this->runAction(sequence);
}

void TwitchLoginPopup::updateStatusCallback() {
    m_statusLabel->setString("Login process completed");
}

void TwitchLoginPopup::onRefreshPressed(CCObject*) {
    m_loginMenu->setVisible(true);
    m_statusLabel->setVisible(false);
    m_loggedInMenu->setVisible(false);
}

TwitchLoginPopup* TwitchLoginPopup::create() {
    auto ret = new TwitchLoginPopup();
    if (ret && ret->initAnchored(300, 200)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
