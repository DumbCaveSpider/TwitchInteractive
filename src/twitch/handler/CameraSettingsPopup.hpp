#pragma once
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>
#include <functional>
#include <Geode/ui/TextInput.hpp>

using namespace cocos2d;

class CameraSettingsPopup : public geode::Popup<> {
protected:
    CCLayer* m_mainLayer = nullptr;
    geode::TextInput* m_skewInput = nullptr;
    geode::TextInput* m_rotInput = nullptr;
    geode::TextInput* m_scaleInput = nullptr;
    geode::TextInput* m_timeInput = nullptr;
    std::function<void(float, float, float, float)> m_callback;

    bool setup();
    void onSave(CCObject* sender);
    void onApply(CCObject* sender);
    void onClose(CCObject* sender);
    float getSkew() const;
    float getRotation() const;
    float getScale() const;
    float getTime() const;
public:
    static CameraSettingsPopup* create(float skew, float rot, float scale, float time, std::function<void(float,float,float,float)> callback);
};
