#include "spinlock_mutex.h"
#include <atomic>
#include <cstdint>
#ifdef _WIN32
#include <windows.h>
#define pause(millsec)   Sleep(millsec);
#endif
#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#define pause(millsec) sleep(millsec / 1000);
#endif
namespace locker
{
class spinlock_mutex::impl_t
{
public:
    impl_t(){};
    ~impl_t(){};
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

spinlock_mutex::spinlock_mutex():
p_impl(new impl_t)
{
}

spinlock_mutex::~spinlock_mutex()
{
    delete p_impl;
}

void spinlock_mutex::lock()
{
    while (p_impl->flag.test_and_set(std::memory_order_acquire))
    {
        pause(0);
    }
}

void spinlock_mutex::unlock()
{
    p_impl->flag.clear(std::memory_order_release);
}
}