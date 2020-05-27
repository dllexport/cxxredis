//
// Created by Mario on 2020/5/18.
//

#include "Object.h"
#include <boost/any.hpp>

Object Object::NONE_OBJECT(ENCODING_TYPE::NONE, boost::any());

Object Object::Clone() {
    return *this;
}

Object& Object::operator=(Object&& rhs) noexcept {
    if (this == &rhs)
        return *this;
    this->any.clear();
    ::new(this)Object(std::move(rhs));
    return *this;
}



bool Object::Expired() {
    if (expire_time == DefaultClock) return false;
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(expire_time - now).count();
    return diff < 0;
}

void Object::SetExpire(Clock&& time) {
    this->expire_time = time;
}

void Object::ClearExpire() {
    this->expire_time = DefaultClock;
}