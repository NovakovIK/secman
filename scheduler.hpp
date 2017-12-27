#ifndef SECMAN_SCHEDULER_H
#define SECMAN_SCHEDULER_H

#include <iomanip>
#include <map>

#include "tread_pool.hpp"
#include "interruptable_sleep.hpp"
#include "cron.hpp"

namespace secman
{
    class Task
    {
    public:
        explicit Task(std::function<void()> &&f, bool recur = false, bool interval = false);

        virtual std::chrono::system_clock::time_point get_new_time() const = 0;

        std::function<void()> f;

        bool recur;
        bool interval;
    };

    class InTask : public Task
    {
    public:
        explicit InTask(std::function<void()> &&f);
        // dummy time_point because it's not used
        std::chrono::system_clock::time_point get_new_time() const override;
    };

    class EveryTask : public Task
    {
    public:
        EveryTask(std::chrono::system_clock::duration time, std::function<void()> &&f, bool interval = false);

        std::chrono::system_clock::time_point get_new_time() const override;
        std::chrono::system_clock::duration time;
    };

    class CronTask : public Task
    {
    public:
        CronTask(const std::string &expression, std::function<void()> &&f);
        std::chrono::system_clock::time_point get_new_time() const override;
        Cron cron;
    };

    inline bool try_parse(std::tm &tm, const std::string &expression, const std::string &format);

    class Scheduler
    {
    public:
        explicit Scheduler(unsigned int max_n_tasks = 4);

        ~Scheduler();

        template<typename _Callable, typename... _Args>
        void in(const std::chrono::system_clock::time_point time, _Callable &&f, _Args &&... args);

        template<typename _Callable, typename... _Args>
        void in(const std::chrono::system_clock::duration time, _Callable &&f, _Args &&... args);

        template<typename _Callable, typename... _Args>
        void at(const std::string &time, _Callable &&f, _Args &&... args);

        template<typename _Callable, typename... _Args>
        void every(const std::chrono::system_clock::duration time, _Callable &&f, _Args &&... args);

// expression format:
// from https://en.wikipedia.org/wiki/Cron#Overview
//    ┌───────────── minute (0 - 59)
//    │ ┌───────────── hour (0 - 23)
//    │ │ ┌───────────── day of month (1 - 31)
//    │ │ │ ┌───────────── month (1 - 12)
//    │ │ │ │ ┌───────────── day of week (0 - 6) (Sunday to Saturday)
//    │ │ │ │ │
//    │ │ │ │ │
//    * * * * *
        template<typename _Callable, typename... _Args>
        void cron(const std::string &expression, _Callable &&f, _Args &&... args);

        template<typename _Callable, typename... _Args>
        void interval(const std::chrono::system_clock::duration time, _Callable &&f, _Args &&... args);

    private:
        std::atomic<bool> done;

        secman::InterruptableSleep sleeper;

        std::multimap<std::chrono::system_clock::time_point, std::shared_ptr<Task>> tasks;
        std::mutex lock;
        tp::thread_pool threads;

        void add_task(const std::chrono::system_clock::time_point time, std::shared_ptr<Task> t);

        void manage_tasks();
    };
}

#endif