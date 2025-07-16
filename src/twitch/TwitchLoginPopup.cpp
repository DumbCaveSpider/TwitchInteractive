#include "TwitchLoginPopup.hpp"

#include "TwitchDashboard.hpp"

#include <memory>
#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>

#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>

bool TwitchLoginPopup::setup()
{
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    setTitle("Twitch Connection");
    setID("twitch-login-popup"_spr);

    // Set IDs for the login popup elements
    m_mainLayer->setID("twitch-login-main-layer");

    // Get channel name first to determine button text
    std::string channelName = "";

    auto twitchMod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api");
    if (twitchMod)
    {
        auto savedChannel = twitchMod->getSavedValue<std::string>("twitch-channel");
        if (!savedChannel.empty())
            channelName = savedChannel;
    }
    else
    {
        log::error("TwitchChatAPI mod not found while getting Twitch channel name");
    };

    // Create login button with appropriate text
    std::string buttonText = channelName.empty() ? "Connect to Twitch" : "Open Dashboard";

    auto loginBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create(buttonText.c_str(), "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(TwitchLoginPopup::onLoginPressed));
    loginBtn->setID("twitch-login-main-button");

    // Center the menu within the popup's main layer
    auto layerSize = m_mainLayer->getContentSize();

    m_loginMenu = CCMenu::create();
    m_loginMenu->setID("twitch-login-menu");
    m_loginMenu->addChild(loginBtn);
    m_loginMenu->setPosition(layerSize.width / 2, layerSize.height / 2);

    m_mainLayer->addChild(m_loginMenu);

    if (!channelName.empty())
    {
        // Create a label to show the authenticated channel
        auto userLabel = CCLabelBMFont::create(("Login as: " + channelName).c_str(), "bigFont.fnt");
        userLabel->setPosition(layerSize.width / 2, layerSize.height / 2 + 50);
        userLabel->setScale(0.4f);
        userLabel->setAlignment(kCCTextAlignmentCenter);
        userLabel->setID("twitch-login-user-label");

        m_mainLayer->addChild(userLabel);
    };

    // Create status label for showing login status
    m_statusLabel = CCLabelBMFont::create("Ready to connect", "bigFont.fnt");
    m_statusLabel->setPosition(layerSize.width / 2, layerSize.height / 2);
    m_statusLabel->setScale(0.5f);
    m_statusLabel->setAlignment(kCCTextAlignmentCenter);
    m_statusLabel->setVisible(false);
    m_statusLabel->setID("twitch-login-status-label");

    m_mainLayer->addChild(m_statusLabel);

    // Create empty menu for logged in state (no refresh button)
    m_loggedInMenu = CCMenu::create();
    m_loggedInMenu->setPosition(layerSize.width / 2, layerSize.height / 2 - 25);
    m_loggedInMenu->setVisible(false);
    m_loggedInMenu->setID("twitch-login-logged-in-menu");

    m_mainLayer->addChild(m_loggedInMenu);

    return true;
};

void TwitchLoginPopup::onLoginPressed(CCObject *)
{
    // Check if TwitchChatAPI is available
    auto api = TwitchChatAPI::get();
    if (!api)
    {
        log::error("TwitchChatAPI is not available");

        m_statusLabel->setVisible(true);
        m_statusLabel->setString("Twitch API not available!");

        return;
    };

    // Update UI to show checking state
    m_loginMenu->setVisible(false);
    m_statusLabel->setVisible(true);
    m_statusLabel->setString("Checking connection status...");
    m_loggedInMenu->setVisible(true);

    log::debug("Starting Twitch connection check");

    // Check if Twitch channel is configured - if so, proceed directly to dashboard
    if (checkTwitchChannelExists())
    {
        log::debug("Channel name exists, proceeding directly to dashboard");
        m_statusLabel->setString("Opening dashboard...");

        // Proceed directly to dashboard after a short delay
        auto delayAction = CCDelayTime::create(1.0f);
        auto openDashboardAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::openDashboard));
        auto sequence = CCSequence::create(delayAction, openDashboardAction, nullptr);

        runAction(sequence);
        return;
    };

    // Register callback for when connection is established (for new logins)
    auto validityFlag = m_validityFlag; // Capture the shared_ptr by value

    api->registerOnConnectedCallback([this, validityFlag]()
                                     {
        // Check if this object is still valid using the shared validity flag
        if (!validityFlag || !*validityFlag) {
            log::warn("TwitchLoginPopup was destroyed before authentication callback executed");
            return;
        };

        // Additional safety check
        if (!m_isActive || !m_statusLabel) {
            log::warn("TwitchLoginPopup became invalid or inactive before authentication callback");
            return;
        };

        log::debug("Twitch account authentication completed, checking channel configuration");

        // Check if Twitch channel is configured before proceeding to dashboard
        if (!checkTwitchChannelExists()) {
            log::error("Twitch channel is not configured");
            m_statusLabel->setString("Twitch channel not configured!\nRetrying authentication...");

            // Wait a moment then retry the authentication process
            auto delayAction = CCDelayTime::create(3.0f);
            auto retryAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::retryAuthenticationProcess));
            auto sequence = CCSequence::create(delayAction, retryAction, nullptr);

            if (validityFlag && *validityFlag && m_isActive && m_statusLabel) runAction(sequence);

            return;
        };

        m_statusLabel->setString("Authenticated!\nOpening dashboard...");

        // Now that user is authenticated and channel is configured, proceed to dashboard
        auto delayAction = CCDelayTime::create(3.0f);
        auto openDashboardAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::openDashboard));
        auto sequence = CCSequence::create(delayAction, openDashboardAction, nullptr);

        // Final safety check before opening dashboard
        if (validityFlag && *validityFlag && m_isActive && m_statusLabel) {
            runAction(sequence);
        } else {
            log::warn("TwitchLoginPopup became invalid, cannot open dashboard");
        }; });

    // Prompt login without forcing (this should not prompt if already logged in)
    api->promptLogin(false);

    // Set up a timeout to check if we're already connected but callback wasn't triggered
    auto timeoutAction = CCDelayTime::create(5.0f); // Wait 5 seconds
    auto checkConnectionAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::checkExistingConnection));
    auto timeoutSequence = CCSequence::create(timeoutAction, checkConnectionAction, nullptr);

    // Schedule timeout check with a unique tag
    stopActionByTag(999); // Stop any previous timeout
    timeoutSequence->setTag(999);

    runAction(timeoutSequence);
};

void TwitchLoginPopup::checkExistingConnection()
{
    // This method is called after a timeout to check if user is already connected
    // but the callback wasn't triggered (e.g., already authenticated)

    // Check if we're still in the "checking" state (not already processed)
    if (!m_statusLabel || !m_statusLabel->isVisible())
    {
        log::debug("Connection check timeout but UI already processed");
        return;
    };

    // Check if the status label still shows "Checking connection status..."
    std::string currentStatus = m_statusLabel->getString();

    if (currentStatus != "Checking connection status...")
    {
        log::debug("Connection check timeout but status already changed to: {}", currentStatus);
        return;
    };

    log::debug("Connection check timeout reached, checking channel configuration");

    // Stop the timeout action to prevent multiple calls
    stopActionByTag(999);

    // Check if Twitch channel is configured before proceeding to dashboard
    if (!checkTwitchChannelExists())
    {
        log::error("Twitch channel is not configured during timeout check");
        m_statusLabel->setString("Twitch channel not configured!\nRetrying authentication...");

        // Wait a moment then retry the authentication process
        auto delayAction = CCDelayTime::create(2.0f);
        auto retryAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::retryAuthenticationProcess));
        auto sequence = CCSequence::create(delayAction, retryAction, nullptr);

        if (m_isActive && m_statusLabel)
            runAction(sequence);

        return;
    };

    // Update status and loop back to authorization check
    m_statusLabel->setString("Timeout reached.\nRetrying authorization...");

    // Check if TwitchChatAPI is available
    auto api = TwitchChatAPI::get();
    if (!api)
    {
        log::error("TwitchChatAPI is not available during timeout retry");

        m_statusLabel->setString("API error!");
        resetToLogin();

        return;
    };

    // Restart the authorization process
    log::debug("Restarting Twitch authorization check after timeout");

    // Register callback for when connection is established (for new logins)
    auto validityFlag = m_validityFlag; // Capture the shared_ptr by value
    api->registerOnConnectedCallback([this, validityFlag]()
                                     {
        // Check if this object is still valid using the shared validity flag
        if (!validityFlag || !*validityFlag) {
            log::warn("TwitchLoginPopup was destroyed before authentication callback executed");
            return;
        };

        // Additional safety check
        if (!m_isActive || !m_statusLabel) {
            log::warn("TwitchLoginPopup became invalid or inactive before authentication callback");
            return;
        };

        log::debug("Twitch account authentication completed after timeout retry");

        // Check if Twitch channel is configured before proceeding to dashboard
        if (!checkTwitchChannelExists()) {
            log::error("Twitch channel is not configured");
            m_statusLabel->setString("Twitch channel not configured!\nRetrying authentication...");

            // Wait a moment then retry the authentication process
            auto delayAction = CCDelayTime::create(2.0f);
            auto retryAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::retryAuthenticationProcess));
            auto sequence = CCSequence::create(delayAction, retryAction, nullptr);

            if (validityFlag && *validityFlag && m_isActive && m_statusLabel) runAction(sequence);

            return;
        };

        m_statusLabel->setString("Authenticated!\nOpening dashboard...");

        // Now that user is authenticated and channel is configured, proceed to dashboard
        auto delayAction = CCDelayTime::create(1.0f);
        auto openDashboardAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::openDashboard));
        auto sequence = CCSequence::create(delayAction, openDashboardAction, nullptr);

        // Final safety check before opening dashboard
        if (validityFlag && *validityFlag && m_isActive && m_statusLabel) {
            runAction(sequence);
        } else {
            log::warn("TwitchLoginPopup became invalid, cannot open dashboard");
        }; });

    // Prompt login without forcing (this should not prompt if already logged in)
    api->promptLogin(false);

    // Set up another timeout to check if we're already connected but callback wasn't triggered
    auto timeoutAction = CCDelayTime::create(5.0f); // Wait 5 seconds
    auto checkConnectionAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::checkExistingConnection));
    auto timeoutSequence = CCSequence::create(timeoutAction, checkConnectionAction, nullptr);

    // Schedule timeout check with a unique tag
    stopActionByTag(999); // Stop any previous timeout
    timeoutSequence->setTag(999);
    runAction(timeoutSequence);
};

void TwitchLoginPopup::openDashboard()
{
    // Close this popup and open the dashboard
    auto dashboard = TwitchDashboard::create();
    dashboard->show();
    keyBackClicked(); // Close the login popup
};

void TwitchLoginPopup::resetToLogin()
{
    // Stop any pending timeout actions
    stopActionByTag(999);

    m_loginMenu->setVisible(true);
    m_statusLabel->setVisible(false);
    m_loggedInMenu->setVisible(false);
};

void TwitchLoginPopup::onAuthenticationCompleted()
{
    // Check if this object is still valid and active
    if (!m_isActive || !m_statusLabel)
    {
        log::warn("TwitchLoginPopup became invalid before authentication completed callback");
        return;
    };

    log::debug("Twitch account authentication completed, checking channel configuration");

    // Check if Twitch channel is configured before proceeding to dashboard
    if (!checkTwitchChannelExists())
    {
        log::error("Twitch channel is not configured");
        m_statusLabel->setString("Twitch channel not configured!\nRetrying authentication...");

        // Wait a moment then retry the authentication process
        auto delayAction = CCDelayTime::create(2.0f);
        auto retryAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::retryAuthenticationProcess));
        auto sequence = CCSequence::create(delayAction, retryAction, nullptr);

        if (m_isActive && m_statusLabel)
            runAction(sequence);

        return;
    };

    m_statusLabel->setString("Authenticated!\nOpening dashboard...");

    // Now that user is authenticated, proceed directly to dashboard
    auto delayAction = CCDelayTime::create(1.0f);
    auto openDashboardAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::openDashboard));
    auto sequence = CCSequence::create(delayAction, openDashboardAction, nullptr);

    // Final safety check before opening dashboard
    if (m_isActive && m_statusLabel)
    {
        runAction(sequence);
    }
    else
    {
        log::warn("TwitchLoginPopup became invalid, cannot open dashboard");
    };
};

void TwitchLoginPopup::onAuthenticationTimeout()
{
    // Check if this object is still valid and active
    if (!m_isActive || !m_statusLabel)
    {
        log::warn("TwitchLoginPopup became invalid before authentication timeout callback");
        return;
    };

    log::debug("Authentication timeout reached - assuming user is already authenticated");

    // Check if Twitch channel is configured first
    if (!checkTwitchChannelExists())
    {
        log::error("Twitch channel is not configured during timeout check");
        m_statusLabel->setString("Twitch channel not configured!\nRetrying authentication...");

        // Wait a moment then retry the authentication process
        auto delayAction = CCDelayTime::create(2.0f);
        auto retryAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::retryAuthenticationProcess));
        auto sequence = CCSequence::create(delayAction, retryAction, nullptr);

        if (m_isActive && m_statusLabel)
            runAction(sequence);

        return;
    };

    // User is authenticated and channel is configured, proceed directly to dashboard
    log::debug("User already authenticated and channel is configured, proceeding to dashboard");
    m_statusLabel->setString("Already authenticated!\nOpening dashboard...");

    auto delayAction = CCDelayTime::create(1.0f);
    auto openDashboardAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::openDashboard));
    auto sequence = CCSequence::create(delayAction, openDashboardAction, nullptr);

    if (m_isActive && m_statusLabel)
        runAction(sequence);
};

bool TwitchLoginPopup::checkTwitchChannelExists()
{
    // Get the TwitchChatAPI mod
    auto twitchMod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api");
    if (!twitchMod)
    {
        log::error("TwitchChatAPI mod not found");
        return false;
    };

    // Get the saved Twitch channel value
    auto channelName = twitchMod->getSavedValue<std::string>("twitch-channel");

    // Check if the channel name exists and is not empty
    if (channelName.empty())
    {
        log::debug("Twitch channel is not configured or is empty");
        return false;
    };

    log::debug("Twitch channel found: {}", channelName);
    return true;
};

void TwitchLoginPopup::retryAuthenticationProcess()
{
    // Check if this object is still valid and active
    if (!m_isActive || !m_statusLabel)
    {
        log::warn("TwitchLoginPopup became invalid before retry authentication");
        return;
    };

    log::debug("Retrying authentication process due to missing channel configuration");

    // Reset to "checking connection status" state
    m_statusLabel->setString("Checking connection status...");

    // Check if TwitchChatAPI is available
    auto api = TwitchChatAPI::get();
    if (!api)
    {
        log::error("TwitchChatAPI is not available during retry");
        m_statusLabel->setString("Twitch API not available!");
        resetToLogin();
        return;
    };

    // Register callback for when connection is established
    auto validityFlag = m_validityFlag; // Capture the shared_ptr by value
    api->registerOnConnectedCallback([this, validityFlag]()
                                     {
        // Check if this object is still valid using the shared validity flag
        if (!validityFlag || !*validityFlag) {
            log::warn("TwitchLoginPopup was destroyed before authentication callback executed");
            return;
        };

        // Additional safety check
        if (!m_isActive || !m_statusLabel) {
            log::warn("TwitchLoginPopup became invalid or inactive before authentication callback");
            return;
        };

        log::debug("Twitch account authentication completed during retry, checking channel configuration");

        // Check if Twitch channel is configured before proceeding to dashboard
        if (!checkTwitchChannelExists()) {
            log::error("Twitch channel is not configured");
            m_statusLabel->setString("Twitch channel not configured!\nRetrying authentication...");

            // Wait a moment then retry the authentication process
            auto delayAction = CCDelayTime::create(2.0f);
            auto retryAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::retryAuthenticationProcess));
            auto sequence = CCSequence::create(delayAction, retryAction, nullptr);

            if (validityFlag && *validityFlag && m_isActive && m_statusLabel) runAction(sequence);

            return;
        };

        m_statusLabel->setString("Authenticated!\nOpening dashboard...");

        // Now that user is authenticated and channel is configured, proceed to dashboard
        auto delayAction = CCDelayTime::create(1.0f);
        auto openDashboardAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::openDashboard));
        auto sequence = CCSequence::create(delayAction, openDashboardAction, nullptr);

        // Final safety check before opening dashboard
        if (validityFlag && *validityFlag && m_isActive && m_statusLabel) {
            runAction(sequence);
        } else {
            log::warn("TwitchLoginPopup became invalid, cannot open dashboard");
        }; });

    // Prompt login without forcing (this should not prompt if already logged in)
    api->promptLogin(false);

    // Set up a timeout to check if we're already connected but callback wasn't triggered
    auto timeoutAction = CCDelayTime::create(5.0f); // Wait 5 seconds
    auto checkConnectionAction = CCCallFunc::create(this, callfunc_selector(TwitchLoginPopup::checkExistingConnection));
    auto timeoutSequence = CCSequence::create(timeoutAction, checkConnectionAction, nullptr);

    // Schedule timeout check with a unique tag
    stopActionByTag(999); // Stop any previous timeout
    timeoutSequence->setTag(999);

    runAction(timeoutSequence);
};

std::string TwitchLoginPopup::getAuthenticatedUsername()
{
    // Get the TwitchChatAPI mod
    auto twitchMod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api");
    if (!twitchMod)
    {
        log::error("TwitchChatAPI mod not found");
        return "";
    };

    // Get the saved Twitch username value
    auto username = twitchMod->getSavedValue<std::string>("twitch-username");
    if (username.empty())
    { // Check if the username exists and is not empty
        log::debug("Twitch username is not configured or is empty");
        return "";
    };

    log::debug("Twitch username found: {}", username);
    return username;
};

TwitchLoginPopup::~TwitchLoginPopup()
{
    m_isActive = false;
    if (m_validityFlag)
        *m_validityFlag = false;

    log::debug("TwitchLoginPopup destructor called");
};

TwitchLoginPopup *TwitchLoginPopup::create()
{
    auto ret = new TwitchLoginPopup();

    if (ret && ret->initAnchored(300.f, 200.f))
    {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};