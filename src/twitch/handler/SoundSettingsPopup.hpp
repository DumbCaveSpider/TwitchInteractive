#pragma once
#include <Geode/Geode.hpp>
#include "../command/CommandSettingsPopup.hpp"

using namespace geode::prelude;
using namespace cocos2d;

class SoundSettingsPopup : public geode::Popup<>
{
protected:
    CommandSettingsPopup *m_parent = nullptr;
    int m_actionIdx = 0;
    std::string m_selectedSound;
    std::function<void(const std::string &)> m_onSave;
    std::function<void(const std::vector<std::string> &)> updateSoundList;
    geode::TextInput *m_soundSearchInput = nullptr;
    std::string m_lastSoundSearchString;

    bool setup() override;
    void onSaveBtn(CCObject *);
    void onSoundSelect(CCObject *);
    void onPlaySound(CCObject *sender);
    void onClose(CCObject *);

public:
    static SoundSettingsPopup *create(CommandSettingsPopup *parent, int actionIdx, const std::string &selectedSound, std::function<void(const std::string &)> onSave);
    void onSelectSound(CCObject *sender);
    void onTextInput(geode::TextInput *input, const std::string &text);
    void onSoundSearchPoll(float);
};
