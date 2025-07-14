#include "CameraSettingsPopup.hpp"
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>

using namespace cocos2d;
using namespace geode::prelude;

bool CameraSettingsPopup::setup() {
    setTitle("Camera Settings");
    setID("camera-settings-popup");
    // Create a content layer for all UI elements, parented to the popup for proper animation
    auto popupSize = this->getContentSize();
    auto m_contentLayer = CCLayer::create();
    m_contentLayer->setContentSize(popupSize);
    m_contentLayer->setAnchorPoint({0, 0});
    m_contentLayer->setPosition({0, 0});
    this->addChild(m_contentLayer);

    float fieldWidth = 70.f;
    float fieldHeight = 32.f;
    float spacing = 10.f;
    float rowY = popupSize.height / 2 + 20.f;
    float rowStartX = popupSize.width / 2 - (2 * fieldWidth + 1.5f * spacing);
    float labelY = rowY + fieldHeight / 2 - 24.f;
    float inputY = rowY - 20.f;

    // Labels and inputs in a row: Skew, Rotation, Scale, Time
    const char* names[] = {"Skew", "Rotation", "Scale", "Time"};
    geode::TextInput** inputs[] = { &m_skewInput, &m_rotInput, &m_scaleInput, &m_timeInput };
    for (int i = 0; i < 4; ++i) {
        float x = rowStartX + i * (fieldWidth + spacing);
        // Label above input
        auto label = CCLabelBMFont::create(names[i], "bigFont.fnt");
        label->setScale(0.5f);
        label->setAnchorPoint({0.5f, 0.5f});
        label->setPosition(x + fieldWidth / 2, labelY);
        m_contentLayer->addChild(label);
        // Input
        *inputs[i] = geode::TextInput::create(fieldWidth, names[i], "chatFont.fnt");
        (*inputs[i])->setPosition(x + fieldWidth / 2, inputY);
        (*inputs[i])->setScale(0.6f);
        (*inputs[i])->setMaxCharCount(6);
        m_contentLayer->addChild(*inputs[i]);
    }

    // Save button centered below the row
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.7f),
        this,
        menu_selector(CameraSettingsPopup::onSave)
    );
    saveBtn->setID("camera-save-btn");
    float btnY = inputY - fieldHeight - 30.f;
    auto menu = CCMenu::create();
    saveBtn->setPosition(0.f, 0.f);
    menu->addChild(saveBtn);
    menu->setPosition(popupSize.width / 2, btnY);
    m_contentLayer->addChild(menu);
    return true;
}

void CameraSettingsPopup::onSave(CCObject* sender) {
    if (m_callback) {
        float skew = getSkew();
        float rot = getRotation();
        float scale = getScale();
        float time = getTime();
        m_callback(skew, rot, scale, time);
    }
    this->onClose(sender);
}

std::string CameraSettingsPopup::formatCameraLabel(float skew, float rot, float scale, float time) {
    char buf[128];
    snprintf(buf, sizeof(buf), "Skew: %.2f, Rot: %.2f, Scale: %.2f, Time: %.2fs", skew, rot, scale, time);
    return std::string(buf);
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
        if (ret->initAnchored(330.f, 180.f)) {
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
