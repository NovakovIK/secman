#include <thread>
#include <iostream>
#include "Scheduler.hpp"
#include "PriorityQueue.hpp"

void Scheduler::schedule(const TimePoint &timePoint, const Task &task)
{
    queue.insert(timePoint, task);
}

void Scheduler::runLoop()
{
    auto task = queue.pop();

    auto delay = task.first - std::chrono::system_clock::now();

    std::this_thread::sleep_for(delay);

    std::thread task_thread(task.second);

    // TODO: save task thread to track execution status...

    task_thread.detach();
}

Scheduler::~Scheduler()
{
    // TODO: join or detach all threads.
}
