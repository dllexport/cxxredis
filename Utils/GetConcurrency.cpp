//
// Created by Mario on 2020/5/30.
//

#include "GetConcurrency.h"

#ifdef __linux__
#include <boost/thread.hpp>
#endif

unsigned int GetConcurrency() noexcept {
#ifdef __linux__
    return boost::thread::physical_concurrency();
#else
    return 4;
#endif
}