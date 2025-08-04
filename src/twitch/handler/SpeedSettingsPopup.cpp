#include <Geode/Geode.hpp>
#include "SpeedSettingsPopup.hpp"

using namespace geode::prelude;
using namespace cocos2d;

bool SpeedSettingsPopup::setup() {
    setTitle("Speed Player Settings");
    setID("speed-player-settings-popup");
    auto popupSize = getContentSize();

    m_mainLayer = cocos2d::CCLayer::create();
    m_mainLayer->setContentSize(popupSize);
    m_mainLayer->setAnchorPoint({0, 0});
    m_mainLayer->setPosition(0, 0);
    this->m_noElasticity = true;
    addChild(m_mainLayer);

    auto layout = ColumnLayout::create()
                      ->setGap(12.f)
                      ->setAxisAlignment(AxisAlignment::Center)
                      ->setCrossAxisAlignment(AxisAlignment::Center);
    m_mainLayer->setLayout(layout);

    float centerX = m_mainLayer->getContentSize().width / 2.0f;
    float centerY = m_mainLayer->getContentSize().height / 2.0f;
    float verticalSpacing = 30.0f;
    float startY = centerY + verticalSpacing;

    auto inputStack = CCNode::create();
    inputStack->setContentSize({popupSize.width, 100.0f});
    inputStack->setAnchorPoint({0.5f, 0.5f});
    inputStack->setPosition(centerX, startY - 20.f);

    float inputSpacing = 100.0f;
    float inputY = 40.0f;

    // Speed label and input
    auto speedLabel = CCLabelBMFont::create("Speed", "bigFont.fnt");
    speedLabel->setScale(0.5f);
    speedLabel->setAnchorPoint({0.5f, 0.5f});
    speedLabel->setAlignment(kCCTextAlignmentCenter);
    speedLabel->setPosition(centerX - inputSpacing / 2, inputY + 20.0f);
    inputStack->addChild(speedLabel);

    m_speedInput = TextInput::create(80, "Speed", "chatFont.fnt");
    m_speedInput->setCommonFilter(CommonFilter::Float);
    m_speedInput->setString(fmt::format("{:.2f}", m_speed).c_str());
    m_speedInput->setScale(0.7f);
    m_speedInput->setAnchorPoint({0.5f, 0.5f});
    m_speedInput->setPosition(centerX - inputSpacing / 2, inputY);
    inputStack->addChild(m_speedInput);

    // Duration label and input
    auto durationLabel = CCLabelBMFont::create("Duration", "bigFont.fnt");
    durationLabel->setScale(0.5f);
    durationLabel->setAnchorPoint({0.5f, 0.5f});
    durationLabel->setAlignment(kCCTextAlignmentCenter);
    durationLabel->setPosition(centerX + inputSpacing / 2, inputY + 20.0f);
    inputStack->addChild(durationLabel);

    m_durationInput = TextInput::create(80, "Duration (secs)", "chatFont.fnt");
    m_durationInput->setCommonFilter(CommonFilter::Float);
    m_durationInput->setString(fmt::format("{:.2f}", m_duration).c_str());
    m_durationInput->setScale(0.7f);
    m_durationInput->setAnchorPoint({0.5f, 0.5f});
    m_durationInput->setPosition(centerX + inputSpacing / 2, inputY);
    inputStack->addChild(m_durationInput);

    m_mainLayer->addChild(inputStack);

    // Create menu for apply button
    auto menu = CCMenu::create();
    menu->setContentSize(m_mainLayer->getContentSize());
    menu->setAnchorPoint({0.5f, 0.5f});
    menu->setPosition(centerX, centerY - verticalSpacing);

    auto applyBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f), this, menu_selector(SpeedSettingsPopup::onSaveBtn));
    applyBtn->setID("speed-settings-apply-btn");
    applyBtn->setPosition(0, -15.f);
    menu->addChild(applyBtn);

    m_mainLayer->addChild(menu);

    return true;
}

void SpeedSettingsPopup::onSaveBtn(CCObject*) {
    std::string speedStr = m_speedInput ? m_speedInput->getString() : "1.0";
    std::string durationStr = m_durationInput ? m_durationInput->getString() : "0.5";
    float speed = strtof(speedStr.c_str(), nullptr);
    float duration = strtof(durationStr.c_str(), nullptr);
    if (speed <= 0.0f) {
        Notification::create("Speed must be positive!", NotificationIcon::Error)->show();
        return;
    }
    if (duration < 0.0f) {
        Notification::create("Duration must be non-negative!", NotificationIcon::Error)->show();
        return;
    }
    if (m_onSave) m_onSave(speed, duration);
    onClose(nullptr);
}

SpeedSettingsPopup* SpeedSettingsPopup::create(int actionIdx, float defaultSpeed, float defaultDuration, std::function<void(float, float)> onSave) {
    auto ret = new SpeedSettingsPopup();
    ret->m_actionIdx = actionIdx;
    ret->m_speed = defaultSpeed;
    ret->m_duration = defaultDuration;
    ret->m_onSave = onSave;
    if (ret->initAnchored(300.f, 150.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}