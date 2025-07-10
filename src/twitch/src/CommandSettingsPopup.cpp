#include "../CommandSettingsPopup.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;


bool CommandSettingsPopup::setup(TwitchCommand command) {
    this->setTitle(fmt::format("!{} settings", command.name));
    this->setID("command-settings-popup");

    return true;
};

CommandSettingsPopup* CommandSettingsPopup::create(TwitchCommand command) {
    auto ret = new CommandSettingsPopup();
    // initAnchored takes width, height, and then the parameters for setup
    if (ret && ret->initAnchored(300.f, 200.f, command)) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}