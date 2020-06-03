#pragma once
#include <memory>
#include <functional>
#include <list>
#include <string>
#include <thread/runnable.h>
namespace brct
{
class Calculator:public running::IRunnable
{
public:
    using Expression = std::pair<int,std::string>;
    using ExpressionList = std::list<Expression>;

    Calculator();
    ~Calculator();
    void setResultSubscriber(std::function<void (int,std::string&&)>&& result_subscriber);
    bool calculate(ExpressionList &expression_list);
    void run(const std::atomic_bool &terminate) override;
private:
    std::function<void (int,std::string&&)> result_subscriber_;
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
}