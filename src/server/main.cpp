#include <signal.h>
#include <atomic>
#include <iostream>
#include "version.h"
#include "nc_server.h"
#include <net/newtork_initializer.h>
static std::atomic_bool g_stop_program;
#if defined(__linux__) || defined(__APPLE__)
void sig_int_handler(__attribute__((unused)) int s)
{
    g_stop_program = true;
}
#elif defined(_WIN32)
#include <Windows.h>
BOOL __stdcall CloseHandlerFn(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
        g_stop_program = true;
        ExitThread(0);
        break;
    default: break;
    };
    return FALSE;
}
#endif
static bool init_startup_params(int ac, char** av, brct::NcServer::settings_t &setting);
static void show_syntax()
{
    std::cout << "Syntax of command:\n \tnc_server [--version] | [--help] | [ [--listen_ip=?] [--listen_port=?] ]\n\n\
PARAM                    DESCRIPTION\n\
--version                Show program version\n\
--help                   Show this help\n\
--listen_ip              IP address for clients (default: 0.0.0.0)\n\
--listen_port            TCP port for clients (default: 23)\n\n\
EXAMPLE: \tnc_server --listen_ip=0.0.0.0 --listen_port=23\n\
" << std::endl;
}
static void show_version()
{
    std::cout << "Version " NC_SERVER_VERSION << std::endl;
}
int main(int argc, char** argv)
{
#if defined(__linux__) || defined(__APPLE__)
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sig_int_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
#elif defined(_WIN32)
    SystemParametersInfoA(SPI_GETHUNGAPPTIMEOUT, 10000, NULL, 0);
    SetConsoleCtrlHandler(CloseHandlerFn, TRUE);
#endif
    brct::NcServer::settings_t settings;
    if (!init_startup_params(argc, argv, settings)) return 0;
    brct::NcServer nc_server(settings);
    nc_server.run(g_stop_program);
    return 0;
}
bool init_ushort_param(const std::string &value, uint16_t &param)
{
    try
    {
        param = static_cast<uint16_t>(std::stoi(value, 0, 10));
    }
    catch (...) { return false; };
    return true;
}
static bool init_startup_params(int ac, char** av, brct::NcServer::settings_t &setting)
{
    std::string param;
    for (int i = 1; i < ac; ++i)
    {
        param = av[i];
        if (param.find("--version") != std::string::npos)
        {
            show_version();
            return false;
        }
        else if (param.find("--help") != std::string::npos)
        {
            show_syntax();
            return false;
        }
        else if (param.find("--listen_ip=") != std::string::npos)
        {
            std::string ip = param.substr(12);
            if (!net::isValidIp(ip))
            {
                std::cout << "Invalid param listen_ip." << std::endl;
                return false;
            }
            setting.listen_ip = ip;
        }
        else  if (param.find("--listen_port=") != std::string::npos)
        {
            if (!init_ushort_param(param.substr(14), setting.listen_port))
            {
                std::cout << "Invalid param listen_port." << std::endl;
                return false;
            }
        }
        else
        {
            std::cout << "Unknown param " << param << ". --help for details." << std::endl;
            return false;
        }
    }
    return true;
}