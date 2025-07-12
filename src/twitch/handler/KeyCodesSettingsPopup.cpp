#include "KeyCodesSettingsPopup.hpp"
#include <Geode/Geode.hpp>
using namespace geode::prelude;
using namespace cocos2d;

bool KeyCodesSettingsPopup::setup(std::string keyCode) {
    m_keyCode = keyCode;
    setTitle("Edit Key Code");
    setID("keycodes-settings-popup");
    float y = 100.f;
    float x = m_mainLayer->getContentSize().width / 2;
    m_keyLabel = CCLabelBMFont::create(keyCode.empty() ? "Press any key..." : keyCode.c_str(), "bigFont.fnt");
    m_keyLabel->setID("keycode-label");
    m_keyLabel->setPosition(x, y);
    m_keyLabel->setScale(0.7f);
    m_mainLayer->addChild(m_keyLabel);
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(KeyCodesSettingsPopup::onSave)
    );
    saveBtn->setID("keycode-save-btn");
    auto menu = CCMenu::create();
    menu->addChild(saveBtn);
    menu->setPosition(x, y - 40.f);
    m_mainLayer->addChild(menu);
    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);
    return true;
}

void KeyCodesSettingsPopup::onSave(cocos2d::CCObject* sender) {
    if (m_onSelect) m_onSelect(m_keyCode);
    onClose(nullptr);
}
void KeyCodesSettingsPopup::keyDown(cocos2d::enumKeyCodes key) {
    // Simple key code to string mapping
    std::string keyStr;
    if (key >= KEY_A && key <= KEY_Z) {
        keyStr = std::string(1, 'A' + (key - KEY_A));
    } else {
        switch (key) {
            case KEY_Space: keyStr = "Space"; break;
            case KEY_Enter: keyStr = "Enter"; break;
            case KEY_Escape: keyStr = "Escape"; break;
            case KEY_Left: keyStr = "Left"; break;
            case KEY_Right: keyStr = "Right"; break;
            case KEY_Up: keyStr = "Up"; break;
            case KEY_Down: keyStr = "Down"; break;
            case KEY_Tab: keyStr = "Tab"; break;
            case KEY_Backspace: keyStr = "Backspace"; break;
            case KEY_Shift: keyStr = "Shift"; break;
            case KEY_Control: keyStr = "Ctrl"; break;
            case KEY_Alt: keyStr = "Alt"; break;
            default: keyStr = fmt::format("KeyCode({})", (int)key); break;
        }
    }
    m_keyCode = keyStr;
    if (m_keyLabel) m_keyLabel->setString(keyStr.c_str());
}

KeyCodesSettingsPopup* KeyCodesSettingsPopup::create(const std::string& keyCode, std::function<void(const std::string&)> onSelect) {
    auto ret = new KeyCodesSettingsPopup();
    ret->m_onSelect = onSelect;
    if (ret && ret->initAnchored(220.f, 160.f, keyCode)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
