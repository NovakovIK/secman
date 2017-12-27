#include "cron.hpp"


void secman::add(std::tm &tm, std::chrono::system_clock::duration time)
{
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    auto tp_adjusted = tp + time;
    auto tm_adjusted = std::chrono::system_clock::to_time_t(tp_adjusted);
    tm = *std::localtime(&tm_adjusted);
}

void secman::verify_and_set(const std::string &token, const std::string &expression, int &field, const int lower_bound,
                            const int upper_bound, const bool adjust)
{
    if (token == "*")
        field = -1;
    else
    {
        field = std::stoi(token);
        if (field < lower_bound || field > upper_bound) throw std::runtime_error("cron out of range: " + expression);
        if (adjust)
            field--;
    }
}

secman::Cron::Cron(const std::string &expression)
{
    std::istringstream iss(expression);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};

    if (tokens.size() != 5) throw std::runtime_error("malformed cron string: " + expression);

    verify_and_set(tokens[0], expression, minute, 0, 59);
    verify_and_set(tokens[1], expression, hour, 0, 23);
    verify_and_set(tokens[2], expression, day, 1, 31);
    verify_and_set(tokens[3], expression, month, 1, 12, true);
    verify_and_set(tokens[4], expression, day_of_week, 0, 6);
}

std::chrono::system_clock::time_point secman::Cron::cron_to_next() const
{
    // get current time as a tm object
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm next(*std::localtime(&now));
    // it will always at least run the next minute
    next.tm_sec = 0;
    add(next, std::chrono::minutes(1));
    while (true)
    {
        if (month != -1 && next.tm_mon != month)
        {
            // add a month
            // if this will bring us over a year, increment the year instead and reset the month
            if (next.tm_mon + 1 > 11)
            {
                next.tm_mon = 0;
                next.tm_year++;
            }
            else
                next.tm_mon++;

            next.tm_mday = 1;
            next.tm_hour = 0;
            next.tm_min = 0;
            continue;
        }
        if (day != -1 && next.tm_mday != day)
        {
            add(next, std::chrono::hours(24));
            next.tm_hour = 0;
            next.tm_min = 0;
            continue;
        }
        if (day_of_week != -1 && next.tm_wday != day_of_week)
        {
            add(next, std::chrono::hours(24));
            next.tm_hour = 0;
            next.tm_min = 0;
            continue;
        }
        if (hour != -1 && next.tm_hour != hour)
        {
            add(next, std::chrono::hours(1));
            next.tm_min = 0;
            continue;
        }
        if (minute != -1 && next.tm_min != minute)
        {
            add(next, std::chrono::minutes(1));
            continue;
        }
        break;
    }

    // telling mktime to figure out dst
    next.tm_isdst = -1;
    return std::chrono::system_clock::from_time_t(std::mktime(&next));
}