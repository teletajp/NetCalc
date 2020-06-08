#include "tcp_connector.h"
#include "net/newtork_initializer.h"
#include <future>
namespace brct
{
constexpr int RCV_BUF_SIZE = 10*1024;
class TcpConnector::Impl
{
public:
    Impl():fd(INVALID_SOCKET){};
    ~Impl();
    bool isInit() { return fd != INVALID_SOCKET; }
    bool init(const std::string &ip, const uint16_t port);
    bool connect(const std::string &ip, const uint16_t port);
    std::string send(const std::string &data);
    std::string receive();
    void close();
    int fd;
    struct sockaddr_in sa;
    std::string info;
};
bool TcpConnector::Impl::connect(const std::string &ip, const uint16_t port)
{
    if (fd != int(INVALID_SOCKET))
    {
        info = "Always connected";
        return false;
    }
    if (!init(ip, port))
    {
        return false;
    }
    if (::connect(fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
    {
        info = "Can't connect to server";
        closesocket(fd);
        fd = INVALID_SOCKET;
        return false;
    }
    return true;
}
bool TcpConnector::Impl::init(const std::string &ip, const uint16_t port)
{
    int domain = AF_INET;
    int socket_type = SOCK_STREAM;
    int proto = IPPROTO_TCP;
    if (inet_pton(domain, ip.c_str(), &(sa.sin_addr)) != 1)
    {
        info = "Invalid ip address.";
        return false;
    }
    info = ip + ":" + std::to_string(port);
    sa.sin_family = domain;
    sa.sin_port = htons(port);
    fd = (int)socket(domain, socket_type, proto);
    if (fd == int(INVALID_SOCKET))
    {
        info = "Can't create socket.";
        return false;
    }
    //int optval = 1;
    //setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
    return true;
}

TcpConnector::Impl::~Impl()
{
    if (fd != int(INVALID_SOCKET))
        closesocket(fd);
}
std::string TcpConnector::Impl::send(const std::string &data)
{
    if (fd == int(INVALID_SOCKET)) return "Error: Because disconnected";
    if (::send(fd, data.data(), data.size(), 0) == -1)
    {
        close();
        return "Send data error. Disconnect from server.";
    }
    auto answer = std::async(std::launch::async, &TcpConnector::Impl::receive, this);
    if (answer.wait_for(std::chrono::seconds(5)) == std::future_status::timeout)
    {
        close();
        return "Timeout for answer expired. Close connection.";
    }
    return answer.get();
}
std::string TcpConnector::Impl::receive()
{
    char buffer[RCV_BUF_SIZE];
    int ret = recv(fd, buffer, RCV_BUF_SIZE, 0);
    if (ret <= 0)
    {
        close();
        return "Send data error. Disconnect from server.";
    }
    return std::string(buffer);
}
void TcpConnector::Impl::close()
{
    shutdown(fd, SD_BOTH);
    closesocket(fd);
    fd = INVALID_SOCKET;
}
TcpConnector::TcpConnector():
pimpl_(std::make_unique<Impl>())
{
}

TcpConnector::~TcpConnector()
{
}
int TcpConnector::getSocket()
{
    return pimpl_->fd;
}
bool TcpConnector::isValid()
{
    return pimpl_->isInit();
}
bool TcpConnector::connect(const std::string &ip, const uint16_t port, std::string &msg)
{
    if (!pimpl_->connect(ip, port))
    {
        msg = pimpl_->info;
        return false;
    }
    return true;
}
std::string TcpConnector::send(const std::string &data)
{
    return pimpl_->send(data);
}
bool TcpConnector::disconnect()
{
    if (!isValid()) return false;
    pimpl_->close();
    return true;
}
}