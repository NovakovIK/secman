#include "scheduler.hpp"

secman::Task::Task(std::function<void()> &&f, bool recur, bool interval) : f(std::move(f)), recur(recur), interval(interval) {}

secman::InTask::InTask(std::function<void()> &&f) : Task(std::move(f)) {}

std::chrono::system_clock::time_point secman::InTask::get_new_time() const
{
    return std::chrono::system_clock::time_point(std::chrono::system_clock::duration(0));
}

secman::EveryTask::EveryTask(std::chrono::system_clock::duration time, std::function<void()> &&f, bool interval) : Task(std::move(f), true, interval), time(time) {}

std::chrono::system_clock::time_point secman::EveryTask::get_new_time() const
{
    return std::chrono::system_clock::now() + time;
};

secman::CronTask::CronTask(const std::string &expression, std::function<void()> &&f) : Task(std::move(f), true), cron(expression) {}

std::chrono::system_clock::time_point secman::CronTask::get_new_time() const
{
    return cron.cron_to_next();
}

bool secman::try_parse(std::tm &tm, const std::string &expression, const std::string &format)
{
    std::stringstream ss(expression);
    return !(ss >> std::get_time(&tm, format.c_str())).fail();
}

secman::Scheduler::Scheduler(unsigned int max_n_tasks) : done(false), threads(max_n_tasks + 1)
{
    threads.push([this](int)
                 {
                     while (!done)
                     {
                         if (tasks.empty())
                         {
                             sleeper.sleep();
                         }
                         else
                         {
                             auto time_of_first_task = (*tasks.begin()).first;
                             sleeper.sleep_until(time_of_first_task);
                         }
                         std::lock_guard<std::mutex> l(lock);
                         manage_tasks();
                     }
                 });
}

secman::Scheduler::~Scheduler()
{
    done = true;
    sleeper.interrupt();
}

template<typename _Callable, typename... _Args>
void secman::Scheduler::in(const std::chrono::system_clock::time_point time, _Callable &&f, _Args &&... args)
{
    std::shared_ptr<Task> t = std::make_shared<InTask>(
            std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...));
    add_task(time, std::move(t));
}

template<typename _Callable, typename... _Args>
void secman::Scheduler::in(const std::chrono::system_clock::duration time, _Callable &&f, _Args &&... args)
{
    in(std::chrono::system_clock::now() + time, std::forward<_Callable>(f), std::forward<_Args>(args)...);
}

template<typename _Callable, typename... _Args>
void secman::Scheduler::at(const std::string &time, _Callable &&f, _Args &&... args)
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
void secman::Scheduler::every(const std::chrono::system_clock::duration time, _Callable &&f, _Args &&... args)
{
    std::shared_ptr<Task> t = std::make_shared<EveryTask>(time, std::bind(std::forward<_Callable>(f),
                                                                          std::forward<_Args>(args)...));
    auto next_time = t->get_new_time();
    add_task(next_time, std::move(t));
}

template<typename _Callable, typename... _Args>
void secman::Scheduler::cron(const std::string &expression, _Callable &&f, _Args &&... args)
{
    std::shared_ptr<Task> t = std::make_shared<CronTask>(expression, std::bind(std::forward<_Callable>(f),
                                                                               std::forward<_Args>(args)...));
    auto next_time = t->get_new_time();
    add_task(next_time, std::move(t));
}

template<typename _Callable, typename... _Args>
void secman::Scheduler::interval(const std::chrono::system_clock::duration time, _Callable &&f, _Args &&... args)
{
    std::shared_ptr<Task> t = std::make_shared<EveryTask>(time, std::bind(std::forward<_Callable>(f),
                                                                          std::forward<_Args>(args)...), true);
    add_task(std::chrono::system_clock::now(), std::move(t));
}

void secman::Scheduler::add_task(const std::chrono::system_clock::time_point time, std::shared_ptr<secman::Task> t)
{
    std::lock_guard<std::mutex> l(lock);
    tasks.emplace(time, std::move(t));
    sleeper.interrupt();
}

void secman::Scheduler::manage_tasks()
{
    auto end_of_tasks_to_run = tasks.upper_bound(std::chrono::system_clock::now());

    // if there are any tasks to be run and removed
    if (end_of_tasks_to_run != tasks.begin())
    {
        decltype(tasks) recurred_tasks;

        for (auto i = tasks.begin(); i != end_of_tasks_to_run; ++i)
        {

            auto &task = (*i).second;

            if (task->interval)
            {
                // if it's an interval task, add the task back after f() is completed
                threads.push([this, task](int)
                             {
                                 task->f();
                                 add_task(task->get_new_time(), task);
                             });
            }
            else
            {
                threads.push([task](int)
                             {
                                 task->f();
                             });
                // calculate time of next run and add the new task to the tasks to be recurred
                if (task->recur)
                    recurred_tasks.emplace(task->get_new_time(), std::move(task));
            }
        }

        // remove the completed tasks
        tasks.erase(tasks.begin(), end_of_tasks_to_run);

        // re-add the tasks that are recurring
        for (auto &task : recurred_tasks)
            tasks.emplace(task.first, std::move(task.second));
    }
}
