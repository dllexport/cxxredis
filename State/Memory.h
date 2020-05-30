//
// Created by Mario on 2020/5/27.
//

#pragma once

#include <cstddef>
#include "../Utils/RSS.h"
#include "../Utils/Singleton.h"

// memory unit kb
struct Memory : public Singleton<Memory> {
    Memory() {

    }
    float mem_fragmentation_ratio() {
        return getCurrentRSS() / (1024 * used_memory);
    }

    size_t used_memory = 1;
};


