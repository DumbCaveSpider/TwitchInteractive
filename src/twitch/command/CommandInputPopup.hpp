#pragma once

#include <Geode/Geode.hpp>
#include <functional>

using namespace geode::prelude;

class CommandInputPopup : public Popup<>
{
protected:
    TextInput *m_nameInput = nullptr;
    TextInput *m_descInput = nullptr;
    TextInput *m_cooldownInput = nullptr; // Cooldown input
    CCLabelBMFont *m_titleLabel = nullptr;
    CCLabelBMFont *m_descLabel = nullptr;
    std::function<void(const std::string &, const std::string &)> m_callback;
    bool m_isEditing = false;
    std::string m_originalName = "";
    std::string m_originalDesc = "";
    int m_cooldownSeconds = 0;

    bool setup() override;
    void onAdd(CCObject *sender);
    CCMenu *createButtonMenu();

public:
    static CommandInputPopup *create(std::function<void(const std::string &, const std::string &)> callback);

    static CommandInputPopup *createForEdit(
        const std::string &commandName,
        const std::string &commandDesc,
        std::function<void(const std::string &, const std::string &)> editCallback);

    void setCallback(std::function<void(const std::string &, const std::string &)> callback);
    void setupForEdit(const std::string &commandName, const std::string &commandDesc);
};