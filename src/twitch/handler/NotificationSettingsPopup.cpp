#include "NotificationSettingsPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;
using namespace cocos2d;

bool NotificationSettingsPopup::setup(std::string notificationText)
{
    m_notificationText = notificationText;

    setTitle("Edit Notification");
    setID("notification-settings-popup");

    float y = 120.f;
    float x = m_mainLayer->getContentSize().width / 2;

    m_noElasticity = true;

    // Text input for notification
    m_input = TextInput::create(180, "Notification text", "bigFont.fnt");
    m_input->setCommonFilter(CommonFilter::Any);
    m_input->setID("notification-input");
    m_input->setString(notificationText.c_str());
    m_input->setPosition(x, y);
    m_input->setScale(0.7f);

    m_mainLayer->addChild(m_input);

    // Time input for notification duration (seconds)
    float timeY = y - 20.f;
    m_timeInput = TextInput::create(80, "Time (s)", "bigFont.fnt");
    m_timeInput->setCommonFilter(CommonFilter::Float);
    m_timeInput->setID("notification-time-input");
    m_timeInput->setString(fmt::format("{:.2f}", m_time).c_str());
    m_timeInput->setPosition(x, timeY);
    m_timeInput->setScale(0.6f);
    m_mainLayer->addChild(m_timeInput);

    // Icon selector UI
    float iconY = y - 48.f;
    float arrowOffset = 60.f;

    // Left arrow
    auto leftArrowSprite = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    m_leftArrow = CCMenuItemSpriteExtra::create(
        leftArrowSprite,
        this,
        menu_selector(NotificationSettingsPopup::onLeftIcon));
    m_leftArrow->setID("left-arrow");
    m_leftArrow->setPosition(x - arrowOffset, iconY);

    // Right arrow (ensure correct flip and scale)
    auto rightArrowSprite = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
    m_rightArrow = CCMenuItemSpriteExtra::create(
        rightArrowSprite,
        this,
        menu_selector(NotificationSettingsPopup::onRightIcon));
    m_rightArrow->setID("right-arrow");
    m_rightArrow->setPosition(x + arrowOffset, iconY);

    m_iconLabel = CCLabelBMFont::create("Info", "bigFont.fnt");
    m_iconLabel->setID("notification-icon-label");
    m_iconLabel->setScale(0.6f);
    m_iconLabel->setAnchorPoint({0.5f, 0.5f});
    m_iconLabel->setPosition(x, iconY);

    m_mainLayer->addChild(m_iconLabel);

    auto iconMenu = CCMenu::create();
    iconMenu->setPosition(0, 0);

    iconMenu->addChild(m_leftArrow);
    iconMenu->addChild(m_rightArrow);

    m_mainLayer->addChild(iconMenu);

    // Save button
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(NotificationSettingsPopup::onSave));
    saveBtn->setID("notification-save-btn");

    auto menu = CCMenu::create();
    menu->setPosition(x, iconY - 40.f);

    menu->addChild(saveBtn);

    m_mainLayer->addChild(menu);

    updateIconLabel();

    return true;
};

void NotificationSettingsPopup::onSave(cocos2d::CCObject *sender)
{
    std::string text = m_input ? m_input->getString() : "";
    // Parse time
    float time = 1.0f;
    if (m_timeInput) {
        std::string t = m_timeInput->getString();
        t.erase(0, t.find_first_not_of(" \t\n\r"));
        if (!t.empty()) {
            auto parsed = numFromString<float>(t);
            if (parsed) time = parsed.unwrap();
        }
    }
    m_time = time;
    if (m_onSelect)
        m_onSelect(text, m_iconType, m_time);

    onClose(nullptr);
};

void NotificationSettingsPopup::onLeftIcon(cocos2d::CCObject *sender)
{
    int icon = static_cast<int>(m_iconType);
    icon = (icon - 1 + 6) % 6; // 6 icon types

    m_iconType = static_cast<NotificationIconType>(icon);

    updateIconLabel();
};

void NotificationSettingsPopup::onRightIcon(cocos2d::CCObject *sender)
{
    int icon = static_cast<int>(m_iconType);
    icon = (icon + 1) % 6;

    m_iconType = static_cast<NotificationIconType>(icon);

    updateIconLabel();
};

void NotificationSettingsPopup::updateIconLabel()
{
    const char *names[] = {"None", "Info", "Success", "Warning", "Error", "Loading"};
    int icon = static_cast<int>(m_iconType);

    if (m_iconLabel && icon >= 0 && icon < 6)
        m_iconLabel->setString(names[icon]);

    // Always reset arrow button scale/position in case they were changed by click effects
    float x = m_mainLayer->getContentSize().width / 2;
    float iconY = m_iconLabel->getPositionY();
    float arrowOffset = 60.f;
};

NotificationSettingsPopup *NotificationSettingsPopup::create(const std::string &notificationText, std::function<void(const std::string &, NotificationIconType, float)> onSelect, NotificationIconType iconType, float time)
{
    auto ret = new NotificationSettingsPopup();

    ret->m_onSelect = onSelect;
    ret->m_iconType = iconType;
    ret->m_time = time;

    if (ret && ret->initAnchored(220.f, 180.f, notificationText))
    {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};