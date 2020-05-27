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
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/lexical_cast.hpp>
#include "../Utils/SmartPtr.h"
using Clock = std::chrono::time_point<std::chrono::system_clock>;
static const Clock DefaultClock;

enum class ENCODING_TYPE : uint8_t {
    NONE = 0,
    STRING = 1,
    LIST = 1 << 2,
};

struct Object : public SmartPtr<Object> {

    Object() : encoding(ENCODING_TYPE::NONE), any(nullptr), expire_time() {}

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

    Object Clone();

    Object& operator=(Object&& rhs) noexcept;

    bool operator==(Object const& rhs) const {
        return this == &rhs;
    }

    bool operator!=(Object const& rhs) const {
        return this != &rhs;
    }

    bool Expired();

    void SetExpire(Clock&& time);

    void ClearExpire();

    ENCODING_TYPE encoding;
    boost::any any;
    static Object NONE_OBJECT;

private:
    Clock expire_time;

    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<SmartPtr<Object>>(*this);
        ar & encoding;
        ar & boost::serialization::make_binary_object(&expire_time, sizeof(Clock));
        switch (encoding) {
            case ENCODING_TYPE::STRING: {
                std::string test = boost::any_cast<std::string>(any);
                ar & boost::serialization::make_nvp("string", test);
            }
        }
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<SmartPtr<Object>>(*this);
        ar & encoding;
        ar & boost::serialization::make_binary_object(&expire_time, sizeof(Clock));
        switch (encoding) {
            case ENCODING_TYPE::STRING: {
                std::string str_load;
                ar & boost::serialization::make_nvp("string", str_load);
                any = str_load;
            }
        }
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
};


