#pragma once

#include <Geode/Geode.hpp>
#include <Geode/Bindings.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;


class PlayLayerEvent {
public:
    static void killPlayer();
    static void jumpPlayerHold(int playerIdx);
    static void jumpPlayerTap(int playerIdx);
    // Accepts key and duration (in seconds, 0 = instant press/release)
    static void pressKey(const std::string& key, float duration = 0.f);

    // Move player left or right
    // playerIdx: 1 or 2, moveRight: true for right, false for left
    static void movePlayer(int playerIdx, bool moveRight);
};
