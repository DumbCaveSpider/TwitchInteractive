#include "CameraSettingsPopup.hpp"
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>

using namespace cocos2d;
using namespace geode::prelude;

bool CameraSettingsPopup::setup() {
    setTitle("Camera Settings");
    setID("camera-settings-popup");

    auto popupSize = this->getContentSize();
    m_mainLayer = CCLayer::create();
    m_mainLayer->setContentSize(popupSize);
    m_mainLayer->setAnchorPoint({0, 0});
    m_mainLayer->setPosition(0, 0);
    this->addChild(m_mainLayer);

    float fieldWidth = 70.f;
    float fieldHeight = 32.f;
    float spacing = 18.f;
    float startY = popupSize.height / 2 + 40.f;
    float labelX = popupSize.width / 2 - fieldWidth;
    float inputX = popupSize.width / 2 + 10.f;

    // Skew
    auto skewLabel = CCLabelBMFont::create("Skew", "bigFont.fnt");
    skewLabel->setScale(0.5f);
    skewLabel->setAnchorPoint({1, 0.5f});
    skewLabel->setPosition(labelX, startY);
    m_mainLayer->addChild(skewLabel);
    m_skewInput = geode::TextInput::create(fieldWidth, "Skew", "chatFont.fnt");
    m_skewInput->setPosition(inputX, startY);
    m_skewInput->setScale(0.6f);
    m_skewInput->setMaxCharCount(6);
    m_mainLayer->addChild(m_skewInput);

    // Rotation
    auto rotLabel = CCLabelBMFont::create("Rotation", "bigFont.fnt");
    rotLabel->setScale(0.5f);
    rotLabel->setAnchorPoint({1, 0.5f});
    rotLabel->setPosition(labelX, startY - (fieldHeight + spacing));
    m_mainLayer->addChild(rotLabel);
    m_rotInput = geode::TextInput::create(fieldWidth, "Rotation", "chatFont.fnt");
    m_rotInput->setPosition(inputX, startY - (fieldHeight + spacing));
    m_rotInput->setScale(0.6f);
    m_rotInput->setMaxCharCount(6);
    m_mainLayer->addChild(m_rotInput);

    // Scale
    auto scaleLabel = CCLabelBMFont::create("Scale", "bigFont.fnt");
    scaleLabel->setScale(0.5f);
    scaleLabel->setAnchorPoint({1, 0.5f});
    scaleLabel->setPosition(labelX, startY - 2 * (fieldHeight + spacing));
    m_mainLayer->addChild(scaleLabel);
    m_scaleInput = geode::TextInput::create(fieldWidth, "Scale", "chatFont.fnt");
    m_scaleInput->setPosition(inputX, startY - 2 * (fieldHeight + spacing));
    m_scaleInput->setScale(0.6f);
    m_scaleInput->setMaxCharCount(6);
    m_mainLayer->addChild(m_scaleInput);

    // Time
    auto timeLabel = CCLabelBMFont::create("Time", "bigFont.fnt");
    timeLabel->setScale(0.5f);
    timeLabel->setAnchorPoint({1, 0.5f});
    timeLabel->setPosition(labelX, startY - 3 * (fieldHeight + spacing));
    m_mainLayer->addChild(timeLabel);
    m_timeInput = geode::TextInput::create(fieldWidth, "Time", "chatFont.fnt");
    m_timeInput->setPosition(inputX, startY - 3 * (fieldHeight + spacing));
    m_timeInput->setScale(0.6f);
    m_timeInput->setMaxCharCount(6);
    m_mainLayer->addChild(m_timeInput);

    // Save and Apply buttons
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(CameraSettingsPopup::onSave)
    );
    saveBtn->setID("camera-save-btn");
    auto applyBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Apply", "bigFont.fnt", "GJ_button_02.png", 0.5f),
        this,
        menu_selector(CameraSettingsPopup::onApply)
    );
    applyBtn->setID("camera-apply-btn");
    float btnY = startY - 4 * (fieldHeight + spacing) - 20.f;
    auto menu = CCMenu::create();
    saveBtn->setPosition(-60.f, 0.f);
    applyBtn->setPosition(60.f, 0.f);
    menu->addChild(saveBtn);
    menu->addChild(applyBtn);
    menu->setPosition(popupSize.width / 2, btnY);
    m_mainLayer->addChild(menu);
    return true;
}

void CameraSettingsPopup::onSave(CCObject* sender) {
    if (m_callback) {
        m_callback(getSkew(), getRotation(), getScale(), getTime());
    }
    this->onClose(sender);
}

void CameraSettingsPopup::onApply(CCObject* sender) {
    // Optionally apply changes live
    if (m_callback) {
        m_callback(getSkew(), getRotation(), getScale(), getTime());
    }
}

void CameraSettingsPopup::onClose(CCObject* sender) {
    this->removeFromParentAndCleanup(true);
}

float CameraSettingsPopup::getSkew() const {
    return std::stof(m_skewInput->getString());
}
float CameraSettingsPopup::getRotation() const {
    return std::stof(m_rotInput->getString());
}
float CameraSettingsPopup::getScale() const {
    return std::stof(m_scaleInput->getString());
}
float CameraSettingsPopup::getTime() const {
    return std::stof(m_timeInput->getString());
}

CameraSettingsPopup* CameraSettingsPopup::create(float skew, float rot, float scale, float time, std::function<void(float,float,float,float)> callback) {
    auto ret = new CameraSettingsPopup();
    if (ret) {
        ret->m_callback = callback;
        if (ret->initAnchored(320.f, 280.f)) {
            ret->autorelease();
            ret->m_skewInput->setString(fmt::format("{:.2f}", skew).c_str());
            ret->m_rotInput->setString(fmt::format("{:.2f}", rot).c_str());
            ret->m_scaleInput->setString(fmt::format("{:.2f}", scale).c_str());
            ret->m_timeInput->setString(fmt::format("{:.2f}", time).c_str());
            return ret;
        }
        CC_SAFE_DELETE(ret);
    }
    return nullptr;
}
