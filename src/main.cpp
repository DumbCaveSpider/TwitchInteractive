#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include "./twitch/TwitchLoginPopup.hpp"

using namespace geode::prelude;

class $modify(MyCreatorLayer, CreatorLayer) {
    bool init() {
        if (!CreatorLayer::init()) return false;

        // Add Twitch button to the CreatorLayer
        auto twitchBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Twitch", "bigFont.fnt", "GJ_button_05.png", 0.6f),
            this,
            menu_selector(MyCreatorLayer::onTwitchPressed)
        );

        // Find an existing menu to add our button to, or create a new one
        auto menu = getChildByID("creator-buttons-menu");
        if (!menu) menu = getChildByID("social-media-menu");

        if (menu) {
            static_cast<CCMenu*>(menu)->addChild(twitchBtn);
            // Position it at the bottom of existing buttons
            twitchBtn->setPosition(0, -60);
        } else {
            // Create our own menu if we can't find existing ones
            auto newMenu = CCMenu::create();
            newMenu->addChild(twitchBtn);
            newMenu->setPosition(CCDirector::sharedDirector()->getWinSize().width - 50, 50);

            addChild(newMenu);
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