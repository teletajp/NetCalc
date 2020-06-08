#include "tcp_client.h"
#include <mutex>
#include <condition_variable>
#include <list>
#include <vector>
#include <sstream>
#include <net/newtork_initializer.h>
#include <timer/clock.h>
#include "tcp_connector.h"
namespace brct
{
constexpr int MSG_BUF_SIZE = 10*1024;
static void splitString(const std::string& input, std::vector<std::string> &output)
{
    std::istringstream iss(input);
    output.insert(output.end(), std::istream_iterator<std::string>{iss},
                                 std::istream_iterator<std::string>()); 
}
class TcpClient::Impl
{
public:
    Impl() {command_buffer.reserve(MSG_BUF_SIZE); is_run = false;};
    ~Impl() = default;
    bool processCommand(std::string &command);
    bool processHelpCmd();
    bool processConnectCmd(const std::string &command);
    bool processDisconnectCmd(const std::string &command);
    bool processUserCommand(const std::string &command);
    void transmit(const std::string & msg){if (transmit_result) transmit_result(msg);}
    void send_commands();
    std::mutex cmd_mtx;
    std::condition_variable new_command;
    std::list<std::string> input_list;
    std::list<std::string> work_list;
    std::string command_buffer;
    std::function<void (const std::string&)> transmit_result;
    TcpConnector tcp_connector;
    timer::Clock command_timer;
    bool is_run;
};
bool TcpClient::Impl::processCommand(std::string &command)
{
    if (command.substr(0, sizeof("help")-1) == "help") return processHelpCmd();
    if (command.substr(0, sizeof("connect")-1) == "connect") return processConnectCmd(command);
    if (command.substr(0, sizeof("disconnect")-1) == "disconnect") return processDisconnectCmd(command);
    return processUserCommand(command);
}
bool TcpClient::Impl::processHelpCmd()
{
    static const std::string help_msg = "AVAILABLE COMMANDS:\n\
\tconnect          Connect to server (format: connect server_ip server_port. EXAMPLE.: connect 127.0.0.1 23)\n\
\tdisconnect       Disconnect from server\n\n\
When connection establish you can use single or comma separeted list of arithmetic expressions. Last expression MUST end with symbol '=', this indicate end of command.\n\
EXAMPLE:\n\
\t2+2*2=\n\
\t2+2*2, (2+2)*2, 3+3*3=\n";
    if (transmit_result) transmit_result(help_msg);
    return true;
}
bool TcpClient::Impl::processConnectCmd(const std::string &command)
{
    if (tcp_connector.isValid())
    {
        transmit("Always connected!\n");
        return false;
    }
    std::vector<std::string> tokens;
    splitString(command, tokens);
    if (tokens.size() != 3)
    {
        transmit("Error in command. See help.");
        return false;
    }
    const std::string &ip = tokens[1];
    if (!net::isValidIp(ip))
    {
        transmit("Invalid param ip.");
        return false;
    }
    const std::string &port_str = tokens[2];
    unsigned short port = 0;
    try
    {port = static_cast<uint16_t>(std::stoi(port_str, 0, 10));}
    catch (...)
    {
        transmit("Invalid param port.");
        return false;
    };
    std::string msg;
    if (!tcp_connector.connect(ip, port, msg))
    {
        transmit(msg);
        return false;
    }
    transmit("Connected");
    return true;
}
bool TcpClient::Impl::processDisconnectCmd(const std::string &)
{
    if (!tcp_connector.disconnect())
    {
        transmit("Always disconnected");
        return false;
    }
    transmit("Disconnected");
    return true;
}
bool TcpClient::Impl::processUserCommand(const std::string &command)
{
    if (!tcp_connector.isValid())
    {
        transmit("Client not connected.");
        return false;
    }
    command_buffer.insert(command_buffer.end(), command.begin(), command.end());
    if (command_buffer.size() > MSG_BUF_SIZE)
    {
        transmit("Command to big.");
        command_buffer.clear();
        return false;
    }
    else if (command_buffer.back() == '=')
    {//msg complite
        if (is_run)
        {
            {
                std::lock_guard<std::mutex> lock(cmd_mtx);
                input_list.emplace_back(command_buffer);
            }
            new_command.notify_one();
            command_buffer.clear();
        }
        else
        {
            command_timer.start();
            auto answer = tcp_connector.send(command_buffer);
            command_timer.stop();
            transmit(answer);
            transmit("command time: " + std::to_string(command_timer.seconds()));
        }
    }
    return true;
}
void TcpClient::Impl::send_commands()
{
    for (const auto &cmd: work_list)
    {
        transmit(tcp_connector.send(cmd));
    }
    work_list.clear();
}
TcpClient::TcpClient():
pimpl_(std::make_unique<Impl>())
{
}

TcpClient::~TcpClient()
{
}

void TcpClient::run(const std::atomic_bool &terminate)
{
    pimpl_->is_run = true;
    while(!terminate)
    {
        std::unique_lock<std::mutex> lock(pimpl_->cmd_mtx);
        if (pimpl_->new_command.wait_for(lock, std::chrono::seconds(3), [&] { return !terminate && !pimpl_->input_list.empty(); }))
        {
            pimpl_->work_list.splice(pimpl_->work_list.end(), pimpl_->input_list);
            lock.unlock();
        }
        pimpl_->send_commands();
    }
    pimpl_->is_run = false;
}
void TcpClient::setTransmitResultCb(std::function<void (const std::string&)> recepient)
{
    pimpl_->transmit_result = recepient;
}
bool TcpClient::processCommand(std::string &command)
{
    if (command.empty()) return true;
    std::string cmd;
    cmd.swap(command);
    cmd.erase(cmd.begin(), std::find_if(cmd.begin(), cmd.end(), [](const char &ch){return !std::isspace(ch);}));
    cmd.erase(std::find_if(cmd.rbegin(), cmd.rend(), [](const char &ch){return !std::isspace(ch);}).base(), cmd.end());
    if (cmd.empty()) return true;
    return pimpl_->processCommand(cmd);
}
}