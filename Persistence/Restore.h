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
        if (is_empty(ifs)){
            return;
        }
        boost::archive::binary_iarchive ia(ifs);
        auto& dbs = Database::GetInstance()->dbs;
        ia >> dbs;
        ifs.close();
        std::cout << "restored\n";
    }

    static bool is_empty(std::ifstream& pFile)
    {
        return pFile.peek() == std::ifstream::traits_type::eof();
    }
};
