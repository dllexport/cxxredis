//
// Created by Mario on 2020/5/26.
//

#pragma once
#include <sstream>
#include <iostream>
#include <thread>
#include "../Database.h"
#include <boost/archive/binary_oarchive.hpp>
#include <fstream>

class Dump {
public:

    static void BGSAVE() {
        std::thread([](){
            pid_t fork_res = fork();
            if (fork_res < 0) {
                std::cout << "Fork error\n";
                return;
            }
            if (fork_res == 0) {
                doDump();
                return;
            }
            if (fork_res > 0) {
                wait(&fork_res);
            }
        }).detach();
    }

    static void SAVE() {
        doDump();
    }

private:
    static void doDump() {
        std::ofstream ofs("dump.rdb");
        boost::archive::binary_oarchive oa(ofs);
        oa << Database::GetInstance()->dbs;
        ofs.close();
        std::cout << "dumped\n";
    }
};
