//
// Created by ivan-novakov on 12/10/17.
//

#ifndef SECMAN_SCHEDULER_HPP
#define SECMAN_SCHEDULER_HPP


#include <chrono>
#include "Task.hpp"
#include "PriorityQueue.hpp"

class Scheduler
{
public:
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    ~Scheduler();

    void schedule(const TimePoint &timePoint, const Task &task);

    void runLoop();

private:

    PriorityQueue<TimePoint, Task> queue;
};


#endif //SECMAN_SCHEDULER_HPP
