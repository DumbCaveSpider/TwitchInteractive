#include "PlayLayerEvent.hpp"



#include <Geode/loader/Loader.hpp>

namespace {
    bool g_pendingKillPlayer = false;

    class KillPlayerScheduler : public cocos2d::CCNode {
    public:
        void update(float) {
            auto playLayer = PlayLayer::get();
            if (playLayer && g_pendingKillPlayer) {
                playLayer->destroyPlayer(playLayer->m_player1, nullptr);
                g_pendingKillPlayer = false;
                this->unscheduleAllSelectors();
                this->removeFromParentAndCleanup(true);
            }
        }
        static void start() {
            auto node = new KillPlayerScheduler();
            node->autorelease();
            CCDirector::sharedDirector()->getRunningScene()->addChild(node);
            node->schedule(schedule_selector(KillPlayerScheduler::update), 0.1f);
        }
    };
}

void PlayLayerEvent::killPlayer() {
    g_pendingKillPlayer = true;
    Loader::get()->queueInMainThread([] {
        auto playLayer = PlayLayer::get();
        if (playLayer && g_pendingKillPlayer) {
            playLayer->destroyPlayer(playLayer->m_player1, nullptr);
            g_pendingKillPlayer = false;
        } else if (g_pendingKillPlayer) {
            KillPlayerScheduler::start();
        }
    });
}
