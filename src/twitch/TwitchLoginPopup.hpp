#pragma once

#include <Geode/Geode.hpp>
#include <memory>

using namespace geode::prelude;

class TwitchLoginPopup : public Popup<> {
protected:
    CCMenu* m_loginMenu = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    CCMenu* m_loggedInMenu = nullptr;
    int m_tokenCheckRetries = 0;
    bool m_isActive = true;
    bool m_isWaitingForAuth = false;
    std::shared_ptr<bool> m_validityFlag = std::make_shared<bool>(true);
    static const int MAX_TOKEN_CHECK_RETRIES = 10;

    bool setup() override;
    void onLoginPressed(CCObject* sender);
    void requestTokenAccess();
    void onAuthenticationCompleted();
    void onAuthenticationTimeout();
    void checkExistingConnection();
    void openDashboard();
    void resetToLogin();
    bool checkTwitchChannelExists();
public:
    ~TwitchLoginPopup();
    static TwitchLoginPopup* create();
};
