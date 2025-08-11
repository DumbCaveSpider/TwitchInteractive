#include "JumpscareSettingsPopup.hpp"
#include <Geode/utils/file.hpp>
#include <Geode/loader/Dirs.hpp>

using namespace geode::prelude;
using namespace cocos2d;

bool JumpscareSettingsPopup::setup()
{
    setTitle("Jumpscare Settings");
    setID("jumpscare-settings-popup");
    m_noElasticity = true;

    auto size = m_mainLayer->getContentSize();

    float margin = 30.f;
    float inputW = size.width - margin * 2;

    // File name input (inside custom jumpscare folder)
    m_urlInput = TextInput::create(static_cast<int>(inputW), "Image file name", "bigFont.fnt");
    m_urlInput->setCommonFilter(CommonFilter::Any);
    m_urlInput->setString(m_initUrl);
    m_urlInput->setPosition(size.width / 2.f, size.height / 2.f + 30.f);
    m_mainLayer->addChild(m_urlInput);

    // Fade time input
    m_fadeInput = TextInput::create(120, "Fade (s)", "bigFont.fnt");
    m_fadeInput->setCommonFilter(CommonFilter::Float);
    m_fadeInput->setString(fmt::format("{:.2f}", m_initFade));
    m_fadeInput->setPosition(size.width / 2.f, size.height / 2.f - 10.f);
    m_fadeInput->setScale(0.9f);
    m_mainLayer->addChild(m_fadeInput);

    auto menu = CCMenu::create();
    menu->setPosition(0, 0);
    m_mainLayer->addChild(menu);

    // Info icon at the bottom center
    auto infoSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    infoSprite->setScale(0.7f);
    auto infoBtn = CCMenuItemSpriteExtra::create(
        infoSprite,
        this,
        menu_selector(JumpscareSettingsPopup::onInfoBtn));
    infoBtn->setID("jumpscare-info-btn");
    float infoY = 20.f;
    float infoX = size.width - 20.f;
    infoBtn->setPosition(infoX, infoY);
    auto infoMenu = CCMenu::create();
    infoMenu->setPosition(0, 0);
    infoMenu->addChild(infoBtn);
    m_mainLayer->addChild(infoMenu);

    // Create both buttons first to measure their widths
    auto saveBtnSprite = ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f);
    auto openBtnSprite = ButtonSprite::create("Open Folder", "bigFont.fnt", "GJ_button_05.png", 0.6f);
    float spacing = 16.f;
    float saveW = saveBtnSprite->getContentSize().width * saveBtnSprite->getScale();
    float openW = openBtnSprite->getContentSize().width * openBtnSprite->getScale();
    float totalW = saveW + openW + spacing;
    float centerX = size.width / 2.f;
    float y = 30.f;

    auto saveBtn = CCMenuItemSpriteExtra::create(
        saveBtnSprite,
        this,
        menu_selector(JumpscareSettingsPopup::onSaveBtn));
    auto openBtn = CCMenuItemSpriteExtra::create(
        openBtnSprite,
        this,
        menu_selector(JumpscareSettingsPopup::onOpenFolder));
    openBtn->setID("jumpscare-open-folder-btn");

    // Position: [openBtn][spacing][saveBtn] centered
    float openX = centerX - totalW / 2.f + openW / 2.f;
    float saveX = openX + openW / 2.f + spacing + saveW / 2.f;
    openBtn->setPosition(openX, y);
    saveBtn->setPosition(saveX, y);
    menu->addChild(openBtn);
    menu->addChild(saveBtn);

    return true;
}

void JumpscareSettingsPopup::onInfoBtn(cocos2d::CCObject *)
{
    // FLAlertLayer doesn't fit correctly with these text so this is better
    geode::createQuickPopup(
        "How to Import Images",
        "To use a custom image for the jumpscare, click <cg>'Open Folder'</c> and place your <co>PNG or JPG</c> file in the folder that opens.\n"
        "Then, enter the file name (including extension) in the <cg>'Image file name'</c> box. The image will be displayed when the jumpscare action is triggered.\n"
        "<cg>If you want the image to cover the entire screen, make sure the image is the same resolution as your screen.</c>",
        "OK",
        nullptr,
        360.f,
        [](FLAlertLayer*, bool) {},
        false,
        false
    )->show();
}

void JumpscareSettingsPopup::onSaveBtn(cocos2d::CCObject *)
{
    std::string url = m_urlInput ? m_urlInput->getString() : "";
    std::string fadeStr = m_fadeInput ? m_fadeInput->getString() : "0";

    // Trim
    auto trim = [](std::string &s)
    {
        if (s.empty())
            return;
        s.erase(0, s.find_first_not_of(" \t\n\r"));
        size_t end = s.find_last_not_of(" \t\n\r");
        if (end != std::string::npos)
            s.erase(end + 1);
        else
            s.clear();
    };
    trim(url);
    trim(fadeStr);

    float fade = 0.0f;
    if (!fadeStr.empty())
        if (auto parsed = numFromString<float>(fadeStr))
            fade = parsed.unwrap();

    if (m_onSave)
        m_onSave(url, fade);
    this->removeFromParentAndCleanup(true);
}

void JumpscareSettingsPopup::onOpenFolder(cocos2d::CCObject *)
{
    auto base = Mod::get()->getConfigDir();
    auto folder = (base / "jumpscare").string();
    if (!geode::utils::file::createDirectoryAll(folder))
    {
        log::warn("[JumpscareSettingsPopup] Failed to create jumpscare folder: {}", folder);
    }
    if (auto res = geode::utils::file::openFolder(folder); !res)
    {
        log::warn("[JumpscareSettingsPopup] Failed to open jumpscare folder: {}", folder);
    }
}

JumpscareSettingsPopup *JumpscareSettingsPopup::create(const std::string &initUrl, float initFade, std::function<void(const std::string &, float)> onSave)
{
    auto ret = new JumpscareSettingsPopup();
    if (ret)
    {
        ret->m_onSave = onSave;
        ret->m_initUrl = initUrl;
        ret->m_initFade = initFade;
        // Reasonable default size
        if (ret->initAnchored(360.f, 180.f))
        {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
    }
    return nullptr;
}
