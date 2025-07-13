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

    // Move player left or right by a distance
    // playerIdx: 1 or 2, moveRight: true for right, false for left, distance: how far to move
    static void movePlayer(int playerIdx, bool moveRight, float distance = 0.f);
};
