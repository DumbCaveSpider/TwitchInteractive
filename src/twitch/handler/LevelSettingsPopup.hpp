#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

class LevelSettingsPopup : public geode::Popup<>
{
protected:
    TextInput* m_levelInput = nullptr;
    std::string m_value;
    std::function<void(const std::string&)> m_callback;

    bool setup() override;
    void onOpen(CCObject* sender);
    void onSave(CCObject* sender);

public:
    static LevelSettingsPopup* create(const std::string& value, std::function<void(const std::string&)> callback);
};
