//
// Created by Mario on 2020/5/27.
//

#pragma once
#include <unordered_map>
#include <vector>
#include <boost/intrusive_ptr.hpp>
#include "../Primitives/Object.h"

class VolatileMap {

private:
    std::vector<std::unordered_map<std::string, boost::intrusive_ptr<Object>>> volatile_map;
};


