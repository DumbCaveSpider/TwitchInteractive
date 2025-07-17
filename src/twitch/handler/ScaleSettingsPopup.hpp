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
    std::function<void(float)> m_onSave;
    float m_scaleValue = 1.0f;

    bool setup() override;
    void onSaveBtn(CCObject*);

public:
    static ScaleSettingsPopup* create(CommandSettingsPopup* parent, int actionIdx, float scaleValue, std::function<void(float)> onSave);
};
