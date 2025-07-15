#include "./twitch/TwitchLoginPopup.hpp"
#include "./twitch/TwitchDashboard.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

class $modify(MyCreatorLayer, CreatorLayer) {
    bool init() {
        if (!CreatorLayer::init()) return false;

        // Add Twitch button to the CreatorLayer using custom PNG sprite
        auto twitchBtnSprite = CCSprite::create("AW_creatortwitchBtn_001.png"_spr);
        auto twitchBtn = CCMenuItemSpriteExtra::create(
            twitchBtnSprite,
            this,
            menu_selector(MyCreatorLayer::onTwitchPressed)
        );
        twitchBtn->setID("twitch-button"_spr);
        // Find an existing menu to add our button to, or create a new one
        auto menu = getChildByID("creator-buttons-menu");
        if (!menu) menu = getChildByID("social-media-menu");

        if (menu) {
            as<CCMenu*>(menu)->addChild(twitchBtn);
            // Position it at the bottom of existing buttons
            twitchBtn->setPosition(0, -60);
        } else {
            // Create our own menu if we can't find existing ones
            auto newMenu = CCMenu::create();
            newMenu->addChild(twitchBtn);
            newMenu->setPosition(CCDirector::sharedDirector()->getWinSize().width - 50, 50);

            addChild(newMenu);
        };

        // login circle button on the side
        if (auto bottomLeftMenu = getChildByID("bottom-left-menu")) {
            auto twitchBtnSprite = CircleButtonSprite::createWithSpriteFrameName(
                "gj_twitchIcon_001.png", // change this to a custom texture
                1.f,
                CircleBaseColor::Green,
                CircleBaseSize::Medium
            );
            twitchBtnSprite->setScale(0.875f);

            auto twitchBtn = CCMenuItemSpriteExtra::create(
                twitchBtnSprite,
                this,
                menu_selector(MyCreatorLayer::onTwitchPressed)
            );
            twitchBtn->setID("login-button"_spr);
            twitchBtn->setZOrder(10);

            bottomLeftMenu->addChild(twitchBtn);
            bottomLeftMenu->updateLayout(true);
        };

        return true;
    };

    void onTwitchPressed(CCObject*) {
        // Check if Twitch Chat API mod is loaded
        auto mod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api");

        if (!mod || !mod->isEnabled()) {
            FLAlertLayer::create(
                "Mod Missing",
                "The <cb>Twitch Chat API</c> mod is required but not found or disabled. Please install and enable it.",
                "OK"
            )->show();
            return;
        };

        auto popup = TwitchLoginPopup::create();
        popup->show();
    };
};


class $modify(MyPauseLayer, PauseLayer) {
    struct Fields {
        CCLabelBMFont* m_twitchStatusLabel = nullptr;
    };

    void customSetup() {
        PauseLayer::customSetup();

        // Add Twitch listening status label
        if (!m_fields->m_twitchStatusLabel) {
            m_fields->m_twitchStatusLabel = CCLabelBMFont::create("Twitch Chat: Listening", "bigFont.fnt");
            m_fields->m_twitchStatusLabel->setAnchorPoint({0.0f, 0.0f});
            m_fields->m_twitchStatusLabel->setID("twitch-status-label"_spr);
            // Bottom left, 10px from left and bottom
            m_fields->m_twitchStatusLabel->setPosition({35.0f, 15.0f});
            m_fields->m_twitchStatusLabel->setScale(0.3f);
            m_fields->m_twitchStatusLabel->setZOrder(100);
            m_fields->m_twitchStatusLabel->setColor({163, 92, 255}); // Twitch purple
            m_fields->m_twitchStatusLabel->setOpacity(50);
            addChild(m_fields->m_twitchStatusLabel);
        }
        updateTwitchStatusLabel();

        // login circle button in right side menu
        if (auto rightMenu = getChildByID("right-button-menu")) {
            auto twitchBtnSprite = CircleButtonSprite::createWithSpriteFrameName(
                "gj_twitchIcon_001.png", // change this to a custom texture
                1.f,
                CircleBaseColor::Green,
                CircleBaseSize::Medium
            );
            twitchBtnSprite->setScale(0.625f);

            auto twitchBtn = CCMenuItemSpriteExtra::create(
                twitchBtnSprite,
                this,
                menu_selector(MyPauseLayer::onTwitchPressed)
            );
            twitchBtn->setID("login-button"_spr);
            twitchBtn->setZOrder(10);

            rightMenu->addChild(twitchBtn);
            rightMenu->updateLayout(true);
        }
    }


    void updateTwitchStatusLabel() {
        if (!m_fields->m_twitchStatusLabel) return;
        if (TwitchDashboard::isListening()) {
            m_fields->m_twitchStatusLabel->setString("Twitch: Listening");
            m_fields->m_twitchStatusLabel->setVisible(true);
        } else {
            m_fields->m_twitchStatusLabel->setVisible(false);
        }
    }

    void onEnter() {
        PauseLayer::onEnter();
        updateTwitchStatusLabel();
    }

    void onTwitchPressed(CCObject*) {
        // Check if Twitch Chat API mod is loaded
        auto mod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api");

        if (!mod || !mod->isEnabled()) {
            FLAlertLayer::create(
                "Mod Missing",
                "The <cb>Twitch Chat API</c> mod is required but not found or disabled. Please install and enable it.",
                "OK"
            )->show();
            return;
        }

        auto popup = TwitchLoginPopup::create();
        popup->show();
    }
};