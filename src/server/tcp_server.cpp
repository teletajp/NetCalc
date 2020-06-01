#include <map>
#include <mutex>
#include <fmt/format.h>
#include "tcp_server.h"
#include "connection.h"
#include "listener.h"
#define printNotice(msg) fmt::print(stdout, "[NOTICE] SERVER: {}\n", msg)
#define printError(msg) fmt::print(stderr, "[ERROR] SERVER: {}\n", msg)
namespace brct
{
class TcpServer::Impl
{
public:
    Impl(const std::string &ip, uint16_t port);
    ~Impl();
    std::string ip_;
    uint16_t port_;
    Listener listener_;
    const std::string info_;
    std::mutex connection_mtx_;
    std::map<int, std::unique_ptr<Connection>> all_connections_;
};

TcpServer::Impl::Impl(const std::string &ip, uint16_t port):
ip_(ip),
port_(port),
listener_(ip, port, false),
info_(fmt::format("{}:{}", ip, port))
{
    
}

TcpServer::Impl::~Impl()
{
}

TcpServer::TcpServer(const std::string &ip,uint16_t port):
pimpl_(std::make_unique<Impl>(ip, port))
{
    using std::placeholders::_1;
    engine_.setReadCb(std::bind(&TcpServer::onRead, this, _1));
    engine_.setWriteCb(std::bind(&TcpServer::onWrite, this, _1));
    using std::placeholders::_2;
    engine_.setErrorCb(std::bind(&TcpServer::onError, this, _1, _2));
}
TcpServer::~TcpServer()
{
}
int TcpServer::listenerOnRead()
{
    std::string info;
    const int new_fd = pimpl_->listener_.accept(info);
    if (new_fd == INVALID_SOCKET)
    {
        printError(info);
        return -1;
    }
    else
    {
        printNotice("New connection " + info);
        try
        {
            auto res = pimpl_->all_connections_.emplace(new_fd, std::make_unique<Connection>(new_fd, info));
            if (!res.second)
            {
                printError(fmt::format("Connection {} always exist. Logic error.", info));
                return -1;
            }
            res.first->second->setProcessor(processor_);
        }
        catch(const std::exception& e)
        {
            printError(fmt::format("Add connection {} exception {}}.", info, e.what()));
            deleteConnection(new_fd);
            return -1;
        }
        
        
        if (!net::setSocketNoBlock(new_fd))
        {
            printError(fmt::format("Connection {} setSocketNoBlock error.", info));
            deleteConnection(new_fd);
            return -1;
        }
        if (!engine_.addSocket(new_fd))
        {
            printError(fmt::format("Connection {} addSocket error.", info));
            deleteConnection(new_fd);
            return -1;
        }
    }
    return 0;
}
int TcpServer::clientOnRead(int fd)
{
    auto find_it = pimpl_->all_connections_.find(fd);
    if (find_it == pimpl_->all_connections_.end())
    {
        printError("Connection not exist. Logic error.");
        return -1;
    }
    auto &connection = find_it->second;
    const auto ret = connection->receive();
    if ( ret != Connection::ErrorCode::OK)
    {
        if (ret == Connection::ErrorCode::PeerCloseConnection)
            printNotice(fmt::format("Connection {} peer close connection.", connection->getInfo()));
        else
            printError(fmt::format("Connection {} receive error {}.", connection->getInfo(), Connection::errorMsg(ret)));
        deleteConnection(fd);
    }
    return 0;
}
int TcpServer::onRead(const int fd)
{
    if (fd == pimpl_->listener_.getSocket())
        return listenerOnRead();
    return clientOnRead(fd);
}
int TcpServer::onWrite(const int /*fd*/)
{
    //std::cout << fmt::format("{}", fd) << std::endl;
    return 0;
}
int TcpServer::onError(const int fd, const std::string &error_message)
{
    if (fd == pimpl_->listener_.getSocket())
        printError("Error on listen socket " + error_message);
    return 0;
}
void TcpServer::run(const std::atomic_bool &terminate)
{
    if (!pimpl_->listener_.isValid())
        printError("Listener not valid. Check port setting.");
    else
    {
        printNotice("Server started");
        if (!engine_.addSocket(pimpl_->listener_.getSocket()))
            printError("Server engine can't add listen socket.");
        else
        {
            while (!terminate)
            {
                engine_.nextLoop();
            }
        }
    }
    printNotice("Server stoped");
}
void TcpServer::deleteConnection(int fd)
{
    if (fd == INVALID_SOCKET) return;
    auto find_it = pimpl_->all_connections_.find(fd);
    if (find_it == pimpl_->all_connections_.end())
    {
        printError("deleteConnection: Connection not exist. Logic error.");
    }
    else
    {
        auto &connection = find_it->second;
        printNotice("deleteConnection: Close connection " + connection->getInfo());
        std::lock_guard<std::mutex> lock(pimpl_->connection_mtx_);
        engine_.delSocket(fd);
        pimpl_->all_connections_.erase(find_it);
    }
}

void TcpServer::setProcessor(std::function<void (std::vector<uint8_t>&, const std::string &)> &&processor)
{
    processor_= std::move(processor);};
}