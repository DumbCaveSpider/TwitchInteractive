#pragma once
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>
#include <functional>

using namespace geode::prelude;
using namespace cocos2d;
using namespace extension;

class ColorPlayerSettingsPopup : public geode::Popup<>
{
protected:
    cocos2d::extension::CCControlColourPicker *m_colorPicker = nullptr;
    cocos2d::ccColor3B m_selectedColor = {255, 255, 255};
    std::function<void(const cocos2d::ccColor3B &)> m_callback;
    cocos2d::CCLayer *m_mainLayer = nullptr;
    TextInput *m_rInput = nullptr;
    TextInput *m_gInput = nullptr;
    TextInput *m_bInput = nullptr;

    void onApplyRGB(cocos2d::CCObject *sender);
    bool setup() override;
    void onColorChanged(cocos2d::CCObject *sender, cocos2d::extension::CCControlEvent event);
    void onSave(cocos2d::CCObject *sender);

public:
    static ColorPlayerSettingsPopup *create(const cocos2d::ccColor3B &initialColor, std::function<void(const cocos2d::ccColor3B &)> callback);
    cocos2d::ccColor3B getSelectedColor() const { return m_selectedColor; }
};
