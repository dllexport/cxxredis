//
// Created by Mario on 2020/5/27.
//

#pragma once
#include <cstdint>

enum class ENCODING_TYPE : uint8_t {
    NONE = 0,
    STRING = 1,
    LIST = 1 << 2,
};
