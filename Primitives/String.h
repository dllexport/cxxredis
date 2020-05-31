//
// Created by Mario on 2020/5/16.
//
#pragma once
#include <string>

class String
{
public:
    static std::pair<int, const std::string&> GET(uint8_t db_index, const std::string &key);

    static std::pair<int, std::string> GETRANGE(uint8_t db_index, const std::string &key, std::string&& start_str, std::string&& end_str);

    static std::pair<int, std::string> GETSET(uint8_t db_index, const std::string &key, std::string &&value);

    static std::pair<int, int> STRLEN(uint8_t db_index, const std::string &key);

    static std::pair<int, int> SETEX(uint8_t db_index, const std::string &key, std::string &&value, std::string &&expire_time);

    static std::pair<int, int> PSETEX(uint8_t db_index, const std::string &key, std::string &&value, std::string &&expire_time);

    static std::pair<int, int> SET(uint8_t db_index, const std::string &key, std::string &&value);

    static std::pair<int, int> SETNX(uint8_t db_index, const std::string &key, std::string &&value);

    /*
     * apply to existing string obj only
     * string obj should be a number
     * return <res, const std::string&>
     */
    static std::pair<int, const std::string &> INCR(uint8_t db_index, const std::string &key);
    static std::pair<int, const std::string &> INCRBY(uint8_t db_index, const std::string &key, std::string &&by);
    static std::pair<int, const std::string &> DECR(uint8_t db_index, const std::string &key);
    static std::pair<int, const std::string &> DECRBY(uint8_t db_index, const std::string &key, std::string &&by);

    /*
     * append to existing string obj only
     * return total len
     * return 0 if failed
     */
    static std::pair<int, int> APPEND(uint8_t db_index, const std::string &key, std::string &&append_value);

    static const std::string empty;
private:
    ~String() = delete;

    static std::pair<int, const std::string &> IncrDecrbyOP(uint8_t db_index, const std::string &key, std::string &&by, bool incr);

    // sec_or_msec
    // true for sec false for msec
    static std::pair<int, int> SetExOP(uint8_t db_index, const std::string &key, std::string &&value, std::string && expire_time, bool sec_or_msec);

};
