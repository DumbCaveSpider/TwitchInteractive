#include "ProfileSettingsPopup.hpp"
namespace web = geode::utils::web;

bool ProfileSettingsPopup::setup()
{
    setTitle("Profile Settings");
    setID("profile-settings-popup");

    float y = 60.f;
    float x = m_mainLayer->getContentSize().width / 2;

    m_accountIdInput = TextInput::create(120, "Username", "bigFont.fnt");
    m_accountIdInput->setCommonFilter(CommonFilter::Any);
    m_accountIdInput->setID("account-id-input");
    m_accountIdInput->setString(m_accountId.c_str());
    if (m_accountId.empty()) {
        m_accountId = "ArcticWoof";
        m_accountIdInput->setString(m_accountId.c_str());
    }
    m_accountIdInput->setPosition(x - 20, y + 10);

    m_mainLayer->addChild(m_accountIdInput);

    this->m_noElasticity = true;

    // Add profile open button next to textbox
    auto openBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Open", "bigFont.fnt", "GJ_button_05.png", 0.3f),
        this,
        menu_selector(ProfileSettingsPopup::onOpenProfile));
    openBtn->setID("profile-open-btn");
    openBtn->setPosition(x + 70, y + 10);

    auto openMenu = CCMenu::create();
    openMenu->setPosition(0, 0);

    openMenu->addChild(openBtn);

    m_mainLayer->addChild(openMenu);

    // Add save button below
    auto saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_01.png", 0.6f),
        this,
        menu_selector(ProfileSettingsPopup::onSave));
    saveBtn->setID("save-btn");
    saveBtn->setPosition(0, 0);

    auto menu = CCMenu::create();
    menu->setPosition(x, y - 30);

    menu->addChild(saveBtn);

    m_mainLayer->addChild(menu);

    return true;
};

void ProfileSettingsPopup::onOpenProfile(CCObject *sender)
{
    std::string username = m_accountIdInput ? m_accountIdInput->getString() : "";
    bool wasEmpty = username.empty();

    // Default fallback username to myself cuz im cool
    if (username.empty())
        username = "ArcticWoof";
    if (wasEmpty && m_accountIdInput)
        m_accountIdInput->setString(username.c_str());

    // Trim whitespace
    username.erase(0, username.find_first_not_of(" \t\n\r"));
    if (!username.empty())
        username.erase(username.find_last_not_of(" \t\n\r") + 1);

    // getGJUsers20.php stuff
    auto url = std::string("https://www.boomlings.com/database/getGJUsers20.php");
    auto urlEncode = [](const std::string &s)
    {
        static const char hex[] = "0123456789ABCDEF";
        std::string out;
        out.reserve(s.size() * 3);
        for (unsigned char c : s)
        {
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~')
                out.push_back(static_cast<char>(c));
            else if (c == ' ')
                out.push_back('+');
            else
            {
                out.push_back('%');
                out.push_back(hex[(c >> 4) & 0xF]);
                out.push_back(hex[c & 0xF]);
            }
        }
        return out;
    };

    // Me using gdbrowser api for fetching basic stuff: easy face
    // Me using robtop endpoint for fetching basic stuff: extreme face
    std::string postData = "gameVersion=22&binaryVersion=40&gdw=0&str=" + urlEncode(username) + "&page=0&total=0&secret=Wmfd2893gb7";

    auto request = web::WebRequest();
    request.header("Content-Type", "application/x-www-form-urlencoded");
    request.bodyString(postData);

    request.post(url)
        .listen([this](web::WebResponse *res)
                {
            if (!res || !res->ok()) {
                Notification::create("User cannot be found", NotificationIcon::Error, 1.5f)->show();
                return;
            }
            auto resp = res->string().unwrapOrDefault();
            if (resp == "-1") {
                Notification::create("User cannot be found", NotificationIcon::Error, 1.5f)->show();
                return;
            }

            size_t pipe = resp.find('|');
            std::string firstUser = (pipe == std::string::npos ? resp : resp.substr(0, pipe));

            std::vector<std::string> fields;
            size_t start = 0;
            while (true) {
                size_t pos = firstUser.find(":", start);
                if (pos == std::string::npos) {
                    fields.push_back(firstUser.substr(start));
                    break;
                }
                fields.push_back(firstUser.substr(start, pos - start));
                start = pos + 1;
            }

            int accountId = 0;
            for (size_t i = 0; i + 1 < fields.size(); i += 2) {
                if (fields[i] == "16") {
                    const std::string& accountIdStr = fields[i + 1];
                    if (!accountIdStr.empty() && accountIdStr.find_first_not_of("-0123456789") == std::string::npos) {
                        accountId = numFromString<int>(accountIdStr).unwrapOrDefault();
                    }
                    break;
                }
            }

            if (accountId > 0) {
                if (auto page = ProfilePage::create(accountId, false)) {
                    page->show();
                    return;
                }
            }

            Notification::create("User cannot be found", NotificationIcon::Error, 1.5f)->show();
            this->onClose(this); },
                [](web::WebProgress *) {},
                [this]()
                {
                    Notification::create("User cannot be found", NotificationIcon::Error, 1.5f)->show();
                    this->onClose(this);
                });
};

void ProfileSettingsPopup::onSave(CCObject *sender)
{
    std::string newId = m_accountIdInput ? m_accountIdInput->getString() : "";
    if (newId.empty()) {
        newId = "ArcticWoof";
        if (m_accountIdInput)
            m_accountIdInput->setString(newId.c_str());
    }
    if (m_callback)
        m_callback(newId);

    onClose(sender);
};

ProfileSettingsPopup *ProfileSettingsPopup::create(const std::string &accountId, std::function<void(const std::string &)> callback)
{
    auto ret = new ProfileSettingsPopup();

    ret->m_accountId = accountId;
    ret->m_callback = callback;

    if (ret && ret->initAnchored(220.f, 120.f))
    {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};