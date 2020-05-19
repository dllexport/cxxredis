//
// Created by Mario on 2020/5/16.
//

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Encoding.h"
#include "Utils/Singleton.h"

class Database : public Singleton<Database> {
public:

    uint8_t Set(uint8_t db_index, const std::string&& key, Object&& object) {
        auto& db = this->dbs[db_index];
        auto it = db.find(key);
        if (it == db.end()) {
            db.emplace(key, std::move(object));
            return 0;
        }
        it->second = std::move(object);
        return 0;
    }

    Object& Get(uint8_t db_index, const std::string&& key) {
        auto& db = this->dbs[db_index];
        auto it = db.find(key);
        if (it != db.end()) {
            return it->second;
        }
        return Object::NONE_OBJECT;
    }

    void Del(uint8_t db_index, const std::string&& key) {
        auto& db = this->dbs[db_index];
        db.erase(key);
    }

private:
    friend class Singleton<Database>;
    Database() : dbs(std::vector<std::unordered_map<std::string, Object>>(16)) {}
    Database(const Database& other) {}

    uint8_t db_count = 16;
    std::vector<std::unordered_map<std::string, Object>> dbs;
};
