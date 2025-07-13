#include "HandbookPopup.hpp"
#include <Geode/Geode.hpp>

bool HandbookPopup::setup() {
    setTitle("Twitch Interactive Handbook");
    setID("handbook-popup");
    float width = 480.f;
    float height = 300.f;
    this->setContentSize({width, height});

    return true;
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
