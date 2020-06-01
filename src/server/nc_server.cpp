#include "nc_server.h"
namespace brct
{
NcServer::NcServer(const settings_t &settings):
settings_(settings),
tcp_server_(settings.listen_ip, settings.listen_port)
{}
void NcServer::run(std::atomic_bool &terminate)
{
    tcp_server_.run(terminate);
}
}