//
// Created by Mario on 2020/5/16.
//

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Primitives/Object.h"
#include "Utils/Singleton.h"
#include "State/Config.h"
#include <boost/intrusive_ptr.hpp>
#include "Persistence/Serialization/SerialObject.h"
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

class Database : public Singleton<Database> {
public:

    uint8_t Set(uint8_t db_index, const std::string&& key, Object* object) {
        auto& db = this->dbs[db_index];
        auto it = db.find(key);
        if (it != db.end()) {
            db.erase(it);
            db.insert({key, object});
            return 0;
        }
        db.insert({key, object});
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
        auto& db = this->dbs[db_index];
        auto it = std::begin(db);
        while(it != std::end(db)) {
            // clear expired key
            if (it->second->Expired())
                it = db.erase(it);
            else {
                ans.push_back(it->first);
                ++it;
            }
        }
        return ans;
    }

    uint32_t DBCount() {
        return dbs.size();
    }
private:
    friend class Singleton<Database>;
    friend class Dump;
    friend class Restore;
    Database() : dbs(std::vector<std::unordered_map<std::string, boost::intrusive_ptr<Object>>>(Config::GetInstance()->db_count)) {}
    Database(const Database& other) {}

    std::vector<std::unordered_map<std::string, boost::intrusive_ptr<Object>>> dbs;

    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive & ar, Database& o, const unsigned int version)
    {
        ar & dbs;
    }
};
