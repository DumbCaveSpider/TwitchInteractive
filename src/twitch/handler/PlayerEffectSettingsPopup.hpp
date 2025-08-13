#pragma once
#include <Geode/Geode.hpp>
#include <vector>

using namespace geode::prelude;

enum class PlayerEffectType
{
    Death,
    Spawn
};

class PlayerEffectSettingsPopup : public geode::Popup<>
{
protected:
    std::vector<PlayerEffectType> m_types{PlayerEffectType::Death, PlayerEffectType::Spawn};
    int m_selectedIdx = 0; // index into m_types
    CCLabelBMFont *m_valueLabel = nullptr;
    std::function<void(PlayerEffectType, int)> m_onSave;
    int m_playerIdx = 1; // 1 or 2
    CCMenuItemSpriteExtra *m_leftBtn = nullptr;
    CCMenuItemSpriteExtra *m_rightBtn = nullptr;
    CCMenuItemSpriteExtra *m_player1Btn = nullptr;
    CCMenuItemSpriteExtra *m_player2Btn = nullptr;

    bool setup() override;
    void onLeft(CCObject *);
    void onRight(CCObject *);
    void onSaveBtn(CCObject *);
    void updateValueLabel();
    void onChoosePlayer1(CCObject *);
    void onChoosePlayer2(CCObject *);
    void updatePlayerButtons();
    void updateArrowPositions();
    PlayerEffectType getSelected() const { return m_types.empty() ? PlayerEffectType::Death : m_types[m_selectedIdx % (int)m_types.size()]; }

public:
    static PlayerEffectSettingsPopup *create(PlayerEffectType initial, int initialPlayer, std::function<void(PlayerEffectType, int)> onSave);
};
