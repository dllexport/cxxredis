//
// Created by Mario on 2020/5/16.
//
#pragma once
#include <string>
#include <boost/lexical_cast.hpp>
#include "Database.h"
#include "Errors.h"

class String {
public:
    static const std::string& Get(uint8_t db_index, const std::string&& key) {
        auto db = Database::GetInstance();
        auto& object = db->Get(db_index, std::forward<const std::string>(key));
        if (object.encoding != ENCODING_TYPE::STRING) return String::empty;
        return boost::any_cast<const std::string&>(object.any);
    }

    static std::string_view GetRange(uint8_t db_index, const std::string&& key, int32_t start, int32_t end) {
        auto db = Database::GetInstance();
        auto& object = db->Get(db_index, std::forward<const std::string>(key));
        if (object.encoding != ENCODING_TYPE::STRING) return String::empty;
        auto& str = boost::any_cast<const std::string&>(object.any);
        auto size = str.size();
        if (start < 0) start += size;
        if (end < 0) end += size;
        if (start < 0 && end < 0 && end < start) return String::empty;
        return std::string_view(str).substr(start % size, (end - start) % size + 1);
    }

    // return <err, string>
    static std::pair<bool, std::string> GetSet(uint8_t db_index, const std::string&& key, std::string&& value) {
        auto db = Database::GetInstance();
        auto& object = db->Get(db_index, std::forward<const std::string>(key));
        std::string result;
        // if key not exist, insert it into hashmap
        if (object.encoding == ENCODING_TYPE::NONE)
        {
            db->Set(db_index,
                           std::forward<const std::string&&>(key),
                           Object(ENCODING_TYPE::STRING, std::forward<std::string&&>(value)));
            return {true, String::empty};
        }
        // if key is not String type
        if (object.encoding != ENCODING_TYPE::STRING) {
            return {false, String::empty};
        }
        result = boost::any_cast<std::string>(object.any);
        object.any = std::forward<std::string&&>(value);
        return {true, result};
    }

    static std::pair<bool, uint32_t> Strlen(uint8_t db_index, const std::string&& key) {
        auto db = Database::GetInstance();
        auto& object = db->Get(db_index, std::forward<const std::string>(key));
        if (object.encoding == ENCODING_TYPE::NONE) return {true, 0};
        if (object.encoding == ENCODING_TYPE::STRING) return {true, boost::any_cast<const std::string&>(object.any).size()};
        return {false, 0};
    }

    static uint8_t Set(uint8_t db_index, const std::string&& key, std::string&& value) {
        auto db = Database::GetInstance();
        return db->Set(db_index,
                std::forward<const std::string&&>(key),
                        Object(ENCODING_TYPE::STRING, std::forward<std::string&&>(value)));
    }

    /*
     * apply to existing string obj only
     * string obj should be a number
     * return <res, const std::string&>
     */
    static std::pair<bool, const std::string&> Incr(uint8_t db_index, const std::string&& key) {
        return IncrDecrbyOP(db_index, std::forward<const std::string&&>(key), "1", true);
    }
    static std::pair<bool, const std::string&> Incrby(uint8_t db_index, const std::string&& key, std::string&& by) {
        return IncrDecrbyOP(db_index, std::forward<const std::string&&>(key), std::forward<std::string&&>(by), true);
    }
    static std::pair<bool, const std::string&> Decr(uint8_t db_index, const std::string&& key) {
        return IncrDecrbyOP(db_index, std::forward<const std::string&&>(key), "1", false);
    }
    static std::pair<bool, const std::string&> Decrby(uint8_t db_index, const std::string&& key, std::string&& by) {
        return IncrDecrbyOP(db_index, std::forward<const std::string&&>(key), std::forward<std::string&&>(by), false);
    }

    /*
     * append to existing string obj only
     * return total len
     * return 0 if failed
     */
    static uint32_t Append(uint8_t db_index, const std::string&& key, std::string&& append_value) {
        auto db = Database::GetInstance();
        auto& object = db->Get(db_index, std::forward<const std::string>(key));
        if (object.encoding != ENCODING_TYPE::STRING) return 0;
        auto& str = boost::any_cast<std::string&>(object.any);
        str.append(std::forward<std::string&&>(append_value));
        return str.size();
    }

private:
    ~String() = delete;
    static const std::string empty;
    static std::pair<bool, const std::string&> IncrDecrbyOP(uint8_t db_index, const std::string&& key, std::string&& by, bool incr) {
        auto db = Database::GetInstance();
        auto& object = db->Get(db_index, std::forward<const std::string>(key));
        if (object.encoding != ENCODING_TYPE::STRING) return {false, String::empty};
        auto& str = boost::any_cast<std::string&>(object.any);
        try {
            auto result = boost::lexical_cast<int64_t>(str) + (incr ? boost::lexical_cast<int64_t>(by) : -boost::lexical_cast<double>(by));
            str = boost::lexical_cast<std::string>(result);
            return {true, str};
        } catch (...) {}
        try {
            auto result = boost::lexical_cast<double>(str) + (incr ? boost::lexical_cast<double>(by) : -boost::lexical_cast<double>(by));
            str = boost::lexical_cast<std::string>(result);
            return {true, str};
        } catch (...) {}

        return {false, str};
    }

};

