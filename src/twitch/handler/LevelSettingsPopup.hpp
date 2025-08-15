#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

class CCMenuItemToggler;

class LevelSettingsPopup : public geode::Popup<>
{
protected:
    TextInput* m_levelInput = nullptr;
    std::string m_value;
    std::function<void(const std::string&, bool)> m_callback;
    CCMenuItemToggler* m_forceToggle = nullptr;
    bool m_forcePlay = false;

    bool setup() override;
    void onOpen(CCObject* sender);
    void onSave(CCObject* sender);
    void onToggleForce(CCObject* sender);

public:
    static LevelSettingsPopup* create(const std::string& value, bool initForce, std::function<void(const std::string&, bool)> callback);
};
