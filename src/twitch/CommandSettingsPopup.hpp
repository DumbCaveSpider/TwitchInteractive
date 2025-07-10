#pragma once
#include <Geode/ui/Popup.hpp>
#include "TwitchCommandManager.hpp"

// specify parameters for the setup function in the Popup<...> template
class CommandSettingsPopup : public geode::Popup<TwitchCommand> {
protected:
    // setup function receives the parameters specified in the template
    bool setup(TwitchCommand command) override;

public:
    static CommandSettingsPopup* create(TwitchCommand command) {
        auto ret = new CommandSettingsPopup();
        // initAnchored takes width, height, and then the parameters for setup
        if (ret && ret->initAnchored(300.f, 200.f, command)) {
            ret->autorelease();
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
