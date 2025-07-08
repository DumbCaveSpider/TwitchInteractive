#pragma once

#include <Geode/Geode.hpp>
#include <functional>

using namespace geode::prelude;

class CommandInputPopup : public Popup<> {
protected:
    TextInput* m_nameInput = nullptr;
    TextInput* m_descInput = nullptr;
    CCLabelBMFont* m_titleLabel = nullptr;
    CCLabelBMFont* m_descLabel = nullptr;
    std::function<void(const std::string&, const std::string&)> m_callback;

    bool setup() override;
    void onAdd(CCObject* sender);
    void onCancel(CCObject* sender);

public:
    static CommandInputPopup* create(std::function<void(const std::string&, const std::string&)> callback);
    void setCallback(std::function<void(const std::string&, const std::string&)> callback);
};