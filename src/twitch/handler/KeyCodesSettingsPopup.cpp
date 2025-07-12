#include "KeyCodesSettingsPopup.hpp"
#include <Geode/Geode.hpp>
using namespace geode::prelude;
using namespace cocos2d;

bool KeyCodesSettingsPopup::setup(std::string keyCode) {
    // Parse key and duration if present (format: key|duration)
    m_keyCode = keyCode;
    m_holdDuration = "";
    size_t pipePos = keyCode.find("|");
    if (pipePos != std::string::npos) {
        m_keyCode = keyCode.substr(0, pipePos);
        m_holdDuration = keyCode.substr(pipePos + 1);
    }
    setTitle("Edit Key Code");
    setID("keycodes-settings-popup");
    float y = 100.f;
    float x = m_mainLayer->getContentSize().width / 2;
    m_keyLabel = CCLabelBMFont::create(m_keyCode.empty() ? "Press key..." : m_keyCode.c_str(), "goldFont.fnt");
    m_keyLabel->setID("keycode-label");
    m_keyLabel->setPosition(x, y + 10.f);
    m_keyLabel->setScale(1.2f);
    m_mainLayer->addChild(m_keyLabel);

    // Add textbox for hold duration
    m_durationInput = geode::TextInput::create(180.f, "Hold Duration (leave blank for infinite)", "chatFont.fnt");
    m_durationInput->setPosition(x, y - 30.f);
    m_durationInput->setID("keycode-duration-input");
    m_durationInput->setString(m_holdDuration);
    m_durationInput->setScale(0.7f);
    m_mainLayer->addChild(m_durationInput);

    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(KeyCodesSettingsPopup::onSave)
    );
    saveBtn->setID("keycode-save-btn");
    auto menu = CCMenu::create();
    menu->addChild(saveBtn);
    menu->setPosition(x, y - 70.f);
    m_mainLayer->addChild(menu);
    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);
    return true;
}

void KeyCodesSettingsPopup::onSave(cocos2d::CCObject* sender) {
    if (m_durationInput) m_holdDuration = m_durationInput->getString();
    // Validate hold duration: must be blank or a valid number
    bool valid = true;
    if (!m_holdDuration.empty()) {
        // Accept integer or float (e.g. 1, 1.5, .5)
        bool seenDot = false;
        for (size_t i = 0; i < m_holdDuration.size(); ++i) {
            char c = m_holdDuration[i];
            if (c == '.') {
                if (seenDot) { valid = false; break; }
                seenDot = true;
            } else if (!isdigit(c)) {
                valid = false; break;
            }
        }
        if (!valid || m_holdDuration == "." || m_holdDuration == "-") {
            Notification::create("Hold duration must be a number!", NotificationIcon::Error)->show();
            return;
        }
    }
    std::string result = m_keyCode;
    if (!m_holdDuration.empty()) result += "|" + m_holdDuration;
    if (m_onSelect) m_onSelect(result);
    onClose(nullptr);
}
void KeyCodesSettingsPopup::keyDown(cocos2d::enumKeyCodes key) {
    // Convert key code to string representation
    std::string keyStr;
    if (key >= KEY_A && key <= KEY_Z) {
        keyStr = std::string(1, 'A' + (key - KEY_A));
    } else if (key >= 48 && key <= 57) { // ASCII '0'-'9'
        keyStr = std::string(1, static_cast<char>(key));
    } else {
        switch (key) {
            case KEY_Space: keyStr = "Space"; break;
            case KEY_Enter: keyStr = "Enter"; break;
            case KEY_Escape: keyStr = "Escape"; break;
            case KEY_Left: keyStr = "Left"; break;
            case KEY_Right: keyStr = "Right"; break;
            case KEY_Up: keyStr = "Up"; break;
            case KEY_Down: keyStr = "Down"; break;
            case KEY_Tab: keyStr = "Tab"; break;
            case KEY_Backspace: keyStr = "Backspace"; break;
            case KEY_Shift: keyStr = "Shift"; break;
            case KEY_Control: keyStr = "Ctrl"; break;
            case KEY_Alt: keyStr = "Alt"; break;
            case 20: keyStr = "CapsLock"; break;
            case 160: keyStr = "LeftShift"; break;
            case 161: keyStr = "RightShift"; break;
            case 4101: keyStr = ";"; break;
            case 4097: keyStr = "="; break;
            case 188: keyStr = ","; break;
            case 189: keyStr = "-"; break;
            case 190: keyStr = "."; break;
            case 4103: keyStr = "/"; break;
            case 4096: keyStr = "`"; break;
            case 4098: keyStr = "["; break;
            case 4100: keyStr = "\\"; break;
            case 4099: keyStr = "]"; break;
            case 4102: keyStr = "'"; break;
            default:
                // If in ASCII printable range
                if (key >= 32 && key <= 126) {
                    keyStr = std::string(1, static_cast<char>(key));
                } else {
                    keyStr = fmt::format("KeyCode({})", (int)key);
                }
                break;
        }
    }
    m_keyCode = keyStr;
    if (m_keyLabel) m_keyLabel->setString(keyStr.c_str());
}

KeyCodesSettingsPopup* KeyCodesSettingsPopup::create(const std::string& keyCode, std::function<void(const std::string&)> onSelect) {
    auto ret = new KeyCodesSettingsPopup();
    ret->m_onSelect = onSelect;
    if (ret && ret->initAnchored(220.f, 180.f, keyCode)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
