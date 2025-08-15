#include "LevelSettingsPopup.hpp"
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/utils/ranges.hpp>

bool LevelSettingsPopup::setup()
{
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

    // Force Play checkbox at bottom-left
    auto checkboxMenu = CCMenu::create();
    checkboxMenu->setPosition(0, 0);

    float leftPad = 20.f;
    float bottomPad = 18.f;
    m_forceToggle = CCMenuItemToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        this,
        menu_selector(LevelSettingsPopup::onToggleForce));
    m_forceToggle->setScale(0.6f);
    m_forceToggle->setID("force-play-toggle");
    m_forceToggle->setPosition(leftPad, bottomPad);
    checkboxMenu->addChild(m_forceToggle);

    auto forceLabel = CCLabelBMFont::create("Force\nPlay", "bigFont.fnt");
    forceLabel->setScale(0.35f);
    forceLabel->setAnchorPoint({0.f, 0.5f});
    forceLabel->setPosition(leftPad + (m_forceToggle->getContentSize().width * m_forceToggle->getScale()) / 2.f + 5.f, bottomPad);
    m_mainLayer->addChild(forceLabel);
    m_mainLayer->addChild(checkboxMenu);

    // Restore initial state if set via create
    if (m_forcePlay && m_forceToggle)
        m_forceToggle->toggle(true);

    return true;
}

void LevelSettingsPopup::onOpen(CCObject *sender)
{
    std::string query = m_levelInput ? m_levelInput->getString() : m_value;
    query.erase(0, query.find_first_not_of(" \t\n\r"));
    if (!query.empty())
        query.erase(query.find_last_not_of(" \t\n\r") + 1);

    if (query.empty())
    {
        Notification::create("Enter a level ID", NotificationIcon::Warning, 1.5f)->show();
        return;
    }

    // If numeric -> fetch by ID
    auto glm = GameLevelManager::sharedState();
    if (!glm)
    {
        Notification::create("Level manager unavailable", NotificationIcon::Error, 1.5f)->show();
        return;
    }

    auto showLevel = [](GJGameLevel *lvl)
    {
        if (!lvl)
            return false;
        if (auto scene = LevelInfoLayer::scene(lvl, false))
        {
            CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.3f, scene));
            return true;
        }
        return false;
    };

    if (query.find_first_not_of("0123456789") == std::string::npos)
    {
        int levelID = numFromString<int>(query).unwrapOrDefault();
        if (levelID <= 0)
        {
            Notification::create("Invalid level ID", NotificationIcon::Error, 1.5f)->show();
            return;
        }
        if (m_forceToggle && m_forceToggle->isOn())
        {
            // Confirm destructive action first
            geode::createQuickPopup(
                "Force Play",
                "Opening with <cy>Force Play</c> will <cr>discard any unsaved changes in this command and may crash.</c> <cg>Continue?</c>",
                "No",
                "Yes",
                320.f,
                [this, levelID, glm](FLAlertLayer *, bool btn)
                {
                    if (!btn)
                        return; // No
                    // proceed with force play
                    Notification::create("Preparing level...", NotificationIcon::Loading, 1.0f)->show();
                    auto so = GJSearchObject::create(SearchType::Search, std::to_string(levelID));
                    glm->getOnlineLevels(so);
                    struct ForcePlayRunner : public CCNode
                    {
                        GameLevelManager *m_glm = nullptr;
                        int m_levelID = 0;
                        int m_attempts = 0;
                        int m_maxAttempts = 100;
                        static ForcePlayRunner *create(GameLevelManager *glm, int id)
                        {
                            auto n = new ForcePlayRunner();
                            n->m_glm = glm;
                            n->m_levelID = id;
                            n->autorelease();
                            return n;
                        }
                        void onTick(float)
                        {
                            if (!m_glm)
                            {
                                cleanupSelf();
                                return;
                            }
                            if (auto lvl = m_glm->getSavedLevel(m_levelID))
                            {
                                if (auto scene = PlayLayer::scene(lvl, false, false))
                                    CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.3f, scene));
                                cleanupSelf();
                                return;
                            }
                            if (++m_attempts >= m_maxAttempts)
                            {
                                Notification::create("Level fetch timeout", NotificationIcon::Error, 1.5f)->show();
                                cleanupSelf();
                            }
                        }
                        void cleanupSelf()
                        {
                            this->unschedule(schedule_selector(ForcePlayRunner::onTick));
                            this->removeFromParentAndCleanup(true);
                        }
                    };
                    if (auto scene = CCDirector::sharedDirector()->getRunningScene())
                    {
                        auto runner = ForcePlayRunner::create(glm, levelID);
                        scene->addChild(runner);
                        runner->schedule(schedule_selector(ForcePlayRunner::onTick), 0.1f);
                    }
                },
                false,
                false)
                ->show();
            return;
        }
        else
        {
            if (auto lvl = glm->getSavedLevel(levelID))
            {
                if (showLevel(lvl))
                    return;
            }
            // fun fact, you can get the main level of gd using this XDDD
            if (auto lvl = glm->getMainLevel(levelID, true))
            {
                if (showLevel(lvl))
                    return;
            }
            // Fallback: search by ID as text
            auto so = GJSearchObject::create(SearchType::Search, std::to_string(levelID));
            glm->getOnlineLevels(so);
            Notification::create("Fetching level...", NotificationIcon::Loading, 1.0f)->show();
            return;
        }
    }

    Notification::create("Please provide a numeric level ID", NotificationIcon::Warning, 1.5f)->show();
}

void LevelSettingsPopup::onSave(CCObject *sender)
{
    std::string newVal = m_levelInput ? m_levelInput->getString() : m_value;
    if (m_callback)
        m_callback(newVal, m_forceToggle && m_forceToggle->isOn());
    onClose(sender);
}

void LevelSettingsPopup::onToggleForce(CCObject *sender)
{
    m_forcePlay = m_forceToggle && m_forceToggle->isOn();
}

LevelSettingsPopup *LevelSettingsPopup::create(const std::string &value, bool initForce, std::function<void(const std::string &, bool)> callback)
{
    auto ret = new LevelSettingsPopup();
    ret->m_value = value;
    ret->m_callback = callback;
    ret->m_forcePlay = initForce;
    if (ret && ret->initAnchored(260.f, 120.f))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
