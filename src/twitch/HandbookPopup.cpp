#include "HandbookPopup.hpp"
#include <Geode/Geode.hpp>


bool HandbookPopup::setup() {
    setTitle("Twitch Interactive Handbook");
    setID("handbook-popup");
    float width = 480.f;
    float height = 300.f;
    this->setContentSize({width, height});

    // Centered button menu
    float btnY = height / 2;
    float btnX = width / 2;
    auto eventsBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Events", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(HandbookPopup::onEventsBtn)
    );
    eventsBtn->setID("handbook-events-btn");
    eventsBtn->setPosition({btnX, btnY});

    auto menu = CCMenu::create();
    menu->addChild(eventsBtn);
    menu->setPosition(0, 0);
    m_mainLayer->addChild(menu);

    return true;
}

void HandbookPopup::onEventsBtn(CCObject*) {
    std::string md =
        "# Events\n\n"
        "Event are actions that can be added to the command.\n\n"
        "- Each event represents a specific in-game action (e.g., jump, kill player, keycode).\n"
        "- You can add, remove, and reorder event nodes in the command settings.\n"
        "- Some events accept arguments, such as which player to affect.\n\n"
        "**Tip:** Use `${arg}` in event arguments to let chat users provide their own values!";
    geode::MDPopup::create("Events Help", md, "OK", nullptr, [] (bool) {})->show();
}

HandbookPopup* HandbookPopup::create() {
    auto ret = new HandbookPopup();
    if (ret && ret->initAnchored(480.f, 290.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
