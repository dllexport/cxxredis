//
// Created by Mario on 2020/5/16.
//
#pragma once
#include <cstdint>
#include <boost/any.hpp>
#include <type_traits>
#include <iostream>


enum class ENCODING_TYPE : uint8_t {
    NONE = 0,
    STRING = 1,
    INTEGER = 1 << 1,
    FLOAT = 1 << 2,
    LIST = 1 << 3,
};

struct Object {
    template<class Any>
    Object(ENCODING_TYPE encoding_type, Any any) : encoding(encoding_type), any(any) {}

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

    /*bool operator==(const Object& rhs) const {
        return this == &rhs;
    }*/
    bool operator == (Object const& rhs) const {
        return this == &rhs;
    }

    bool operator != (Object const& rhs) const {
        return this != &rhs;
    }

    ENCODING_TYPE encoding;
    boost::any any;

    static Object NONE_OBJECT;

};


