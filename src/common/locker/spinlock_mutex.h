#pragma once
namespace locker
{
class spinlock_mutex
{
public:
    spinlock_mutex();
    ~spinlock_mutex();
    void lock();
    void unlock();
private:
    class impl_t;
    impl_t* p_impl;
};
}