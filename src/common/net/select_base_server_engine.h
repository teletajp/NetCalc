#pragma once
#include <set>
#include "newtork_initializer.h"
#include "iserver_engine.h"
namespace net
{
template<class Owner>
class SelectBaseServerEngine : public IServerEngine<Owner>
{
public:
    SelectBaseServerEngine() { FD_ZERO(&main_set_); };
    void setReadCb(const RCallBack rcb) override { rcb_ = rcb; };
    void setWriteCb(const WCallBack wcb) override { wcb_ = wcb; };
    void setErrorCb(const ECallBack ecb) override{ ecb_ = ecb; };
    bool nextLoop() override;
    bool addSocket(int fd) override;
    bool delSocket(int fd) override;
private:
    RCallBack rcb_;
    WCallBack wcb_;
    ECallBack ecb_;
    std::set<int> fd_set_;
    fd_set main_set_, rfds_, wfds_, efds_;
    const timeval select_timeout_ = {1, 0};
    int max_fd_ = 0;
};

template<class Owner>
inline bool SelectBaseServerEngine<Owner>::nextLoop()
{
    if (max_fd_ == 0) return true;

    timeval ttv = select_timeout_;
    /*wfds_ =*/ rfds_ = efds_ = main_set_;
    if (select(max_fd_ + 1, &rfds_, NULL, &efds_, &ttv) > 0)
    {
        auto it_next = fd_set_.begin();
        auto end = fd_set_.end();
        int fd = int(INVALID_SOCKET);
        for (auto it = it_next; it_next != end;)
        {
            it = it_next;
            ++it_next;
            fd = *it;
            if (FD_ISSET(fd, &rfds_) && rcb_) rcb_(fd);
            //if (FD_ISSET(fd, &wfds_) && wcb_) wcb_(fd);
            if (FD_ISSET(fd, &efds_) && ecb_) ecb_(fd, "Error on socket");
        }
    }
    return true;
}

template<class Owner>
inline bool SelectBaseServerEngine<Owner>::addSocket(int fd)
{
#if defined(__linux__) || defined(__APPLE__)
    if (fd >= FD_SETSIZE) return false;
#elif defined(_WIN32)
    if (main_set_.fd_count >= FD_SETSIZE) return false;
#endif
    if (fd_set_.emplace(fd).second)
    {
        if (fd > max_fd_) max_fd_ = fd;
        FD_SET(fd, &main_set_);
        return true;
    }
    return false;
}

template<class Owner>
inline bool SelectBaseServerEngine<Owner>::delSocket(int fd)
{
    FD_CLR(fd, &main_set_);
    fd_set_.erase(fd);
    if (fd == max_fd_)
    {
        max_fd_ = 0;
        for (auto fd : fd_set_)
        {
            if (fd > max_fd_) max_fd_ = fd;
        }
    } 
    return true;
}

}