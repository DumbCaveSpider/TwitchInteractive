#pragma once
#include <cocos2d.h>
#include <functional>

// Helper node to schedule a function call after a delay
class KeyReleaseScheduler : public cocos2d::CCNode
{
public:
    std::function<void()> m_func;
    static KeyReleaseScheduler *create(std::function<void()> func, float delay);
    void onRelease(float);
};
