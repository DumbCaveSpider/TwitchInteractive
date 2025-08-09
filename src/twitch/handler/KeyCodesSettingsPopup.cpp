#include "KeyCodesSettingsPopup.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace cocos2d;

bool KeyCodesSettingsPopup::setup(std::string keyCode)
{
    // Parse key and duration if present (format: keycode:<key>:<duration>)
    m_keyCode = keyCode;
    m_holdDuration = "";

    size_t firstColon = keyCode.find(":");
    size_t secondColon = keyCode.find(":", firstColon + 1);

    if (firstColon != std::string::npos && secondColon != std::string::npos) {
        m_keyCode = keyCode.substr(0, firstColon); // Extract key before the first colon
        m_holdDuration = keyCode.substr(secondColon + 1); // Extract duration after the second colon
    } else if (firstColon != std::string::npos) {
        m_keyCode = keyCode.substr(0, firstColon); // Extract key
        m_holdDuration = keyCode.substr(firstColon + 1); // Extract duration
    } else {
        m_keyCode = keyCode; // No colons, treat the entire string as the key
    }

    setTitle("Edit Key Code");
    setID("keycodes-settings-popup");

    m_noElasticity = true;

    float y = 100.f;
    float x = m_mainLayer->getContentSize().width / 2;

    m_keyLabel = CCLabelBMFont::create(m_keyCode.empty() ? "Press key..." : m_keyCode.c_str(), "goldFont.fnt");
    m_keyLabel->setID("keycode-label");
    m_keyLabel->setPosition(x, y + 10.f);
    m_keyLabel->setScale(1.2f);

    m_mainLayer->addChild(m_keyLabel);

    // Add textbox for hold duration
    m_durationInput = geode::TextInput::create(180.f, "Hold Duration (leave blank for infinite)", "bigFont.fnt");
    m_durationInput->setCommonFilter(CommonFilter::Float);
    m_durationInput->setID("keycode-duration-input");
    m_durationInput->setPosition(x, y - 30.f);
    m_durationInput->setString(m_holdDuration);
    m_durationInput->setScale(0.7f);

    m_mainLayer->addChild(m_durationInput);

    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(KeyCodesSettingsPopup::onSave));
    saveBtn->setID("keycode-save-btn");

    auto menu = CCMenu::create();
    menu->addChild(saveBtn);
    menu->setPosition(x, y - 70.f);

    m_mainLayer->addChild(menu);

    setTouchEnabled(true);
    setKeypadEnabled(true);

    if (m_keyLabel) {
        if (m_keyCode.empty()) {
            m_keyLabel->setString("Press Key...");
        } else {
            m_keyLabel->setString(m_keyCode.c_str());
        }
    }

    // Ensure the duration input displays the hold duration
    if (m_durationInput) {
        m_durationInput->setString(m_holdDuration);
    }

    return true;
};

void KeyCodesSettingsPopup::onSave(cocos2d::CCObject *sender)
{
    if (m_durationInput)
        m_holdDuration = m_durationInput->getString();

    // Validate hold duration: must be blank or a valid number
    bool valid = true;

    if (!m_holdDuration.empty())
    {
        // Accept integer or float (e.g. 1, 1.5, .5)
        bool seenDot = false;

        for (size_t i = 0; i < m_holdDuration.size(); ++i)
        {
            char c = m_holdDuration[i];

            if (c == '.')
            {
                if (seenDot)
                {
                    valid = false;
                    break;
                };

                seenDot = true;
            }
            else if (!isdigit(c))
            {
                valid = false;
                break;
            };
        };

        if (!valid || m_holdDuration == "." || m_holdDuration == "-")
        {
            Notification::create("Hold duration must be a number!", NotificationIcon::Error)->show();
            return;
        };
    };

    // Update the result format to `keycode:<key>:<duration>`
    std::string result = m_keyCode;

    if (!m_holdDuration.empty()) {
        result += ":" + m_holdDuration;
    }

    if (m_onSelect) {
        m_onSelect(result);
    }

    onClose(nullptr);
};

void KeyCodesSettingsPopup::keyDown(cocos2d::enumKeyCodes key)
{
    // Convert key code to string representation
    std::string keyStr;
    if (key >= KEY_A && key <= KEY_Z)
    {
        keyStr = std::string(1, 'A' + (key - KEY_A));
    }
    else if (key >= 48 && key <= 57)
    { // ASCII '0'-'9'
        keyStr = std::string(1, static_cast<char>(key));
    }
    else
    {
        switch (key)
        {
        case KEY_Space:
            keyStr = "Space";
            break;

        case KEY_Enter:
            keyStr = "Enter";
            break;

        case KEY_Escape:
            keyStr = "Escape";
            break;

        case KEY_Left:
            keyStr = "Left";
            break;

        case KEY_Right:
            keyStr = "Right";
            break;

        case KEY_Up:
            keyStr = "Up";
            break;

        case KEY_Down:
            keyStr = "Down";
            break;

        case KEY_Tab:
            keyStr = "Tab";
            break;

        case KEY_Backspace:
            keyStr = "Backspace";
            break;

        case KEY_Shift:
            keyStr = "Shift";
            break;

        case KEY_Control:
            keyStr = "Ctrl";
            break;

        case KEY_Alt:
            keyStr = "Alt";
            break;

        case 20:
            keyStr = "CapsLock";
            break;

        case 160:
            keyStr = "LeftShift";
            break;

        case 161:
            keyStr = "RightShift";
            break;

        case 4101:
            keyStr = ";";
            break;

        case 4097:
            keyStr = "=";
            break;

        case 188:
            keyStr = ",";
            break;

        case 189:
            keyStr = "-";
            break;

        case 190:
            keyStr = ".";
            break;

        case 4103:
            keyStr = "/";
            break;

        case 4096:
            keyStr = "`";
            break;

        case 4098:
            keyStr = "[";
            break;

        case 4100:
            keyStr = "\\";
            break;

        case 4099:
            keyStr = "]";
            break;

        case 4102:
            keyStr = "'";
            break;

        default:
            // If in ASCII printable range
            if (key >= 32 && key <= 126)
            {
                keyStr = std::string(1, static_cast<char>(key));
            }
            else
            {
                keyStr = fmt::format("KeyCode({})", (int)key);
            };

            break;
        };
    };

    m_keyCode = keyStr;
    // Ensure the key label only displays the key
    if (m_keyLabel) {
        m_keyLabel->setString(m_keyCode.c_str());
    }

    // Ensure the duration input only displays the duration
    if (m_durationInput) {
        m_durationInput->setString(m_holdDuration);
    }
};

KeyCodesSettingsPopup *KeyCodesSettingsPopup::create(const std::string &keyCode, std::function<void(const std::string &)> onSelect)
{
    auto ret = new KeyCodesSettingsPopup();

    ret->m_onSelect = onSelect;

    if (ret && ret->initAnchored(220.f, 180.f, keyCode))
    {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};