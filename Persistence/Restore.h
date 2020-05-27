//
// Created by Mario on 2020/5/26.
//

#pragma once
#include <fstream>
#include <iostream>
#include <boost/archive/binary_iarchive.hpp>
#include "../Database.h"

class Restore {
public:

    static void RESTORE() {
        doRestore();
    }

private:
    static void doRestore() {
        std::ifstream ifs("dump.rdb");
        boost::archive::binary_iarchive ia(ifs);
        ia >> Database::GetInstance()->dbs;
        ifs.close();
        std::cout << "restored\n";
    }
};
