#include "SoundSettingsPopup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/loader/Dirs.hpp>
#include <algorithm>
#include <filesystem>

using namespace geode::prelude;
using namespace cocos2d;

// @geode-ignore-all(unknown-resource)

// Unified sound list for settings
static std::vector<std::string> getAvailableSounds()
{
    std::vector<std::string> sounds = {
        "BackOnTrack.mp3",
        "BaseAfterBase.mp3",
        "BlastProcessing.mp3",
        "CantLetGo.mp3",
        "chestClick.ogg",
        "chestLand.ogg",
        "Clubstep.mp3",
        "Clutterfunk.mp3",
        "Cycles.mp3",
        "dangerLoop.mp3",
        "Dash.mp3",
        "Deadlocked.mp3",
        "DJRubRub.mp3",
        "DryOut.mp3",
        "Electrodynamix.mp3",
        "Electroman.mp3",
        "Fingerdash.mp3",
        "GeometricalDominator.mp3",
        "HexagonForce.mp3",
        "Jumper.mp3",
        "jumpscareAudio.mp3", // Funny jumpscare sound
        "magicExplosion.ogg",
        "menuLoop.mp3",
        "Polargeist.mp3",
        "PowerTrip.mp3",
        "secretKey.ogg",
        "secretLoop.mp3",
        "secretLoop02.mp3",
        "secretLoop03.mp3",
        "secretLoop04.ogg",
        "secretShop.mp3",
        "shop.mp3",
        "shop3.mp3",
        "shop4.mp3",
        "shop5.mp3",
        "StayInsideMe.mp3",
        "StereoMadness.mp3",
        "TheoryOfEverything.mp3",
        "TheoryOfEverything2.mp3",
        "TimeMachine.mp3",
        "tower01.mp3",
        "unlockPath.ogg",
        "xStep.mp3",
        "achievement_01.ogg",
        "buyItem01.ogg",
        "buyItem03.ogg",
        "chest07.ogg",
        "chest08.ogg",
        "chestOpen01.ogg",
        "counter003.ogg",
        "crystal01.ogg",
        "door001.ogg",
        "door01.ogg",
        "door02.ogg",
        "endStart_02.ogg",
        "explode_11.ogg",
        "gold01.ogg",
        "gold02.ogg",
        "grunt01.ogg",
        "grunt02.ogg",
        "grunt03.ogg",
        "highscoreGet02.ogg",
        "playSound_01.ogg",
        "quitSound_01.ogg",
        "reward01.ogg",
    };

    // Add user-provided sfx from the mod config directory (mod-id/sfx subfolder)
    auto sfxDir = geode::dirs::getModConfigDir() / "arcticwoof.twitch_interactive" / "sfx";
    std::error_code ec;
    if (std::filesystem::exists(sfxDir, ec))
    {
        for (auto it = std::filesystem::directory_iterator(sfxDir, ec);
             !ec && it != std::filesystem::end(it);
             it.increment(ec))
        {
            const auto &entry = *it;
            if (!entry.is_regular_file(ec))
                continue;
            auto path = entry.path();
            std::string ext = path.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".mp3" || ext == ".ogg")
            {
                std::string name = path.filename().string();
                if (std::find(sounds.begin(), sounds.end(), name) == sounds.end())
                    sounds.push_back(name);
            }
        }
    }

    return sounds;
}

// Resolve a sound name to a full path if it exists in the sfx folder
static std::string resolveSfxPath(const std::string &name)
{
    std::error_code ec;
    if (std::filesystem::exists(name, ec))
        return name; // already a valid path

    auto p = geode::dirs::getModConfigDir() / "arcticwoof.twitch_interactive" / "sfx" / name;
    if (std::filesystem::exists(p, ec))
        return p.string();

    return name; // fall back to original (builtin resource)
}

bool SoundSettingsPopup::setup()
{
    setTitle("Sound Effect Settings");
    setID("sound-effect-settings-popup");

    // Unified popup centering and main layer creation
    auto popupSize = getContentSize();
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    setPosition(CCPoint(winSize.width / 2 - popupSize.width / 2, winSize.height / 2 - popupSize.height / 2));

    m_mainLayer = CCLayer::create();
    m_mainLayer->setContentSize(popupSize);
    m_mainLayer->setAnchorPoint({0, 0});
    m_mainLayer->setPosition(0, 0);

    this->m_noElasticity = true;
    addChild(m_mainLayer);

    std::vector<std::string> filteredSounds = getAvailableSounds();
    std::sort(filteredSounds.begin(), filteredSounds.end());
    setTitle("Sound Effect Settings");
    setID("sound-effect-settings-popup");

    // Add search box above the scroll layer
    auto searchBox = TextInput::create(320, "Search sounds");
    searchBox->setID("sound-search-box");
    searchBox->setPosition(popupSize.width / 2, popupSize.height - 70.f);
    m_mainLayer->addChild(searchBox);

    m_soundSearchInput = searchBox;

    // Create text inputs for advanced sound options
    auto speedInput = TextInput::create(100, "Speed", "bigFont.fnt");
    speedInput->setID("sound-speed-input");
    speedInput->setCommonFilter(CommonFilter::Float);
    speedInput->setPosition(popupSize.width - 100.f, popupSize.height - 70.f);
    m_mainLayer->addChild(speedInput);

    auto volumeInput = TextInput::create(100, "Volume", "bigFont.fnt");
    volumeInput->setID("sound-volume-input");
    volumeInput->setCommonFilter(CommonFilter::Float);
    volumeInput->setPosition(popupSize.width - 100.f, popupSize.height - 120.f);
    m_mainLayer->addChild(volumeInput);

    auto pitchInput = TextInput::create(100, "Pitch", "bigFont.fnt");
    pitchInput->setID("sound-pitch-input");
    pitchInput->setCommonFilter(CommonFilter::Float);
    pitchInput->setPosition(popupSize.width - 100.f, popupSize.height - 170.f);
    m_mainLayer->addChild(pitchInput);

    auto startMillisInput = TextInput::create(100, "Start (ms)", "bigFont.fnt");
    startMillisInput->setID("sound-startmillis-input");
    startMillisInput->setCommonFilter(CommonFilter::Int);
    startMillisInput->setPosition(popupSize.width - 100.f, popupSize.height - 220.f);
    m_mainLayer->addChild(startMillisInput);

    auto endMillisInput = TextInput::create(100, "End (ms)", "bigFont.fnt");
    endMillisInput->setID("sound-endmillis-input");
    endMillisInput->setCommonFilter(CommonFilter::Int);
    endMillisInput->setPosition(popupSize.width - 100.f, popupSize.height - 270.f);
    m_mainLayer->addChild(endMillisInput);

    // Add labels for text inputs
    auto speedLabel = CCLabelBMFont::create("Speed", "bigFont.fnt");
    speedLabel->setPosition(speedInput->getPositionX(), speedInput->getPositionY() + 25.f);
    m_mainLayer->addChild(speedLabel);

    auto volumeLabel = CCLabelBMFont::create("Volume", "bigFont.fnt");
    volumeLabel->setPosition(volumeInput->getPositionX(), volumeInput->getPositionY() + 25.f);
    m_mainLayer->addChild(volumeLabel);

    auto pitchLabel = CCLabelBMFont::create("Pitch", "bigFont.fnt");
    pitchLabel->setPosition(pitchInput->getPositionX(), pitchInput->getPositionY() + 25.f);
    m_mainLayer->addChild(pitchLabel);

    auto startMillisLabel = CCLabelBMFont::create("Start (ms)", "bigFont.fnt");
    startMillisLabel->setPosition(startMillisInput->getPositionX(), startMillisInput->getPositionY() + 25.f);
    m_mainLayer->addChild(startMillisLabel);

    auto endMillisLabel = CCLabelBMFont::create("End (ms)", "bigFont.fnt");
    endMillisLabel->setPosition(endMillisInput->getPositionX(), endMillisInput->getPositionY() + 25.f);
    m_mainLayer->addChild(endMillisLabel);

    speedLabel->setScale(0.5f);
    volumeLabel->setScale(0.5f);
    pitchLabel->setScale(0.5f);
    startMillisLabel->setScale(0.5f);
    endMillisLabel->setScale(0.5f);

    speedInput->setPositionY(speedInput->getPositionY());
    volumeInput->setPositionY(volumeInput->getPositionY());
    pitchInput->setPositionY(pitchInput->getPositionY());
    startMillisInput->setPositionY(startMillisInput->getPositionY());
    endMillisInput->setPositionY(endMillisInput->getPositionY());

    // Prefill fields from existing action string if available
    {
        float speed = 1.0f;
        float volume = 1.0f;
        float pitch = 0.0f;
        int startMillis = 0;
        int endMillis = 0;

        std::string actionIdRaw;
        if (m_parent && m_actionIdx >= 0 && m_actionIdx < static_cast<int>(m_parent->m_commandActions.size()))
            actionIdRaw = m_parent->m_commandActions[m_actionIdx];

        size_t firstColon = actionIdRaw.find(":");
        if (firstColon != std::string::npos && firstColon + 1 < actionIdRaw.size())
        {
            std::string rest = actionIdRaw.substr(firstColon + 1);
            std::vector<std::string> parts;
            size_t start = 0;
            while (true)
            {
                size_t pos = rest.find(":", start);
                if (pos == std::string::npos)
                {
                    parts.push_back(rest.substr(start));
                    break;
                }
                parts.push_back(rest.substr(start, pos - start));
                start = pos + 1;
            }

            auto trim = [](std::string s)
            {
                s.erase(0, s.find_first_not_of(" \t\n\r"));
                s.erase(s.find_last_not_of(" \t\n\r") + 1);
                return s;
            };
            for (auto &p : parts)
                p = trim(p);

            if (!parts.empty())
            {
                if (m_selectedSound.empty())
                    m_selectedSound = parts[0];
                if (parts.size() >= 2)
                    speed = numFromString<float>(parts[1]).unwrapOrDefault();
                if (parts.size() >= 3)
                    volume = numFromString<float>(parts[2]).unwrapOrDefault();
                if (parts.size() >= 4)
                    pitch = numFromString<float>(parts[3]).unwrapOrDefault();
                if (parts.size() >= 5)
                    startMillis = numFromString<int>(parts[4]).unwrapOrDefault();
                if (parts.size() >= 6)
                    endMillis = numFromString<int>(parts[5]).unwrapOrDefault();
            }
        }

        speedInput->setString(fmt::format("{:.2f}", speed));
        volumeInput->setString(fmt::format("{:.2f}", volume));
        pitchInput->setString(fmt::format("{:.2f}", pitch));
        startMillisInput->setString(fmt::format("{}", startMillis));
        endMillisInput->setString(fmt::format("{}", endMillis));
    }

    // Scroll area setup
    auto scrollSize = CCSize(320, 150);
    auto scrollLayer = ScrollLayer::create(scrollSize);
    scrollLayer->setPosition(popupSize.width / 2 - scrollSize.width / 2, popupSize.height / 2 - scrollSize.height / 2 - 10.f);
    scrollLayer->setID("sound-scroll");
    scrollLayer->setTouchPriority(-100);

    auto scrollBg = CCScale9Sprite::create("square02_001.png");
    scrollBg->setID("sound-scroll-background");
    scrollBg->setContentSize(scrollSize);
    scrollBg->setOpacity(50);
    scrollBg->setScale(1.05f);
    scrollBg->setAnchorPoint({0.5f, 0.5f});
    scrollBg->setPosition(scrollLayer->getPositionX() + scrollSize.width / 2, scrollLayer->getPositionY() + scrollSize.height / 2);
    m_mainLayer->addChild(scrollBg, -1);

    m_mainLayer->addChild(scrollLayer);

    // Add sound nodes to the scroll layer
    auto updateSoundList = [this, scrollLayer, scrollSize](const std::vector<std::string> &sounds)
    {
        float nodeHeight = 36.f;
        float nodeGap = 8.f;
        float contentHeight = std::max(scrollSize.height, (nodeHeight + nodeGap) * sounds.size());
        auto contentLayer = CCLayer::create();
        contentLayer->setContentSize(CCSize(scrollSize.width, contentHeight));
        contentLayer->setAnchorPoint({0, 0});
        contentLayer->setPosition(0, 0);

        float y = contentHeight - nodeHeight / 2;
        float centerX = scrollSize.width / 2;
        for (size_t i = 0; i < sounds.size(); ++i)
        {
            auto node = CCNode::create();
            node->setContentSize(CCSize(scrollSize.width, nodeHeight));
            node->setAnchorPoint({0.5f, 0.5f});
            node->setPosition(centerX, y);

            // Background
            auto bg = CCScale9Sprite::create("square02_small.png");
            bg->setContentSize(CCSize(scrollSize.width, nodeHeight));
            bg->setOpacity(60);
            bg->setAnchorPoint({0, 0});
            bg->setPosition(0, 0);
            node->addChild(bg, -1);

            auto labelSprite = CCLabelBMFont::create(sounds[i].c_str(), "bigFont.fnt");
            labelSprite->setScale(0.5f); // Initial scale
            // Highlight preselected sound (if any)
            if (!this->m_selectedSound.empty() && this->m_selectedSound == sounds[i])
            {
                labelSprite->setColor({0, 255, 0});
            }
            auto labelBtn = CCMenuItemSpriteExtra::create(
                labelSprite,
                this,
                menu_selector(SoundSettingsPopup::onSelectSound));
            labelBtn->setID("sound-label-btn-" + std::to_string(i));
            labelBtn->setUserObject(CCString::create(sounds[i]));
            labelBtn->setAnchorPoint({0, 0.5f});
            labelBtn->setPosition(18.f, nodeHeight / 2);

            // Play button for preview
            auto playSprite = CCSprite::createWithSpriteFrameName("GJ_musicOnBtn_001.png");
            if (!playSprite)
                playSprite = CCSprite::create("GJ_musicOnBtn_001.png");
            playSprite->setScale(0.7f); // Initial scale
            auto playBtn = CCMenuItemSpriteExtra::create(
                playSprite,
                this,
                menu_selector(SoundSettingsPopup::onPlaySound));
            playBtn->setID("sound-play-btn-" + std::to_string(i));
            playBtn->setUserObject(CCString::create(sounds[i]));
            playBtn->setAnchorPoint({0.5, 0.5f});
            playBtn->setPosition(scrollSize.width - 30.f, nodeHeight / 2);

            // Select button for selecting sound
            auto selectSprite = CCSprite::createWithSpriteFrameName("GJ_backBtn_001.png");
            if (!selectSprite)
                selectSprite = CCSprite::create("GJ_backBtn_001.png");
            selectSprite->setScale(0.55f); // Initial scale
            auto selectBtn = CCMenuItemSpriteExtra::create(
                selectSprite,
                this,
                menu_selector(SoundSettingsPopup::onSelectSound));
            selectBtn->setID("sound-select-btn-" + std::to_string(i));
            selectBtn->setUserObject(CCString::create(sounds[i]));
            selectBtn->setAnchorPoint({0.5, 0.5f});
            selectBtn->setPosition(scrollSize.width - 70.f, nodeHeight / 2);

            auto btnMenu = CCMenu::create();
            btnMenu->addChild(labelBtn);
            btnMenu->addChild(playBtn);
            btnMenu->addChild(selectBtn);
            btnMenu->setPosition(0, 0);
            btnMenu->setContentSize(node->getContentSize());
            node->addChild(btnMenu);

            contentLayer->addChild(node);
            y -= (nodeHeight + nodeGap);
        }

        scrollLayer->m_contentLayer->removeAllChildren();
        scrollLayer->m_contentLayer->addChild(contentLayer);
        scrollLayer->m_contentLayer->setContentSize(contentLayer->getContentSize());
        scrollLayer->scrollToTop();
    };
    updateSoundList(filteredSounds);

    // Store update function for later use
    this->updateSoundList = updateSoundList;

    // Polling approach for search input
    this->schedule(schedule_selector(SoundSettingsPopup::onSoundSearchPoll), 0.1f);
    m_lastSoundSearchString = "";

    // Add Custom SFX button (bottom-left) and Save button (center below scroll)
    auto customBtnSprite = ButtonSprite::create("Custom SFX", "bigFont.fnt", "GJ_button_01.png", 0.5f);
    auto customBtn = CCMenuItemSpriteExtra::create(customBtnSprite, this, menu_selector(SoundSettingsPopup::onOpenCustomSfx));
    customBtn->setID("sound-custom-sfx-btn");

    auto saveBtnSprite = ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f);
    auto saveBtn = CCMenuItemSpriteExtra::create(saveBtnSprite, this, menu_selector(SoundSettingsPopup::onSaveBtn));
    saveBtn->setID("sound-settings-save-btn");

    // Create a btnMenu for the button
    auto btnMenu = CCMenu::create();
    btnMenu->setID("sound-settings-btn-btnMenu");
    btnMenu->addChild(customBtn);
    btnMenu->addChild(saveBtn);

    // Position the btnMenu centered horizontally, below the scroll layer
    float btnY = scrollLayer->getPositionY() - 25.f;
    // Place custom on bottom-left, save centered
    customBtn->setPosition({35.f, btnY});
    customBtn->setAnchorPoint({0, 0.5f});
    saveBtn->setPosition({popupSize.width / 2, btnY});
    btnMenu->setPosition(0, 0);

    m_mainLayer->addChild(btnMenu);

    searchBox->setPosition(searchBox->getPositionX() - 70.f, searchBox->getPositionY());
    scrollBg->setPosition(scrollBg->getPositionX() - 70.f, scrollBg->getPositionY());
    scrollLayer->setPosition(scrollLayer->getPositionX() - 70.f, scrollLayer->getPositionY());

    return true;
}

void SoundSettingsPopup::onSoundSearchPoll(float)
{
    if (!m_soundSearchInput || !updateSoundList)
        return;
    std::string text = m_soundSearchInput->getString();
    if (text != m_lastSoundSearchString)
    {
        auto allSounds = getAvailableSounds();
        std::sort(allSounds.begin(), allSounds.end());
        std::vector<std::string> filtered;
        if (text.empty())
        {
            filtered = allSounds;
        }
        else
        {
            std::string searchLower = text;
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
            for (const auto &s : allSounds)
            {
                std::string sLower = s;
                std::transform(sLower.begin(), sLower.end(), sLower.begin(), ::tolower);
                if (sLower.find(searchLower) != std::string::npos)
                    filtered.push_back(s);
            }
        }
        updateSoundList(filtered);
        m_lastSoundSearchString = text;
    }
}

void SoundSettingsPopup::onSaveBtn(CCObject *)
{
    // Retrieve text inputs
    auto speedInput = typeinfo_cast<TextInput *>(m_mainLayer->getChildByID("sound-speed-input"));
    auto volumeInput = typeinfo_cast<TextInput *>(m_mainLayer->getChildByID("sound-volume-input"));
    auto pitchInput = typeinfo_cast<TextInput *>(m_mainLayer->getChildByID("sound-pitch-input"));
    auto startMillisInput = typeinfo_cast<TextInput *>(m_mainLayer->getChildByID("sound-startmillis-input"));
    auto endMillisInput = typeinfo_cast<TextInput *>(m_mainLayer->getChildByID("sound-endmillis-input"));

    // Check if any input is empty
    if (!speedInput || speedInput->getString().empty() ||
        !volumeInput || volumeInput->getString().empty() ||
        !pitchInput || pitchInput->getString().empty() ||
        !startMillisInput || startMillisInput->getString().empty() ||
        !endMillisInput || endMillisInput->getString().empty())
    {
        Notification::create("Please fill in all fields before saving.", NotificationIcon::Error)->show();
        return;
    }

    // Parse values
    float speed = numFromString<float>(speedInput->getString()).unwrapOrDefault();
    float volume = numFromString<float>(volumeInput->getString()).unwrapOrDefault();
    float pitch = numFromString<float>(pitchInput->getString()).unwrapOrDefault();
    int startMillis = numFromString<int>(startMillisInput->getString()).unwrapOrDefault();
    int endMillis = numFromString<int>(endMillisInput->getString()).unwrapOrDefault();

    // Build param part for action string: <sound>:<speed>:<volume>:<pitch>:<start>:<end>
    std::string paramPart = fmt::format("{}:{:.2f}:{:.2f}:{:.2f}:{}:{}", m_selectedSound, speed, volume, pitch, startMillis, endMillis);

    if (m_onSave)
        m_onSave(paramPart);

    // Update parent CommandSettingsPopup action node and settings label
    if (m_parent && m_actionIdx >= 0)
    {
        // Save selected sound with params to the action node
        if (m_parent->m_commandActions.size() > static_cast<size_t>(m_actionIdx))
        {
            m_parent->m_commandActions[m_actionIdx] = "sound_effect:" + paramPart;
        }
        // Update the settings text label in the action node
        if (m_parent->m_actionContent)
        {
            auto children = m_parent->m_actionContent->getChildren();
            if (children && m_actionIdx >= 0 && m_actionIdx < children->count())
            {
                auto actionNode = typeinfo_cast<CCNode *>(children->objectAtIndex(m_actionIdx));
                if (actionNode)
                {
                    std::string settingsLabelId = "action-settings-label-" + std::to_string(m_actionIdx);
                    if (auto settingsLabel = typeinfo_cast<CCLabelBMFont *>(actionNode->getChildByID(settingsLabelId)))
                    {
                        auto summary = fmt::format("{} | Speed {:.2f}, Volume {:.2f}, Pitch {:.2f}, {}-{} ms", m_selectedSound, speed, volume, pitch, startMillis, endMillis);
                        settingsLabel->setString(summary.c_str());
                    }
                }
            }
        }
    }
    // Stop the audio pls
    auto audioEngine = FMODAudioEngine::sharedEngine();
    if (audioEngine != nullptr)
    {
        audioEngine->stopAllEffects();
    }
    onClose(nullptr);
}

void SoundSettingsPopup::onClose(CCObject *)
{
    // Stop the audio if playing
    auto audioEngine = FMODAudioEngine::sharedEngine();
    if (audioEngine != nullptr)
    {
        audioEngine->stopAllEffects();
    }
    removeFromParentAndCleanup(true);
}

void SoundSettingsPopup::onOpenCustomSfx(CCObject *)
{
    // Ensure the directory exists then open it
    auto base = geode::dirs::getModConfigDir();
    auto sfx = (base / "arcticwoof.twitch_interactive" / "sfx").string();
    if (!geode::utils::file::createDirectoryAll(sfx))
    {
        log::warn("[SoundSettingsPopup] Failed to create sfx folder: {}", sfx);
    }
    if (auto res = geode::utils::file::openFolder(sfx); !res)
    {
        log::warn("[SoundSettingsPopup] Failed to open sfx folder: {}", sfx);
    }
}

// Select sound button handler
void SoundSettingsPopup::onSelectSound(CCObject *sender)
{
    auto btn = typeinfo_cast<CCMenuItemSpriteExtra *>(sender);
    if (!btn || !btn->getUserObject())
        return;
    std::string sound = static_cast<CCString *>(btn->getUserObject())->getCString();
    m_selectedSound = sound;

    // Find parent content layer and update all buttons
    if (btn->getParent() && btn->getParent()->getParent())
    {
        auto contentLayer = btn->getParent()->getParent()->getParent();
        if (contentLayer)
        {
            auto children = contentLayer->getChildren();
            if (children)
            {
                for (int i = 0; i < children->count(); ++i)
                {
                    auto node = typeinfo_cast<CCNode *>(children->objectAtIndex(i));
                    if (!node)
                        continue;
                    CCMenu *btnMenu = nullptr;
                    // Try to find btnMenu by iterating children
                    for (int j = 0; j < node->getChildren()->count(); ++j)
                    {
                        auto child = node->getChildren()->objectAtIndex(j);
                        btnMenu = typeinfo_cast<CCMenu *>(child);
                        if (btnMenu)
                            break;
                    };

                    if (btnMenu && btnMenu->getChildren())
                    {
                        for (int j = 0; j < btnMenu->getChildren()->count(); ++j)
                        {
                            auto btnChild = typeinfo_cast<CCMenuItemSpriteExtra *>(btnMenu->getChildren()->objectAtIndex(j));
                            if (btnChild)
                            {
                                auto btnSound = btnChild->getUserObject() ? static_cast<CCString *>(btnChild->getUserObject())->getCString() : "";
                                // Scale up selected, scale down others
                                if (btnChild->getID().find("sound-play-btn-") == 0)
                                {
                                    // Only scale the play button's sprite
                                    auto playSprite = typeinfo_cast<CCSprite *>(btnChild->getNormalImage());
                                }
                                else if (btnChild->getID().find("sound-label-btn-") == 0)
                                {
                                    // Only scale the label sprite
                                    auto labelSprite = typeinfo_cast<CCLabelBMFont *>(btnChild->getNormalImage());
                                    if (labelSprite)
                                        labelSprite->setColor(btnSound == sound ? ccColor3B{0, 255, 0} : ccColor3B{255, 255, 255});
                                };
                            };
                        };
                    };
                };
            };
        };
    };
};

// Play sound preview handler
void SoundSettingsPopup::onPlaySound(CCObject *sender)
{
    // Disallow preview while in a level (PlayLayer exists)
    if (PlayLayer::get())
    {
        FLAlertLayer::create("Preview not available", "You can't preview sounds while playing a level.", "OK")->show();
        return;
    }

    auto btn = typeinfo_cast<CCMenuItemSpriteExtra *>(sender);
    if (!btn || !btn->getUserObject())
        return;

    std::string sound = static_cast<CCString *>(btn->getUserObject())->getCString();
    auto audioEngine = FMODAudioEngine::sharedEngine();

    if (audioEngine != nullptr)
    {
        audioEngine->stopAllEffects();

        // Retrieve values from text inputs
        auto speedInput = typeinfo_cast<TextInput *>(m_mainLayer->getChildByID("sound-speed-input"));
        auto volumeInput = typeinfo_cast<TextInput *>(m_mainLayer->getChildByID("sound-volume-input"));
        auto pitchInput = typeinfo_cast<TextInput *>(m_mainLayer->getChildByID("sound-pitch-input"));
        auto startMillisInput = typeinfo_cast<TextInput *>(m_mainLayer->getChildByID("sound-startmillis-input"));
        auto endMillisInput = typeinfo_cast<TextInput *>(m_mainLayer->getChildByID("sound-endmillis-input"));

        float speed = numFromString<float>(speedInput->getString()).unwrapOrDefault();
        float volume = numFromString<float>(volumeInput->getString()).unwrapOrDefault();
        float pitch = numFromString<float>(pitchInput->getString()).unwrapOrDefault();
        int startMillis = numFromString<int>(startMillisInput->getString()).unwrapOrDefault();
        int endMillis = numFromString<int>(endMillisInput->getString()).unwrapOrDefault();

        // Play sound with advanced options (resolve sfx path if needed)
        auto soundPath = resolveSfxPath(sound);
        audioEngine->playEffectAdvanced(soundPath, speed, 0.0f, volume, pitch, false, false, startMillis, endMillis, 0, 0, false, 0, false, false, 0, 0.0f, 0.f, 0);
    }
};

SoundSettingsPopup *SoundSettingsPopup::create(CommandSettingsPopup *parent, int actionIdx, const std::string &selectedSound, std::function<void(const std::string &)> onSave)
{
    auto ret = new SoundSettingsPopup();

    ret->m_parent = parent;
    ret->m_actionIdx = actionIdx;
    ret->m_selectedSound = selectedSound;
    ret->m_onSave = onSave;

    // If selectedSound is empty, try extract from parent's action string for convenience
    if (ret->m_selectedSound.empty() && parent && actionIdx >= 0 && actionIdx < static_cast<int>(parent->m_commandActions.size()))
    {
        std::string raw = parent->m_commandActions[actionIdx];
        size_t c = raw.find(":");
        if (c != std::string::npos && c + 1 < raw.size())
        {
            std::string rest = raw.substr(c + 1);
            size_t nc = rest.find(":");
            ret->m_selectedSound = (nc == std::string::npos) ? rest : rest.substr(0, nc);
        }
    }

    if (ret && ret->initAnchored(520.f, 280.f))
    {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};
