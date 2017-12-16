#ifndef SECMAN_TASK_HPP
#define SECMAN_TASK_HPP


#include <functional>

class Task
{
public:
    explicit Task(const std::string &command);

    void operator()();

private:
    std::string command;
};


#endif //SECMAN_TASK_HPP
