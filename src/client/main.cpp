#include <protocol_v2_3.pb.h>
#include "server/ucs_server.h"
#include "server/newtork_initializer.h"
#include <iostream>
#include <fstream>
#include <fmt/format.h>
#include <logger/mt_logger.h>
#include <cache_timer/cache_timer.h>
#include <signal.h>
#include <filesystem>
#include "version.h"
bool g_stop_program = false;
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
namespace fs = std::filesystem;
static bool init_startup_params(int ac, char** av, ucs::ucs_server::settings_t &ucs_setting);
static void show_syntax()
{
    std::cout << "Syntax of command:\n \tucs_server [--version] | [--help] | [ --db_host=? --db_port=? --db_name=? --user=? --passw=? \
[--listen_ip=?] [--listen_port=?] [--storage_path=?] [--log_path=?] [--report_path=?] [--db_connections_count=?] [--windows_paths] [--save_stat_minutes=?] [--disable_mover]]\n\n\
PARAM                    DESCRIPTION\n\
--version                Show program version\n\
--help                   Show this help\n\
--db_host                Database host name or ip\n\
--db_port                Database tcp port\n\
--db_name                Database name\n\
--user                   Database user\n\
--passw                  Database user password\n\
--db_connections_count   Count of connections to database (each in standalone thread)\n\
--listen_ip              IP address for clients (default: 0.0.0.0)\n\
--listen_port            TCP port for clients (default: 10320)\n\
--storage_path           Path to save media files (default: ./storage)\n\
--log_path               Path for log files (default: ./log)\n\
--report_path            Path for report's files (results of parsing incomming data, default: ./storage)\n\
--windows_paths          Use windows path separator in sql queries path parameters\n\
--save_stat_minutes      Time period in minutes for save screen statistic in log (default: 1)\n\
--disable_mover          Disable file mover thread (set ONLY if media files saves to local directory, this improve performance. Default: false)\n\
--use_cameras_filtr      Use cameras' filtr for saving files. Default: false)\n\n\
EXAMPLE: \tucs_server --listen_ip=0.0.0.0 --listen_port=10320 \
--storage_path=\".\" --log_path=\".\" --report_path=\".\" --db_host=127.0.0.1 --db_port=3306 --db_name=safecity \
--user=safecity --passw=safecity --db_connections_count=1 --windows_paths --save_stat_minutes=1 --disable_mover\n\
" << std::endl;
}
static void show_version()
{
    std::cout << "Version " UCS_VERSION << std::endl;
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
    SystemParametersInfoA(SPI_GETHUNGAPPTIMEOUT, 40000, NULL, 0);
    SetConsoleCtrlHandler(CloseHandlerFn, TRUE);
#endif
    ucs::ucs_server::settings_t ucs_setting;
    fs::path full_program_name = argv[0];
    fs::path parent_path = full_program_name.parent_path();
    if (parent_path.empty())
    {
        try {parent_path = fs::current_path();} catch(const fs::filesystem_error &ex)
        {std::cout << "Get current path error " << ex.what() << std::endl; return 0;}
    }
    fs::path program_name = full_program_name.filename();
    if (!init_startup_params(argc, argv, ucs_setting))
        return 0;
#ifdef _WIN32
    std::string base = program_name.concat(".log").string();
    std::string log_path = ucs_setting.log_path.string();
#elif defined(__linux__) || defined(__APPLE__)
    std::string base = program_name.concat(".log").u8string();
    std::string log_path = ucs_setting.log_path.u8string();
#endif
    g_cache_timer.start();
    {
        logget_ptr_t logger = std::make_shared<MTLogger>(base, log_path, size_t(1024UL * 1024UL * 2048UL));
        std::unique_ptr<running::AutoThread> logger_thread;
        logger_thread = std::make_unique<running::AutoThread>(dynamic_cast<running::IRunnable*>((MTLogger*)&(*logger)));
        ucs::ucs_server server(ucs_setting, logger.get());
        server.run(g_stop_program);
    }
    g_cache_timer.stop();
    return 0;
}

static bool init_path_param(const std::string &path_str,  fs::path &path)
{
    std::size_t bpos = 0;
    std::size_t len = path_str.length();
    if (path_str.front()=='\"'){++bpos; --len;}
    if (path_str.back()=='\"') --len;

    try
    {
        path = fs::absolute(path_str.substr(bpos, len));
        path /= "";
    }
    catch (const fs::filesystem_error & er)
    {
        std::cout << "init_path_param error path" << path_str << ". Error " << er.what() << std::endl;
        return false;
    }
    catch (const std::bad_alloc& ex)
    {
        std::cout << "init_path_param exception path " << path_str << ". Exception " << ex.what() << std::endl;
        return false;
    }
    return true;
}
static bool create_path(const fs::path &path)
{
    try
    {
        if (!fs::exists(path))
            fs::create_directories(path);
    }
    catch (const fs::filesystem_error & er)
    {
        std::cout << "Error path " << path << " can't be created. Error " << er.what() << std::endl;
        return false;
    }
    catch (const std::exception & ex)
    {
        std::cout << "Error path " << path << " can't be created. Exception " << ex.what() << std::endl;
        return false;
    }
    return true;
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
static bool init_startup_params(int ac, char** av, ucs::ucs_server::settings_t &ucs_setting)
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
        else if (param.find("--db_host=") != std::string::npos)
        {
            ucs_setting.host_name = param.substr(10);
        }
        else if (param.find("--db_port=") != std::string::npos)
        {
            if (!init_ushort_param(param.substr(10), ucs_setting.port))
            {
                std::cout << "Invalid param db_port." << std::endl;
                return false;
            }
        }
        else if (param.find("--db_name=") != std::string::npos)
        {
            ucs_setting.db_name = param.substr(10);
        }
        else if (param.find("--user=") != std::string::npos)
        {
            ucs_setting.user_name = param.substr(7);
        }
        else if (param.find("--passw=") != std::string::npos)
        {
            ucs_setting.password = param.substr(8);
        }
        else  if (param.find("--db_connections_count=") != std::string::npos)
        {
            if (!init_ushort_param(param.substr(23), ucs_setting.db_connections_count))
            {
                std::cout << "Invalid param db_connections_count." << std::endl;
                return false;
            }
        }
        else if (param.find("--listen_ip=") != std::string::npos)
        {
            std::string ip = param.substr(12);
            if (!isValidIp(ip))
            {
                std::cout << "Invalid param listen_ip." << std::endl;
                return false;
            }
            ucs_setting.listen_ip = ip;
        }
        else  if (param.find("--listen_port=") != std::string::npos)
        {
            if (!init_ushort_param(param.substr(14), ucs_setting.listen_port))
            {
                std::cout << "Invalid param listen_port." << std::endl;
                return false;
            }
        }
        else if (param.find("--storage_path=") != std::string::npos)
        {
            if (!init_path_param(param.substr(15), ucs_setting.storage_path) ||
                !create_path(ucs_setting.storage_path))
            {
                std::cout << "Invalid param storage_path." << std::endl;
                return false;
            }
        }
        else if (param.find("--log_path=") != std::string::npos)
        {
            if (!init_path_param(param.substr(11), ucs_setting.log_path) ||
                 !create_path(ucs_setting.log_path))
            {
                std::cout << "Invalid param log_path." << std::endl;
                return false;
            }
        }
        else if (param.find("--report_path=") != std::string::npos)
        {
            if (!init_path_param(param.substr(14), ucs_setting.report_path) ||
                 !create_path(ucs_setting.report_path))
            {
                std::cout << "Invalid param report_path." << std::endl;
                return false;
            }
        }
        
        else if (param.find("--windows_paths") != std::string::npos)
        {
            ucs_setting.windows_paths = true;
        }
        else  if (param.find("--save_stat_minutes=") != std::string::npos)
        {
            if (!init_ushort_param(param.substr(20), ucs_setting.save_stat_minutes))
            {
                std::cout << "Invalid param save_stat_minutes." << std::endl;
                return false;
            }
        }
        else  if (param.find("--disable_mover") != std::string::npos)
        {
            ucs_setting.disable_mover = true;
        }
        else  if (param.find("--use_cameras_filtr") != std::string::npos)
        {
            ucs_setting.use_cameras_filtr = true;
        }
        else
        {
            std::cout << "Unknown param " << param << ". --help for details." << std::endl;
            return false;
        }
    }
    return true;
}