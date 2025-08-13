#pragma once

#include <memory>
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class TwitchLoginPopup : public Popup<>
{
protected:
    CCMenu *m_loginMenu = nullptr;
    CCMenu *m_loggedInMenu = nullptr;

    CCLabelBMFont *m_statusLabel = nullptr;
    CCMenuItemSpriteExtra* m_changeBtn = nullptr;
    CCLabelBMFont* m_changeStatusLabel = nullptr;
    CCLabelBMFont* m_userLabel = nullptr;

    bool m_isActive = true;
    bool m_isWaitingForAuth = false;

    std::shared_ptr<bool> m_validityFlag = std::make_shared<bool>(true);

    bool setup() override;

    void onLoginPressed(CCObject *sender);
    void onAuthenticationCompleted();
    void onAuthenticationTimeout();
    void checkExistingConnection();
    void retryAuthenticationProcess();
    void openDashboard();
    void resetToLogin();
    void onChangeAccount(CCObject* sender);

    bool checkTwitchChannelExists();

    std::string getAuthenticatedUsername();

public:
    ~TwitchLoginPopup();
    static TwitchLoginPopup *create();
};