#pragma once
#include <Geode/Geode.hpp>
#include <functional>

using namespace geode::prelude;

class SpeedSettingsPopup : public geode::Popup<> {
protected:
    int m_actionIdx = 0;
    TextInput* m_speedInput = nullptr;
    TextInput* m_durationInput = nullptr;
    std::function<void(float, float)> m_onSave;
    float m_speed = 1.0f;
    float m_duration = 0.5f;
    CCLayer* m_mainLayer = nullptr;

    bool setup() override;
    void onSaveBtn(CCObject*);

public:
    static SpeedSettingsPopup* create(int actionIdx, float defaultSpeed, float defaultDuration, std::function<void(float, float)> onSave);
    float getSpeed() const { return m_speed; }
    float getDuration() const { return m_duration; }
};
