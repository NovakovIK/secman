#include "args.hpp"

#ifndef SECMAN_CLI_HPP

struct cli : args::group<cli>
{
    static const char* help()
    {
        return "cli";
    }
};

#define SECMAN_CLI_HPP

#endif