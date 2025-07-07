#include "TwitchLoginPopup.hpp"
#include "TwitchDashboard.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>
#include <memory>


bool TwitchLoginPopup::setup() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    this->setTitle("Twitch Connection");
    
    // Create login button
    auto loginBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Connect to Twitch", "bigFont.fnt", "GJ_button_01.png", 0.6f),
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
    m_statusLabel = CCLabelBMFont::create("Ready to connect", "bigFont.fnt");
    m_statusLabel->setPosition(layerSize.width / 2, layerSize.height / 2 + 15);
    m_statusLabel->setScale(0.5f);
    m_statusLabel->setVisible(false);
    this->m_mainLayer->addChild(m_statusLabel);
    
    // Create empty menu for logged in state (no refresh button)
    m_loggedInMenu = CCMenu::create();
    m_loggedInMenu->setPosition(layerSize.width / 2, layerSize.height / 2 - 25);
    m_loggedInMenu->setVisible(false);
    this->m_mainLayer->addChild(m_loggedInMenu);
    
    return true;
}

void TwitchLoginPopup::onLoginPressed(CCObject*) {
    // Reset retry counter for new login attempt
    m_tokenCheckRetries = 0;
    
    // Check if TwitchChatAPI is available
    auto api = TwitchChatAPI::get();
    if (!api) {
        log::error("TwitchChatAPI is not available");
        m_statusLabel->setVisible(true);
        m_statusLabel->setString("Twitch API not available!");
        return;
    }
    
    // Update UI to show checking state
    m_loginMenu->setVisible(false);
    m_statusLabel->setVisible(true);
    m_statusLabel->setString("Checking connection status...");
    m_loggedInMenu->setVisible(true);
    
    log::debug("Starting Twitch connection check");
    
    try {
        // Register callback for when connection is established (for new logins)
        auto validityFlag = m_validityFlag; // Capture the shared_ptr by value
        api->registerOnConnectedCallback([this, validityFlag]() {
            // Check if this object is still valid using the shared validity flag
            if (!validityFlag || !*validityFlag) {
                log::warn("TwitchLoginPopup was destroyed before authentication callback executed");
                return;
            }
            
            // Additional safety check
            if (!this || !m_isActive || !this->m_statusLabel) {
                log::warn("TwitchLoginPopup became invalid or inactive before authentication callback");
                return;
            }
            
            log::debug("Twitch account authentication completed, now requesting token access");
            m_statusLabel->setString("Authenticated...");
            
            // Now that user is authenticated, request token access as a separate step
            auto delayAction = CCDelayTime::create(1.0f);
            auto requestTokenAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::requestTokenAccess));
            auto sequence = CCSequence::create(delayAction, requestTokenAction, nullptr);
            
            // Final safety check before requesting token
            if (validityFlag && *validityFlag && this && m_isActive && this->m_statusLabel) {
                this->runAction(sequence);
            } else {
                log::warn("TwitchLoginPopup became invalid, cannot request token access");
            }
        });
        
        // Prompt login without forcing (this should not prompt if already logged in)
        api->promptLogin(false);
        
        // Set up a timeout to check if we're already connected but callback wasn't triggered
        auto timeoutAction = CCDelayTime::create(15.0f); // Wait 5 seconds (increased from 2)
        auto checkConnectionAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::checkExistingConnection));
        auto timeoutSequence = CCSequence::create(timeoutAction, checkConnectionAction, nullptr);
        
        // Schedule timeout check with a unique tag
        this->stopActionByTag(999); // Stop any previous timeout
        timeoutSequence->setTag(999);
        this->runAction(timeoutSequence);
        
    } catch (const std::exception& e) {
        log::error("Exception during connection check: {}", e.what());
        m_statusLabel->setString("Connection error occurred!");
        resetToLogin();
    } catch (...) {
        log::error("Unknown exception during connection check");
        m_statusLabel->setString("Connection error occurred!");
        resetToLogin();
    }
}

void TwitchLoginPopup::checkExistingConnection() {
    // This method is called after a timeout to check if user is already connected
    // but the callback wasn't triggered (e.g., already authenticated)
    
    // Check if we're still in the "checking" state (not already processed)
    if (!m_statusLabel || !m_statusLabel->isVisible()) {
        log::debug("Connection check timeout but UI already processed");
        return;
    }
    
    // Check if the status label still shows "Checking connection status..."
    std::string currentStatus = m_statusLabel->getString();
    if (currentStatus != "Checking connection status... (timeout reached)") {
        log::debug("Connection check timeout but status already changed to: {}", currentStatus);
        return;
    }
    
    log::debug("Connection check timeout reached. Checking again if user is authenticated.");
    
    // Stop the timeout action to prevent multiple calls
    this->stopActionByTag(999);
    
    // Double check if the status has changed or not after timeout
    m_statusLabel->setString("Checking connection status...");
    
    // Proceed directly to token request
    auto validityFlag = m_validityFlag;
    auto delayAction = CCDelayTime::create(1.0f);
    auto requestTokenAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::requestTokenAccess));
    auto sequence = CCSequence::create(delayAction, requestTokenAction, nullptr);
    
    // Final safety check before requesting token
    if (validityFlag && *validityFlag && this && m_isActive && this->m_statusLabel) {
        this->runAction(sequence);
    } else {
        log::warn("TwitchLoginPopup became invalid during connection check");
        resetToLogin();
    }
}

void TwitchLoginPopup::openDashboard() {
    // Close this popup and open the dashboard
    auto dashboard = TwitchDashboard::create();
    dashboard->show();
    this->keyBackClicked(); // Close the login popup
}

void TwitchLoginPopup::resetToLogin() {
    // Stop any pending timeout actions
    this->stopActionByTag(999);
    
    m_loginMenu->setVisible(true);
    m_statusLabel->setVisible(false);
    m_loggedInMenu->setVisible(false);
    m_tokenCheckRetries = 0; // Reset retry counter
}

void TwitchLoginPopup::requestTokenAccess() {
    // Stop any pending timeout checks since we're now proceeding with token request
    this->stopActionByTag(999);
    
    // Check if Twitch channel is configured first (before API checks)
    if (!checkTwitchChannelExists()) {
        log::error("Twitch channel is not configured");
        m_statusLabel->setString("Twitch channel not configured!\nPlease set up your channel first.");
        
        // Reset to login state after showing the message
        auto delayAction = CCDelayTime::create(3.0f);
        auto resetAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::resetToLogin));
        auto sequence = CCSequence::create(delayAction, resetAction, nullptr);
        this->runAction(sequence);
        return;
    }
    
    // Check if TwitchChatAPI is available
    auto api = TwitchChatAPI::get();
    if (!api) {
        log::error("TwitchChatAPI is not available in requestTokenAccess");
        m_statusLabel->setString("API error!");
        resetToLogin();
        return;
    }
    
    m_statusLabel->setString("Requesting chat token permission...");
    
    // Request token permission from authenticated user
    // Note: This will always show a popup asking the user to grant token access
    // even if they've already granted it before, because the API doesn't provide
    // a way to silently check for existing token permissions
    auto validityFlag = m_validityFlag; // Capture the shared_ptr by value
    api->getToken([this, validityFlag](const geode::Result<std::string>& result) {
        // Check if this object is still valid using the shared validity flag
        if (!validityFlag || !*validityFlag) {
            log::warn("TwitchLoginPopup was destroyed before token callback executed");
            return;
        }
        
        // Additional safety check
        if (!this || !m_isActive || !this->m_statusLabel) {
            log::warn("TwitchLoginPopup became invalid before token callback");
            return;
        }
        
        if (result.isOk() && !result.unwrap().empty()) {
            m_statusLabel->setString("Chat Token Permission granted!\nOpening dashboard!");
            
            // Log the successful token grant
            log::debug("User granted chat token access, token received");
            
            // Create and show the dashboard after a short delay
            auto delayAction = CCDelayTime::create(1.5f);
            auto openDashboardAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::openDashboard));
            auto sequence = CCSequence::create(delayAction, openDashboardAction, nullptr);
            
            // Check if this object is still valid and active before running action
            if (validityFlag && *validityFlag && this && m_isActive && this->m_statusLabel) {
                this->runAction(sequence);
            } else {
                log::warn("TwitchLoginPopup became invalid or inactive, cannot run dashboard action");
            }
        } else if (result.isErr()) {
            // Check if the error indicates user cancellation
            auto error = result.unwrapErr();
            if (error.find("cancelled") != std::string::npos || 
                error.find("canceled") != std::string::npos ||
                error.find("user_denied") != std::string::npos ||
                error.find("access_denied") != std::string::npos) {
                
                log::info("User cancelled the Twitch authentication popup");
                m_statusLabel->setString("Authentication cancelled by user.");
                
                // Reset to login state after showing the message briefly
                auto delayAction = CCDelayTime::create(2.0f);
                auto resetAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::resetToLogin));
                auto sequence = CCSequence::create(delayAction, resetAction, nullptr);
                
                // Check if this object is still valid and active before running action
                if (validityFlag && *validityFlag && this && m_isActive && this->m_statusLabel) {
                    this->runAction(sequence);
                } else {
                    log::warn("TwitchLoginPopup became invalid or inactive, cannot run reset action");
                }
            } else {
                log::error("Token request failed with error: {}", error);
                m_statusLabel->setString("Connection failed. Please try again.");
                
                // Reset to login state after showing the message briefly
                auto delayAction = CCDelayTime::create(3.0f);
                auto resetAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::resetToLogin));
                auto sequence = CCSequence::create(delayAction, resetAction, nullptr);
                
                // Check if this object is still valid and active before running action
                if (validityFlag && *validityFlag && this && m_isActive && this->m_statusLabel) {
                    this->runAction(sequence);
                } else {
                    log::warn("TwitchLoginPopup became invalid or inactive, cannot run reset action");
                }
            }
        } else {
            log::warn("User denied chat token access or token request failed");
            m_statusLabel->setString("Twitch Chat Token permission denied.");
            
            // Reset to login state after showing the message briefly
            auto delayAction = CCDelayTime::create(4.0f);
            auto resetAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::resetToLogin));
            auto sequence = CCSequence::create(delayAction, resetAction, nullptr);
            
            // Check if this object is still valid and active before running action
            if (validityFlag && *validityFlag && this && m_isActive && this->m_statusLabel) {
                this->runAction(sequence);
            } else {
                log::warn("TwitchLoginPopup became invalid or inactive, cannot run reset action");
            }
        }
    });
}

void TwitchLoginPopup::onAuthenticationCompleted() {
    // Check if this object is still valid and active
    if (!m_isActive || !this->m_statusLabel) {
        log::warn("TwitchLoginPopup became invalid before authentication completed callback");
        return;
    }
    
    log::debug("Twitch account authentication completed, now requesting token access");
    m_statusLabel->setString("Authenticated...");
    
    // Now that user is authenticated, request token access as a separate step
    auto delayAction = CCDelayTime::create(1.0f);
    auto requestTokenAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::requestTokenAccess));
    auto sequence = CCSequence::create(delayAction, requestTokenAction, nullptr);
    
    // Final safety check before requesting token
    if (m_isActive && this->m_statusLabel) {
        this->runAction(sequence);
    } else {
        log::warn("TwitchLoginPopup became invalid, cannot request token access");
    }
}

void TwitchLoginPopup::onAuthenticationTimeout() {
    // Check if this object is still valid and active
    if (!m_isActive || !this->m_statusLabel) {
        log::warn("TwitchLoginPopup became invalid before authentication timeout callback");
        return;
    }
    
    log::debug("Authentication timeout reached - checking if user is already authenticated");
    
    // Check if Twitch channel is configured first
    if (!checkTwitchChannelExists()) {
        log::error("Twitch channel is not configured during timeout check");
        m_statusLabel->setString("Twitch channel not configured!\nPlease set up your channel first.");
        
        // Reset to login state after showing the message
        auto delayAction = CCDelayTime::create(3.0f);
        auto resetAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::resetToLogin));
        auto sequence = CCSequence::create(delayAction, resetAction, nullptr);
        
        if (m_isActive && this->m_statusLabel) {
            this->runAction(sequence);
        }
        return;
    }
    
    // Check if TwitchChatAPI is available
    auto api = TwitchChatAPI::get();
    if (!api) {
        log::error("TwitchChatAPI is not available during timeout check");
        m_statusLabel->setString("API error!");
        resetToLogin();
        return;
    }
    
    // Try to get an existing token to see if user is already authenticated
    auto validityFlag = m_validityFlag; // Capture the shared_ptr by value
    api->getToken([this, validityFlag](const geode::Result<std::string>& result) {
        // Check if this object is still valid using the shared validity flag
        if (!validityFlag || !*validityFlag) {
            log::warn("TwitchLoginPopup was destroyed before timeout token callback executed");
            return;
        }
        
        // Additional safety check
        if (!this || !m_isActive || !this->m_statusLabel) {
            log::warn("TwitchLoginPopup became invalid before timeout token callback");
            return;
        }
        
        if (result.isOk() && !result.unwrap().empty()) {
            // User already has a token and channel is already validated, proceed directly to dashboard
            log::debug("User already authenticated with existing token and channel is configured");
            m_statusLabel->setString("Already authenticated...");
            
            auto delayAction = CCDelayTime::create(1.0f);
            auto openDashboardAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::openDashboard));
            auto sequence = CCSequence::create(delayAction, openDashboardAction, nullptr);
            
            if (validityFlag && *validityFlag && this && m_isActive && this->m_statusLabel) {
                this->runAction(sequence);
            }
        } else if (result.isErr()) {
            // Check if the error indicates user cancellation
            auto error = result.unwrapErr();
            if (error.find("cancelled") != std::string::npos || 
                error.find("canceled") != std::string::npos ||
                error.find("user_denied") != std::string::npos ||
                error.find("access_denied") != std::string::npos) {
                
                log::info("User cancelled the Twitch authentication during timeout check");
                m_statusLabel->setString("Authentication cancelled by user.");
                
                // Reset to login state after showing the message briefly
                auto delayAction = CCDelayTime::create(2.0f);
                auto resetAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::resetToLogin));
                auto sequence = CCSequence::create(delayAction, resetAction, nullptr);
                
                if (validityFlag && *validityFlag && this && m_isActive && this->m_statusLabel) {
                    this->runAction(sequence);
                }
            } else {
                // No existing token, prompt user for permission
                log::debug("No existing token found, requesting chat token permission from user");
                this->requestTokenAccess();
            }
        } else {
            // No existing token, prompt user for permission
            log::debug("Requesting chat token permission from user");
            this->requestTokenAccess();
        }
    });
}

bool TwitchLoginPopup::checkTwitchChannelExists() {
    try {
        // Get the TwitchChatAPI mod
        auto twitchMod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api");
        if (!twitchMod) {
            log::error("TwitchChatAPI mod not found");
            return false;
        }
        
        // Get the saved Twitch channel value
        auto channelName = twitchMod->getSavedValue<std::string>("twitch-channel");
        
        // Check if the channel name exists and is not empty
        if (channelName.empty()) {
            log::debug("Twitch channel is not configured or is empty");
            return false;
        }
        
        log::debug("Twitch channel found: {}", channelName);
        return true;
        
    } catch (const std::exception& e) {
        log::error("Exception while checking Twitch channel: {}", e.what());
        return false;
    } catch (...) {
        log::error("Unknown exception while checking Twitch channel");
        return false;
    }
}

TwitchLoginPopup::~TwitchLoginPopup() {
    m_isActive = false;
    if (m_validityFlag) {
        *m_validityFlag = false;
    }
    log::debug("TwitchLoginPopup destructor called");
}

TwitchLoginPopup* TwitchLoginPopup::create() {
    auto ret = new TwitchLoginPopup();
    if (ret && ret->initAnchored(300.f, 200.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
