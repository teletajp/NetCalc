#include <signal.h>
#include <atomic>
#include <iostream>
#include "version.h"
#include "nc_client.h"
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

int main(int, char**)
{
#if defined(__linux__) || defined(__APPLE__)
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sig_int_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
#elif defined(_WIN32)
    //SystemParametersInfoA(SPI_GETHUNGAPPTIMEOUT, 1000, NULL, 0);
    SetConsoleCtrlHandler(CloseHandlerFn, TRUE);
#endif
    brct::NcClient nc_client;
    nc_client.run(g_stop_program);
    return 0;
}