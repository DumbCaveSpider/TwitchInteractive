#include "LevelSettingsPopup.hpp"
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/utils/ranges.hpp>

bool LevelSettingsPopup::setup() {
    setTitle("Level Info Settings");
    setID("open-level-settings-popup");

    float y = 60.f;
    float x = m_mainLayer->getContentSize().width / 2;

    m_levelInput = TextInput::create(160, "Level ID", "bigFont.fnt");
    m_levelInput->setCommonFilter(CommonFilter::Any);
    m_levelInput->setID("level-input");
    m_levelInput->setString(m_value.c_str());
    m_levelInput->setPosition(x - 20, y + 10);

    m_mainLayer->addChild(m_levelInput);

    this->m_noElasticity = true;

    auto openBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Open", "bigFont.fnt", "GJ_button_05.png", 0.3f),
        this,
        menu_selector(LevelSettingsPopup::onOpen));
    openBtn->setID("level-open-btn");
    openBtn->setPosition(x + 90, y + 10);

    auto openMenu = CCMenu::create();
    openMenu->setPosition(0, 0);
    openMenu->addChild(openBtn);
    m_mainLayer->addChild(openMenu);

    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(LevelSettingsPopup::onSave));
    saveBtn->setID("save-btn");

    auto menu = CCMenu::create();
    menu->setPosition(x, y - 30);
    menu->addChild(saveBtn);
    m_mainLayer->addChild(menu);

    return true;
}

void LevelSettingsPopup::onOpen(CCObject* sender) {
    std::string query = m_levelInput ? m_levelInput->getString() : m_value;
    query.erase(0, query.find_first_not_of(" \t\n\r"));
    if (!query.empty()) query.erase(query.find_last_not_of(" \t\n\r") + 1);

    if (query.empty()) {
        Notification::create("Enter a level ID", NotificationIcon::Warning, 1.5f)->show();
        return;
    }

    // If numeric -> fetch by ID
    auto glm = GameLevelManager::sharedState();
    if (!glm) {
        Notification::create("Level manager unavailable", NotificationIcon::Error, 1.5f)->show();
        return;
    }

    auto showLevel = [](GJGameLevel* lvl) {
        if (!lvl) return false;
        if (auto scene = LevelInfoLayer::scene(lvl, false)) {
            CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.3f, scene));
            return true;
        }
        return false;
    };

    if (query.find_first_not_of("0123456789") == std::string::npos) {
        int levelID = numFromString<int>(query).unwrapOrDefault();
        if (levelID <= 0) {
            Notification::create("Invalid level ID", NotificationIcon::Error, 1.5f)->show();
            return;
        }
        if (auto lvl = glm->getSavedLevel(levelID)) {
            if (showLevel(lvl)) return;
        }
        // fun fact, you can get the main level of gd using this XDDD
        if (auto lvl = glm->getMainLevel(levelID, true)) {
            if (showLevel(lvl)) return;
        }
        // Fallback: search by ID as text
        auto so = GJSearchObject::create(SearchType::Search, std::to_string(levelID));
        glm->getOnlineLevels(so);
        Notification::create("Fetching level...", NotificationIcon::Loading, 1.0f)->show();
        return;
    }

    Notification::create("Please provide a numeric level ID", NotificationIcon::Warning, 1.5f)->show();
}

void LevelSettingsPopup::onSave(CCObject* sender) {
    std::string newVal = m_levelInput ? m_levelInput->getString() : m_value;
    if (m_callback) m_callback(newVal);
    onClose(sender);
}

LevelSettingsPopup* LevelSettingsPopup::create(const std::string& value, std::function<void(const std::string&)> callback) {
    auto ret = new LevelSettingsPopup();
    ret->m_value = value;
    ret->m_callback = callback;
    if (ret && ret->initAnchored(260.f, 120.f)) { ret->autorelease(); return ret; }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
