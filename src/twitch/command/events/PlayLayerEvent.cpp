#include "KeyReleaseScheduler.hpp"
#include "PlayLayerEvent.hpp"
#include <Geode/loader/Loader.hpp>
#include <Geode/Bindings.hpp>
#include <Geode/Geode.hpp>
#include <cocos2d.h>
using namespace geode::prelude;

// Helper to parse color from string (format: "R,G,B")
cocos2d::ccColor3B parseColorString(const std::string& str) {
    int r = 255, g = 255, b = 255;
    sscanf(str.c_str(), "%d,%d,%d", &r, &g, &b);
    return { static_cast<GLubyte>(r), static_cast<GLubyte>(g), static_cast<GLubyte>(b) };
};

// Set player color (playerIdx: 1, 2, or 3 for both)
void PlayLayerEvent::setPlayerColor(int playerIdx, const cocos2d::ccColor3B& color) {
    Loader::get()->queueInMainThread([playerIdx, color] {
        auto playLayer = PlayLayer::get();
        if (!playLayer) {
            log::debug("[PlayLayerEvent] setPlayerColor: PlayLayer not found");
            return;
        };

        auto setColor = [&](auto* player) {
            if (player) player->setColor(color);
            };

        if (playerIdx == 3) {
            setColor(playLayer->m_player1);
            setColor(playLayer->m_player2);
            log::info("[PlayLayerEvent] Set color for both players: R{} G{} B{}", color.r, color.g, color.b);
        } else {
            auto player = (playerIdx == 2) ? playLayer->m_player2 : playLayer->m_player1;
            if (!player) {
                log::debug("[PlayLayerEvent] Player {} not found", playerIdx);
                return;
            };

            setColor(player);

            log::info("[PlayLayerEvent] Set color for player {}: R{} G{} B{}", playerIdx, color.r, color.g, color.b);
        } });
};

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

// Reverse both players' direction
void PlayLayerEvent::reversePlayer() {
    Loader::get()->queueInMainThread([] {
        auto playLayer = PlayLayer::get();
        if (!playLayer) {
            log::debug("[PlayLayerEvent] reversePlayer: PlayLayer not found");
            return;
        }
        if (playLayer->m_player1) playLayer->m_player1->doReversePlayer(true);
        if (playLayer->m_player2) playLayer->m_player2->doReversePlayer(true);
        log::info("[PlayLayerEvent] Reversed both players"); });
}

// Set player scale (playerIdx: 1, 2, or 3 for both), with optional animation time
void PlayLayerEvent::scalePlayer(int playerIdx, float scale, float time) {
    Loader::get()->queueInMainThread([playerIdx, scale, time] {
        auto playLayer = PlayLayer::get();
        if (!playLayer) {
            log::debug("[PlayLayerEvent] scalePlayer: PlayLayer not found");
            return;
        }
        auto animateScale = [](auto* player, float targetScale, float duration) {
            if (!player) return;
            if (duration > 0.0f) {
                class ScaleAnimScheduler : public cocos2d::CCNode {
                public:
                    float elapsed = 0.f;
                    float duration;
                    float fromScale, toScale;
                    cocos2d::CCNode* target;
                    ScaleAnimScheduler(float d, float fs, float ts, cocos2d::CCNode* tgt)
                        : duration(d), fromScale(fs), toScale(ts), target(tgt) {}
                    void update(float dt) override {
                        elapsed += dt;
                        float t = duration > 0.f ? std::min(elapsed / duration, 1.f) : 1.f;
                        float newScale = fromScale + (toScale - fromScale) * t;
                        if (target) target->setScale(newScale);
                        if (t >= 1.f) {
                            unscheduleAllSelectors();
                            removeFromParentAndCleanup(true);
                        }
                    }
                    static ScaleAnimScheduler* create(float d, float fs, float ts, cocos2d::CCNode* tgt) {
                        auto node = new ScaleAnimScheduler(d, fs, ts, tgt);
                        node->autorelease();
                        return node;
                    }
                };
                float startScale = player->getScale();
                auto animNode = ScaleAnimScheduler::create(duration, startScale, targetScale, player);
                cocos2d::CCDirector::sharedDirector()->getRunningScene()->addChild(animNode);
                animNode->schedule(schedule_selector(ScaleAnimScheduler::update), 0.f);
            } else {
                player->setScale(targetScale);
            }
            };
        if (playerIdx == 3) {
            animateScale(playLayer->m_player1, scale, time);
            animateScale(playLayer->m_player2, scale, time);
            log::info("[PlayLayerEvent] Set scale for both players: {} (time: {})", scale, time);
        } else {
            auto player = (playerIdx == 2) ? playLayer->m_player2 : playLayer->m_player1;
            if (!player) {
                log::debug("[PlayLayerEvent] Player {} not found", playerIdx);
                return;
            }
            animateScale(player, scale, time);
            log::info("[PlayLayerEvent] Set scale for player {}: {} (time: {})", playerIdx, scale, time);
        } });
}

// Set PlayLayer camera settings from edit_camera action string (format: edit_camera:<skew>:<rot>:<scale>:<time>)
void PlayLayerEvent::setCameraFromString(const std::string& arg) {
    Loader::get()->queueInMainThread([arg] {
        auto playLayer = PlayLayer::get();
        if (!playLayer) {
            log::debug("[PlayLayerEvent] setCameraFromString: PlayLayer not found");
            return;
        }
        // Parse format: edit_camera:<skew>:<rot>:<scale>:<time>
        float skew = 0.f, rot = 0.f, scale = 1.f, time = 0.f;
        size_t first = arg.find(":");
        size_t second = arg.find(":", first + 1);
        size_t third = arg.find(":", second + 1);
        size_t fourth = arg.find(":", third + 1);
        if (first != std::string::npos && second != std::string::npos && third != std::string::npos && fourth != std::string::npos) {
            std::string skewStr = arg.substr(first + 1, second - first - 1);
            std::string rotStr = arg.substr(second + 1, third - second - 1);
            std::string scaleStr = arg.substr(third + 1, fourth - third - 1);
            std::string timeStr = arg.substr(fourth + 1);
            if (!skewStr.empty()) skew = std::stof(skewStr);
            if (!rotStr.empty()) rot = std::stof(rotStr);
            if (!scaleStr.empty()) scale = std::stof(scaleStr);
            if (!timeStr.empty()) time = std::stof(timeStr);
        }
        // Animate camera properties if time > 0, else set instantly
        log::info("[PlayLayerEvent] Setting camera: Skew={} Rot={} Scale={} Time={}", skew, rot, scale, time);
        float startSkew = playLayer->getSkewX();
        float startRot = playLayer->getRotation();
        float startScale = playLayer->getScale();
        if (time > 0.0f) {
            // Animate over 'time' seconds
            class CameraAnimScheduler : public cocos2d::CCNode {
            public:
                float elapsed = 0.f;
                float duration;
                float fromSkew, toSkew;
                float fromRot, toRot;
                float fromScale, toScale;
                cocos2d::CCNode* target;
                CameraAnimScheduler(float d, float fs, float ts, float fr, float tr, float fsc, float tsc, cocos2d::CCNode* tgt)
                    : duration(d), fromSkew(fs), toSkew(ts), fromRot(fr), toRot(tr), fromScale(fsc), toScale(tsc), target(tgt) {}
                void update(float dt) override {
                    elapsed += dt;
                    float t = duration > 0.f ? std::min(elapsed / duration, 1.f) : 1.f;
                    float newSkew = fromSkew + (toSkew - fromSkew) * t;
                    float newRot = fromRot + (toRot - fromRot) * t;
                    float newScale = fromScale + (toScale - fromScale) * t;
                    if (target) {
                        target->setSkewX(newSkew);
                        target->setSkewY(newSkew);
                        target->setRotation(newRot);
                        target->setScale(newScale);
                    }
                    if (t >= 1.f) {
                        unscheduleAllSelectors();
                        removeFromParentAndCleanup(true);
                    }
                }
                static CameraAnimScheduler* create(float d, float fs, float ts, float fr, float tr, float fsc, float tsc, cocos2d::CCNode* tgt) {
                    auto node = new CameraAnimScheduler(d, fs, ts, fr, tr, fsc, tsc, tgt);
                    node->autorelease();
                    return node;
                }
            };
            auto animNode = CameraAnimScheduler::create(time, startSkew, skew, startRot, rot, startScale, scale, playLayer);
            cocos2d::CCDirector::sharedDirector()->getRunningScene()->addChild(animNode);
            animNode->schedule(schedule_selector(CameraAnimScheduler::update), 0.f);
        } else {
            playLayer->setSkewX(skew);
            playLayer->setSkewY(skew);
            playLayer->setRotation(rot);
            playLayer->setScale(scale);
        } });
}

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
        }; });
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
        }; });
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
        }; });
};

// Simulate a keypress by key string (universal, works anywhere in the game if supported)
void PlayLayerEvent::pressKey(const std::string& key, float duration) {
    Loader::get()->queueInMainThread([key, duration] {
        cocos2d::enumKeyCodes keyCode = cocos2d::KEY_None;

        // switch doesnt support strings :/
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
        if (auto dispatcher = cocos2d::CCKeyboardDispatcher::get()) {
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

        log::debug("[PlayLayerEvent] No universal key simulation available for '{}', code {}", key, static_cast<int>(keyCode)); });
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

#ifdef GEODE_IS_GEODE
            // If the player has a velocity property, prefer that
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
        }; });
};