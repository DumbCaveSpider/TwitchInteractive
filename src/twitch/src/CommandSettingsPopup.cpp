#include "../CommandSettingsPopup.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

bool CommandSettingsPopup::setup(CommandSettingsPopup*) {
    this->setTitle("Command Settings");
    this->setID("command-settings-popup");

    return true;
}
