#pragma once

#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>
#include <functional>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;
using namespace cocos2d;
using namespace extension;

class AlertSettingsPopup : public geode::Popup<> {
protected:
    geode::TextInput* m_titleInput = nullptr;
    geode::TextInput* m_descInput = nullptr;
    std::function<void(const std::string&, const std::string&)> m_callback;
    std::string m_initTitle;
    std::string m_initDesc;

    bool setup();
    void onSave(cocos2d::CCObject* sender);
    void onClose(cocos2d::CCObject* sender = nullptr);
public:
    static AlertSettingsPopup* create(const std::string& title, const std::string& desc, std::function<void(const std::string&, const std::string&)> callback);
};
