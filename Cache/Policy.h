//
// Created by Mario on 2020/5/27.
//

#pragma once
#include <cstdint>

enum class CachePolicy : uint8_t {
    NO_EVICTION,
    ALL_KEY_LRU,
    VOLATILE_LRU
};
