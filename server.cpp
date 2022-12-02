#include "CASH.hpp"
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    if(argc < 3){
        std::string exe = argv[0];
        exe = exe.substr(exe.find_last_of("/\\")+1);
        std::cout << "usage: " << exe << " [SERVER_PASS] [CLIENT_PASS]" << std::endl;
        return 1;
    }
    char *s_key = argv[1], *c_key = argv[2];
    server::CASH a = server::CASH("0.0.0.0", s_key, c_key);
    return 0;
}