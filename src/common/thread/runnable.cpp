#include <thread>
#include "runnable.h"
namespace running
{
class AutoThread::Impl
{
    RunnablePtr runnable_object_;
public:
    Impl(RunnablePtr&& runnable_object):runnable_object_(std::move(runnable_object)), raw_object_pointer_(runnable_object_.get()), terminated_(false)
    {
        thread = std::thread(&AutoThread::Impl::run, this, raw_object_pointer_);
    }
    Impl(IRunnable * runnable_object):runnable_object_(), raw_object_pointer_(runnable_object), terminated_(false)
    {
        thread = std::thread(&AutoThread::Impl::run, this, raw_object_pointer_);
    }
    ~Impl()
    {
        terminated_ = true;
        if (thread.joinable())
        {
            if (thread.get_id() == std::this_thread::get_id())
                thread.detach();
            else
                thread.join();
        }
    }

    static void run(Impl *thread, IRunnable* runnable_object)
    {
        try
        {
            Impl::runner(thread, runnable_object);
        }
        catch (...)
        {
        }
        thread->terminated_ = true;
    }

    static void runner(Impl * thread, IRunnable* runnable_object)
    {
        while (!thread->terminated_)
        {
            runnable_object->run(thread->terminated_);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    IRunnable*  raw_object_pointer_;
    std::atomic_bool terminated_;
    std::thread thread;
};


AutoThread::AutoThread(RunnablePtr&& runnable_object):
pimpl_(std::make_unique<Impl>(std::move(runnable_object)))
{

}
AutoThread::AutoThread(IRunnable * runnable_object):
pimpl_(std::make_unique<Impl>(runnable_object))
{
}
AutoThread::~AutoThread()
{
    terminate();
}
bool AutoThread::terminated() const
{
    return pimpl_->terminated_;
}
void AutoThread::terminate()
{
    pimpl_->terminated_ = true;
}
}