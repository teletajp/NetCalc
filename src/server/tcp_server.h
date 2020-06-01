#pragma once
#include "net/select_base_server_engine.h"
#include <thread/runnable.h>
#include <memory>
#include <functional>
#include <vector>
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
    void setProcessor(std::function<void (std::vector<uint8_t>&, const std::string &)> &&processor);
private:
    std::function<void (std::vector<uint8_t>&, const std::string &)>processor_;
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