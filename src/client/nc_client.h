#pragma once
#include <thread/runnable.h>
namespace brct
{
class NcClient: public running::IRunnable
{
public:
    NcClient()=default;
    ~NcClient()=default;
    void run(const std::atomic_bool &terminate) override;
};
}