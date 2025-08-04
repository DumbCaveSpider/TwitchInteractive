#pragma once
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>
#include <functional>

using namespace geode::prelude;

class GravitySettingsPopup : public geode::Popup<> {
protected:
    int m_actionIdx = 0;
    TextInput* m_gravityInput = nullptr;
    TextInput* m_durationInput = nullptr;
    std::function<void(float, float)> m_onSave;
    float m_gravity = 1.0f;
    float m_duration = 0.5f;
    CCLayer* m_mainLayer = nullptr;

    bool setup() override;
    void onApplyBtn(CCObject*);

public:
    static GravitySettingsPopup* create(int actionIdx, float defaultGravity, float defaultDuration, std::function<void(float, float)> onSave);
    float getGravity() const { return m_gravity; }
    float getDuration() const { return m_duration; }
};
