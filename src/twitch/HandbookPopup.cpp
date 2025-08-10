#include "HandbookPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/MDPopup.hpp>

bool HandbookPopup::setup()
{
    setTitle("Twitch Interactive Handbook");
    setID("handbook-popup");

    float width = m_mainLayer->getContentSize().width;
    float height = m_mainLayer->getContentSize().height;

    float sectionSpacing = 70.f;
    float btnX = width / 2;

    float tripleSpacing = 110.f;
    float fixedBtnWidth = 0.5f;

    // Commands Settings menu
    auto topMenu = CCMenu::create();
    topMenu->setContentSize({width, 100.f});
    // Move menu up a bit to avoid overlap with dashboard section
    topMenu->setPosition(0, height / 2 + 10.f);
    topMenu->setID("handbook-top-menu");

    CCLabelBMFont *commandsLabel = CCLabelBMFont::create("Commands Settings Help", "bigFont.fnt");
    commandsLabel->setScale(0.5f);
    commandsLabel->setPosition(width / 2, 88.f); // Near top of menu
    topMenu->addChild(commandsLabel);

    // Top row: Events, Actions, Identifiers
    float topRowY = 56.f;
    auto eventsBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Events", "bigFont.fnt", "GJ_button_01.png", fixedBtnWidth),
        this,
        menu_selector(HandbookPopup::onEventsBtn));
    eventsBtn->setID("handbook-events-btn");
    eventsBtn->setPosition({width / 2 - tripleSpacing, topRowY});

    auto actionBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Actions", "bigFont.fnt", "GJ_button_01.png", fixedBtnWidth),
        this,
        menu_selector(HandbookPopup::onActionBtn));
    actionBtn->setID("handbook-action-btn");
    actionBtn->setPosition({width / 2, topRowY});

    auto identifiersBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Identifiers", "bigFont.fnt", "GJ_button_01.png", fixedBtnWidth),
        this,
        menu_selector(HandbookPopup::onIdentifiersBtn));
    identifiersBtn->setID("handbook-identifiers-btn");
    identifiersBtn->setPosition({width / 2 + tripleSpacing, topRowY});

    // Bottom row: User/Role button centered below the top row, with spacing
    float bottomRowY = 16.f;
    auto userRoleBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("User/Role", "bigFont.fnt", "GJ_button_01.png", fixedBtnWidth),
        this,
        menu_selector(HandbookPopup::onUserRoleBtn));
    userRoleBtn->setID("handbook-userrole-btn");
    userRoleBtn->setPosition({width / 2, bottomRowY});

    topMenu->addChild(eventsBtn);
    topMenu->addChild(actionBtn);
    topMenu->addChild(identifiersBtn);
    topMenu->addChild(userRoleBtn);

    m_mainLayer->addChild(topMenu);

    // Dashboard section
    auto dashMenu = CCMenu::create();
    dashMenu->setID("handbook-dashboard-menu");
    dashMenu->setContentSize({width, 72.f});
    dashMenu->setPosition({0.f, height / 2.f - 70.f}); // Centered lower

    CCLabelBMFont *dashLabel = CCLabelBMFont::create("Dashboard Help", "bigFont.fnt");
    dashLabel->setScale(0.5f);
    dashLabel->setPosition({width / 2, 60.f});

    dashMenu->addChild(dashLabel);

    // Dashboard button
    auto dashBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Dashboard", "bigFont.fnt", "GJ_button_01.png", fixedBtnWidth),
        this,
        menu_selector(HandbookPopup::onDashboardBtn));
    dashBtn->setID("handbook-dashboard-btn");
    dashBtn->setPosition({width / 2 - 80.f, 24.f});

    dashMenu->addChild(dashBtn);

    // Commands button
    auto commandsBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Commands", "bigFont.fnt", "GJ_button_01.png", fixedBtnWidth),
        this,
        menu_selector(HandbookPopup::onCommandsBtn));
    commandsBtn->setID("handbook-commands-btn");
    commandsBtn->setPosition({width / 2 + 80.f, 24.f});

    dashMenu->addChild(commandsBtn);

    m_mainLayer->addChild(dashMenu);

    // Support section
    auto supportMenu = CCMenu::create();
    supportMenu->setID("handbook-support-menu");
    supportMenu->setContentSize({width, 72.f});
    supportMenu->setPosition({0.f, 0.f});

    // Support label (centered)
    CCLabelBMFont *supportLabel = CCLabelBMFont::create("Support", "bigFont.fnt");
    supportLabel->setScale(0.5f);
    supportLabel->setAnchorPoint({0.5f, 0.5f});
    supportLabel->setPosition({width / 2, 60.f});
    supportMenu->addChild(supportLabel);

    // Discord button
    auto discordIcon = CCSprite::createWithSpriteFrameName("gj_discordIcon_001.png");
    float iconScale = 0.9f;
    if (discordIcon)
    {
        discordIcon->setScale(iconScale);
    }
    auto discordBtn = CCMenuItemSpriteExtra::create(
        discordIcon,
        this,
        menu_selector(HandbookPopup::onDiscordBtn));
    discordBtn->setID("handbook-discord-btn");
    discordBtn->setAnchorPoint({0.5f, 0.5f});
    discordBtn->setPosition({width / 2, 30.f});
    supportMenu->addChild(discordBtn);

    m_mainLayer->addChild(supportMenu);

    return true;
}
// Support Discord button callback
void HandbookPopup::onDiscordBtn(CCObject *)
{
    geode::utils::web::openLinkInBrowser("https://discord.gg/gXcppxTNxC");
}

// Handbook MD instructions

void HandbookPopup::onUserRoleBtn(CCObject *)
{
    std::string md =
        "# Command User/Role Restrictions\n\n"
        "Command User/Role restrictions let you control who can use a command in your Twitch chat. You can open User/Role restriction at the top right of the command settings\n\n"

        "## Available Restrictions\n"
        "- **User**: Only the specified username can use the command. *(Leave blank for any user)*\n"
        "- **Everyone**: No restrictions, anyone can use the command *(Have all checkbox unticked)*.\n"
        "- **VIP**: Users with Twitch VIP role can use the command.\n"
        "- **Mod**: Users with Twitch Mod role can use the command.\n"
        "- **Subscriber**: Users with Twitch Subscriber role can use the command.\n"
        "- **Streamer**: User logged in on this dashboard can use the command.\n\n"

        "You can combine multiple restrictions. If any are set, the user must match at least one to use the command.\n\n"
        "**Tip:** Use role restrictions to protect important or powerful commands!";

    MDPopup::create("User/Role Help", md, "OK", nullptr, [](bool) {})->show();
}

void HandbookPopup::onCommandsBtn(CCObject *)
{
    std::string md =
        "# Commands\n\n"
        "Commands are custom triggers that viewers can use in Twitch chat to interact with your game.\n\n"

        "## How to Use\n"
        "- Every command starts with an exclamation mark (e.g., `!say`).\n"
        "- You can add, edit, remove or toggle commands in the Dashboard.\n"
        "- Apply cooldown on a specific command to prevent spamming by setting a cooldown in the settings.\n"
        "- Click on the command name to copy the chat command to your clipboard.\n"
        "- Each command can have one or multiple actions that are executed when the command is triggered.\n\n"

        "**Tip:** Use commands to make your stream interactive and fun!";

    MDPopup::create("Commands Help", md, "OK", nullptr, [](bool) {})->show();
};

void HandbookPopup::onDashboardBtn(CCObject *)
{
    std::string md =
        "# Dashboard\n\n"
        "The Dashboard is your main control center for the Twitch Interactive mod.\n\n"

        "## How to Use\n"
        "- By default, all commands are disabled upon game bootup and only starts listening when you open the Dashboard once\n"
        "- You can change each commands you created in the Dashboard with the buttons on the right.\n"
        "- If you want to pause all commands in the chat, you can toggle the 'Listen' button in the Dashboard.\n"
        "- You can disable certain commands by unchecking the 'Enabled' checkbox in the Dashboard.\n\n"

        "**Tip:** Use the Dashboard to quickly test, enable, or disable commands while streaming!";

    MDPopup::create("Dashboard Help", md, "OK", nullptr, [](bool) {})->show();
};

void HandbookPopup::onIdentifiersBtn(CCObject *)
{
    std::string md =
        "# Identifiers\n\n"
        "Identifiers are special placeholders you can use in actions to insert dynamic values from Twitch chat.\n\n"

        "## Available Identifiers\n"
        "- `${arg}`: Replaced with the argument(s) provided by the user in chat.\n"
        "- `${rng<min>:<max>}`: Replace with a random range between the min and max. *eg. `${rng1:10}` will output a random number between 1 and 10.*\n"
        "- `${username}`: Replaced with the username of the user who triggered the command.\n"
        "- `${displayname}`: Replaced with the display name of the user who triggered the command.\n"
        "- `${userid}`: Replaced with the user ID of the user who triggered the command.\n"
        "- `${streamer}`: Replaced with the configured Twitch channel (streamer's username).\n\n"

        "## Usage Example\n"
        "- If your notification action is `${displayname}: ${arg}!`, and a user types `!say Hello`, the notification will show `ArcticWoofLive: Hello`\n\n"

        "**Tip:** Identifiers can be used in any action argument that supports text.";

    MDPopup::create("Identifiers Help", md, "OK", nullptr, [](bool) {})->show();
};

void HandbookPopup::onActionBtn(CCObject *)
{
    std::string md =
        "# Actions\n\n"
        "Actions are the list of events that the command will trigger when activated.\n\n"
        "- Some actions can be modified using the settings button.\n"
        "- Actions are executed in order, from top to bottom.\n"
        "- You can move actions up or down to change their order of execution.\n"
        "- You can remove actions by clicking the delete button on the right.\n"
        "- Add multiple actions to a command to create complex effects.\n"
        "- Use the **Wait** action to delay each action by set seconds.\n\n"

        "**Tip:** Combine multiple actions for complex command effects!";

    MDPopup::create("Actions Help", md, "OK", nullptr, [](bool) {})->show();
};

void HandbookPopup::onEventsBtn(CCObject *)
{
    std::string md =
        "# Events\n\n"
        "Events are actions that can be added to the command.\n\n"

        "- Each event represents a specific in-game action (e.g., jump, kill player, keycode).\n"
        "- Click the info icon for more details on each event.\n"
        "- Some events accept arguments, such as which player to affect.\n"
        "- You can search for specific events using the search box\n"

        "## Got an Events Suggestions?\n"
        "- Consider contacting ArcticWoof via Discord (@arcticwoof) on my server for events suggestions.\n\n";

    MDPopup::create("Events Help", md, "OK", nullptr, [](bool) {})->show();
};

HandbookPopup *HandbookPopup::create()
{
    auto ret = new HandbookPopup();
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    if (ret && ret->initAnchored(winSize.width - 100.f, winSize.height - 40.f))
    {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};