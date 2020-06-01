#include "newtork_initializer.h"
namespace net
{
NetworkInitializer g_network_initializer;

bool setSocketNoBlock(int fd)
{
    int ret = 0;
#if defined(__linux__) || defined(__APPLE__)
    int flags;
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    ret = ioctl(fd, FIONBIO, &flags);
#endif
#endif

#ifdef _WIN32
    unsigned long mode_ = 1;
    ret = ioctlsocket(fd, FIONBIO, &mode_);
#endif
    if (ret >= 0) return true;
    return false;
}
bool setSocketReuse(int fd)
{
    if (fd == INVALID_SOCKET) return false;
    int optval = 1;
    return 0 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
}
bool isValidIp(const std::string & ip)
{

    bool isV4 = true;
    std::string::size_type beg, end;
    beg = end = 0;
    end = ip.find('.', beg);
    if (end == std::string::npos)
    {
        end = ip.find(':', beg);
        if (end == std::string::npos) return false;
        isV4 = false;
    }

    if (isV4)
    {
        in_addr buf;
        if (inet_pton(AF_INET, ip.c_str(), (PVOID)&buf) > 0) return true;
    }
    else
    {
        in6_addr buf;
        if (inet_pton(AF_INET6, ip.c_str(), (PVOID)&buf) > 0) return true;
    }
    return false;
}
}