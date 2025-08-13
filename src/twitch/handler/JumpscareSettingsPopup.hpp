#pragma once

#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>
#include <functional>
#include <Geode/ui/TextInput.hpp>
#include <vector>

using namespace geode::prelude;
using namespace cocos2d;
using namespace extension;

class JumpscareSettingsPopup : public geode::Popup<>
{
protected:
    // File selection state
    std::vector<std::string> m_files; // filenames inside config/jumpscare
    int m_selectedIdx = -1;           // index into m_files, -1 when none
    CCLabelBMFont *m_fileLabel = nullptr;
    CCMenuItemSpriteExtra *m_leftBtn = nullptr;
    CCMenuItemSpriteExtra *m_rightBtn = nullptr;

    geode::TextInput *m_fadeInput = nullptr;
    geode::TextInput *m_scaleInput = nullptr;
    std::function<void(const std::string &, float, float)> m_onSave;
    std::string m_initUrl;
    float m_initFade = 0.5f;
    float m_initScale = 1.0f;

    bool setup() override;
    void onSaveBtn(cocos2d::CCObject *);
    void onOpenFolder(cocos2d::CCObject *);
    void onInfoBtn(cocos2d::CCObject *);
    void onLeftFile(cocos2d::CCObject *);
    void onRightFile(cocos2d::CCObject *);
    void onRefreshFiles(cocos2d::CCObject *);
    void onTestJumpscare(cocos2d::CCObject *);
    void cleanupTempNode(cocos2d::CCNode *node);
    void loadFiles();
    void updateFileLabel();
    void updateArrowPositions();

public:
    static JumpscareSettingsPopup *create(const std::string &initUrl, float initFade, float initScale, std::function<void(const std::string &, float, float)> onSave);
};
