//
// Created by Mario on 2020/5/17.
//

#pragma once
#include <utility>

template<class T>
class Singleton {
public:
    template<class ... Args>
    static auto GetInstance(Args&& ... args) {
        static T instance(std::forward<Args>(args)...);
        return &instance;
    }
};
