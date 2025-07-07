#include "TwitchDashboard.hpp"

#include <Geode/Geode.hpp>
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>

using namespace geode::prelude;

bool TwitchDashboard::setup() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    this->setTitle("Twitch Dashboard");
    auto layerSize = this->m_mainLayer->getContentSize();
    
    // Check if TwitchChatAPI is available
    auto api = TwitchChatAPI::get();
    if (!api) {
        log::error("TwitchChatAPI is not available in TwitchDashboard::setup");
        return false;
    }
    
    // Dashboard is now opened - no need to request token here since 
    // user already went through the full login flow
    log::debug("TwitchDashboard opened successfully");
    
    return true;
}

void TwitchDashboard::onClose(CCObject* sender) {
    Popup::onClose(sender);
}

TwitchDashboard* TwitchDashboard::create() {
    auto ret = new TwitchDashboard();
    if (ret && ret->initAnchored(540.f, 290.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
