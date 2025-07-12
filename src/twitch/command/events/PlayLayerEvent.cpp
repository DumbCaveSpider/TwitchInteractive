#include "PlayLayerEvent.hpp"

#include <Geode/loader/Loader.hpp>

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

void PlayLayerEvent::killPlayer() {
    log::debug("[PlayLayerEvent] killPlayer called");
    g_pendingKillPlayer = true;

    Loader::get()->queueInMainThread([] {
        auto playLayer = PlayLayer::get();

        if (playLayer && g_pendingKillPlayer) {
            log::debug("[PlayLayerEvent] killPlayer: Executing kill immediately");

            playLayer->destroyPlayer(playLayer->m_player1, nullptr);
            g_pendingKillPlayer = false;
        } else if (g_pendingKillPlayer) {
            KillPlayerScheduler::start();
        };
                                     });
};

void PlayLayerEvent::jumpPlayer(int playerIdx) {
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

            log::info("[PlayLayerEvent] jumpPlayer: Making both players jump");
        } else {
            auto player = (playerIdx == 2) ? playLayer->m_player2 : playLayer->m_player1;

            if (!player) {
                log::debug("[PlayLayerEvent] jumpPlayer: Player{} not found", playerIdx);
                return;
            };

            log::info("[PlayLayerEvent] jumpPlayer: Making player{} jump", playerIdx);
            player->pushButton(PlayerButton::Jump);
        };
    });
}

// Simulate a keypress by key string (universal, works anywhere in the game if supported)
void PlayLayerEvent::pressKey(const std::string& key) {
    Loader::get()->queueInMainThread([key] {
        cocos2d::enumKeyCodes keyCode = cocos2d::KEY_None;
        if (key == "Space" || key == "Jump") keyCode = cocos2d::KEY_Space;
        else if (key == "Left") keyCode = cocos2d::KEY_Left;
        else if (key == "Right") keyCode = cocos2d::KEY_Right;
        else if (key == "Up") keyCode = cocos2d::KEY_Up;
        else if (key == "Down") keyCode = cocos2d::KEY_Down;
        else if (key.length() == 1 && std::isalpha(key[0])) keyCode = static_cast<cocos2d::enumKeyCodes>(std::toupper(key[0]));

        if (keyCode == cocos2d::KEY_None) {
            log::info("[PlayLayerEvent] pressKey: Unrecognized key '{}', no action taken", key);
            return;
        }


        auto glView = cocos2d::CCDirector::sharedDirector()->getOpenGLView();
        if (glView) {
#if defined(GEODE_SUPPORTS_SIMULATE_KEY) || defined(CCEGLVIEW_SUPPORTS_SIMULATE_KEY)
            glView->simulateKeyDown(keyCode);
            glView->simulateKeyUp(keyCode);
            log::info("[PlayLayerEvent] pressKey: Simulated universal key event for '{}' (code {})", key, static_cast<int>(keyCode));
            return;
#endif
        }

        // Fallback: Only works in PlayLayer
        auto playLayer = PlayLayer::get();
        if (playLayer) {
            if (keyCode == cocos2d::KEY_Space || keyCode == cocos2d::KEY_Up) {
                if (playLayer->m_player1) playLayer->m_player1->pushButton(PlayerButton::Jump);
                log::info("[PlayLayerEvent] pressKey: Fallback - Simulated Jump for player1");
            } else if (keyCode == cocos2d::KEY_Left) {
                if (playLayer->m_player1) playLayer->m_player1->pushButton(PlayerButton::Left);
                log::info("[PlayLayerEvent] pressKey: Fallback - Simulated Left for player1");
            } else if (keyCode == cocos2d::KEY_Right) {
                if (playLayer->m_player1) playLayer->m_player1->pushButton(PlayerButton::Right);
                log::info("[PlayLayerEvent] pressKey: Fallback - Simulated Right for player1");
            } else {
                log::info("[PlayLayerEvent] pressKey: Fallback - Key '{}' not mapped in PlayLayer", key);
            }
        } else {
            log::info("[PlayLayerEvent] pressKey: No universal key simulation available for '{}', code {}", key, static_cast<int>(keyCode));
        }
    });
}