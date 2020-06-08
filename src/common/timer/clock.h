#pragma once
#include <chrono>
namespace timer
{
class Clock
{
private:
    std::chrono::system_clock::time_point start_;
    std::chrono::system_clock::time_point stop_;

public:
    Clock(/* args */)=default;
    ~Clock()=default;
    void start() { start_ = std::chrono::system_clock::now(); };
    void stop() { stop_ = std::chrono::system_clock::now(); };
    double seconds() { return std::chrono::duration_cast<std::chrono::milliseconds>(stop_ - start_).count()/1000.00; }
};
}