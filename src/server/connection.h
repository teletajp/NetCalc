#pragma once
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include "calculator.h"
namespace logger{class ILogger;}
namespace brct
{
class Connection
{
public:
    enum class ErrorCode
    {
        OK = 0,
        ConnectionNotInit,
        ReadError,
        InvalidState,
        SendError,
        UnexpectedMsg,
        UnknownMsg,
        PeerCloseConnection,
    };
    static const char* errorMsg(ErrorCode);
    Connection(int fd, const std::string &info);
    ~Connection();
    Connection(Connection&& con);
    ErrorCode receive();
    ErrorCode send(const std::string &data);
    const std::string &getInfo() const;
    void setProcessor(const std::function<bool (Calculator::Expression &)> &processor);
private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

}