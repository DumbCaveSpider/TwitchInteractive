#include "KeyReleaseScheduler.hpp"
#include "PlayLayerEvent.hpp"
#include <Geode/loader/Loader.hpp>
#include <Geode/Bindings.hpp>
#include <Geode/Geode.hpp>

namespace {
    bool g_pendingKillPlayer = false;

    class KillPlayerScheduler : public cocos2d::CCNode {
    public:
        void update(float) {
            auto playLayer = PlayLayer::get();
            if (playLayer && g_pendingKillPlayer) {
                log::debug("[PlayLayerEvent] KillPlayerScheduler: Executing kill player");

                playLayer->destroyPlayer(playLayer->m_player1, nullptr);
                g_pendingKillPlayer = false;

                unscheduleAllSelectors();
                removeFromParentAndCleanup(true);
            };
        };

        static void start() {
            log::debug("[PlayLayerEvent] KillPlayerScheduler: Scheduling kill player");

            auto node = new KillPlayerScheduler();
            node->autorelease();

            CCDirector::sharedDirector()->getRunningScene()->addChild(node);
            node->schedule(schedule_selector(KillPlayerScheduler::update), 0.1f);
        };
    };
};

// Simulate holding the jump button for a short duration
void PlayLayerEvent::jumpPlayerTap(int playerIdx) {
    Loader::get()->queueInMainThread([playerIdx] {
        auto playLayer = PlayLayer::get();
        if (!playLayer) {
            log::debug("[PlayLayerEvent] holdJumpPlayer: PlayLayer not found");
            return;
        };

        auto pressAndRelease = [](auto* player) {
            if (!player) return;
            player->pushButton(PlayerButton::Jump);
            // Use KeyReleaseScheduler to delay the release
            auto node = KeyReleaseScheduler::create([player]() {
                player->releaseButton(PlayerButton::Jump);
                                                    }, 0.2f);
            cocos2d::CCDirector::sharedDirector()->getRunningScene()->addChild(node);
            };

        if (playerIdx == 3) {
            pressAndRelease(playLayer->m_player1);
            pressAndRelease(playLayer->m_player2);
            log::info("[PlayLayerEvent] Both players hold jump");
        } else {
            auto player = (playerIdx == 2) ? playLayer->m_player2 : playLayer->m_player1;
            if (!player) {
                log::debug("[PlayLayerEvent] Player{} not found", playerIdx);
                return;
            };

            log::info("[PlayLayerEvent] Player {} jump", playerIdx);
            pressAndRelease(player);
        };
                                     });
};

void PlayLayerEvent::killPlayer() {
    log::debug("[PlayLayerEvent] destroyPlayer called");
    g_pendingKillPlayer = true;

    Loader::get()->queueInMainThread([] {
        auto playLayer = PlayLayer::get();

        if (playLayer && g_pendingKillPlayer) {
            log::debug("[PlayLayerEvent] destroyPlayer: Executing now");

            playLayer->destroyPlayer(playLayer->m_player1, nullptr);
            g_pendingKillPlayer = false;
        } else if (g_pendingKillPlayer) {
            KillPlayerScheduler::start();
        };
                                     });
};

void PlayLayerEvent::jumpPlayerHold(int playerIdx) {
    Loader::get()->queueInMainThread([playerIdx] {
        auto playLayer = PlayLayer::get();
        if (!playLayer) {
            log::debug("[PlayLayerEvent] jumpPlayer: PlayLayer not found");
            return;
        };

        // P1 = 1 
        // P2 = 2
        // Both = 3      

        if (playerIdx == 3) {
            if (playLayer->m_player1) playLayer->m_player1->pushButton(PlayerButton::Jump);
            if (playLayer->m_player2) playLayer->m_player2->pushButton(PlayerButton::Jump);

            log::info("[PlayLayerEvent] Both players hold jump");
        } else {
            auto player = (playerIdx == 2) ? playLayer->m_player2 : playLayer->m_player1;
            if (!player) {
                log::debug("[PlayLayerEvent] Player {} not found", playerIdx);
                return;
            };

            log::info("[PlayLayerEvent] Player {} hold jump", playerIdx);
            player->pushButton(PlayerButton::Jump);
        };
                                     });
};

// Simulate a keypress by key string (universal, works anywhere in the game if supported)
void PlayLayerEvent::pressKey(const std::string& key, float duration) {
    Loader::get()->queueInMainThread([key, duration] {
        cocos2d::enumKeyCodes keyCode = cocos2d::KEY_None;

        if (key == "Space" || key == "Jump") keyCode = cocos2d::KEY_Space;
        else if (key == "Left") keyCode = cocos2d::KEY_Left;
        else if (key == "Right") keyCode = cocos2d::KEY_Right;
        else if (key == "Up") keyCode = cocos2d::KEY_Up;
        else if (key == "Down") keyCode = cocos2d::KEY_Down;
        else if (key.length() == 1 && std::isalpha(key[0])) keyCode = static_cast<cocos2d::enumKeyCodes>(std::toupper(key[0]));

        if (keyCode == cocos2d::KEY_None) {
            log::debug("[PlayLayerEvent] Unrecognized key '{}', no action taken", key);
            return;
        };

        // Use CCKeyboardDispatcher for global key simulation
        auto dispatcher = cocos2d::CCKeyboardDispatcher::get();
        if (dispatcher) {
            dispatcher->dispatchKeyboardMSG(keyCode, true, 0); // key down

            if (duration > 0.f) {
                auto node = KeyReleaseScheduler::create([dispatcher, keyCode]() {
                    dispatcher->dispatchKeyboardMSG(keyCode, false, 0); // key up
                                                        }, duration);
                cocos2d::CCDirector::sharedDirector()->getRunningScene()->addChild(node);
            } else {
                dispatcher->dispatchKeyboardMSG(keyCode, false, 0); // key up
            };

            log::info("[PlayLayerEvent] Simulated universal key event for '{}' (code {}), duration {}", key, static_cast<int>(keyCode), duration);
            return;
        };

        log::debug("[PlayLayerEvent] No universal key simulation available for '{}', code {}", key, static_cast<int>(keyCode));
                                     });
};

// Move player left or right by a distance
void PlayLayerEvent::movePlayer(int playerIdx, bool moveRight, float distance) {
    Loader::get()->queueInMainThread([playerIdx, moveRight, distance] {
        auto playLayer = PlayLayer::get();
        if (!playLayer) {
            log::debug("[PlayLayerEvent] movePlayer: PlayLayer not found");
            return;
        };

        auto player = (playerIdx == 2) ? playLayer->m_player2 : playLayer->m_player1;
        if (!player) return;

        // If distance is 0, fallback to button simulation
        if (distance > 0.f) {
            // Estimate how long to hold the button based on player speed
            float speed = 240.f; // Default speed if not available

            // If the player has a velocity property, prefer that
#ifdef GEODE_IS_GEODE
            if (player->m_xAccel != 0.f) speed = std::abs(player->m_xAccel);
#endif

            float duration = std::abs(distance) / speed;
            if (duration < 0.05f) duration = 0.05f; // Minimum press duration

            auto btn = moveRight ? PlayerButton::Right : PlayerButton::Left;
            player->pushButton(btn);

            // Release after calculated duration
            auto node = KeyReleaseScheduler::create([player, btn]() {
                player->releaseButton(btn);
                                                    }, duration);

            cocos2d::CCDirector::sharedDirector()->getRunningScene()->addChild(node);
            log::info("[PlayLayerEvent] Simulated move for player {} {} by distance {} (duration {}s, speed {})", playerIdx, moveRight ? "right" : "left", distance, duration, speed);
        } else {
            // Simulate left/right movement by pushing the corresponding button
            auto btn = moveRight ? PlayerButton::Right : PlayerButton::Left;
            player->pushButton(btn);

            // Release after a short delay
            auto node = KeyReleaseScheduler::create([player, btn]() {
                player->releaseButton(btn);
                                                    }, 0.2f);
            cocos2d::CCDirector::sharedDirector()->getRunningScene()->addChild(node);

            log::info("[PlayLayerEvent] Moved player {} {} (button sim)", playerIdx, moveRight ? "right" : "left");
        };
                                     });
};