#include "nc_client.h"
#include "terminal.h"
#include "tcp_client.h"
#include <thread>
#include <string>
namespace brct
{
void NcClient::run(const std::atomic_bool &terminate)
{
    Terminal terminal;
    TcpClient tcp_client;
    std::function<bool (std::string&)> processor = std::bind(&TcpClient::processCommand, &tcp_client, std::placeholders::_1);
    terminal.setProcessor(processor);
    std::function<void (const std::string&)> result_recepient_cb = std::bind(&Terminal::print, &terminal, std::placeholders::_1);
    tcp_client.setTransmitResultCb(result_recepient_cb);
    running::AutoThread terminal_thread(&terminal);
    //running::AutoThread tcp_client_thread(&tcp_client);
    while(!terminate &&
          !terminal_thread.terminated() /*&&
          !tcp_client_thread.terminated()*/)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
}