#include <Geode/Geode.hpp>
#include "ScaleSettingsPopup.hpp"

using namespace geode::prelude;
using namespace cocos2d;

bool ScaleSettingsPopup::setup()
{
    setTitle("Scale Player Settings");
    setID("scale-player-settings-popup");

    auto popupSize = getContentSize();

    m_mainLayer = cocos2d::CCLayer::create();
    m_mainLayer->setContentSize(popupSize);
    m_mainLayer->setAnchorPoint({0, 0});
    m_mainLayer->setPosition(0, 0);

    this->m_noElasticity = true;
    addChild(m_mainLayer);

    // Layout for vertical stacking
    auto layout = ColumnLayout::create()
                      ->setGap(12.f)
                      ->setAxisAlignment(AxisAlignment::Center)
                      ->setCrossAxisAlignment(AxisAlignment::Center);
    m_mainLayer->setLayout(layout);

    // Center coordinates for main layer
    float centerX = m_mainLayer->getContentSize().width / 2.0f;
    float centerY = m_mainLayer->getContentSize().height / 2.0f;

    // Center everything vertically in the popup
    float verticalSpacing = 30.0f;
    float startY = centerY + verticalSpacing;

    // Scale label
    auto scaleLabel = CCLabelBMFont::create("Scale", "bigFont.fnt");
    scaleLabel->setScale(0.5f);
    scaleLabel->setAnchorPoint({0.5f, 0.5f});
    scaleLabel->setPosition(centerX, startY);
    scaleLabel->setAlignment(kCCTextAlignmentCenter);
    m_mainLayer->addChild(scaleLabel);

    // Create menu for both input and button
    auto menu = CCMenu::create();
    menu->setContentSize(m_mainLayer->getContentSize());
    menu->setAnchorPoint({0.5f, 0.5f});
    menu->setPosition(centerX, centerY - verticalSpacing);

    // Scale input as menu item
    m_scaleInput = TextInput::create(80, "Scale", "chatFont.fnt");
    m_scaleInput->setString(fmt::format("{:.2f}", m_scaleValue).c_str());
    m_scaleInput->setScale(0.7f);
    m_scaleInput->setAnchorPoint({0.5f, 0.5f});
    m_scaleInput->setPosition(0, 30.0f); // position relative to menu center
    menu->addChild(m_scaleInput);

    // Save button
    auto saveBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f), this, menu_selector(ScaleSettingsPopup::onSaveBtn));
    saveBtn->setID("scale-settings-save-btn");
    saveBtn->setPosition(0, -10.0f); // position relative to menu center
    menu->addChild(saveBtn);

    m_mainLayer->addChild(menu);

    return true;
}

void ScaleSettingsPopup::onSaveBtn(CCObject *)
{
    std::string scaleStr = m_scaleInput->getString();
    float scale = strtof(scaleStr.c_str(), nullptr);
    if (scale <= 0.0f)
    {
        Notification::create("Scale must be positive!", NotificationIcon::Error)->show();
        return;
    }
    if (m_onSave)
        m_onSave(scale);
    this->removeFromParentAndCleanup(true);
}

ScaleSettingsPopup *ScaleSettingsPopup::create(CommandSettingsPopup *parent, int actionIdx, float scaleValue, std::function<void(float)> onSave)
{
    auto ret = new ScaleSettingsPopup();
    if (ret != nullptr)
    {
        ret->m_parent = parent;
        ret->m_actionIdx = actionIdx;
        ret->m_onSave = onSave;
        ret->m_scaleInput = nullptr;
        ret->m_scaleValue = scaleValue;
        if (ret->initAnchored(300.f, 160.f))
        {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
    }
    return nullptr;
}
