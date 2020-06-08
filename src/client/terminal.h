#pragma once
#include <thread/runnable.h>
#include <functional>
#include <string>
namespace brct
{
class Terminal : public running::IRunnable
{
public:
    Terminal() = default;
    ~Terminal() = default;
    void run(const std::atomic_bool &terminate) override;
    void setProcessor(std::function<bool (std::string&)> &processor){processor_ = processor;};
    void print(const std::string& msg);
private:
    std::function<bool (std::string&)> processor_;
};
}