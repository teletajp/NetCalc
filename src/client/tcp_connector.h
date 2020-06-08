#pragma once
#include <memory>
#include <string>
namespace brct
{
class TcpConnector
{
public:
    TcpConnector();
    ~TcpConnector();
    int getSocket();
    bool isValid();
    /**
     * @brief Соединение с сервером
     * 
     * @param ip IP сервера
     * @param port TCP порт сервера
     * @param is_blocked Блокирующий режим или нет
     * @param msg Сообщение
     * @return true Соединение успешно
     * @return false Соединение не успешно в msg возвращается информация об ощибке
     */
    bool connect(const std::string &ip, const uint16_t port, std::string &msg);
    std::string send(const std::string &data);
    bool disconnect();
private:
    TcpConnector(const TcpConnector&) = delete;
    TcpConnector(TcpConnector&&) = delete;
    TcpConnector& operator= (const TcpConnector&) = delete;
    TcpConnector& operator= (TcpConnector&&) = delete;
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

}