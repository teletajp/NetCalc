#include "calculator.h"
#include <mutex>
#include <fmt/format.h>
#include <condition_variable>
#include "calculator.hpp"//third-party library
#define printNotice(id, msg) fmt::print(stdout, "[NOTICE] CALCULATOR{}: {}\n", id, msg)
#define printError(id, msg) fmt::print(stderr, "[ERROR] CALCULATOR{}: {}\n",id, msg)
namespace brct
{
static int next_calulator_id; 
class Calculator::Impl
{
public:
    const int id;
    Impl():id(++next_calulator_id){};
    ~Impl() = default;
    bool calculate(ExpressionList &expression_list);
    std::mutex exp_mutex;
    ExpressionList work_list;
    ExpressionList input_list;
    std::condition_variable new_data_event;
};
bool Calculator::Impl::calculate(ExpressionList &expression_list)
{
    try
    {
        std::lock_guard<std::mutex> lock(exp_mutex);
        input_list.splice(input_list.end(), expression_list);
    }
    catch(const std::exception ex){return false;}
    new_data_event.notify_one();
    return true;
}
Calculator::Calculator():
pimpl_(std::make_unique<Impl>())
{
}
Calculator::~Calculator()
{

}
void Calculator::setResultSubscriber(std::function<void (int,std::string&&)>&& result_subscriber)
{
    result_subscriber_ = std::move(result_subscriber);
}
bool Calculator::calculate(ExpressionList &expression_list)
{
    return pimpl_->calculate(expression_list);
}
 void Calculator::run(const std::atomic_bool &terminate)
 {
     printNotice(pimpl_->id, "Start thread");
     while(!terminate)
     {
         std::unique_lock<std::mutex> lock(pimpl_->exp_mutex);
         if(pimpl_->new_data_event.wait_for(lock, std::chrono::seconds(3), [&]{return !terminate && !pimpl_->input_list.empty();}))
         {
             pimpl_->work_list.swap(pimpl_->input_list);
             lock.unlock();
             for (const auto& expression: pimpl_->work_list)
             {
                 std::string result_expression;
                 try
                 {
                     int64_t res = calculator::eval<int64_t>(expression.second);
                     result_expression = fmt::format("{} = {}\r\n", expression.second, res);
                     printNotice(pimpl_->id, result_expression);
                 }
                 catch(const std::exception& e)
                 {
                     result_expression = fmt::format("{} = EXCEPTION ({})\r\n", expression.second, e.what());
                     printError(pimpl_->id, result_expression);
                 }
                 if (result_subscriber_)
                     result_subscriber_(expression.first, std::move(result_expression));
             }
             pimpl_->work_list.clear();
         }
     }
     printNotice(pimpl_->id, "Stop thread");
 }
}