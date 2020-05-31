//
// Created by Mario on 2020/5/30.
//

#pragma once
#include <boost/thread.hpp>
#include "../Utils/Singleton.h"
#include "../Utils/GetConcurrency.h"

struct Config : public Singleton<Config> {
public:
    int db_count = 16 * GetConcurrency();
};


