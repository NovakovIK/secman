#include "Task.hpp"

Task::Task(const std::string &command) : command(command) {}

void Task::operator()()
{
    system(command.c_str());
}
