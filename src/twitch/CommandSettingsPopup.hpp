#pragma once
#include <Geode/ui/Popup.hpp>
#include "TwitchCommandManager.hpp"

// specify parameters for the setup function in the Popup<...> template
class CommandSettingsPopup : public geode::Popup<TwitchCommand> {
protected:
    TextInput* m_notificationInput = nullptr;
    TwitchCommand m_command = TwitchCommand("", "", "", 0); // Provide default constructor values
    bool setup(TwitchCommand command) override;
    void onSave(CCObject* sender);

public:
    static CommandSettingsPopup* create(TwitchCommand command);
    std::string getNotificationText() const;
};
