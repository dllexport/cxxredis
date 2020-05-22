//
// Created by Mario on 2020/5/20.
//
#include "Client.h"
#include <thread>
int main()
{
    std::regex reg("(set|SET)\\s+");
    auto str = std::string("set    ");
    auto res = std::regex_match(str, reg);
    Client client;
    client.Run();
}