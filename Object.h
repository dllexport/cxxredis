//
// Created by Mario on 2020/5/16.
//
#pragma once
#include <cstdint>
#include <boost/any.hpp>
#include <type_traits>
#include <chrono>
#include <iostream>
#include <time.h>

#include "SmartPtr.h"
using Clock = std::chrono::time_point<std::chrono::system_clock>;
static const Clock DefaultClock;

enum class ENCODING_TYPE : uint8_t {
    NONE = 0,
    STRING = 1,
    INTEGER = 1 << 1,
    FLOAT = 1 << 2,
    LIST = 1 << 3,
};

struct Object : public SmartPtr<Object> {
    template<class Any>
    Object(ENCODING_TYPE encoding_type, Any any) : encoding(encoding_type), any(any) {}

    template<class Any>
    Object(ENCODING_TYPE encoding_type, Any any, Clock expire_time) : encoding(encoding_type), any(any), expire_time(expire_time) {}

    Object(ENCODING_TYPE encoding_type, const char* any) = delete;

    Object(const Object& other) {
        this->encoding = other.encoding;
        this->any = other.any;
#ifdef OBJECT_DEBUG
        std::cout << "Object Copy\n";
        fflush(stdout);
#endif
    }

    Object(Object&& other) {
        this->encoding = other.encoding;
        this->any = std::move(other.any);
        other.encoding = ENCODING_TYPE::NONE;
#ifdef OBJECT_DEBUG
        std::cout << "Object Moved\n";
        fflush(stdout);
#endif
    }

    ~Object() {
#ifdef OBJECT_DEBUG
        std::cout << "Object Die\n";
        fflush(stdout);
#endif
    }

    Object Clone() {
        return *this;
    }

    Object& operator=(Object&& rhs) noexcept {
        if (this == &rhs)
            return *this;
        this->any.clear();
        ::new(this)Object(std::move(rhs));
        return *this;
    }

    bool operator == (Object const& rhs) const {
        return this == &rhs;
    }

    bool operator != (Object const& rhs) const {
        return this != &rhs;
    }

    bool Expired() {
        if (expire_time == DefaultClock) return false;
        auto now = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(expire_time - now).count();
        return diff < 0;
    }

    void SetExpire(Clock&& time) {
        this->expire_time = time;
    }

    void ClearExpire() {
        this->expire_time = DefaultClock;
    }

    ENCODING_TYPE encoding;
    boost::any any;
    static Object NONE_OBJECT;

private:
    Clock expire_time;
};


