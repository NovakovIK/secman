#ifndef SECMAN_INTERRUPTABLE_SLEEP_H
#define SECMAN_INTERRUPTABLE_SLEEP_H

#include <chrono>
#include <thread>
#include <future>
#include <mutex>
#include <sstream>

namespace secman
{
    class InterruptableSleep
    {
        // InterruptableSleep offers a sleep that can be interrupted by any thread.
        // It can be interrupted multiple times
        // and be interrupted before any sleep is called (the sleep will immediately complete)
        // Has same interface as condition_variables and futures, except with sleep instead of wait.
        // For a given object, sleep can be called on multiple threads safely, but is not recommended as behaviour is undefined.

    public:
        InterruptableSleep();
        InterruptableSleep(const InterruptableSleep &) = delete;
        InterruptableSleep(InterruptableSleep &&) noexcept = delete;
        ~InterruptableSleep() noexcept = default;
        InterruptableSleep& operator=(const InterruptableSleep &) noexcept = delete;
        InterruptableSleep& operator=(InterruptableSleep &&) noexcept = delete;

        void sleep_for(std::chrono::system_clock::duration duration);
        void sleep_until(std::chrono::system_clock::time_point time);
        void sleep();
        void interrupt();

    private:
        bool interrupted;
        std::mutex m;
        std::condition_variable cv;
    };
}

#endif