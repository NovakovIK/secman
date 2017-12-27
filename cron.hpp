#ifndef SECMAN_CRON_H
#define SECMAN_CRON_H

#include <chrono>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

namespace secman
{
    inline void add(std::tm &tm, std::chrono::system_clock::duration time);

    inline void verify_and_set(const std::string& token, const std::string &expression,
                               int &field, const int lower_bound, const int upper_bound, const bool adjust = false);

    class Cron
    {
    public:
        explicit Cron(const std::string &expression);

        // http://stackoverflow.com/a/322058/1284550
        std::chrono::system_clock::time_point cron_to_next() const;

        int minute, hour, day, month, day_of_week;
    };
}



#endif