#pragma once

#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>
#include <functional>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;
using namespace cocos2d;
using namespace extension;

class JumpscareSettingsPopup : public geode::Popup<>
{
protected:
    geode::TextInput* m_urlInput = nullptr; // now holds file name only
    geode::TextInput* m_fadeInput = nullptr;
    std::function<void(const std::string&, float)> m_onSave;
    std::string m_initUrl;
    float m_initFade = 0.5f;

    bool setup() override;
    void onSaveBtn(cocos2d::CCObject*);
    void onOpenFolder(cocos2d::CCObject*);
    void onInfoBtn(cocos2d::CCObject*);

public:
    static JumpscareSettingsPopup* create(const std::string& initUrl, float initFade, std::function<void(const std::string&, float)> onSave);
};
