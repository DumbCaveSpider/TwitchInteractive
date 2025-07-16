#pragma once
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <functional>
#include <string>

using namespace geode::prelude;
using namespace cocos2d;

class LevelInfoSettingsPopup : public geode::Popup<> {
protected:
    geode::TextInput* m_levelIdInput = nullptr;
    std::function<void(const std::string&)> m_callback;
    std::string m_initLevelId;

    bool setup();
    void onSave(CCObject* sender);
    void onClose(CCObject* sender = nullptr);
public:
    static LevelInfoSettingsPopup* create(const std::string& levelId, std::function<void(const std::string&)> callback);
};
