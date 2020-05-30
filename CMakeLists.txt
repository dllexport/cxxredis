cmake_minimum_required(VERSION 3.14)
project(cxxredis)

set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS regex system coroutine context thread serialization REQUIRED)
enable_testing()

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(cxxredis
            main.cpp
            Primitives/String.cpp
            Primitives/String.h
            Primitives/Object.h
            Database.h
            Net/Server.cpp
            Net/Server.h
            Utils/Singleton.h
            Primitives/List.cpp
            Primitives/List.h
            Primitives/ListStruct.h
            Primitives/Object.cpp
            Utils/SmartPtr.h
            Utils/linenoise.h
            Utils/linenoise.c
            Net/HandleAlloc.h
            Primitives/ListOps.h
            Persistence/Dump.cpp
            Persistence/Dump.h
            Persistence/Restore.cpp
            Persistence/Restore.h
            Persistence/Serialization/SerialObject.h
            Net/Session.cpp
            Net/Session.h Persistence/Serialization/SerialAny.h
            Primitives/Encoding.h
            Cache/Policy.h
            Cache/VolatileMap.cpp
            Cache/VolatileMap.h
            State/Memory.cpp State/Memory.h
            Utils/RSS.h Utils/RSS.cpp
            Utils/GetConcurrency.h Utils/GetConcurrency.cpp
            Protocol/bproto.h Protocol/BProto.pb.h Protocol/BProto.pb.cc Protocol/universal.pb.h Protocol/universal.pb.cc State/Config.cpp State/Config.h Net/IOTransfer.cpp Net/IOTransfer.h Net/IOExecutor.h)
    add_executable(cxxredis-cli Cli/main.cpp
            Cli/Client.h Cli/Client.cpp
            Protocol/bproto.h
            Utils/linenoise.h Utils/linenoise.c
            Protocol/BProto.pb.h Protocol/BProto.pb.cc Protocol/universal.pb.h Protocol/universal.pb.cc)
    target_link_libraries (cxxredis-cli /usr/local/Cellar/protobuf/3.12.1/lib/libprotobuf.a)
    target_link_libraries (cxxredis-cli /usr/local/Cellar/protobuf/3.12.1/lib/libprotoc.a)
    target_link_libraries (cxxredis-cli ${Boost_LIBRARIES})
    target_link_libraries (cxxredis ${Boost_LIBRARIES})
    target_link_libraries (cxxredis /usr/local/Cellar/protobuf/3.12.1/lib/libprotobuf.a)
    target_link_libraries (cxxredis /usr/local/Cellar/protobuf/3.12.1/lib/libprotoc.a)

#    add_executable(test_dump Tests/Persistence/test_dump.cpp)
#    target_link_libraries (test_dump ${Boost_LIBRARIES})
#    add_test (Persistence test_dump)
endif()


