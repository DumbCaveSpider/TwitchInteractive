#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <string>
#include <functional>

class KeyCodesSettingsPopup : public geode::Popup<std::string> {
protected:
    std::string m_keyCode;
    std::string m_holdDuration;
    cocos2d::CCLabelBMFont* m_keyLabel = nullptr;
    geode::TextInput* m_durationInput = nullptr;
    void keyDown(cocos2d::enumKeyCodes key) override;
    std::function<void(const std::string&)> m_onSelect;
    bool setup(std::string keyCode) override;
    void onSave(cocos2d::CCObject* sender);
public:
    static KeyCodesSettingsPopup* create(const std::string& keyCode, std::function<void(const std::string&)> onSelect);
};
