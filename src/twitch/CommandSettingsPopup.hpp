#pragma once
#include <Geode/ui/Popup.hpp>
#include "TwitchCommandManager.hpp"

// specify parameters for the setup function in the Popup<...> template
class CommandSettingsPopup : public geode::Popup<TwitchCommand> {
protected:
    // setup function receives the parameters specified in the template
    bool setup(TwitchCommand command) override;

public:
    static CommandSettingsPopup* create(TwitchCommand command);
};
