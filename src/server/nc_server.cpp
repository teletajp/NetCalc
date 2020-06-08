#include "nc_server.h"
#include "calculator.h"
#include <thread/runnable.h>
#include <thread>
namespace brct
{
NcServer::NcServer(const settings_t &settings):
settings_(settings)
{}
void NcServer::run(std::atomic_bool &terminate)
{
    TcpServer tcp_server(settings_.listen_ip, settings_.listen_port);
    Calculator calculator;
    std::function<bool (Calculator::Expression&)> cal_processor = std::bind(&Calculator::calculate, &calculator, std::placeholders::_1);
    std::function<void (int, std::string&&)> resuls_sender = std::bind(&TcpServer::send, &tcp_server, std::placeholders::_1, std::placeholders::_2);
    tcp_server.setProcessor(std::move(cal_processor));
    calculator.setResultSubscriber(std::move(resuls_sender));
    running::AutoThread tcp_server_thread(&tcp_server);
    running::AutoThread calculator_thread(&calculator);
    while(!terminate &&
          !calculator_thread.terminated() &&
          !tcp_server_thread.terminated())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
}