#include "terminal.h"
#include "version.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <net/newtork_initializer.h>
namespace brct
{
void Terminal::run(const std::atomic_bool &terminate)
{
    std::cout << "Terminal NcClient " NC_CLIENT_VERSION << " started." << "\nhelp for help\nCtrl+D/Ctrl+C for exit" << std::endl;
    std::string line;
    while (!terminate)
    {
        if (!std::getline(std::cin, line)) break;
        if (processor_) processor_(line);
    }
    std::cout << "Terminal stoped." << std::endl;
}
void Terminal::print(const std::string& msg)
{
    std::cout << msg << std::endl;
}
}