#include "TwitchDashboard.hpp"

#include <Geode/Geode.hpp>
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>

using namespace geode::prelude;

bool TwitchDashboard::setup() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    this->setTitle("Twitch Dashboard");
    auto layerSize = this->m_mainLayer->getContentSize();
    
    // Check if TwitchChatAPI is available
    auto api = TwitchChatAPI::get();
    if (!api) {
        log::error("TwitchChatAPI is not available in TwitchDashboard::setup");
        return false;
    }
    
    // Get the Twitch channel name for the welcome message
    std::string channelName = "Unknown";
    try {
        auto twitchMod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api");
        if (twitchMod) {
            auto savedChannel = twitchMod->getSavedValue<std::string>("twitch-channel");
            if (!savedChannel.empty()) {
                channelName = savedChannel;
            }
        }
    } catch (const std::exception& e) {
        log::error("Exception while getting Twitch channel name: {}", e.what());
    } catch (...) {
        log::error("Unknown exception while getting Twitch channel name");
    }
    
    // Create welcome label
    std::string welcomeText = "Welcome " + channelName + "!";
    m_welcomeLabel = CCLabelBMFont::create(welcomeText.c_str(), "bigFont.fnt");
    m_welcomeLabel->setPosition(layerSize.width / 2, layerSize.height / 2 + 100);
    m_welcomeLabel->setScale(0.7f);
    this->m_mainLayer->addChild(m_welcomeLabel);
    
    // Create button menu
    m_buttonMenu = CCMenu::create();
    m_buttonMenu->setPosition(layerSize.width / 2, layerSize.height / 2 + 40);
    this->m_mainLayer->addChild(m_buttonMenu);
    
    // Create "Get Token" button
    auto getTokenBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Get Chat Token", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(TwitchDashboard::onGetTokenPressed)
    );
    m_buttonMenu->addChild(getTokenBtn);
    
    // Create token display label (initially hidden)
    m_tokenLabel = CCLabelBMFont::create("", "chatFont.fnt");
    m_tokenLabel->setPosition(layerSize.width / 2, layerSize.height / 2 - 20);
    m_tokenLabel->setScale(0.4f);
    m_tokenLabel->setAlignment(kCCTextAlignmentCenter);
    m_tokenLabel->setVisible(false);
    this->m_mainLayer->addChild(m_tokenLabel);
    
    // Dashboard is now opened - no need to request token here since 
    // user already went through the full login flow
    log::debug("TwitchDashboard opened successfully for channel: {}", channelName);
    
    return true;
}

void TwitchDashboard::onClose(CCObject* sender) {
    Popup::onClose(sender);
}

void TwitchDashboard::onGetTokenPressed(CCObject* sender) {
    // Check if TwitchChatAPI is available
    auto api = TwitchChatAPI::get();
    if (!api) {
        log::error("TwitchChatAPI is not available");
        if (m_tokenLabel) {
            m_tokenLabel->setString("Error: Twitch API not available!");
            m_tokenLabel->setVisible(true);
        }
        return;
    }
    
    // Show loading message
    if (m_tokenLabel) {
        m_tokenLabel->setString("Requesting token permission...");
        m_tokenLabel->setVisible(true);
    }
    
    log::debug("Requesting Twitch chat token from user");
    
    // Request token from user
    api->getToken([this](const geode::Result<std::string>& result) {
        if (!this || !m_tokenLabel) {
            log::warn("TwitchDashboard became invalid before token callback");
            return;
        }
        
        if (result.isOk() && !result.unwrap().empty()) {
            std::string token = result.unwrap();
            log::debug("Token received successfully, length: {}", token.length());
            
            // Display the token (truncated for security)
            std::string displayToken = "Token: " + token.substr(0, 20) + "...";
            m_tokenLabel->setString(displayToken.c_str());
            m_tokenLabel->setVisible(true);
            
            // Also log the full token for debugging (remove in production)
            log::info("Full token: {}", token);
        } else if (result.isErr()) {
            // Handle error
            auto error = result.unwrapErr();
            log::error("Token request failed: {}", error);
            
            if (error.find("cancelled") != std::string::npos || 
                error.find("canceled") != std::string::npos ||
                error.find("user_denied") != std::string::npos ||
                error.find("access_denied") != std::string::npos) {
                
                m_tokenLabel->setString("Token request cancelled by user");
            } else {
                std::string errorMsg = "Token request failed: " + error;
                m_tokenLabel->setString(errorMsg.c_str());
            }
            m_tokenLabel->setVisible(true);
        } else {
            log::warn("Token request returned empty result");
            m_tokenLabel->setString("Token request failed: Empty response");
            m_tokenLabel->setVisible(true);
        }
    });
}

TwitchDashboard* TwitchDashboard::create() {
    auto ret = new TwitchDashboard();
    if (ret && ret->initAnchored(540.f, 290.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
