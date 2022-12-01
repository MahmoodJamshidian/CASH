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
    char *c_key = new char[16], *s_key = new char[16];
    std::string _s_key = argv[1], _c_key = argv[2];
    for(int i = 0; i < _s_key.size(); i++)
    {
        s_key[i] = _s_key[i];
    }
    for(int i = 0; i < _c_key.size(); i++)
    {
        c_key[i] = _c_key[i];
    }

    server::CASH a = server::CASH("0.0.0.0", s_key, c_key);
    return 0;
}