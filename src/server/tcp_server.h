#pragma once
#include "net/select_base_server_engine.h"
#include <thread/runnable.h>
#include <memory>
#include <functional>
#include <vector>
#include "calculator.h"
namespace brct
{
class TcpServer: public running::IRunnable
{
    net::SelectBaseServerEngine<TcpServer> engine_;
public:
    TcpServer(const std::string &ip, uint16_t port);
    ~TcpServer();

    int onRead(const int fd);
    int onWrite(const int fd);
    int onError(const int fd, const std::string &error_message);
    void run(const std::atomic_bool &terminate) override;
    void setProcessor(std::function<bool (Calculator::Expression&)> &&processor);
    void send(int fd, std::string&& data);
private:
    std::function<bool (Calculator::Expression&)>processor_;
    TcpServer(const TcpServer&) = delete;
    TcpServer(TcpServer&&) = delete;
    TcpServer& operator= (const TcpServer&) = delete;
    TcpServer& operator= (TcpServer&&) = delete;
    void deleteConnection(int fd);
    int listenerOnRead();
    int clientOnRead(int fd);
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
}