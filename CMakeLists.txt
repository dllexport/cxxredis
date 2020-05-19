cmake_minimum_required(VERSION 3.16)
project(cxxredis)

set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS regex system coroutine context thread REQUIRED)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(cxxredis main.cpp String.cpp String.h Encoding.h Database.h Net/Server.cpp Net/Server.h Utils/Singleton.h List.cpp List.h ListStruct.h Encoding.cpp Errors.h)
    target_link_libraries (cxxredis ${Boost_LIBRARIES})
endif()