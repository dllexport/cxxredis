//
// Created by root on 2020/5/31.
//

#pragma once

#include <functional>
#include "../Net/Session.h"

using CommandDefaultHandler = std::function<void(const boost::intrusive_ptr<Session>&, int)>;