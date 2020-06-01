#pragma once
#include <memory>
#include <string>
namespace brct
{
class Listener
{
public:
    Listener(uint16_t port, bool is_blocked);
    Listener(std::string ip, uint16_t port, bool is_blocked);
    ~Listener();
    int getSocket();
    bool isValid();
    int accept(std::string &info);
private:
    Listener(const Listener&) = delete;
    Listener(Listener&&) = delete;
    Listener& operator= (const Listener&) = delete;
    Listener& operator= (Listener&&) = delete;
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

}