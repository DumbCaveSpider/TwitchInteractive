#pragma once
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <string>
#include <functional>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;
using namespace cocos2d;

class ProfileSettingsPopup : public geode::Popup<>
{
protected:
    std::string m_accountId;
    std::function<void(const std::string &)> m_callback;
    TextInput *m_accountIdInput = nullptr;

    bool setup() override;
    void onSave(CCObject *sender);
    void onOpenProfile(CCObject *sender);

public:
    static ProfileSettingsPopup *create(const std::string &accountId, std::function<void(const std::string &)> callback);
};
