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
