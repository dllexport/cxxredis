//
// Created by Mario on 2020/5/20.
//

#pragma once
#include <cstdint>
#include "BProto.pb.h"

struct BProtoHeader {
    uint32_t payload_len;
    Command  payload_cmd;
    constexpr static uint8_t Size() {
        return sizeof(BProtoHeader);
    }
};

#define BProtoHeaderSize BProtoHeader::Size()
#define BProtoHeaderOffset(p) p + BProtoHeaderSize
#define BuildBProtoHeader(p, bytes) ((BProtoHeader*)p)->payload_len = bytes