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
    dashBtn->setPosition({width / 2 - 120.f, 24.f});

    dashMenu->addChild(dashBtn);

    auto tutorialBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Tutorial", "bigFont.fnt", "GJ_button_01.png", fixedBtnWidth),
        this,
        menu_selector(HandbookPopup::onTutorialBtn));
    tutorialBtn->setID("handbook-tutorial-btn");
    tutorialBtn->setPosition({width / 2, 24.f});
    dashMenu->addChild(tutorialBtn);

    // Commands button
    auto commandsBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Commands", "bigFont.fnt", "GJ_button_01.png", fixedBtnWidth),
        this,
        menu_selector(HandbookPopup::onCommandsBtn));
    commandsBtn->setID("handbook-commands-btn");
    commandsBtn->setPosition({width / 2 + 120.f, 24.f});

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

    float cornerScale = 1.25f;
    // Top-left
    auto cornerTL = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
    cornerTL->setRotation(90);
    cornerTL->setAnchorPoint({0.f, 0.f});
    cornerTL->setPosition({0.f, height});
    cornerTL->setScale(cornerScale);
    cornerTL->setID("corner-tl");
    m_mainLayer->addChild(cornerTL);

    // Top-right
    auto cornerTR = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
    cornerTR->setPosition({width, height});
    cornerTR->setAnchorPoint({0.f, 0.f});
    cornerTR->setRotation(180);
    cornerTR->setScale(cornerScale);
    cornerTR->setID("corner-tr");
    m_mainLayer->addChild(cornerTR);

    // Bottom-left
    auto cornerBL = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
    cornerBL->setScale(cornerScale);
    cornerBL->setAnchorPoint({0.f, 0.f});
    cornerBL->setZOrder(1);
    cornerBL->setID("corner-bl");
    m_mainLayer->addChild(cornerBL);

    // Bottom-right
    auto cornerBR = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
    cornerBR->setRotation(270);
    cornerBR->setAnchorPoint({0.f, 0.f});
    cornerBR->setPosition({width, 0.f});
    cornerBR->setScale(cornerScale);
    cornerBR->setID("corner-br");
    m_mainLayer->addChild(cornerBR);

    // goof ball
    auto king = CCSprite::create("shrug.png"_spr);
    king->setAnchorPoint({0.f, 0.f});
    king->setRotation(10);
    king->setPosition({5.f, 20.f});
    king->setID("king");
    king->setScale(2.0f);
    m_mainLayer->addChild(king);

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

        "- Every command starts with an exclamation mark (e.g., `!say`).\n"
        "- Add, edit, remove or toggle commands in the Dashboard.\n"
        "- Apply cooldown on a specific command to prevent spamming by setting a cooldown in the settings. (Setting the cooldown to '0' for no cooldown.\n"
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

        "- By default, all commands are disabled upon game bootup and only starts listening when you open the Dashboard once\n"
        "- Change each commands you created in the Dashboard with the buttons on the right.\n"
        "- Pause all commands in the chat by toggling the 'Listen' button in the Dashboard. When the button is red, all commands won't be executed by chat anymore.\n"
        "- Disable certain commands by unchecking the 'Enabled' checkbox in the Dashboard.\n\n"

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
        "- Move actions up or down to change their order of execution.\n"
        "- Copy actions to duplicate the value on that specific action.\n"
        "- Remove actions by clicking the delete button on the right.\n"
        "- Add multiple actions to a command to create complex effects.\n"
        "- "
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
        "- Click on the plus button to add the event into the action list"
        "- Search for specific events using the search box\n"
        "- Some events are hidden behind an experimental features. You can enable it at the Mod Settings\n\n"

        "## Got an Events Suggestions?\n"
        "- Consider contacting ArcticWoof via Discord (@arcticwoof) on my server for events suggestions.\n\n";

    MDPopup::create("Events Help", md, "OK", nullptr, [](bool) {})->show();
};

void HandbookPopup::onTutorialBtn(CCObject *)
{
    std::string md =
        "# Tutorial\n\n"
        "This handbook will teach you how to create a common commands used by various of streamers to understand how to use this mod.\n"
        "There will be 3 example commands you can make and each ranges from simple command to more complex ones.\n\n"
        "## Tutorial 1: Create a Simple Jump Command\n\n"
        "1. Click 'New Command'.\n"
        "2. Create a new command called `jump`.\n"
        "3. **(Optional)** Add a cooldown in Settings to prevent spam.\n"
        "4. Add a Jump Event: Click the + on the 'Jump'.\n"
        "5. **(Optional)** Click on the Jump Action Setting and pick which Player to affect.\n"
        "6. Click Save on the Jump Settings and the Command Settings.\n"
        "7. Play any level to test out the command."
        "7. Now type `!jump` in chat to test the command.\n\n"
        "## Tutorial 2: Create a Notification Command with Custom Chat\n\n"
        "1. Click 'New Command'.\n"
        "2. Set the command name. Example: `say`.\n"
        "3. **(Optional)** Add a cooldown in Settings to prevent spam.\n"
        "4. Add an Action: Click the + on the 'Notification'.\n"
        "5. In the text, type: `${displayname}: ${arg}` to show the user's name and message.\n"
        "6. Press 'Save' on the command settings.\n"
        "7. Test it in chat: type `!say hello` and you'll see a popup: `YourName: hello`.\n"
        "## Tutorial 3: Create a Complex Jumpscare command\n\n"
        "1. Click 'New Command'.\n"
        "2. Set the command name. Example: `jumpscare`.\n"
        "3. **(Optional)** Add a cooldown in Settings to prevent spam.\n"
        "4. Add an Action: Click the + on the 'Jumpscare'.\n"
        "5. In the jumpscare settings, click 'Open Folder' and drag and drop any image on the folder.\n"
        "6. Click refresh and you should see your image name on the selection, change the fade or scale and click 'Save'.\n"
        "7. Now add another action: Click the + on the 'Sound Effect'.\n"
        "8. In the sound effect settings, choose any sounds provided or use a custom one by opening 'Custom SFX' and drag and drop an audio file to the folder.\n"
        "9. Search for specific sounds using the search box and click the arrow button or the audio name to select.\n"
        "10. Press 'Save' on the command settings\n"
        "11. Test it in chat: type `!jumpscare` and your image will show and the audio will play!\n"
        "### Helpful Tips:\n"
        "- Use User/Role to limit who can use the command.\n"
        "- Add multiple actions for more effects (e.g., Wait, Keycode).";

    MDPopup::create("Basic Command Tutorial", md, "OK", nullptr, [](bool) {})->show();
}

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
