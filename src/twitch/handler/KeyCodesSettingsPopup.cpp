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
    m_input = geode::TextInput::create(180, "Key code (e.g. A, F1, Enter)", "bigFont.fnt");
    m_input->setID("keycode-input");
    m_input->setString(keyCode.c_str());
    m_input->setPosition(x, y);
    m_input->setScale(0.7f);
    m_mainLayer->addChild(m_input);
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
    return true;
}

void KeyCodesSettingsPopup::onSave(cocos2d::CCObject* sender) {
    std::string text = m_input ? m_input->getString() : "";
    if (m_onSelect) m_onSelect(text);
    onClose(nullptr);
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
