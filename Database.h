//
// Created by Mario on 2020/5/16.
//

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Object.h"
#include "Utils/Singleton.h"
#include <boost/intrusive_ptr.hpp>

class Database : public Singleton<Database> {
public:

    uint8_t Set(uint8_t db_index, const std::string&& key, Object* object) {
        auto& db = this->dbs[db_index];
        auto it = db.find(key);
        if (it == db.end()) {
            db.emplace(key, object);
            return 0;
        }
        it->second.reset(object);
        return 0;
    }

    boost::intrusive_ptr<Object> Get(uint8_t db_index, const std::string&& key) {
        auto& db = this->dbs[db_index];
        auto it = db.find(key);
        if (it != db.end()) {
            // if key expired, delete it and return nullptr
            if (it->second->Expired()) {
                db.erase(it);
                return nullptr;
            }
            return it->second;
        }
        return nullptr;
    }

    void Del(uint8_t db_index, const std::string&& key) {
        auto& db = this->dbs[db_index];
        db.erase(key);
    }

    std::vector<std::string> KEYS(uint8_t db_index) {
        std::vector<std::string> ans;
        for (auto &it : this->dbs[db_index]) {
            ans.push_back(it.first);
        }
        return ans;
    }

private:
    friend class Singleton<Database>;
    Database() : dbs(std::vector<std::unordered_map<std::string, boost::intrusive_ptr<Object>>>(16)) {}
    Database(const Database& other) {}

    uint8_t db_count = 16;
    std::vector<std::unordered_map<std::string, boost::intrusive_ptr<Object>>> dbs;
};
