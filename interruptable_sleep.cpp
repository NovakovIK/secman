#include "interruptable_sleep.hpp"

secman::InterruptableSleep::InterruptableSleep() : interrupted(false) {}

void secman::InterruptableSleep::sleep_for(std::chrono::system_clock::duration duration)
{
    std::unique_lock<std::mutex> ul(m);
    cv.wait_for(ul, duration, [this] { return interrupted; });
    interrupted = false;
}

void secman::InterruptableSleep::sleep_until(std::chrono::system_clock::time_point time)
{
    std::unique_lock<std::mutex> ul(m);
    cv.wait_until(ul, time, [this] { return interrupted; });
    interrupted = false;
}

void secman::InterruptableSleep::sleep()
{
    std::unique_lock<std::mutex> ul(m);
    cv.wait(ul, [this] { return interrupted; });
    interrupted = false;
}

void secman::InterruptableSleep::interrupt()
{
    std::lock_guard<std::mutex> lg(m);
    interrupted = true;
    cv.notify_one();
}