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

    inline bool try_parse(std::tm &tm, const std::string &expression, const std::string &format)
    {
        std::stringstream ss(expression);
        return !(ss >> std::get_time(&tm, format.c_str())).fail();
    }

    class Scheduler
    {
    public:
        explicit Scheduler(unsigned int max_n_tasks = 4);

        ~Scheduler();

        template<typename _Callable, typename... _Args>
        void in(const std::chrono::system_clock::time_point time, _Callable &&f, _Args &&... args)
        {
            std::shared_ptr<Task> t = std::make_shared<InTask>(
                    std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...));
            add_task(time, std::move(t));
        }

        template<typename _Callable, typename... _Args>
        void in(const std::chrono::system_clock::duration time, _Callable &&f, _Args &&... args)
        {
            in(std::chrono::system_clock::now() + time, std::forward<_Callable>(f), std::forward<_Args>(args)...);
        }


        template<typename _Callable, typename... _Args>
        void at(const std::string &time, _Callable &&f, _Args &&... args)
        {
            // get current time as a tm object
            auto time_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm tm = *std::localtime(&time_now);

            // our final time as a time_point
            std::chrono::system_clock::time_point tp;

            if (try_parse(tm, time, "%H:%M:%S"))
            {
                // convert tm back to time_t, then to a time_point and assign to final
                tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

                // if we've already passed this time, the user will mean next day, so add a day.
                if (std::chrono::system_clock::now() >= tp)
                    tp += std::chrono::hours(24);
            } else if (try_parse(tm, time, "%Y-%m-%d %H:%M:%S"))
            {
                tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            } else if (try_parse(tm, time, "%Y/%m/%d %H:%M:%S"))
            {
                tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            } else
            {
                // could not parse time
                throw std::runtime_error("Cannot parse time string: " + time);
            }

            in(tp, std::forward<_Callable>(f), std::forward<_Args>(args)...);
        }

        template<typename _Callable, typename... _Args>
        void every(const std::chrono::system_clock::duration time, _Callable &&f, _Args &&... args)
        {
            std::shared_ptr<Task> t = std::make_shared<EveryTask>(time, std::bind(std::forward<_Callable>(f),
                                                                                  std::forward<_Args>(args)...));
            auto next_time = t->get_new_time();
            add_task(next_time, std::move(t));
        }

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
        void cron(const std::string &expression, _Callable &&f, _Args &&... args)
        {
            std::shared_ptr<Task> t = std::make_shared<CronTask>(expression, std::bind(std::forward<_Callable>(f),
                                                                                       std::forward<_Args>(args)...));
            auto next_time = t->get_new_time();
            add_task(next_time, std::move(t));
        }

        template<typename _Callable, typename... _Args>
        void interval(const std::chrono::system_clock::duration time, _Callable &&f, _Args &&... args)
        {
            std::shared_ptr<Task> t = std::make_shared<EveryTask>(time, std::bind(std::forward<_Callable>(f),
                                                                                  std::forward<_Args>(args)...), true);
            add_task(std::chrono::system_clock::now(), std::move(t));
        }


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