#pragma once
#include "thread/runnable.h"
#include <string>
#include <memory>
#include <functional>
namespace brct
{
class TcpClient : public running::IRunnable
{
private:
    
public:
    TcpClient();
    ~TcpClient();
    void run(const std::atomic_bool &terminate) override;
    void setTransmitResultCb(std::function<void (const std::string&)> recepient);
    bool processCommand(std::string &command);
private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
}