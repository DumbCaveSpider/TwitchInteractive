#pragma once

#include "command/events/PlayLayerEvent.hpp"
#include "command/events/PlayerObjectEvent.hpp"

#include <string>
#include <array>
#include <vector>
#include <functional>
#include <unordered_map>
#include <filesystem>
#include <random>

#include <Geode/Geode.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/ui/LazySprite.hpp>
#include "command/events/KeyReleaseScheduler.hpp"

#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>

using namespace geode::prelude;
namespace web = geode::utils::web;


// Command arguments
enum class CommandIdentifiers
{
    Argument = 0,
    Username = 1,
    Displayname = 2,
    UserID = 3,
    Streamer = 4
};

// Enums for the type of callback
enum class CommandActionType
{
    Notification = 0,
    Keybind = 1,
    Chat = 2,
    Event = 3,
    Wait = 4
};

// Helper for countdown logging
struct CountdownLogger : public CCObject
{
    float remaining;
    std::string commandName;
    size_t actionIndex;

    CountdownLogger(float rem, const std::string &cmd, size_t idx) : remaining(rem), commandName(cmd), actionIndex(idx) {};

    void log(CCObject *)
    {
        log::info("Wait countdown for command '{}', action {}: {:.2f} second(s) remaining", commandName, actionIndex, remaining);
    };
};

// A quick command action
struct TwitchCommandAction
{
    matjson::Value toJson() const;
    static TwitchCommandAction fromJson(const matjson::Value &v);
    CommandActionType type = CommandActionType::Notification; // Type of callback
    std::string arg = "";                                     // A string to pass to the callback
    float index = 0.f;                                        // Priority order

    TwitchCommandAction(
        CommandActionType actType = CommandActionType::Notification,
        const std::string &actArg = "",
        float actIndex = 0.f) : type(actType), arg(actArg), index(actIndex) {};
};

// Template for a command
struct TwitchCommand
{
    matjson::Value toJson() const;
    static TwitchCommand fromJson(const matjson::Value &v);
    std::string name;        // All lowercase name of the command
    std::string description; // Brief description of the command

    std::vector<TwitchCommandAction> actions; // List of actions in order

    // User/role restrictions
    std::string allowedUser;
    bool allowVip = false;
    bool allowMod = false;
    bool allowStreamer = false;
    bool allowSubscriber = false;

    bool showCooldown = false; // Show cooldown notification when on cooldown

    bool enabled = true; // If the command is enabled
    int cooldown = 0;    // Cooldown in seconds

    std::function<void(const std::string &)> callback; // Custom callback

    TwitchCommand(
        const std::string &cmdName = "",
        const std::string &cmdDesc = "",
        int cmdCooldown = 0,
        std::vector<TwitchCommandAction> cmdActions = {},
        const std::string &allowedUser_ = "",
        bool allowVip_ = false,
        bool allowMod_ = false,
        bool allowStreamer_ = false,
        bool allowSubscriber_ = false) : name(cmdName), description(cmdDesc), cooldown(cmdCooldown), actions(cmdActions),
                                         allowedUser(allowedUser_), allowVip(allowVip_), allowMod(allowMod_), allowStreamer(allowStreamer_), allowSubscriber(allowSubscriber_) {};
};

class TwitchCommandManager
{
private:
    std::vector<TwitchCommand> m_commands;
    bool m_isListening = false;
    void loadCommands();
    std::string getSavePath() const;

public:
    static TwitchCommandManager *getInstance();
    ~TwitchCommandManager();

    void addCommand(const TwitchCommand &command);
    void removeCommand(const std::string &name);
    void enableCommand(const std::string &name, bool enable);
    std::vector<TwitchCommand> &getCommands();

    void saveCommands();

    void handleChatMessage(const ChatMessage &chatMessage);
};

extern std::unordered_map<std::string, time_t> commandCooldowns;

// Sequential Action Execution
// Future: Use an enum for identifiers (e.g., ${arg}, ${username}, etc.) (here in this file btw! i think)
struct ActionContext : public CCObject
{
    std::vector<TwitchCommandAction> actions;
    size_t index = 0;
    std::string commandName;
    std::string username;
    std::string displayName;
    std::string userID;
    std::string commandArgs;
    std::string streamerUsername;
    TwitchCommandManager *manager = nullptr;

    // Helper to replace identifiers in action arguments
    std::string replaceIdentifiers(const std::string &input)
    {
        std::string result = input;

        // Replace all ${arg} with commandArgs
        size_t pos = 0;
        while ((pos = result.find("${arg}", pos)) != std::string::npos)
        {
            result.replace(pos, 6, commandArgs);
            pos += commandArgs.length();
        };

        // Replace all ${username} with username
        pos = 0;
        while ((pos = result.find("${username}", pos)) != std::string::npos)
        {
            result.replace(pos, 11, username);
            pos += username.length();
        };

        // Replace all ${displayname} with displayName
        pos = 0;
        while ((pos = result.find("${displayname}", pos)) != std::string::npos)
        {
            result.replace(pos, 14, displayName);
            pos += displayName.length();
        };

        // Replace all ${userid} with userID
        pos = 0;
        while ((pos = result.find("${userid}", pos)) != std::string::npos)
        {
            result.replace(pos, 9, userID);
            pos += userID.length();
        };

        // Replace all ${streamer} with the configured Twitch channel (streamer's username)
        pos = 0;
        if (auto twitchMod = Loader::get()->getLoadedMod("alphalaneous.twitch_chat_api"))
        {
            streamerUsername = twitchMod->getSavedValue<std::string>("twitch-channel");
        };

        while ((pos = result.find("${streamer}", pos)) != std::string::npos)
        {
            result.replace(pos, 11, streamerUsername);
            pos += streamerUsername.length();
        };

        // Replace all ${rng<min>:<max>} with a random integer in [min, max]
        size_t rpos = 0;
        static std::mt19937 rng(std::random_device{}());
        while ((rpos = result.find("${rng", rpos)) != std::string::npos)
        {
            size_t start = rpos;           // position of '${rng'
            size_t paramsBegin = rpos + 5; // after '${rng'
            size_t endBrace = result.find('}', paramsBegin);
            if (endBrace == std::string::npos)
                break; // malformed; stop processing further

            std::string params = result.substr(paramsBegin, endBrace - paramsBegin); // expected '<min>:<max>'

            // Expect optional '<' and '>' around min:max; tolerate both with or without angle brackets
            // Strip optional leading '<' and trailing '>'
            if (!params.empty() && params.front() == '<' && params.back() == '>')
                params = params.substr(1, params.size() - 2);

            size_t colon = params.find(':');
            if (colon == std::string::npos)
            {
                rpos = endBrace + 1; // skip malformed
                continue;
            }

            auto trim = [](std::string s)
            {
                s.erase(0, s.find_first_not_of(" \t\n\r"));
                s.erase(s.find_last_not_of(" \t\n\r") + 1);
                return s;
            };

            std::string minStr = trim(params.substr(0, colon));
            std::string maxStr = trim(params.substr(colon + 1));

            auto isInt = [](const std::string &s) -> bool
            {
                if (s.empty())
                    return false;
                size_t i = 0;
                if (s[0] == '-' || s[0] == '+')
                    i = 1;
                if (i >= s.size())
                    return false;
                for (; i < s.size(); ++i)
                    if (s[i] < '0' || s[i] > '9')
                        return false;
                return true;
            };

            if (!isInt(minStr) || !isInt(maxStr))
            {
                rpos = endBrace + 1;
                continue;
            }

            int minV = numFromString<int>(minStr).unwrapOrDefault();
            int maxV = numFromString<int>(maxStr).unwrapOrDefault();
            if (minV > maxV)
                std::swap(minV, maxV);

            std::uniform_int_distribution<int> dist(minV, maxV);
            int value = dist(rng);
            std::string replacement = std::to_string(value);

            result.replace(start, endBrace - start + 1, replacement);
            rpos = start + replacement.size();
        }

        return result;
    }

    void execute(CCObject *obj)
    {
        auto *ctx = static_cast<ActionContext *>(obj);

        if (!ctx || ctx->index >= ctx->actions.size())
        {
            if (ctx)
                ctx->release();
            return;
        };

        // Debug log: print the full action order and current action
        std::ostringstream orderLog;
        orderLog << "Action order for command '" << ctx->commandName << "': ";

        for (size_t i = 0; i < ctx->actions.size(); ++i)
        {
            const auto &a = ctx->actions[i];

            orderLog << "[" << i << "] type=" << (int)a.type << ", arg=" << a.arg << ", index=" << a.index;

            if (i == ctx->index)
                orderLog << " <-- executing";

            orderLog << "; ";
        };

        log::debug("{}", orderLog.str());

        const auto &action = ctx->actions[ctx->index];

        std::string processedArg = ctx->replaceIdentifiers(action.arg);
        log::info("Executing action {}: type={}, arg={}, index={}", ctx->index, (int)action.type, processedArg, action.index);

        // Handle Wait type
        if (action.type == CommandActionType::Wait)
        {
            float delay = action.index;

            if (delay <= 0.f && !processedArg.empty() && processedArg.find_first_not_of("-.0123456789") == std::string::npos)
                delay = numFromString<float>(processedArg).unwrapOrDefault();

            delay = std::round(delay * 1000.0f) / 1000.0f;

            if (delay > 0.f)
            {
                log::info("Waiting for {:.2f} seconds before next action (command '{}', action {})", delay, ctx->commandName, ctx->index);

                if (auto scene = CCDirector::sharedDirector()->getRunningScene())
                {
                    int intDelay = static_cast<int>(delay);

                    for (int i = 1; i <= intDelay; ++i)
                    {
                        auto logger = new CountdownLogger(intDelay - i + 1, ctx->commandName, ctx->index);

                        scene->runAction(CCSequence::create(
                            CCDelayTime::create(static_cast<float>(i)),
                            CCCallFuncO::create(logger, callfuncO_selector(CountdownLogger::log), logger),
                            nullptr));
                    };
                };

                ctx->index++;

                auto seq = CCSequence::create(
                    CCDelayTime::create(delay),
                    CCCallFuncO::create(ctx, callfuncO_selector(ActionContext::execute), ctx),
                    nullptr);

                if (auto scene = CCDirector::sharedDirector()->getRunningScene())
                {
                    ctx->retain();
                    scene->runAction(seq);
                }
                else
                {
                    ctx->execute(ctx);
                };

                return;
            };
        };

        // Handle Keybind type: arg format "<key>[:<durationSeconds>]"; empty duration = infinite hold
        if (action.type == CommandActionType::Keybind)
        {
            std::string keyStr;
            float duration = -1.f; // negative = infinite

            size_t colon = processedArg.find(":");
            if (colon != std::string::npos)
            {
                keyStr = processedArg.substr(0, colon);
                std::string durStr = processedArg.substr(colon + 1);
                if (!durStr.empty() && durStr.find_first_not_of("-.0123456789") == std::string::npos)
                    duration = numFromString<float>(durStr).unwrapOrDefault();
            }
            else
            {
                keyStr = processedArg;
            }

            // trim keyStr
            keyStr.erase(0, keyStr.find_first_not_of(" \t\n\r"));
            keyStr.erase(keyStr.find_last_not_of(" \t\n\r") + 1);

            auto toUpper = [](std::string s)
            {
                geode::utils::string::toUpperIP(s);
                return s;
            };

            cocos2d::enumKeyCodes code = static_cast<cocos2d::enumKeyCodes>(0);
            bool found = false;
            std::string up = toUpper(keyStr);

            if (keyStr.size() == 1)
            {
                char c = keyStr[0];
                if (c >= 'a' && c <= 'z')
                    c = static_cast<char>(c - 'a' + 'A');

                if (c >= 'A' && c <= 'Z')
                {
                    code = static_cast<cocos2d::enumKeyCodes>(cocos2d::KEY_A + (c - 'A'));
                    found = true;
                }
                else if (c >= '0' && c <= '9')
                {
                    code = static_cast<cocos2d::enumKeyCodes>(c);
                    found = true;
                }
                else
                {
                    // punctuation by known VK codes used elsewhere
                    switch (c)
                    {
                    case ';':
                        code = static_cast<cocos2d::enumKeyCodes>(4101);
                        found = true;
                        break;
                    case '=':
                        code = static_cast<cocos2d::enumKeyCodes>(4097);
                        found = true;
                        break;
                    case ',':
                        code = static_cast<cocos2d::enumKeyCodes>(188);
                        found = true;
                        break;
                    case '-':
                        code = static_cast<cocos2d::enumKeyCodes>(189);
                        found = true;
                        break;
                    case '.':
                        code = static_cast<cocos2d::enumKeyCodes>(190);
                        found = true;
                        break;
                    case '/':
                        code = static_cast<cocos2d::enumKeyCodes>(4103);
                        found = true;
                        break;
                    case '`':
                        code = static_cast<cocos2d::enumKeyCodes>(4096);
                        found = true;
                        break;
                    case '[':
                        code = static_cast<cocos2d::enumKeyCodes>(4098);
                        found = true;
                        break;
                    case '\\':
                        code = static_cast<cocos2d::enumKeyCodes>(4100);
                        found = true;
                        break;
                    case ']':
                        code = static_cast<cocos2d::enumKeyCodes>(4099);
                        found = true;
                        break;
                    case '\'':
                        code = static_cast<cocos2d::enumKeyCodes>(4102);
                        found = true;
                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {
                // named keys
                if (up == "SPACE")
                {
                    code = cocos2d::KEY_Space;
                    found = true;
                }
                else if (up == "ENTER" || up == "RETURN")
                {
                    code = cocos2d::KEY_Enter;
                    found = true;
                }
                else if (up == "ESC" || up == "ESCAPE")
                {
                    code = cocos2d::KEY_Escape;
                    found = true;
                }
                else if (up == "LEFT")
                {
                    code = cocos2d::KEY_Left;
                    found = true;
                }
                else if (up == "RIGHT")
                {
                    code = cocos2d::KEY_Right;
                    found = true;
                }
                else if (up == "UP")
                {
                    code = cocos2d::KEY_Up;
                    found = true;
                }
                else if (up == "DOWN")
                {
                    code = cocos2d::KEY_Down;
                    found = true;
                }
                else if (up == "TAB")
                {
                    code = cocos2d::KEY_Tab;
                    found = true;
                }
                else if (up == "BACKSPACE" || up == "BKSP")
                {
                    code = cocos2d::KEY_Backspace;
                    found = true;
                }
                else if (up == "SHIFT")
                {
                    code = cocos2d::KEY_Shift;
                    found = true;
                }
                else if (up == "CTRL" || up == "CONTROL")
                {
                    code = cocos2d::KEY_Control;
                    found = true;
                }
                else if (up == "ALT")
                {
                    code = cocos2d::KEY_Alt;
                    found = true;
                }
                else if (up == "CAPSLOCK")
                {
                    code = static_cast<cocos2d::enumKeyCodes>(20);
                    found = true;
                }
                else if (up == "LEFTSHIFT")
                {
                    code = static_cast<cocos2d::enumKeyCodes>(160);
                    found = true;
                }
                else if (up == "RIGHTSHIFT")
                {
                    code = static_cast<cocos2d::enumKeyCodes>(161);
                    found = true;
                }
            }

            if (!found)
            {
                log::warn("[Keybind] Unknown key string '{}'", keyStr);
            }
            else
            {
                auto disp = CCDirector::sharedDirector()->getKeyboardDispatcher();
                if (disp)
                {
                    // Press
                    disp->dispatchKeyboardMSG(code, true, false);

                    // Release
                    if (duration <= 0.f)
                    {
                        disp->dispatchKeyboardMSG(code, false, false);
                    }
                    else
                    {
                        if (auto scene = CCDirector::sharedDirector()->getRunningScene())
                        {
                            auto node = KeyReleaseScheduler::create([disp, code]()
                                                                    { disp->dispatchKeyboardMSG(code, false, false); }, duration);
                            if (node)
                                scene->addChild(node);
                        }
                        else
                        {
                            disp->dispatchKeyboardMSG(code, false, false);
                        }
                    }
                }
            }
        }

        // Handle other Event types
        if (action.type == CommandActionType::Event)
        {
            // Jumpscare event: jumpscare:<fileName>:<fade>:<scale>
            if (processedArg.rfind("jumpscare:", 0) == 0)
            {
                std::string imageJS;
                float fade = 0.5f;
                float scaleMul = 1.0f;
                size_t firstColon = processedArg.find(":");
                size_t secondColon = (firstColon != std::string::npos ? processedArg.find(":", firstColon + 1) : std::string::npos);
                size_t thirdColon = (secondColon != std::string::npos ? processedArg.find(":", secondColon + 1) : std::string::npos);
                if (firstColon != std::string::npos)
                {
                    if (secondColon != std::string::npos)
                    {
                        imageJS = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                        std::string fadeStr;
                        if (thirdColon != std::string::npos)
                        {
                            fadeStr = processedArg.substr(secondColon + 1, thirdColon - secondColon - 1);
                            std::string scaleStr = processedArg.substr(thirdColon + 1);
                            scaleStr.erase(0, scaleStr.find_first_not_of(" \t\n\r"));
                            if (!scaleStr.empty() && scaleStr.find_first_not_of("-.0123456789") == std::string::npos)
                                scaleMul = numFromString<float>(scaleStr).unwrapOrDefault();
                            if (!(scaleMul > 0.f))
                                scaleMul = 1.0f;
                        }
                        else
                        {
                            fadeStr = processedArg.substr(secondColon + 1);
                        }
                        fadeStr.erase(0, fadeStr.find_first_not_of(" \t\n\r"));
                        if (!fadeStr.empty() && fadeStr.find_first_not_of("-.0123456789") == std::string::npos)
                            fade = numFromString<float>(fadeStr).unwrapOrDefault();
                    }
                    else
                    {
                        imageJS = processedArg.substr(firstColon + 1);
                    }
                }

                // Trim file name
                imageJS.erase(0, imageJS.find_first_not_of(" \t\n\r"));
                imageJS.erase(imageJS.find_last_not_of(" \t\n\r") + 1);

                if (imageJS.empty())
                {
                    log::warn("[Jumpscare] Empty file name; skipping (command: {})", ctx->commandName);
                }
                else if (auto scene = CCDirector::sharedDirector()->getRunningScene())
                {
                    // Fullscreen layer to host the sprite and capture focus
                    auto layer = CCLayerColor::create({0, 0, 0, 0});
                    layer->setID("jumpscare-layer");
                    scene->addChild(layer, 9999);

                    // Create LazySprite as a holder that covers the screen
                    auto win = CCDirector::sharedDirector()->getWinSize();
                    auto ls = geode::LazySprite::create({win.width, win.height}, true);
                    if (!ls)
                    {
                        log::warn("[Jumpscare] Failed to create LazySprite for file: {}", imageJS);
                    }
                    else
                    {
                        ls->setID("jumpscare-image");
                        ls->setAnchorPoint({0.5f, 0.5f});
                        ls->setPosition({win.width / 2.f, win.height / 2.f});
                        ls->setOpacity(255);

                        layer->addChild(ls);

                        // Build absolute path to custom jumpscare folder
                        auto base = Mod::get()->getConfigDir();
                        std::string fullPath;

                        // If token is 'random', pick a random file in the folder
                        std::string imgLower = imageJS; geode::utils::string::toLowerIP(imgLower);
                        if (imgLower == "random")
                        {
                            std::vector<std::string> files;
                            auto dir = (base / "jumpscare").string();
                            if (auto rd = geode::utils::file::readDirectory(dir, false))
                            {
                                for (auto const &entry : rd.unwrap())
                                {
                                    std::filesystem::path p = entry;
                                    std::filesystem::path full = p.is_absolute() ? p : (base / "jumpscare" / p);
                                    std::error_code ec;
                                    if (std::filesystem::is_regular_file(full, ec))
                                        files.push_back(full.string());
                                }
                            }
                            if (!files.empty())
                            {
                                static std::mt19937 rng(std::random_device{}());
                                std::uniform_int_distribution<size_t> dist(0, files.size() - 1);
                                fullPath = files[dist(rng)];
                            }
                            else
                            {
                                log::warn("[Jumpscare] No files found in jumpscare folder.");
                            }
                        }
                        else
                        {
                            fullPath = (base / "jumpscare" / imageJS).string();
                        }
                        if (!std::filesystem::exists(fullPath))
                        {
                            log::warn("[Jumpscare] Image file does not exist: {}", fullPath);
                        }
                        // Load image into LazySprite
                        ls->loadFromFile(fullPath);
                        if (scaleMul > 0.f && scaleMul != 1.f)
                            ls->setScale(scaleMul);

                        // Fade-out after a small hold; if fade <= 0, just remove instantly
                        float hold = 0.25f;
                        float fadeDur = std::max(0.f, fade);

                        // Fade the lazysprite
                        auto fadeSeq = CCSequence::create(
                            CCDelayTime::create(hold),
                            CCFadeTo::create(fadeDur, 0),
                            CCCallFunc::create(ls, callfunc_selector(CCNode::removeFromParent)),
                            nullptr);
                        ls->runAction(fadeSeq);

                        // Remove the layer after the fade is done
                        float totalTime = hold + fadeDur;
                        auto removeLayerSeq = CCSequence::create(
                            CCDelayTime::create(totalTime),
                            CCCallFunc::create(layer, callfunc_selector(CCNode::removeFromParent)),
                            nullptr);
                        layer->runAction(removeLayerSeq);
                    }
                }
            }
            // Keycode event: keycode:<key>[:<durationSeconds>]
            if (processedArg.rfind("keycode:", 0) == 0)
            {
                std::string rest = processedArg.substr(8);

                // split key and optional duration
                std::string keyStr;
                float duration = -1.f; // negative = infinite hold
                size_t colon = rest.find(":");
                if (colon != std::string::npos)
                {
                    keyStr = rest.substr(0, colon);
                    std::string durStr = rest.substr(colon + 1);
                    // trim
                    durStr.erase(0, durStr.find_first_not_of(" \t\n\r"));
                    if (!durStr.empty())
                    {
                        if (durStr.find_first_not_of("-.0123456789") == std::string::npos)
                            duration = numFromString<float>(durStr).unwrapOrDefault();
                        else
                            duration = 0.f; // malformed -> tap
                    }
                }
                else
                {
                    keyStr = rest;
                }

                // trim key
                keyStr.erase(0, keyStr.find_first_not_of(" \t\n\r"));
                keyStr.erase(keyStr.find_last_not_of(" \t\n\r") + 1);

                auto toUpper = [](std::string s)
                {
                    geode::utils::string::toUpperIP(s);
                    return s;
                };

                cocos2d::enumKeyCodes code = static_cast<cocos2d::enumKeyCodes>(0);
                bool found = false;
                std::string up = toUpper(keyStr);

                if (keyStr.size() == 1)
                {
                    char c = keyStr[0];
                    if (c >= 'a' && c <= 'z')
                        c = static_cast<char>(c - 'a' + 'A');
                    if (c >= 'A' && c <= 'Z')
                    {
                        code = static_cast<cocos2d::enumKeyCodes>(cocos2d::KEY_A + (c - 'A'));
                        found = true;
                    }
                    else if (c >= '0' && c <= '9')
                    {
                        code = static_cast<cocos2d::enumKeyCodes>(c);
                        found = true;
                    }
                    else
                    {
                        switch (c)
                        {
                        case ';':
                            code = static_cast<cocos2d::enumKeyCodes>(4101);
                            found = true;
                            break;
                        case '=':
                            code = static_cast<cocos2d::enumKeyCodes>(4097);
                            found = true;
                            break;
                        case ',':
                            code = static_cast<cocos2d::enumKeyCodes>(188);
                            found = true;
                            break;
                        case '-':
                            code = static_cast<cocos2d::enumKeyCodes>(189);
                            found = true;
                            break;
                        case '.':
                            code = static_cast<cocos2d::enumKeyCodes>(190);
                            found = true;
                            break;
                        case '/':
                            code = static_cast<cocos2d::enumKeyCodes>(4103);
                            found = true;
                            break;
                        case '`':
                            code = static_cast<cocos2d::enumKeyCodes>(4096);
                            found = true;
                            break;
                        case '[':
                            code = static_cast<cocos2d::enumKeyCodes>(4098);
                            found = true;
                            break;
                        case '\\':
                            code = static_cast<cocos2d::enumKeyCodes>(4100);
                            found = true;
                            break;
                        case ']':
                            code = static_cast<cocos2d::enumKeyCodes>(4099);
                            found = true;
                            break;
                        case '\'':
                            code = static_cast<cocos2d::enumKeyCodes>(4102);
                            found = true;
                            break;
                        default:
                            break;
                        }
                    }
                }
                else
                {
                    if (up == "SPACE")
                    {
                        code = cocos2d::KEY_Space;
                        found = true;
                    }
                    else if (up == "ENTER" || up == "RETURN")
                    {
                        code = cocos2d::KEY_Enter;
                        found = true;
                    }
                    else if (up == "ESC" || up == "ESCAPE")
                    {
                        code = cocos2d::KEY_Escape;
                        found = true;
                    }
                    else if (up == "LEFT")
                    {
                        code = cocos2d::KEY_Left;
                        found = true;
                    }
                    else if (up == "RIGHT")
                    {
                        code = cocos2d::KEY_Right;
                        found = true;
                    }
                    else if (up == "UP")
                    {
                        code = cocos2d::KEY_Up;
                        found = true;
                    }
                    else if (up == "DOWN")
                    {
                        code = cocos2d::KEY_Down;
                        found = true;
                    }
                    else if (up == "TAB")
                    {
                        code = cocos2d::KEY_Tab;
                        found = true;
                    }
                    else if (up == "BACKSPACE" || up == "BKSP")
                    {
                        code = cocos2d::KEY_Backspace;
                        found = true;
                    }
                    else if (up == "SHIFT")
                    {
                        code = cocos2d::KEY_Shift;
                        found = true;
                    }
                    else if (up == "CTRL" || up == "CONTROL")
                    {
                        code = cocos2d::KEY_Control;
                        found = true;
                    }
                    else if (up == "ALT")
                    {
                        code = cocos2d::KEY_Alt;
                        found = true;
                    }
                    else if (up == "CAPSLOCK")
                    {
                        code = static_cast<cocos2d::enumKeyCodes>(20);
                        found = true;
                    }
                    else if (up == "LEFTSHIFT")
                    {
                        code = static_cast<cocos2d::enumKeyCodes>(160);
                        found = true;
                    }
                    else if (up == "RIGHTSHIFT")
                    {
                        code = static_cast<cocos2d::enumKeyCodes>(161);
                        found = true;
                    }
                }

                if (!found)
                {
                    log::warn("[Keycode Event] Unknown key string '{}'", keyStr);
                }
                else
                {
                    auto disp = cocos2d::CCKeyboardDispatcher::get();
                    if (!disp)
                        disp = CCDirector::sharedDirector()->getKeyboardDispatcher();
                    if (disp)
                    {
                        // key down
                        disp->dispatchKeyboardMSG(code, true, false);

                        // schedule release
                        if (duration <= 0.f)
                        {
                            disp->dispatchKeyboardMSG(code, false, false);
                        }
                        else if (auto scene = CCDirector::sharedDirector()->getRunningScene())
                        {
                            auto node = KeyReleaseScheduler::create([disp, code]()
                                                                    { disp->dispatchKeyboardMSG(code, false, false); }, duration);
                            if (node)
                                scene->addChild(node);
                        }
                        else
                        {
                            disp->dispatchKeyboardMSG(code, false, false);
                        }
                    }
                }
            }

            // Noclip event: noclip:true or noclip:false
            if (processedArg.rfind("noclip:", 0) == 0)
            {
                bool enableNoclip = false;
                std::string value = processedArg.substr(7);

                if (value == "true")
                    enableNoclip = true;

                else if (value == "false")
                    enableNoclip = false;

                log::info("Setting noclip to {} (command: {})", enableNoclip ? "true" : "false", ctx->commandName);
                PlayLayerEvent::setNoclip(enableNoclip);
            };

            // Gravity event: gravity:<gravity>:<duration>
            if (processedArg.rfind("gravity:", 0) == 0)
            {
                float gravity = 1.0f;
                float duration = 0.5f;

                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);

                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string gravityStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string durationStr = processedArg.substr(secondColon + 1);

                    if (!gravityStr.empty() && gravityStr.find_first_not_of("-.0123456789") == std::string::npos)
                        gravity = numFromString<float>(gravityStr).unwrapOrDefault();

                    if (!durationStr.empty() && durationStr.find_first_not_of("-.0123456789") == std::string::npos)
                        duration = numFromString<float>(durationStr).unwrapOrDefault();
                };

                log::info("Triggering gravity event: gravity={} duration={} (command: {})", gravity, duration, ctx->commandName);

                auto playLayer = PlayLayer::get();
                if (playLayer && playLayer->m_player1)
                {
                    if (auto event = PlayerObjectEvent::create(playLayer->m_player1, gravity, duration))
                    {
                        playLayer->addChild(event);
                        event->applyGravity();
                    }
                    else
                    {
                        log::warn("[GravityEvent] Failed to create PlayerObjectEvent");
                    };
                }
                else
                {
                    log::warn("[GravityEvent] PlayLayer or player not found");
                };
                // Speed event: speed_player:<speed>:<duration>
            }
            else if (processedArg.rfind("speed_player:", 0) == 0)
            {
                float speed = 1.0f;
                float duration = 0.5f;

                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);

                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string speedStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string durationStr = processedArg.substr(secondColon + 1);

                    if (!speedStr.empty() && speedStr.find_first_not_of("-.0123456789") == std::string::npos)
                        speed = numFromString<float>(speedStr).unwrapOrDefault();

                    if (!durationStr.empty() && durationStr.find_first_not_of("-.0123456789") == std::string::npos)
                        duration = numFromString<float>(durationStr).unwrapOrDefault();
                };

                log::info("Triggering speed event: speed={} duration={} (command: {})", speed, duration, ctx->commandName);

                auto playLayer = PlayLayer::get();
                if (playLayer && playLayer->m_player1)
                {
                    if (auto event = PlayerObjectEvent::create(playLayer->m_player1, 1.0f, 0.0f, speed, duration))
                    {
                        playLayer->addChild(event);
                        event->applySpeed();
                    }
                    else
                    {
                        log::warn("[SpeedEvent] Failed to create PlayerObjectEvent");
                    };
                }
                else
                {
                    log::warn("[SpeedEvent] PlayLayer or player not found");
                };
            }
            else if (processedArg == "kill_player")
            {
                log::info("Triggering kill player event for command: {}", ctx->commandName);
                PlayLayerEvent::killPlayer();
            }
            else if (processedArg == "reverse_player")
            {
                log::info("Triggering reverse player event for command: {}", ctx->commandName);
                PlayLayerEvent::reversePlayer();
            }
            else if (processedArg.rfind("player_effect:", 0) == 0)
            {
                // player_effect:<player>:<kind> or legacy player_effect:<kind>
                int playerIdx = 1;
                std::string kind;
                std::string rest = processedArg.substr(std::string("player_effect:").size());
                size_t sep = rest.find(":");
                if (sep != std::string::npos) {
                    std::string pStr = rest.substr(0, sep);
                    kind = rest.substr(sep + 1);
                    if (!pStr.empty() && pStr.find_first_not_of("-0123456789") == std::string::npos)
                        playerIdx = numFromString<int>(pStr).unwrapOrDefault();
                } else {
                    kind = rest; // legacy
                }
                geode::utils::string::toLowerIP(kind);
                auto playLayer = PlayLayer::get();
                if (playLayer)
                {
                    PlayerObject* target = (playerIdx == 2 ? playLayer->m_player2 : playLayer->m_player1);
                    if (target)
                    {
                        if (kind == "spawn")
                        {
                            log::info("Playing spawn effect on P{} (command: {})", playerIdx, ctx->commandName);
                            target->playSpawnEffect();
                        }
                        else
                        {
                            log::info("Playing death effect on P{} (command: {})", playerIdx, ctx->commandName);
                            target->playDeathEffect();
                        }
                    }
                    else
                    {
                        log::warn("[PlayerEffect] Player {} not available", playerIdx);
                    }
                }
                else
                {
                    log::warn("[PlayerEffect] PlayLayer not found");
                }
            }
            else if (processedArg == "restart_level")
            {
                log::info("Triggering restart level event for command: {}", ctx->commandName);
                PlayLayerEvent::restartLevel();
            }
            else if (processedArg.rfind("edit_camera:", 0) == 0)
            {
                log::info("Triggering edit camera event: {}", processedArg);
                PlayLayerEvent::setCameraFromString(processedArg);
            }
            else if (processedArg.rfind("sound_effect:", 0) == 0 || processedArg.rfind("sound:", 0) == 0)
            {
                // Sound Effects: sound_effect:<sound>:<speed>:<volume>:<pitch>:<start>:<end>
                size_t firstColon = processedArg.find(":");
                if (firstColon == std::string::npos || firstColon + 1 >= processedArg.size())
                {
                    log::warn("Sound effect action triggered but no parameters provided (command: {})", ctx->commandName);
                }
                else
                {
                    std::string rest = processedArg.substr(firstColon + 1);

                    // Split by ':'
                    std::vector<std::string> parts;
                    size_t start = 0;
                    while (true)
                    {
                        size_t pos = rest.find(":", start);
                        if (pos == std::string::npos)
                        {
                            parts.push_back(rest.substr(start));
                            break;
                        }
                        parts.push_back(rest.substr(start, pos - start));
                        start = pos + 1;
                    }

                    auto trim = [](std::string s)
                    {
                        s.erase(0, s.find_first_not_of(" \t\n\r"));
                        s.erase(s.find_last_not_of(" \t\n\r") + 1);
                        return s;
                    };

                    for (auto &p : parts)
                        p = trim(p);

                    std::string soundName = parts.size() >= 1 ? parts[0] : std::string("");

                    auto resolveSfxPath = [](const std::string &name) -> std::string
                    {
                        std::error_code ec;
                        if (std::filesystem::exists(name, ec))
                            return name; // already a path
                        auto p = Mod::get()->getConfigDir() / "sfx" / name;
                        if (std::filesystem::exists(p, ec))
                            return p.string();
                        return name; // fallback to builtin resource
                    };

                    if (soundName.empty())
                    {
                        log::warn("Sound effect action has empty sound name (command: {})", ctx->commandName);
                    }
                    else if (auto audioEngine = FMODAudioEngine::sharedEngine())
                    {
                        // Defaults
                        float speed = 1.0f;
                        float volume = 1.0f;
                        float pitch = 0.0f;
                        int startMillis = 0;
                        int endMillis = 0;

                        // Parse extended parameters if provided
                        if (parts.size() >= 2 && parts[1].find_first_not_of("-.0123456789") == std::string::npos)
                            speed = numFromString<float>(parts[1]).unwrapOrDefault();
                        if (parts.size() >= 3 && parts[2].find_first_not_of("-.0123456789") == std::string::npos)
                            volume = numFromString<float>(parts[2]).unwrapOrDefault();
                        if (parts.size() >= 4 && parts[3].find_first_not_of("-.0123456789") == std::string::npos)
                            pitch = numFromString<float>(parts[3]).unwrapOrDefault();
                        if (parts.size() >= 5 && parts[4].find_first_not_of("-0123456789") == std::string::npos)
                            startMillis = numFromString<int>(parts[4]).unwrapOrDefault();
                        if (parts.size() >= 6 && parts[5].find_first_not_of("-0123456789") == std::string::npos)
                            endMillis = numFromString<int>(parts[5]).unwrapOrDefault();

                        // If only legacy param (sound name) was provided, use simple playEffect
                        if (parts.size() == 1)
                        {
                            log::info("Playing sound effect '{}' (legacy) (command: {})", soundName, ctx->commandName);
                            audioEngine->playEffect(resolveSfxPath(soundName));
                        }
                        else
                        {
                            log::info(
                                "Playing sound effect '{}' with speed={} vol={} pitch={} start={} end={} (command: {})",
                                soundName, speed, volume, pitch, startMillis, endMillis, ctx->commandName);
                            auto soundPath = resolveSfxPath(soundName);
                            audioEngine->playEffectAdvanced(
                                soundPath, speed, 0.0f, volume, pitch, false, false, startMillis, endMillis,
                                0, 0, false, 0, false, false, 0, 0.0f, 0.f, 0);
                        }
                    }
                }
            }
            else if (processedArg.rfind("scale_player:", 0) == 0)
            {
                int playerIdx = 1;
                float scale = 1.0f;
                float time = 0.0f;

                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);
                size_t thirdColon = (secondColon != std::string::npos) ? processedArg.find(":", secondColon + 1) : std::string::npos;

                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string scaleStr, timeStr;
                    // If three colons, treat as scale_player:<player>:<scale>:<time>
                    if (thirdColon != std::string::npos)
                    {
                        std::string playerStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);

                        scaleStr = processedArg.substr(secondColon + 1, thirdColon - secondColon - 1);
                        timeStr = processedArg.substr(thirdColon + 1);

                        if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos)
                            playerIdx = numFromString<int>(playerStr).unwrapOrDefault();

                        if (!scaleStr.empty() && scaleStr.find_first_not_of("-.0123456789") == std::string::npos)
                            scale = numFromString<float>(scaleStr).unwrapOrDefault();

                        if (!timeStr.empty() && timeStr.find_first_not_of("-.0123456789") == std::string::npos)
                            time = numFromString<float>(timeStr).unwrapOrDefault();
                    }
                    else
                    {
                        // scale_player:<scale>:<time> (no player index)
                        scaleStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                        timeStr = processedArg.substr(secondColon + 1);

                        if (!scaleStr.empty() && scaleStr.find_first_not_of("-.0123456789") == std::string::npos)
                            scale = numFromString<float>(scaleStr).unwrapOrDefault();
                        if (!timeStr.empty() && timeStr.find_first_not_of("-.0123456789") == std::string::npos)
                            time = numFromString<float>(timeStr).unwrapOrDefault();
                    };
                }
                else if (firstColon != std::string::npos)
                {
                    std::string scaleStr = processedArg.substr(firstColon + 1);

                    if (!scaleStr.empty() && scaleStr.find_first_not_of("-.0123456789") == std::string::npos)
                        scale = numFromString<float>(scaleStr).unwrapOrDefault();
                };

                log::info("Setting scale for player {} to {} (time: {}, command: {})", playerIdx, scale, time, ctx->commandName);
                PlayLayerEvent::scalePlayer(playerIdx, scale, time);
            }
            else if (processedArg.rfind("alert_popup:", 0) == 0)
            {
                std::string title = "-", desc = "-";

                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);

                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    title = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    desc = processedArg.substr(secondColon + 1);

                    if (title.empty())
                        title = "-";
                    if (desc.empty())
                        desc = "-";
                };

                log::info("Showing alert popup: title='{}', desc='{}' (command: {})", title, desc, ctx->commandName);
                FLAlertLayer::create(title.c_str(), desc.c_str(), "OK")->show();
            }
            else if (processedArg == "stop_all_sounds")
            {
                log::info("Stopping all sound effects (command: {})", ctx->commandName);

                if (auto audioEngine = FMODAudioEngine::sharedEngine())
                    audioEngine->stopAllEffects();
            }
            else if (processedArg.rfind("jump:", 0) == 0)
            {
                int playerIdx = 1;
                bool hold = false;

                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);

                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string playerStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    std::string typeStr = processedArg.substr(secondColon + 1);

                    if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos)
                        playerIdx = numFromString<int>(playerStr).unwrapOrDefault();

                    if (typeStr == "hold")
                        hold = true;
                };

                if (hold)
                {
                    PlayLayerEvent::jumpPlayerHold(playerIdx);
                }
                else
                {
                    PlayLayerEvent::jumpPlayerTap(playerIdx);
                };
            }
            else if (processedArg.rfind("move:", 0) == 0)
            {
                int playerIdx = 1;
                bool moveRight = true;

                float distance = 0.f;
                bool validDistance = false;

                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);
                size_t thirdColon = processedArg.find(":", secondColon + 1);

                if (firstColon != std::string::npos && secondColon != std::string::npos && thirdColon != std::string::npos)
                {
                    std::string playerStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);

                    std::string dirStr = processedArg.substr(secondColon + 1, thirdColon - secondColon - 1);
                    std::string distStr = processedArg.substr(thirdColon + 1);

                    if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos)
                        playerIdx = numFromString<int>(playerStr).unwrapOrDefault();

                    if (dirStr == "left")
                        moveRight = false;

                    if (!distStr.empty() && distStr.find_first_not_of("-.0123456789") == std::string::npos)
                    {
                        distance = numFromString<float>(distStr).unwrapOrDefault();
                        validDistance = true;
                    };
                };

                if (!validDistance)
                {
                    log::warn("Ignoring move action: invalid distance value (command: {})", ctx->commandName);
                }
                else
                {
                    log::info("Triggering move event for player {} direction {} distance {} (command: {})", playerIdx, moveRight ? "right" : "left", distance, ctx->commandName);
                    PlayLayerEvent::movePlayer(playerIdx, moveRight, distance);
                };
            }
            else if (processedArg.rfind("color_player:", 0) == 0)
            {
                int playerIdx = 1;
                std::string colorStr;

                size_t firstColon = processedArg.find(":");
                size_t secondColon = processedArg.find(":", firstColon + 1);

                if (firstColon != std::string::npos && secondColon != std::string::npos)
                {
                    std::string playerStr = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                    colorStr = processedArg.substr(secondColon + 1);

                    if (!playerStr.empty() && playerStr.find_first_not_of("-0123456789") == std::string::npos)
                        playerIdx = numFromString<int>(playerStr).unwrapOrDefault();
                }
                else if (firstColon != std::string::npos)
                {
                    colorStr = processedArg.substr(firstColon + 1);
                };

                cocos2d::ccColor3B color = parseColorString(colorStr);

                log::info("Setting color for player {} to {} (command: {})", playerIdx, colorStr, ctx->commandName);
                PlayLayerEvent::setPlayerColor(playerIdx, color);
            }
            else if (processedArg.rfind("profile:", 0) == 0)
            {
                size_t firstColon = processedArg.find(":");
                if (firstColon == std::string::npos)
                    ;
                else
                {
                    std::string query = processedArg.substr(firstColon + 1);
                    // trim
                    query.erase(0, query.find_first_not_of(" \t\n\r"));
                    if (!query.empty())
                        query.erase(query.find_last_not_of(" \t\n\r") + 1);

                    // If numeric, open directly for backward-compat
                    if (!query.empty() && query.find_first_not_of("-0123456789") == std::string::npos)
                    {
                        int accountIdInt = numFromString<int>(query).unwrapOrDefault();
                        if (auto page = ProfilePage::create(accountIdInt, false))
                            page->show();
                    }
                    else
                    {
                        int actionNum = static_cast<int>(ctx->index) + 1;
                        std::string notFoundMsg = std::string("User cannot be found (action #") + std::to_string(actionNum) + ")";
                        
                        // insert funny meme from ProfileSettingsPopup
                        auto url = std::string("https://www.boomlings.com/database/getGJUsers20.php");
                        auto urlEncode = [](const std::string &s) {
                            static const char hex[] = "0123456789ABCDEF";
                            std::string out;
                            out.reserve(s.size() * 3);
                            for (unsigned char c : s) {
                                if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
                                    out.push_back(static_cast<char>(c));
                                } else if (c == ' ') {
                                    out.push_back('+');
                                } else {
                                    out.push_back('%');
                                    out.push_back(hex[(c >> 4) & 0xF]);
                                    out.push_back(hex[c & 0xF]);
                                }
                            }
                            return out;
                        };

                        std::string postData = "gameVersion=22&binaryVersion=40&gdw=0&str=" + urlEncode(query) + "&page=0&total=0&secret=Wmfd2893gb7";

                        auto request = web::WebRequest();
                        request.header("Content-Type", "application/x-www-form-urlencoded");
                        request.bodyString(postData);

                        request.post(url).listen(
                            [query, notFoundMsg](web::WebResponse *res) {
                                if (!res || !res->ok()) {
                                    Notification::create(notFoundMsg, NotificationIcon::Error, 1.5f)->show();
                                    return;
                                }
                                auto resp = res->string().unwrapOrDefault();
                                if (resp == "-1") {
                                    Notification::create(notFoundMsg, NotificationIcon::Error, 1.5f)->show();
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
                                        const std::string &accountIdStr = fields[i + 1];
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
                                Notification::create(notFoundMsg, NotificationIcon::Error, 1.5f)->show();
                            },
                            [](web::WebProgress *) {},
                            [notFoundMsg]() {
                                Notification::create(notFoundMsg, NotificationIcon::Error, 1.5f)->show();
                            });
                    }
                }
            }
            // this thing sucks to work with :(
            else if (processedArg.rfind("open_level:", 0) == 0)
            {
                // Format: open_level:<id>:<true|false>
                size_t firstColon = processedArg.find(":");
                size_t secondColon = (firstColon != std::string::npos ? processedArg.find(":", firstColon + 1) : std::string::npos);
                std::string query;
                bool force = false;
                if (firstColon != std::string::npos) {
                    if (secondColon != std::string::npos) {
                        query = processedArg.substr(firstColon + 1, secondColon - firstColon - 1);
                        std::string forceTok = processedArg.substr(secondColon + 1);
                        // trim
                        auto trim = [](std::string &s){ if(s.empty()) return; s.erase(0, s.find_first_not_of(" \t\n\r")); size_t e=s.find_last_not_of(" \t\n\r"); if(e!=std::string::npos) s.erase(e+1); else s.clear(); };
                        trim(query);
                        trim(forceTok);
                        geode::utils::string::toLowerIP(forceTok);
                        if (forceTok == "true") force = true;
                    } else {
                        query = processedArg.substr(firstColon + 1);
                        // trim
                        query.erase(0, query.find_first_not_of(" \t\n\r"));
                        if (!query.empty()) query.erase(query.find_last_not_of(" \t\n\r") + 1);
                    }
                }

                if (query.empty())
                {
                    int actionNum = static_cast<int>(ctx->index) + 1;
                    std::string msg = std::string("(#") + std::to_string(actionNum) + ") No Level Provided";
                    Notification::create(msg, NotificationIcon::Warning, 1.5f)->show();
                }
                else
                {
                    auto glm = GameLevelManager::sharedState();
                    if (!glm)
                    {
                        Notification::create("Level manager unavailable", NotificationIcon::Error, 1.5f)->show();
                    }
                    else if (query.find_first_not_of("0123456789") == std::string::npos)
                    {
                        int levelID = numFromString<int>(query).unwrapOrDefault();
                        if (levelID <= 0)
                        {
                            Notification::create("Invalid level ID", NotificationIcon::Error, 1.5f)->show();
                        }
                        else
                        {
                            auto startInfo = [&](){
                                if (auto lvl = glm->getSavedLevel(levelID))
                                {
                                    if (auto scene = LevelInfoLayer::scene(lvl, false))
                                        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.3f, scene));
                                    return true;
                                }
                                return false;
                            };

                            if (!force)
                            {
                                if (startInfo()) return; // showed info
                                // Try main level or fetch
                                if (auto lvl = glm->getMainLevel(levelID, true)) {
                                    if (auto scene = LevelInfoLayer::scene(lvl, false))
                                        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.3f, scene));
                                } else {
                                    auto so = GJSearchObject::create(SearchType::Search, std::to_string(levelID));
                                    glm->getOnlineLevels(so);
                                    Notification::create("Fetching level...", NotificationIcon::Loading, 1.0f)->show();
                                }
                            }
                            else
                            {
                                Notification::create("Preparing level...", NotificationIcon::Loading, 1.0f)->show();
                                auto so = GJSearchObject::create(SearchType::Search, std::to_string(levelID));
                                glm->getOnlineLevels(so);
                                struct ForcePlayRunner : public CCNode {
                                    GameLevelManager* m_glm = nullptr; int m_levelID = 0; int m_attempts = 0; int m_maxAttempts = 100;
                                    static ForcePlayRunner* create(GameLevelManager* glm, int id) { auto n = new ForcePlayRunner(); n->m_glm = glm; n->m_levelID = id; n->autorelease(); return n; }
                                    void onTick(float){
                                        if (!m_glm){ cleanupSelf(); return; }
                                        if (auto lvl = m_glm->getSavedLevel(m_levelID)){
                                            if (auto scene = PlayLayer::scene(lvl, false, false))
                                                CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.3f, scene));
                                            cleanupSelf(); return;
                                        }
                                        if (++m_attempts >= m_maxAttempts){ Notification::create("Level fetch timeout", NotificationIcon::Error, 1.5f)->show(); cleanupSelf(); }
                                    }
                                    void cleanupSelf(){ this->unschedule(schedule_selector(ForcePlayRunner::onTick)); this->removeFromParentAndCleanup(true); }
                                };
                                if (auto scene = CCDirector::sharedDirector()->getRunningScene()){
                                    auto runner = ForcePlayRunner::create(glm, levelID);
                                    scene->addChild(runner);
                                    runner->schedule(schedule_selector(ForcePlayRunner::onTick), 0.1f);
                                }
                            }
                        }
                    }
                    else
                    {
                        Notification::create("Please provide a numeric level ID", NotificationIcon::Warning, 1.5f)->show();
                    }
                }
            };
        };

        // Handle Notification type
        if (action.type == CommandActionType::Notification)
        {
            int iconTypeInt = 1;

            std::string notifText;
            std::string argStr = action.arg;
            float notifTime = 1.0f;

            if (argStr.size() >= 13)
            {
                std::string prefix = argStr.substr(0, 13);
                std::string prefixLower = prefix;

                geode::utils::string::toLowerIP(prefixLower);

                if (prefixLower == "notification:")
                {
                    std::string rest = argStr.substr(13);
                    size_t colonPos = rest.find(":");
                    if (colonPos != std::string::npos)
                    {
                        std::string iconPart = rest.substr(0, colonPos);
                        std::string afterIcon = rest.substr(colonPos + 1);
                        size_t timeSep = afterIcon.rfind(":");
                        if (!iconPart.empty() && iconPart.find_first_not_of("0123456789") == std::string::npos)
                        {
                            iconTypeInt = numFromString<int>(iconPart).unwrapOrDefault();
                        }
                        if (timeSep != std::string::npos)
                        {
                            notifText = afterIcon.substr(0, timeSep);
                            std::string timeStr = afterIcon.substr(timeSep + 1);
                            if (!timeStr.empty() && timeStr.find_first_not_of("-.0123456789") == std::string::npos)
                                notifTime = numFromString<float>(timeStr).unwrapOrDefault();
                        }
                        else
                        {
                            notifText = afterIcon;
                        }
                    }
                    else
                    {
                        notifText = rest;
                    };
                }
                else
                {
                    size_t colonPos = argStr.find(":");
                    if (colonPos != std::string::npos)
                    {
                        std::string iconPart = argStr.substr(0, colonPos);
                        std::string afterIcon = argStr.substr(colonPos + 1);
                        size_t timeSep = afterIcon.rfind(":");
                        if (!iconPart.empty() && iconPart.find_first_not_of("0123456789") == std::string::npos)
                        {
                            iconTypeInt = numFromString<int>(iconPart).unwrapOrDefault();
                        }
                        if (timeSep != std::string::npos)
                        {
                            notifText = afterIcon.substr(0, timeSep);
                            std::string timeStr = afterIcon.substr(timeSep + 1);
                            if (!timeStr.empty() && timeStr.find_first_not_of("-.0123456789") == std::string::npos)
                                notifTime = numFromString<float>(timeStr).unwrapOrDefault();
                        }
                        else
                        {
                            notifText = afterIcon;
                        }
                    }
                    else
                    {
                        notifText = argStr;
                    };
                };
            }
            else
            {
                size_t colonPos = argStr.find(":");
                if (colonPos != std::string::npos)
                {
                    std::string iconPart = argStr.substr(0, colonPos);
                    std::string afterIcon = argStr.substr(colonPos + 1);
                    size_t timeSep = afterIcon.rfind(":");
                    if (!iconPart.empty() && iconPart.find_first_not_of("0123456789") == std::string::npos)
                    {
                        iconTypeInt = numFromString<int>(iconPart).unwrapOrDefault();
                    }
                    if (timeSep != std::string::npos)
                    {
                        notifText = afterIcon.substr(0, timeSep);
                        std::string timeStr = afterIcon.substr(timeSep + 1);
                        if (!timeStr.empty() && timeStr.find_first_not_of("-.0123456789") == std::string::npos)
                            notifTime = numFromString<float>(timeStr).unwrapOrDefault();
                    }
                    else
                    {
                        notifText = afterIcon;
                    }
                }
                else
                {
                    notifText = argStr;
                };
            };

            notifText = ctx->replaceIdentifiers(notifText);

            notifText.erase(0, notifText.find_first_not_of(" \t\n\r"));
            notifText.erase(notifText.find_last_not_of(" \t\n\r") + 1);

            NotificationIcon icon = NotificationIcon::Info;

            switch (iconTypeInt)
            {
            case 0:
                icon = NotificationIcon::None;
                break;

            case 1:
                icon = NotificationIcon::Info;
                break;

            case 2:
                icon = NotificationIcon::Success;
                break;

            case 3:
                icon = NotificationIcon::Warning;
                break;

            case 4:
                icon = NotificationIcon::Error;
                break;

            case 5:
                icon = NotificationIcon::Loading;
                break;
            };

            log::info("Showing notification: {} (icon: {}, time: {:.2f}, command: {})", notifText, iconTypeInt, notifTime, ctx->commandName);
            Notification::create(notifText, icon, notifTime)->show();
        };

        // Add more action types here as needed
        ctx->index++;
        ctx->execute(ctx);
    };
};