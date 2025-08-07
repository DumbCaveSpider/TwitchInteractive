#include "KeyReleaseScheduler.hpp"

KeyReleaseScheduler *KeyReleaseScheduler::create(std::function<void()> func, float delay)
{
    auto node = new KeyReleaseScheduler();
    node->m_func = func;

    node->autorelease();

    node->scheduleOnce(schedule_selector(KeyReleaseScheduler::onRelease), delay);

    return node;
};

void KeyReleaseScheduler::onRelease(float)
{
    if (m_func)
        m_func();
    removeFromParentAndCleanup(true);
};