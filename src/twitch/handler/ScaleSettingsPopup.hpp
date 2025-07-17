#pragma once
#include <Geode/Geode.hpp>
#include "../command/CommandSettingsPopup.hpp"

using namespace geode::prelude;


class ScaleSettingsPopup : public geode::Popup<>
{
protected:
    CommandSettingsPopup* m_parent = nullptr;
    int m_actionIdx = 0;
    TextInput* m_scaleInput = nullptr;
    TextInput* m_timeInput = nullptr;
    std::function<void(float, float)> m_onSave;
    float m_scaleValue = 1.0f;
    float m_timeValue = 0.5f;

    bool setup() override;
    void onSaveBtn(CCObject*);

public:
    static ScaleSettingsPopup* create(CommandSettingsPopup* parent, int actionIdx, float scaleValue, float timeValue, std::function<void(float, float)> onSave);
};
