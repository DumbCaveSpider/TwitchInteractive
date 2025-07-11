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

        // Player 1 = 1, Player 2 = 2, Both = 3
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
};