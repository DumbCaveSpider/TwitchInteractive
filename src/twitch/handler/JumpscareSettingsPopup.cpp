#include "JumpscareSettingsPopup.hpp"
#include <Geode/utils/file.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/ui/LazySprite.hpp>
#include <filesystem>

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

    // File selector (left/right + label) for files inside config/jumpscare
    loadFiles();

    float centerX = size.width / 2.f;
    float selectorY = size.height / 2.f + 30.f;

    m_fileLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_fileLabel->setScale(0.7f);
    m_fileLabel->setAnchorPoint({0.5f, 0.5f});
    m_fileLabel->setAlignment(kCCTextAlignmentCenter);

    auto leftSpr = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    auto rightSpr = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");

    auto fileMenu = CCMenu::create();
    fileMenu->setPosition(centerX, selectorY);
    m_leftBtn = CCMenuItemSpriteExtra::create(leftSpr, this, menu_selector(JumpscareSettingsPopup::onLeftFile));
    m_rightBtn = CCMenuItemSpriteExtra::create(rightSpr, this, menu_selector(JumpscareSettingsPopup::onRightFile));
    m_leftBtn->setPosition(-100.f, 0);
    m_rightBtn->setPosition(100.f, 0);
    fileMenu->addChild(m_fileLabel);
    fileMenu->addChild(m_leftBtn);
    fileMenu->addChild(m_rightBtn);
    m_mainLayer->addChild(fileMenu);
    updateFileLabel();
    updateArrowPositions();

    // Fade time input + Scale input side by side
    m_fadeInput = TextInput::create(120, "Fade (s)", "bigFont.fnt");
    m_fadeInput->setCommonFilter(CommonFilter::Float);
    m_fadeInput->setString(fmt::format("{:.2f}", m_initFade));
    float inputsY = size.height / 2.f - 10.f;
    float centerXInputs = size.width / 2.f;
    float offsetX = 70.f;
    m_fadeInput->setPosition(CCPointMake(centerXInputs - offsetX, inputsY));
    m_fadeInput->setScale(0.9f);
    m_mainLayer->addChild(m_fadeInput);

    m_scaleInput = TextInput::create(120, "Scale (x)", "bigFont.fnt");
    m_scaleInput->setCommonFilter(CommonFilter::Float);
    m_scaleInput->setString(fmt::format("{:.2f}", m_initScale));
    m_scaleInput->setPosition(CCPointMake(centerXInputs + offsetX, inputsY));
    m_scaleInput->setScale(0.9f);
    m_mainLayer->addChild(m_scaleInput);

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
    centerX = size.width / 2.f;
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

    // Refresh button at bottom-left
    auto refreshSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    refreshSpr->setScale(0.8f);
    auto refreshBtn = CCMenuItemSpriteExtra::create(refreshSpr, this, menu_selector(JumpscareSettingsPopup::onRefreshFiles));
    refreshBtn->setID("jumpscare-refresh-files-btn");
    refreshBtn->setPosition(20.f + 10.f, 20.f + 10.f);
    menu->addChild(refreshBtn);

    // Test button above the refresh button
    auto testSpr = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
    testSpr->setScale(0.475f);
    auto testBtn = CCMenuItemSpriteExtra::create(testSpr, this, menu_selector(JumpscareSettingsPopup::onTestJumpscare));
    testBtn->setID("jumpscare-test-btn");
    // Place it just above refresh button with small spacing
    testBtn->setPosition(refreshBtn->getPositionX(), refreshBtn->getPositionY() + 48.f);
    menu->addChild(testBtn);

    return true;
}

void JumpscareSettingsPopup::onInfoBtn(cocos2d::CCObject *)
{
    // FLAlertLayer doesn't fit correctly with these text so this is better
    geode::createQuickPopup(
        "How to Import Images",
        "To use a custom image for the jumpscare, click <cg>'Open Folder'</c> and place your <co>PNG, JPG, GIF or WEBP</c> file in the folder that opens.\n"
        "Then, use the <cg>left/right arrows</c> to select an image that will be displayed when the jumpscare action is triggered.\n",
        "OK",
        nullptr,
        360.f,
        [](FLAlertLayer *, bool) {},
        false,
        false)
        ->show();
}

void JumpscareSettingsPopup::onSaveBtn(cocos2d::CCObject *)
{
    std::string url;
    if (m_selectedIdx >= 0 && m_selectedIdx < static_cast<int>(m_files.size()))
        url = m_files[m_selectedIdx];
    else
        url = "";
    std::string fadeStr = m_fadeInput ? m_fadeInput->getString() : "0";
    std::string scaleStr = m_scaleInput ? m_scaleInput->getString() : "1";

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
    trim(scaleStr);

    float fade = 0.0f;
    float scaleMul = 1.0f;
    if (!fadeStr.empty())
        if (auto parsed = numFromString<float>(fadeStr))
            fade = parsed.unwrap();
    if (!scaleStr.empty())
        if (auto parsed = numFromString<float>(scaleStr))
            scaleMul = parsed.unwrap();

    if (m_onSave)
        m_onSave(url, fade, scaleMul);
    this->removeFromParentAndCleanup(true);
}

void JumpscareSettingsPopup::onOpenFolder(cocos2d::CCObject *)
{
    auto base = Mod::get()->getConfigDir();
    auto folder = (base / "jumpscare").string();
    if (auto mk = geode::utils::file::createDirectoryAll(folder); !mk)
    {
        log::warn("[JumpscareSettingsPopup] Failed to create jumpscare folder: {}", folder);
    }
    if (auto res = geode::utils::file::openFolder(folder); !res)
    {
        log::warn("[JumpscareSettingsPopup] Failed to open jumpscare folder: {}", folder);
    }
}

void JumpscareSettingsPopup::cleanupTempNode(cocos2d::CCNode *node)
{
    if (node != nullptr)
    {
        node->removeFromParentAndCleanup(true);
    }
}

void JumpscareSettingsPopup::loadFiles()
{
    m_files.clear();
    auto base = Mod::get()->getConfigDir();
    auto dir = base / "jumpscare";
    if (auto mk = geode::utils::file::createDirectoryAll(dir.string()); !mk)
    {
        log::warn("[JumpscareSettingsPopup] Failed to ensure jumpscare directory exists: {}", dir.string());
    }

    // collect PNG/JPG files
    if (auto res = geode::utils::file::readDirectory(dir.string(), false))
    {
        for (auto const &entry : res.unwrap())
        {
            // entry may be a filesystem::path; convert to string then strip separators
            std::string name = entry.string();
            size_t pos1 = name.find_last_of('/');
            size_t pos2 = name.find_last_of('\\');
            size_t pos = std::string::npos;
            if (pos1 != std::string::npos && pos2 != std::string::npos)
                pos = std::max(pos1, pos2);
            else if (pos1 != std::string::npos)
                pos = pos1;
            else if (pos2 != std::string::npos)
                pos = pos2;
            if (pos != std::string::npos)
                name = name.substr(pos + 1);
            // filter image extensions (case-insensitive)
            auto lower = geode::utils::string::toLower(name);
            if (geode::utils::string::endsWith(lower, ".png") || geode::utils::string::endsWith(lower, ".jpg") || geode::utils::string::endsWith(lower, ".jpeg"))
            {
                m_files.push_back(name);
            }
        }
    }

    // Determine initial selection
    if (!m_files.empty())
    {
        m_selectedIdx = 0;
        // If initUrl matches a file, select it
        if (!m_initUrl.empty())
        {
            for (size_t i = 0; i < m_files.size(); ++i)
            {
                if (geode::utils::string::toLower(m_files[i]) == geode::utils::string::toLower(m_initUrl))
                {
                    m_selectedIdx = static_cast<int>(i);
                    break;
                }
            }
        }
    }
    else
    {
        m_selectedIdx = -1;
    }
}

void JumpscareSettingsPopup::updateFileLabel()
{
    if (!m_fileLabel)
        return;
    if (m_selectedIdx >= 0 && m_selectedIdx < static_cast<int>(m_files.size()))
        m_fileLabel->setString(m_files[m_selectedIdx].c_str());
    else
        m_fileLabel->setString("No Image File Detected");
}

void JumpscareSettingsPopup::updateArrowPositions()
{
    if (!m_fileLabel || !m_leftBtn || !m_rightBtn)
        return;
    float halfWidth = (m_fileLabel->getContentSize().width * m_fileLabel->getScale()) / 2.0f;
    float pad = 24.f;
    m_leftBtn->setPosition(-(halfWidth + pad), 0);
    m_rightBtn->setPosition(+(halfWidth + pad), 0);
}

void JumpscareSettingsPopup::onLeftFile(cocos2d::CCObject *)
{
    if (m_files.empty())
        return;
    m_selectedIdx = (m_selectedIdx - 1 + static_cast<int>(m_files.size())) % static_cast<int>(m_files.size());
    updateFileLabel();
    updateArrowPositions();
}

void JumpscareSettingsPopup::onRightFile(cocos2d::CCObject *)
{
    if (m_files.empty())
        return;
    m_selectedIdx = (m_selectedIdx + 1) % static_cast<int>(m_files.size());
    updateFileLabel();
    updateArrowPositions();
}

void JumpscareSettingsPopup::onRefreshFiles(cocos2d::CCObject *)
{
    int prevIdx = m_selectedIdx;
    std::string prevName = (prevIdx >= 0 && prevIdx < static_cast<int>(m_files.size())) ? m_files[prevIdx] : std::string();
    loadFiles();
    // Try to restore previous selection by name, else keep current logic
    if (!prevName.empty() && !m_files.empty())
    {
        for (size_t i = 0; i < m_files.size(); ++i)
        {
            if (geode::utils::string::toLower(m_files[i]) == geode::utils::string::toLower(prevName))
            {
                m_selectedIdx = static_cast<int>(i);
                break;
            }
        }
    }
    updateFileLabel();
    updateArrowPositions();
}

void JumpscareSettingsPopup::onTestJumpscare(cocos2d::CCObject *)
{
    // Determine selected filename
    if (m_selectedIdx < 0 || m_selectedIdx >= static_cast<int>(m_files.size()))
    {
        geode::createQuickPopup(
            "No Image",
            "No image selected. Add images in the jumpscare folder and select one.",
            "OK",
            nullptr,
            260.f,
            [](FLAlertLayer *, bool) {},
            false,
            false)
            ->show();
        return;
    }

    // Parse fade time from input
    float fade = 0.0f;
    if (m_fadeInput != nullptr)
    {
        auto fadeStr = m_fadeInput->getString();
        if (!fadeStr.empty())
        {
            if (auto parsed = numFromString<float>(fadeStr))
            {
                fade = parsed.unwrap();
            }
        }
    }
    if (fade < 0.0f) fade = 0.0f;

    // Parse scale multiplier
    float scaleMul = 1.0f;
    if (m_scaleInput != nullptr)
    {
        auto scaleStr = m_scaleInput->getString();
        if (!scaleStr.empty())
        {
            if (auto parsed = numFromString<float>(scaleStr))
            {
                scaleMul = parsed.unwrap();
            }
        }
    }
    if (scaleMul <= 0.0f) scaleMul = 1.0f;

    // Build absolute path
    auto base = Mod::get()->getConfigDir();
    auto absPath = (base / "jumpscare" / m_files[m_selectedIdx]).string();

    // Create a temporary layer and a screen-sized LazySprite like in TwitchCommandManager
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (scene == nullptr)
    {
        return;
    }

    auto layer = CCLayerColor::create({0, 0, 0, 0});
    layer->setID("jumpscare-layer");
    scene->addChild(layer, 9999);

    auto win = CCDirector::sharedDirector()->getWinSize();
    auto ls = geode::LazySprite::create({win.width, win.height}, true);
    if (!ls)
    {
        geode::createQuickPopup(
            "Load Failed",
            "Could not create LazySprite.",
            "OK",
            nullptr,
            240.f,
            [](FLAlertLayer *, bool) {},
            false,
            false)
            ->show();
        layer->removeFromParentAndCleanup(true);
        return;
    }
    ls->setID("jumpscare-image");
    ls->setAnchorPoint({0.5f, 0.5f});
    ls->setPosition({win.width / 2.f, win.height / 2.f});
    ls->setOpacity(255);
    layer->addChild(ls);

    if (!std::filesystem::exists(absPath))
    {
        log::warn("[Jumpscare Test] Image file does not exist: {}", absPath);
    }
    ls->loadFromFile(absPath);
    // Apply user scale multiplier relative to cover fit: LazySprite auto-fits area, so scale additionally
    ls->setScale(scaleMul);

    // Hold briefly, then fade out over the input fade duration
    float hold = 0.25f;
    float fadeDur = std::max(0.f, fade);

    auto fadeSeq = CCSequence::create(
        CCDelayTime::create(hold),
        CCFadeTo::create(fadeDur, 0),
        CCCallFunc::create(ls, callfunc_selector(CCNode::removeFromParent)),
        nullptr);
    ls->runAction(fadeSeq);

    auto removeLayerSeq = CCSequence::create(
        CCDelayTime::create(hold + fadeDur),
        CCCallFunc::create(layer, callfunc_selector(CCNode::removeFromParent)),
        nullptr);
    layer->runAction(removeLayerSeq);
}

JumpscareSettingsPopup *JumpscareSettingsPopup::create(const std::string &initUrl, float initFade, float initScale, std::function<void(const std::string &, float, float)> onSave)
{
    auto ret = new JumpscareSettingsPopup();
    if (ret)
    {
    ret->m_onSave = onSave;
    ret->m_initUrl = initUrl;
    ret->m_initFade = initFade;
    ret->m_initScale = initScale;
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
