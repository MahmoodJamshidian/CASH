#include "CASH.hpp"
#include <iostream>
#include <string>
#include <thread>

void clear()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    system("cls");
#else
    system("clear");
#endif
}

int main()
{
    std::string host, c_key, s_key;
    std::cout << "Enter server address: ";
    std::cin >> host;
    std::cout << "Enter server password: ";
    std::cin >> s_key;
    std::cout << "Enter client password: ";
    std::cin >> c_key;
    clear();

    std::cout << "Connecting to CASH://" << host << "/ ... ";

    try
    {
        new client::CASH(host.c_str(), s_key.c_str(), c_key.c_str(), [](client::CASH *connection)
                         {
        utils::CommandResult res;
        std::string cmd;
        std::cout << "done" << std::endl;
        getline(std::cin, cmd);
        while (true)
        {
            std::cout << "$ ";
            cmd = "";
            getline(std::cin, cmd);
            if(cmd == ""){
                continue;
            }
            res = connection->run(cmd.c_str());
            std::cout << res.output << "- exit code: " << res.exitstatus << std::endl;
        } });
    }catch(const std::exception& e){
        std::cout << "faild" << std::endl << "\033[1;31m" << "Error: " << e.what() << "\033[0m" << std::endl;
    }

    return 0;
}