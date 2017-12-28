#include <chrono>
#include "argparse.hpp"
#include "scheduler.hpp"
#include <memory>
#include <cstring>

using namespace std;

void exec(const string &expression)
{
    system(expression.c_str());
}

int main(int argc, const char** argv)
{
    secman::Scheduler s(12);

    // make a new ArgumentParser
    ArgumentParser parser;
    parser.appName("secman");

    // add some arguments to search for
    parser.addArgument("-a", "--at", '+', true);
    //parser.addArgument("-e", "--every", '+', true);
    //parser.addArgument("-i", "--interval", 1, true);
    parser.addArgument("-c", "--cron", '+', true);
    parser.addArgument("-e", "--execute", '+', true);
    parser.addArgument("-l", "--list", true);
    parser.addArgument("-d", "--delete", 1, true);


    // parse the command-line arguments - throws if invalid format
    parser.parse(argc, argv);



    if (parser.count("at"))
    {
        if(parser.count("execute"))
        {
            vector<string> command = parser.retrieve<vector<string>>("execute");

            string command_parse;

            for (auto &i : command)
            {
                command_parse += i + ' ';
            }
            command_parse.push_back('\0');

            vector<string> at = parser.retrieve<vector<string>>("at");

            string at_time;

            for (auto &i : at)
            {
                at_time += i + ' ';
            }
            at_time.erase(at_time.end() - 1);
            at_time.push_back('\0');
            auto command_c = new char[command_parse.size()];
            std::copy(command_parse.begin(), command_parse.end(), command_c);

            auto at_time_c = new char[at_time.size()];
            std::copy(at_time.begin(), at_time.end(), at_time_c);

            cout << at_time_c << endl << command_c << endl;
            s.at(at_time_c, system, command_c);
        }
    }

    if (parser.count("cron"))
    {
        if(parser.count("execute"))
        {
            vector<string> command = parser.retrieve<vector<string>>("execute");

            string command_parse;

            for (auto &i : command)
            {
                command_parse += i + ' ';
            }
            vector<string> cron = parser.retrieve<vector<string>>("cron");

            string cron_time;

            cron_time = cron[0] + ' ' + cron[1] + ' ' + cron[2] + ' ' + cron[3] + ' ' + cron[4] + '\0';

            auto command_c = new char[command_parse.size()];
            std::copy(command_parse.begin(), command_parse.end(), command_c);

            auto cron_time_c = new char[cron_time.size()];
            std::copy(cron_time.begin(), cron_time.end(), cron_time_c);

            cout << cron_time_c << endl << command_c << endl;
            s.cron(cron_time_c, system, command_c);
        }
    }

//    if (parser.count("every"))
//    {
//        vector<string> every = parser.retrieve<vector<string>>("every");
//
//        string str;
//        for (int i = 1; i < every.size(); ++i)
//        {
//            str += every[i] + ' ';
//        }
//
//        auto command = new char[str.size()];
//        std::copy(str.begin(), str.end(), command);
//
//        s.every(std::chrono::seconds(15), system, command);
//    }
//
//    if (parser.count("interval"))
//    {
//        vector<string> interval = parser.retrieve<vector<string>>("interval");
//
//        string str;
//        for (int i = 1; i < interval.size(); ++i)
//        {
//            str += interval[i] + ' ';
//        }
//
//        auto command = new char[str.size()];
//        std::copy(str.begin(), str.end(), command);
//
//        s.interval(std::chrono::seconds(15), system, command);
//    }




    std::this_thread::sleep_for(std::chrono::minutes(10));
    return 0;
}