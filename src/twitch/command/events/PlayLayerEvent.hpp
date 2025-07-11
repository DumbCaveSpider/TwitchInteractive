#pragma once
#include <Geode/Geode.hpp>
#include <Geode/Bindings.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

class PlayLayerEvent {
public:
    static void killPlayer();
    static void jumpPlayer(int playerIdx);
};
