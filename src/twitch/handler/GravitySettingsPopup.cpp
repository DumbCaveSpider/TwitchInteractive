// GravitySettingsPopup.cpp - Unified format for settings popup
#include <Geode/Geode.hpp>
#include "GravitySettingsPopup.hpp"

using namespace geode::prelude;
using namespace cocos2d;

bool GravitySettingsPopup::setup()
{
    setTitle("Gravity Player Settings");
    setID("gravity-player-settings-popup");

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

    // Gravity label and input
    auto gravityLabel = CCLabelBMFont::create("Gravity", "bigFont.fnt");
    gravityLabel->setScale(0.5f);
    gravityLabel->setAnchorPoint({0.5f, 0.5f});
    gravityLabel->setAlignment(kCCTextAlignmentCenter);
    gravityLabel->setPosition(centerX - inputSpacing / 2, inputY + 20.0f);
    inputStack->addChild(gravityLabel);

    m_gravityInput = TextInput::create(80, "Gravity", "bigFont.fnt");
    m_gravityInput->setCommonFilter(CommonFilter::Float);
    m_gravityInput->setString(fmt::format("{:.2f}", m_gravity).c_str());
    m_gravityInput->setScale(0.7f);
    m_gravityInput->setAnchorPoint({0.5f, 0.5f});
    m_gravityInput->setPosition(centerX - inputSpacing / 2, inputY);
    inputStack->addChild(m_gravityInput);

    // Duration label and input
    auto durationLabel = CCLabelBMFont::create("Duration", "bigFont.fnt");
    durationLabel->setScale(0.5f);
    durationLabel->setAnchorPoint({0.5f, 0.5f});
    durationLabel->setAlignment(kCCTextAlignmentCenter);
    durationLabel->setPosition(centerX + inputSpacing / 2, inputY + 20.0f);
    inputStack->addChild(durationLabel);

    m_durationInput = TextInput::create(80, "Duration (secs)", "bigFont.fnt");
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

    auto applyBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f), this, menu_selector(GravitySettingsPopup::onSaveBtn));
    applyBtn->setID("gravity-settings-apply-btn");
    applyBtn->setPosition(0, -15.f);
    menu->addChild(applyBtn);

    m_mainLayer->addChild(menu);

    return true;
}

void GravitySettingsPopup::onSaveBtn(CCObject *)
{
    std::string gravityStr = m_gravityInput ? m_gravityInput->getString() : "1.0";
    std::string durationStr = m_durationInput ? m_durationInput->getString() : "0.5";
    float gravity = strtof(gravityStr.c_str(), nullptr);
    float duration = strtof(durationStr.c_str(), nullptr);
    if (gravity <= 0.0f)
    {
        Notification::create("Gravity must be positive!", NotificationIcon::Error)->show();
        return;
    }
    if (duration < 0.0f)
    {
        Notification::create("Duration must be non-negative!", NotificationIcon::Error)->show();
        return;
    }
    m_gravity = gravity;
    m_duration = duration;
    if (m_onSave)
        m_onSave(gravity, duration);
    this->removeFromParentAndCleanup(true);
}

GravitySettingsPopup *GravitySettingsPopup::create(int actionIdx, float defaultGravity, float defaultDuration, std::function<void(float, float)> onSave)
{
    auto ret = new GravitySettingsPopup();
    if (ret != nullptr)
    {
        ret->m_actionIdx = actionIdx;
        ret->m_onSave = onSave;
        ret->m_gravityInput = nullptr;
        ret->m_durationInput = nullptr;
        ret->m_gravity = defaultGravity;
        ret->m_duration = defaultDuration;
        if (ret->initAnchored(300.f, 150.f))
        {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
    }
    return nullptr;
}