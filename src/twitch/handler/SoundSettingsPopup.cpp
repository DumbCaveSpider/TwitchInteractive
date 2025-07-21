#include "SoundSettingsPopup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>

using namespace geode::prelude;
using namespace cocos2d;

// Unified sound list for settings
static std::vector<std::string> getAvailableSounds()
{
    return {
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
        "jumpscareAudio.mp3",
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
    searchBox->setPosition(popupSize.width / 2, popupSize.height - 80.f);
    m_mainLayer->addChild(searchBox);

    // Scroll area setup
    auto scrollSize = CCSize(320, 150);
    auto scrollLayer = ScrollLayer::create(scrollSize);
    scrollLayer->setPosition(popupSize.width / 2 - scrollSize.width / 2, popupSize.height / 2 - scrollSize.height / 2 - 20.f);
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
    auto updateSoundList = [this, scrollLayer, scrollSize](const std::vector<std::string>& sounds) {
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
            auto bg = CCScale9Sprite::create("square02_001.png");
            bg->setContentSize(CCSize(scrollSize.width, nodeHeight));
            bg->setOpacity(60);
            bg->setAnchorPoint({0, 0});
            bg->setPosition(0, 0);
            node->addChild(bg, -1);

            auto labelSprite = CCLabelBMFont::create(sounds[i].c_str(), "bigFont.fnt");
            labelSprite->setScale(0.5f); // Initial scale
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
            playBtn->setAnchorPoint({1, 0.5f});
            playBtn->setPosition(scrollSize.width - 30.f, nodeHeight / 2);

            auto btnMenu = CCMenu::create();
            btnMenu->addChild(labelBtn);
            btnMenu->addChild(playBtn);
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

    // Add save button below the scroll layer
    auto saveBtnSprite = ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f);
    auto saveBtn = CCMenuItemSpriteExtra::create(saveBtnSprite, this, menu_selector(SoundSettingsPopup::onSaveBtn));
    saveBtn->setID("sound-settings-save-btn");

    // Create a btnMenu for the button
    auto btnMenu = CCMenu::create();
    btnMenu->setID("sound-settings-btn-btnMenu");
    btnMenu->addChild(saveBtn);

    // Position the btnMenu centered horizontally, below the scroll layer
    float btnY = scrollLayer->getPositionY() - 30.f;
    btnMenu->setPosition(popupSize.width / 2, btnY);

    m_mainLayer->addChild(btnMenu);

    return true;
}

void SoundSettingsPopup::onTextInput(geode::TextInput* input, const std::string& searchText) {
    auto allSounds = getAvailableSounds();
    std::sort(allSounds.begin(), allSounds.end());
    std::vector<std::string> filtered;
    if (searchText.empty()) {
        filtered = allSounds;
    } else {
        std::string searchLower = searchText;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
        for (const auto& s : allSounds) {
            std::string sLower = s;
            std::transform(sLower.begin(), sLower.end(), sLower.begin(), ::tolower);
            if (sLower.find(searchLower) != std::string::npos)
                filtered.push_back(s);
        }
    }
    if (updateSoundList) updateSoundList(filtered);
}

void SoundSettingsPopup::onSaveBtn(CCObject *)
{
    if (m_onSave)
        m_onSave(m_selectedSound);

    // Update parent CommandSettingsPopup action node and settings label
    if (m_parent && m_actionIdx >= 0)
    {
        // Save selected sound to the action node
        if (m_parent->m_commandActions.size() > static_cast<size_t>(m_actionIdx))
        {
            m_parent->m_commandActions[m_actionIdx] = "sound:" + m_selectedSound;
        }
        // Update the settings text label in the action node
        if (m_parent->m_actionContent)
        {
            auto children = m_parent->m_actionContent->getChildren();
            if (children && m_actionIdx >= 0 && m_actionIdx < children->count())
            {
                auto actionNode = dynamic_cast<CCNode *>(children->objectAtIndex(m_actionIdx));
                if (actionNode)
                {
                    std::string settingsLabelId = "action-settings-label-" + std::to_string(m_actionIdx);
                    if (auto settingsLabel = dynamic_cast<CCLabelBMFont *>(actionNode->getChildByID(settingsLabelId)))
                    {
                        settingsLabel->setString(m_selectedSound.c_str());
                    }
                }
            }
        }
    }
    // Stop the audio pls
    auto audioEngine = FMODAudioEngine::sharedEngine();
    if (audioEngine != nullptr) {
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

// Select sound button handler
void SoundSettingsPopup::onSelectSound(CCObject *sender)
{
    auto btn = dynamic_cast<CCMenuItemSpriteExtra *>(sender);
    if (!btn || !btn->getUserObject())
        return;
    std::string sound = static_cast<CCString *>(btn->getUserObject())->getCString();
    m_selectedSound = sound;

    // Visual feedback: highlight selected button and scale it up
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
                    auto node = dynamic_cast<CCNode *>(children->objectAtIndex(i));
                    if (!node)
                        continue;
                    CCMenu *btnMenu = nullptr;
                    // Try to find btnMenu by iterating children
                    for (int j = 0; j < node->getChildren()->count(); ++j)
                    {
                        auto child = node->getChildren()->objectAtIndex(j);
                        btnMenu = dynamic_cast<CCMenu *>(child);
                        if (btnMenu)
                            break;
                    }
                    if (btnMenu && btnMenu->getChildren())
                    {
                        for (int j = 0; j < btnMenu->getChildren()->count(); ++j)
                        {
                            auto btnChild = dynamic_cast<CCMenuItemSpriteExtra *>(btnMenu->getChildren()->objectAtIndex(j));
                            if (btnChild)
                            {
                                auto btnSound = btnChild->getUserObject() ? static_cast<CCString *>(btnChild->getUserObject())->getCString() : "";
                                // Scale up selected, scale down others
                                if (btnChild->getID().find("sound-play-btn-") == 0)
                                {
                                    // Only scale the play button's sprite
                                    auto playSprite = dynamic_cast<CCSprite *>(btnChild->getNormalImage());
                                }
                                else if (btnChild->getID().find("sound-label-btn-") == 0)
                                {
                                    // Only scale the label sprite
                                    auto labelSprite = dynamic_cast<CCLabelBMFont *>(btnChild->getNormalImage());
                                    if (labelSprite)
                                    {
                                        labelSprite->setColor(btnSound == sound ? ccColor3B{0, 255, 0} : ccColor3B{255, 255, 255});
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Play sound preview handler
void SoundSettingsPopup::onPlaySound(CCObject *sender)
{
    auto btn = dynamic_cast<CCMenuItemSpriteExtra *>(sender);
    if (!btn || !btn->getUserObject())
        return;
    std::string sound = static_cast<CCString *>(btn->getUserObject())->getCString();
    auto audioEngine = FMODAudioEngine::sharedEngine();
    if (audioEngine != nullptr)
    {
        audioEngine->stopAllEffects();
        audioEngine->playEffect(sound);
    }
}

SoundSettingsPopup *SoundSettingsPopup::create(CommandSettingsPopup *parent, int actionIdx, const std::string &selectedSound, std::function<void(const std::string &)> onSave)
{
    auto ret = new SoundSettingsPopup();
    ret->m_parent = parent;
    ret->m_actionIdx = actionIdx;
    ret->m_selectedSound = selectedSound;
    ret->m_onSave = onSave;
    if (ret && ret->initAnchored(400.f, 280.f))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
