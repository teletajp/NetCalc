#include "listener.h"
#include "net/newtork_initializer.h"
namespace brct
{
class Listener::Impl
{
public:
    Impl(std::string ip, uint16_t port, bool is_blocked);
    ~Impl();
    bool isInit() { return fd_ != INVALID_SOCKET; }
    const uint16_t port_;
    const bool is_blocked_;
    int fd_;
    struct sockaddr_in sa_;
    std::string info_;
};

Listener::Impl::Impl(std::string ip, uint16_t port, bool is_blocked):
port_(port),
is_blocked_(is_blocked)
{
    int domain = AF_INET;
    int socket_type = SOCK_STREAM;
    int proto = IPPROTO_TCP;
    if (inet_pton(domain, ip.c_str(), &(sa_.sin_addr)) != 1)
    {
        ip = "0.0.0.0";
        inet_pton(domain, ip.c_str(), &(sa_.sin_addr));
    }
    info_ = "ip_str:" + std::to_string(port);
    sa_.sin_family = domain;
    sa_.sin_port = htons(port);
    fd_ = (int)socket(domain, socket_type, proto);
    if (fd_ != int(INVALID_SOCKET))
    {
        //int optval = 1;
        //setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
        if (bind(fd_, (struct sockaddr *)&sa_, sizeof(sa_)) != 0)
        {
            closesocket(fd_);
            fd_ = int(INVALID_SOCKET);
        }
        else if(is_blocked_)
        {
            if (!net::setSocketNoBlock(fd_))
            {
                closesocket(fd_);
                fd_ = int(INVALID_SOCKET);
            }
        }

        if (fd_ != int(INVALID_SOCKET))
        {
            if (listen(fd_, SOMAXCONN) != 0)
            {
                closesocket(fd_);
                fd_ = int(INVALID_SOCKET);
            }
        }
    }
}

Listener::Impl::~Impl()
{
    if (fd_ != int(INVALID_SOCKET))
        closesocket(fd_);
}
Listener::Listener(uint16_t port, bool is_blocked):
pimpl_(std::make_unique<Impl>("0.0.0.0", port, is_blocked))
{
}

Listener::Listener(std::string ip, uint16_t port, bool is_blocked):
pimpl_(std::make_unique<Impl>(ip, port, is_blocked))
{
}

Listener::~Listener()
{
}
int Listener::getSocket()
{
    return pimpl_->fd_;
}
bool Listener::isValid()
{
    return pimpl_->isInit();
}
int Listener::accept(std::string &info)
{
    int fd = int(INVALID_SOCKET);
    if (!pimpl_->isInit())
    {
        info = "Listen socket not init";
        return fd;
    }
    sockaddr_in sa;
#ifdef _WIN32
    int sal = sizeof(sockaddr_in);
    fd = (int)::accept(pimpl_->fd_, (SOCKADDR*)&sa, &sal);
#endif
#if defined(__linux__) || defined(__APPLE__)
    int sal = sizeof(sockaddr_in);
    fd = (int)::accept(pimpl_->fd_, (sockaddr*)&sa, (socklen_t*)&sal);
#endif
    if (fd == INVALID_SOCKET) info = "accept failed";
    else
    {
        info = inet_ntoa(sa.sin_addr);
        info.append(":").append(std::to_string(ntohs(sa.sin_port)));
    }
    return fd;
}
}