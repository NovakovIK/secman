#include <thread>
#include "Scheduler.hpp"

int main()
{
    using namespace std::chrono_literals;

    Scheduler scheduler;

    scheduler.schedule(std::chrono::system_clock::now() + 5s, Task("notify-send \"Caption\" \"Message\""));
    scheduler.runLoop();

    std::this_thread::sleep_for(10s);

    return 0;
}