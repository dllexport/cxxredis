//
// Created by Mario on 2020/5/16.
//

#include "String.h"
#include <boost/lexical_cast.hpp>
#include "../Database.h"
#include "../Protocol/bproto.h"

const std::string String::empty = std::string("nil");

std::pair<int, const std::string&> String::GET(uint8_t db_index, const std::string &key)
{
    auto db = Database::GetInstance();
    auto object = db->Get(db_index, std::forward<const std::string>(key));
    if(!object)
        return {universal_command::NOT_EXIST_ERR, String::empty};
    if (object->encoding != ENCODING_TYPE::STRING)
        return {universal_command::NOT_MATCH_ERR, String::empty};
    return {universal_command::OK, boost::any_cast<const std::string &>(object->any)};
}

std::pair<int, std::string> String::GETRANGE(uint8_t db_index, const std::string &key, std::string&& start_str, std::string&& end_str)
{
    auto db = Database::GetInstance();
    auto object = db->Get(db_index, std::forward<const std::string>(key));
    if (!object)
        return {universal_command::NOT_EXIST_ERR, String::empty};
    if (object->encoding != ENCODING_TYPE::STRING)
        return {universal_command::NOT_MATCH_ERR, String::empty};

    int32_t start;
    int32_t end;
    try {
        start = boost::lexical_cast<int32_t>(start_str);
        end = boost::lexical_cast<int32_t>(end_str);
    } catch (...) {
        return {universal_command::PARAM_ERR, String::empty};
    }
    auto &str = boost::any_cast<const std::string &>(object->any);
    auto size = str.size();
    if (start < 0)
        start += size;
    if (end < 0)
        end += size;
    if (start < 0 && end < 0 && end < start)
        return {universal_command::PARAM_ERR, String::empty};
    return {universal_command::OK, str.substr(start % size, (end - start) % size + 1)};
}

// return <err, string>
std::pair<int, std::string> String::GETSET(uint8_t db_index, const std::string &key, std::string &&value)
{
    auto db = Database::GetInstance();
    auto object = db->Get(db_index, std::forward<const std::string>(key));
    std::string result;
    // if key not exist, insert it into hashmap
    if (!object)
    {
        db->Set(db_index,
                std::forward<const std::string &&>(key),
                new Object(ENCODING_TYPE::STRING, std::forward<std::string &&>(value)));
        return {universal_command::OK, String::empty};
    }
    else if (object->encoding != ENCODING_TYPE::STRING) // if key is not String type
    {
        return {universal_command::NOT_MATCH_ERR, String::empty};
    }
    result = boost::any_cast<std::string>(object->any);
    // clear expiration
    object->ClearExpire();
    object->any = std::forward<std::string &&>(value);
    return {universal_command::OK, result};
}

std::pair<int, int> String::STRLEN(uint8_t db_index, const std::string &key)
{
    auto db = Database::GetInstance();
    auto object = db->Get(db_index, std::forward<const std::string>(key));
    if (!object)
        return {(uint32_t)universal_command::NOT_EXIST_ERR, 0};
    if (object->encoding == ENCODING_TYPE::STRING)
        return {(uint32_t)universal_command::OK, boost::any_cast<const std::string &>(object->any).size()};
    return {universal_command::NOT_MATCH_ERR, 0};
}

std::pair<int, int> String::SETEX(uint8_t db_index, const std::string &key, std::string &&value, std::string &&expire_time)
{
    return SetExOP(db_index, std::forward<const std::string &&>(key), std::forward<std::string &&>(value), std::forward<std::string &&>(expire_time), true);
}

std::pair<int, int> String::PSETEX(uint8_t db_index, const std::string &key, std::string &&value, std::string &&expire_time)
{
    return SetExOP(db_index, std::forward<const std::string &&>(key), std::forward<std::string &&>(value), std::forward<std::string &&>(expire_time), false);
}

std::pair<int, int> String::SET(uint8_t db_index, const std::string &key, std::string &&value)
{
    auto db = Database::GetInstance();
    return {universal_command::OK, db->Set(db_index,
                                 std::forward<const std::string &&>(key),
                                 new Object(ENCODING_TYPE::STRING, std::forward<std::string &&>(value)))};
}

std::pair<int, int> String::SETNX(uint8_t db_index, const std::string &key, std::string &&value)
{
    auto db = Database::GetInstance();
    auto object = db->Get(db_index, std::forward<const std::string>(key));
    if (object)
        return {universal_command::EXIST_ERR, 0};
    db->Set(db_index,
            std::forward<const std::string &&>(key),
            new Object(ENCODING_TYPE::STRING, std::forward<std::string &&>(value)));
    return {(uint32_t)universal_command::OK, 1};
}

/*
 * apply to existing string obj only
 * string obj should be a number
 * return <res, const std::string&>
 */
std::pair<int, const std::string &> String::INCR(uint8_t db_index, const std::string &key)
{
    return IncrDecrbyOP(db_index, std::forward<const std::string &&>(key), "1", true);
}
std::pair<int, const std::string &> String::INCRBY(uint8_t db_index, const std::string &key, std::string &&by)
{
    return IncrDecrbyOP(db_index, std::forward<const std::string &&>(key), std::forward<std::string &&>(by), true);
}
std::pair<int, const std::string &> String::DECR(uint8_t db_index, const std::string &key)
{
    return IncrDecrbyOP(db_index, std::forward<const std::string &&>(key), "1", false);
}
std::pair<int, const std::string &> String::DECRBY(uint8_t db_index, const std::string &key, std::string &&by)
{
    return IncrDecrbyOP(db_index, std::forward<const std::string &&>(key), std::forward<std::string &&>(by), false);
}

/*
 * append to existing string obj only
 * return total len
 * return 0 if failed
 */
std::pair<int, int> String::APPEND(uint8_t db_index, const std::string &key, std::string &&append_value)
{
    auto db = Database::GetInstance();
    auto object = db->Get(db_index, std::forward<const std::string>(key));
    if (!object)
        return {universal_command::NOT_EXIST_ERR, 0};
    if (object->encoding != ENCODING_TYPE::STRING)
    {
        return {universal_command::NOT_MATCH_ERR, 0};
    }
    auto &str = boost::any_cast<std::string &>(object->any);
    str.append(std::forward<std::string &&>(append_value));
    return {universal_command::OK, str.size()};
}


std::pair<int, const std::string &> String::IncrDecrbyOP(uint8_t db_index, const std::string &key, std::string &&by, bool incr)
{
    auto db = Database::GetInstance();
    auto object = db->Get(db_index, std::forward<const std::string>(key));
    if (!object)
        return {universal_command::NOT_EXIST_ERR, String::empty};
    if (object->encoding != ENCODING_TYPE::STRING)
        return {universal_command::NOT_MATCH_ERR, String::empty};

    auto &str = boost::any_cast<std::string &>(object->any);
    try
    {
        auto result = boost::lexical_cast<int64_t>(str) + (incr ? boost::lexical_cast<int64_t>(by) : -boost::lexical_cast<double>(by));
        str = boost::lexical_cast<std::string>(result);
        return {universal_command::OK, str};
    }
    catch (...)
    {
    }
    try
    {
        auto result = boost::lexical_cast<double>(str) + (incr ? boost::lexical_cast<double>(by) : -boost::lexical_cast<double>(by));
        str = boost::lexical_cast<std::string>(result);
        return {universal_command::OK, str};
    }
    catch (...)
    {
    }

    return {universal_command::NOT_EXIST_ERR, String::empty};
}


// sec_or_msec
// true for sec false for msec
std::pair<int, int> String::SetExOP(uint8_t db_index, const std::string &key, std::string &&value, std::string && expire_time, bool sec_or_msec)
{
    auto db = Database::GetInstance();
    uint32_t end_time = 0;
    try {
        end_time = boost::lexical_cast<uint32_t>(expire_time);
    }catch(...) {
        return {universal_command::PARAM_ERR, 0};
    }

    std::chrono::milliseconds time_to_plus = sec_or_msec ? std::chrono::seconds(end_time) : std::chrono::milliseconds(end_time);

    db->Set(db_index,
            std::forward<const std::string &&>(key),
            new Object(ENCODING_TYPE::STRING,
                       std::forward<std::string &&>(value),
                       std::chrono::system_clock::now() + time_to_plus));
    return {universal_command::OK, 1};
}