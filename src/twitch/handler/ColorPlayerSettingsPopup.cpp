#include "ColorPlayerSettingsPopup.hpp"
#include <Geode/Geode.hpp>

bool ColorPlayerSettingsPopup::setup() {
    setTitle("Color Player Settings");
    setID("color-player-settings-popup");

    auto popupSize = getContentSize();

    m_mainLayer = cocos2d::CCLayer::create();
    m_mainLayer->setContentSize(popupSize);
    m_mainLayer->setAnchorPoint({ 0, 0 });
    m_mainLayer->setPosition(0, 0);

    m_noElasticity = true;
    addChild(m_mainLayer);

    float pickerY = popupSize.height / 2 + 20.f;

    // Add background effect for color picker area (like main popup)
    auto pickerBg = CCMenu::create();
    pickerBg->setContentSize({ popupSize.width - 40.f, 120.f });
    pickerBg->setPosition(popupSize.width / 2, pickerY);
    pickerBg->setOpacity(180);

    m_mainLayer->addChild(pickerBg, 0);

    m_colorPicker = new cocos2d::extension::CCControlColourPicker();

    if (m_colorPicker && m_colorPicker->init()) {
        m_colorPicker->autorelease();
        m_colorPicker->addTargetWithActionForControlEvents(
            this,
            cccontrol_selector(ColorPlayerSettingsPopup::onColorChanged),
            cocos2d::extension::CCControlEventValueChanged);
        m_colorPicker->setColorValue(m_selectedColor);
        m_colorPicker->setPosition(popupSize.width / 2, pickerY);

        m_mainLayer->addChild(m_colorPicker, 1);
    };

    // RGB Text Inputs below color picker
    float rgbBoxY = pickerY - (m_colorPicker ? m_colorPicker->getContentSize().height * m_colorPicker->getScaleY() / 2 : 50.f) - 16.f;

    float boxWidth = 48.f;
    float boxHeight = 32.f;
    float boxSpacing = 12.f;

    float totalWidth = boxWidth * 3 + boxSpacing * 2;
    float startX = popupSize.width / 2 - totalWidth / 2;

    m_rInput = TextInput::create(boxWidth, "R", "chatFont.fnt");
    m_gInput = TextInput::create(boxWidth, "G", "chatFont.fnt");
    m_bInput = TextInput::create(boxWidth, "B", "chatFont.fnt");

    m_rInput->setPosition(startX + boxWidth / 2, rgbBoxY);
    m_gInput->setPosition(startX + boxWidth + boxSpacing + boxWidth / 2, rgbBoxY);
    m_bInput->setPosition(startX + (boxWidth + boxSpacing) * 2 + boxWidth / 2, rgbBoxY);

    m_rInput->setScale(0.6f);
    m_gInput->setScale(0.6f);
    m_bInput->setScale(0.6f);

    m_rInput->setMaxCharCount(3);
    m_gInput->setMaxCharCount(3);
    m_bInput->setMaxCharCount(3);

    char buf[8];

    snprintf(buf, sizeof(buf), "%d", m_selectedColor.r);
    m_rInput->setString(buf);

    snprintf(buf, sizeof(buf), "%d", m_selectedColor.g);
    m_gInput->setString(buf);

    snprintf(buf, sizeof(buf), "%d", m_selectedColor.b);
    m_bInput->setString(buf);

    m_mainLayer->addChild(m_rInput, 2);
    m_mainLayer->addChild(m_gInput, 2);
    m_mainLayer->addChild(m_bInput, 2);

    // Save and Apply RGB buttons on the same menu, side by side below the color picker
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(ColorPlayerSettingsPopup::onSave));
    saveBtn->setID("color-player-save-btn");

    auto applyBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Apply RGB", "bigFont.fnt", "GJ_button_02.png", 0.5f),
        this,
        menu_selector(ColorPlayerSettingsPopup::onApplyRGB));
    applyBtn->setID("color-player-apply-rgb-btn");

    // Add extra spacing between RGB fields and button menu
    float buttonSpacingY = 32.f;
    float btnMenuY = rgbBoxY - buttonSpacingY;

    // Create a container layer for the button menu, with background effect
    auto btnMenuLayer = cocos2d::CCLayer::create();
    btnMenuLayer->setContentSize({ popupSize.width, 50.f });
    btnMenuLayer->setAnchorPoint({ 0, 0 });
    btnMenuLayer->setPosition(0, btnMenuY - 25.f); // center the 50.f height on btnMenuY

    // Add background effect to button menu
    auto btnMenuBg = CCMenu::create();
    btnMenuBg->setContentSize({ popupSize.width - 40.f, 50.f });
    btnMenuBg->setPosition(btnMenuLayer->getContentSize().width / 2, btnMenuLayer->getContentSize().height / 2);
    btnMenuBg->setOpacity(180);

    btnMenuLayer->addChild(btnMenuBg, 0);

    // Use a CCMenu for button clickability
    float buttonGap = 24.f;

    float saveBtnWidth = saveBtn->getContentSize().width * saveBtn->getScaleX();
    float applyBtnWidth = applyBtn->getContentSize().width * applyBtn->getScaleX();

    float totalButtonWidth = saveBtnWidth + applyBtnWidth + buttonGap;

    float centerX = btnMenuLayer->getContentSize().width / 2;
    float centerY = btnMenuLayer->getContentSize().height / 2;

    auto menu = CCMenu::create();
    menu->setPosition(centerX, centerY);

    saveBtn->setPosition(-totalButtonWidth / 2 + saveBtnWidth / 2, 0.f);
    applyBtn->setPosition(totalButtonWidth / 2 - applyBtnWidth / 2, 0.f);

    menu->addChild(saveBtn);
    menu->addChild(applyBtn);

    btnMenuLayer->addChild(menu, 1);

    m_mainLayer->addChild(btnMenuLayer, 2);

    return true;
};

void ColorPlayerSettingsPopup::onApplyRGB(cocos2d::CCObject* sender) {
    int r = std::max(0, std::min(255, atoi(m_rInput->getString().c_str())));
    int g = std::max(0, std::min(255, atoi(m_gInput->getString().c_str())));
    int b = std::max(0, std::min(255, atoi(m_bInput->getString().c_str())));

    m_selectedColor = cocos2d::ccColor3B{ static_cast<GLubyte>(r), static_cast<GLubyte>(g), static_cast<GLubyte>(b) };

    if (m_colorPicker) m_colorPicker->setColorValue(m_selectedColor);

    char buf[8];

    snprintf(buf, sizeof(buf), "%d", r);
    m_rInput->setString(buf);

    snprintf(buf, sizeof(buf), "%d", g);
    m_gInput->setString(buf);

    snprintf(buf, sizeof(buf), "%d", b);
    m_bInput->setString(buf);
};

void ColorPlayerSettingsPopup::onColorChanged(cocos2d::CCObject* sender, cocos2d::extension::CCControlEvent) {
    if (m_colorPicker) {
        m_selectedColor = m_colorPicker->getColorValue();

        // Update RGB textboxes
        char buf[8];

        snprintf(buf, sizeof(buf), "%d", m_selectedColor.r);
        m_rInput->setString(buf);

        snprintf(buf, sizeof(buf), "%d", m_selectedColor.g);
        m_gInput->setString(buf);

        snprintf(buf, sizeof(buf), "%d", m_selectedColor.b);
        m_bInput->setString(buf);
    };
};

void ColorPlayerSettingsPopup::onSave(cocos2d::CCObject* sender) {
    if (m_callback) m_callback(m_selectedColor);
    onClose(sender);
};

void ColorPlayerSettingsPopup::onClose(cocos2d::CCObject* sender) {
    removeFromParentAndCleanup(true);
};

ColorPlayerSettingsPopup* ColorPlayerSettingsPopup::create(const cocos2d::ccColor3B& initialColor, std::function<void(const cocos2d::ccColor3B&)> callback) {
    auto ret = new ColorPlayerSettingsPopup();

    if (ret != nullptr) {
        ret->m_selectedColor = initialColor;
        ret->m_callback = callback;

        if (ret->initAnchored(320.f, 280.f)) {
            ret->autorelease();
            return ret;
        };

        CC_SAFE_DELETE(ret);
    };

    return nullptr;
};
