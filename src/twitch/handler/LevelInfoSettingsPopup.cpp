#include "LevelInfoSettingsPopup.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace cocos2d;

bool LevelInfoSettingsPopup::setup() {
    float width = 320.f;
    float height = 120.f;
    setTitle("Open Level Info Settings");
    setID("levelinfo-settings-popup");
    this->m_noElasticity = true;

    auto popupSize = m_mainLayer->getContentSize();
    float margin = 30.f;
    float inputWidth = popupSize.width - margin * 2;
    float inputHeight = 32.f;
    float centerY = popupSize.height / 2;
    float btnY = margin;

    // Level ID input
    m_levelIdInput = geode::TextInput::create(inputWidth, "Level ID", "chatFont.fnt");
    m_levelIdInput->setString(m_initLevelId.c_str());
    m_levelIdInput->setPosition(popupSize.width / 2, centerY);
    m_mainLayer->addChild(m_levelIdInput);

    // Save button
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(LevelInfoSettingsPopup::onSave)
    );
    saveBtn->setPosition(popupSize.width / 2, btnY);
    auto menu = CCMenu::create();
    menu->addChild(saveBtn);
    menu->setPosition(0, 0);
    m_mainLayer->addChild(menu);
    return true;
}

void LevelInfoSettingsPopup::onSave(CCObject* sender) {
    if (m_callback) {
        std::string levelId = m_levelIdInput ? m_levelIdInput->getString() : "";
        m_callback(levelId);
    }
    onClose(sender);
}

void LevelInfoSettingsPopup::onClose(CCObject* sender) {
    this->removeFromParentAndCleanup(true);
}

LevelInfoSettingsPopup* LevelInfoSettingsPopup::create(const std::string& levelId, std::function<void(const std::string&)> callback) {
    auto ret = new LevelInfoSettingsPopup();
    if (ret) {
        ret->m_callback = callback;
        ret->m_initLevelId = levelId;
        if (ret->initAnchored(320.f, 160.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
    }
    return nullptr;
}
