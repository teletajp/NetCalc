#include "connection.h"
#include "net/newtork_initializer.h"
#include <vector>
#include <errno.h>
#include <fmt/format.h>
#include <list>
#include <algorithm>
#include <functional>
#define printNotice(msg) fmt::print(stdout, "[NOTICE] CLIENT {}: {}\n", info_, msg)
#define printError(msg) fmt::print(stderr, "[ERROR] CLIENT {}: {}\n", info_, msg)
namespace brct
{
constexpr int RECV_BUF_SIZE = 10*1024;
class Connection::Impl
{
public:
    enum class Status
    {
        NotInit = 0,
        Init,
        Established,
    };
    Impl(int fd, const std::string &info);
    ~Impl();
    Connection::ErrorCode receive();
    Connection::ErrorCode send(const std::string &data);
    bool parse();
    int fd_;
    std::string info_;
    uint8_t rcv_buf_[RECV_BUF_SIZE];
    std::string msg_buf_;
    Calculator::ExpressionList commands_;
    Status status_;
    std::function<bool (Calculator::ExpressionList &)> processor_;
};

Connection::Impl::Impl(int fd, const std::string &info):
fd_(fd),
info_(info),
status_(Status::NotInit)
{
    if (fd_ != INVALID_SOCKET) status_ = Status::Init;
    msg_buf_.reserve(RECV_BUF_SIZE);
}
Connection::Impl::~Impl()
{
    if (fd_ != INVALID_SOCKET)
    {
        shutdown(fd_, SD_BOTH);
        closesocket(fd_);
    }
}
Connection::ErrorCode Connection::Impl::receive()
{
    if (status_ == Status::NotInit) return Connection::ErrorCode::ConnectionNotInit;
    int rc = recv(fd_, (char*)rcv_buf_, RECV_BUF_SIZE, 0);
    if (rc < 0)
    { 
        auto err = errno;
        if (err == EAGAIN || err == EWOULDBLOCK)
            return Connection::ErrorCode::OK;
        return Connection::ErrorCode::ReadError;
    }
    if (rc == 0)
        return Connection::ErrorCode::PeerCloseConnection;
    try{ msg_buf_.insert(msg_buf_.end(), rcv_buf_, rcv_buf_ + rc);}
    catch(const std::exception &ex)
    {
        printError(fmt::format("receive EXCEPTION {}", ex.what()));
        send("Internal server error.");
    };
    //rtrim
    msg_buf_.erase(std::find_if(msg_buf_.rbegin(), msg_buf_.rend(), [](const char &ch ){return !std::isspace(ch);}).base(), msg_buf_.end());
    if (msg_buf_.size() >= RECV_BUF_SIZE)
    {
        auto err_msg = fmt::format("Command {}... too big (max len:{}", msg_buf_.substr(16), RECV_BUF_SIZE);
        printError(err_msg);
        send(err_msg);
    }
    if (msg_buf_.back() == '=')
    {//msg complite
        if (parse())
        {
            if (processor_ && processor_(commands_)) printNotice("processor return true.");
            else printError("processor return false.");
        }
        msg_buf_.clear();
    }
    
    return Connection::ErrorCode::OK;
}
Connection::ErrorCode Connection::Impl::send(const std::string &data)
{
    if (status_ == Status::NotInit) return Connection::ErrorCode::ConnectionNotInit;
    if (data.empty()) return Connection::ErrorCode::OK;
    const size_t need_send = data.length();
    size_t total_send = 0;
    size_t cur_send = 0;
    while (total_send != need_send)
    {
        cur_send = ::send(fd_, data.data() + total_send, need_send - total_send, 0);
        if (cur_send <= 0)
        {
            auto err = errno;
            if (err != EWOULDBLOCK)
            {
                printError(fmt::format("Send data: {}.", data));
                return Connection::ErrorCode::SendError;
            }
            cur_send = 0;
        }
        total_send += cur_send;
    }
    printNotice(fmt::format("Send data: {}", data));
    return Connection::ErrorCode::OK;
}
bool Connection::Impl::parse()
{
    std::string command; command.reserve(64);
    for(const auto& ch : msg_buf_)
    {
        if (ch == ',' && !command.empty())
        {
            printNotice(fmt::format("Detect command {}.", command));
            commands_.emplace_back(fd_,command);
            command.clear();
        }
        else if (ch >= '(' && ch <= '9')  command.push_back(ch);
        else if (std::isspace(ch)) continue;
        else if (ch == '=') break;
        else
        {
            commands_.clear();
            send(fmt::format("Invalid symbol {} in command {}", ch, msg_buf_));
            return false;
        }
    }
    printNotice(fmt::format("Detect command {}.", command));
    commands_.emplace_back(fd_, command);
    return true;
}
Connection::Connection(int fd, const std::string &info):
pimpl_(std::make_unique<Impl>(fd, info))
{
}

Connection::~Connection()
{
}

Connection::Connection(Connection && con)
{
    pimpl_.swap(con.pimpl_);
}

Connection::ErrorCode Connection::receive()
{
    return pimpl_->receive();
}
Connection::ErrorCode Connection::send(const std::string &data)
{
    return pimpl_->send(data);
}
const std::string & Connection::getInfo() const
{
    return pimpl_->info_;
}
void Connection::setProcessor(const std::function<bool (Calculator::ExpressionList &)> &processor)
{
    pimpl_->processor_ = processor;
}
#define CASE_VALUE_STRING(val) case val:return #val
const char* Connection::errorMsg(ErrorCode err)
{
    switch (err)
    {
    CASE_VALUE_STRING(ErrorCode::OK);
    CASE_VALUE_STRING(ErrorCode::ConnectionNotInit);
    CASE_VALUE_STRING(ErrorCode::ReadError);
    CASE_VALUE_STRING(ErrorCode::InvalidState);
    CASE_VALUE_STRING(ErrorCode::SendError);
    CASE_VALUE_STRING(ErrorCode::UnexpectedMsg);
    CASE_VALUE_STRING(ErrorCode::UnknownMsg);
    CASE_VALUE_STRING(ErrorCode::PeerCloseConnection);
    }
    return "UNKNOWN_ERROR";
}
}
