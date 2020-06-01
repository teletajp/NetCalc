#pragma once
#include <string>
#include "tcp_server.h"
namespace brct
{
class NcServer
{
public:
    struct settings_t
    {
        std::string listen_ip;
        uint16_t listen_port;
        settings_t():listen_ip("0.0.0.0"), listen_port(23){};
    };
private:
    settings_t settings_;
    TcpServer tcp_server_;
public:
    NcServer(const settings_t &settings_);
    NcServer(const NcServer&) = delete;
    NcServer(NcServer&&) = delete;
    const NcServer& operator=(const NcServer&) = delete;
    const NcServer& operator=(NcServer&&) = delete;
    ~NcServer() = default;
    void run(std::atomic_bool &terminate);
};
}