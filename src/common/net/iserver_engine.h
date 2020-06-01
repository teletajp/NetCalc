#pragma once
#include <functional>
namespace net
{
using RCallBack = std::function<int(const int /*fd*/)>;
using WCallBack = std::function<int(const int /*fd*/)>;
using ECallBack = std::function<int(const int, const std::string &/*error_message*/)>;
/** @class IServerEngine
 *  @brief Интерфейс для движка сервера
 */
template<class Owner>
class IServerEngine
{
public:
   /**
   * @brief Установка колбека для события чтения
   * @param [in] rcb колбек, передающийся через делегат
   * 
   * Сервер должен хранить колбек на чтение и вызывать его каждый раз при событии появления данных в сокете
   */
    virtual void setReadCb(const RCallBack rcb) = 0;
    /**
    * @brief Установка колбека для события записи
    * @param [in] wcb колбек, передающийся через делегат
    *
    * Сервер должен хранить колбек на запись и вызывать его каждый раз при событии отправки данных в сокете
    */
    virtual void setWriteCb(const WCallBack wcb) = 0;
    /**
    * @brief Установка колбека для события ошибки
    * @param [in] ecb колбек, передающийся через делегат
    *
    * Сервер должен хранить колбек на ошибку и вызывать его каждый раз при событии появления ошибки в сокете
    */
    virtual void setErrorCb(const ECallBack ecb) = 0;
    /**
    * @brief Основной проход цикла
    *
    * В основном цикле происходит проверка дескрипторов сокетов и вызов соответсвующего колбека 
    */
    virtual bool nextLoop() = 0;
    /**
    * @brief Добавление дескриптора сокета в множество контролируемых дескрипторов
    */
    virtual bool addSocket(int fd) = 0;
    /**
    * @brief Удаление дескриптора сокета из множества контролируемых дескрипторов
    */
    virtual bool delSocket(int fd) = 0;
};
}