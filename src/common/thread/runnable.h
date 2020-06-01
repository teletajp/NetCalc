#pragma once
#include <memory>
#include <atomic>
namespace running
{

class IRunnable
{
public:
    virtual ~IRunnable() {};
    virtual void run(const std::atomic_bool &terminate) = 0;
};

using RunnablePtr = std::unique_ptr<IRunnable>;

class AutoThread
{
public:
    AutoThread(RunnablePtr&& runnable_object);
    AutoThread(IRunnable* runnable_object);
    ~AutoThread();
    bool terminated() const;
    void terminate();

    AutoThread(const AutoThread&)            = delete;
    AutoThread(AutoThread&&)                 = delete;
    AutoThread& operator=(const AutoThread&) = delete;
    AutoThread& operator=(AutoThread&&)      = delete;
private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
}