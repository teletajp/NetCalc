#pragma once
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
      #define WIN32_LEAN_AND_MEAN
      #include <WinSock2.h>
      #include <Ws2tcpip.h>
      #pragma comment(lib, "Ws2_32.lib")
    #endif
#endif
#if defined(__linux__) || defined(__APPLE__)
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SD_BOTH             SHUT_RDWR
#define WSAEWOULDBLOCK      EINPROGRESS
#define closesocket         ::close
#define INVALID_SOCKET     (~0)
#define PVOID void*
#endif
#include <string>
namespace net
{
struct NetworkInitializer
{
    bool is_init = true;
    NetworkInitializer()
    {
#ifdef WIN32
        WSADATA wsaData;
        int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (err) is_init = false;
#endif
    }
    ~NetworkInitializer()
    {
#ifdef WIN32
        WSACleanup();
#endif
    }
};
extern NetworkInitializer g_network_initializer;
bool setSocketNoBlock(int fd);
bool setSocketReuse(int fd);
bool isValidIp(const std::string & ip);
}