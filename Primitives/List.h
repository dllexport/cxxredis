//
// Created by Mario on 2020/5/18.
//

#pragma once
#include <string>
#include "../Database.h"
#include "ListStruct.h"
#include "ListOps.h"

#define FWS(x) std::forward<const std::string>(x)
namespace cxxredis {
    namespace list {
        class List {
        public:

            static std::pair<uint32_t, uint32_t> LENGTH(uint8_t db_index, const std::string&& key) {
                auto db = Database::GetInstance();
                auto object = db->Get(db_index, std::forward<const std::string>(key));
                if (!object) {
                    return {(uint32_t) Command::NOTEXIST_ERR, 0};
                }
                if (object->encoding != ENCODING_TYPE::LIST) {
                    return {(uint32_t) Command::NOTMATCH_ERR, 0};
                }
                auto& head = boost::any_cast<ListHead&>(object->any);
                return {(uint32_t) Command::OK, head.count};
            }
//
//
            static std::pair<uint32_t, const std::string &>  LINDEX(uint8_t db_index, const std::string&& key, std::string&& lindex) {
                auto db = Database::GetInstance();
                auto object = db->Get(db_index, std::forward<const std::string>(key));
                if (!object) {
                    return {(uint32_t) Command::NOTEXIST_ERR, String::empty};
                }
                if (object->encoding != ENCODING_TYPE::LIST) {
                    return {(uint32_t) Command::NOTMATCH_ERR, String::empty};
                }
                auto& head = boost::any_cast<ListHead&>(object->any);
                uint32_t index = 0;
                try
                {
                    index = boost::lexical_cast<uint32_t>(lindex);
                }
                catch (...)
                {
                    return {Command::PARAM_ERR, String::empty};
                }
                auto pair_res = ListOps::ListIndexAt(&head, index);

                if (pair_res)
                    return {Command::OK, pair_res->payload};
                return {Command::NOTEXIST_ERR, pair_res->payload};
            }

//          // push element at front
            /*
             * push 1 or multiple value to the head of the list
             * if key not exist, create new one
             * if key exist but type mismatch, return error
             * return list length after push
             */
            static std::pair<uint32_t, uint32_t>
            LPUSH(uint8_t db_index, const std::string &&key, std::vector<std::string> &&values) {
                auto db = Database::GetInstance();
                auto object = db->Get(db_index, std::forward<const std::string>(key));
                if (object && object->encoding != ENCODING_TYPE::LIST) {
                    return {(uint32_t) Command::NOTMATCH_ERR, 0};
                }
                // if list not exist we just create a new one
                if (!object) {
                    auto new_object = new Object(ENCODING_TYPE::LIST, ListHead());
                    db->Set(db_index, std::forward<const std::string>(key), new_object);
                    object = new_object;
                }

                auto& head = boost::any_cast<ListHead&>(object->any);

                for(auto &value : values) {
                    ListOps::ListInsertHead(&head, new ListNode(std::move(value)));
                }
                return {(uint32_t) Command::OK, head.count};
            }
//
//    static std::unique_ptr<ListData> RPop(uint8_t db_index, const std::string&& key) {
//        auto& list_find = List::find(db_index, std::forward<const std::string>(key));
//        if (list_find == Object::NONE_OBJECT) {
//            return nullptr;
//        }
//
//        auto list_head = boost::any_cast<ListHead>(list_find.any);
//        auto last = list_head.last;
//        // if there's only one element in list
//        if (last->prev == nullptr) {
//            Database::GetInstance()->Del(db_index, std::forward<const std::string>(key));
//        }else {
//            last->prev->next = nullptr;
//            list_head.last = last->prev;
//            last->prev = nullptr;
//            --list_head.count;
//            list_find.any = list_head;
//        }
//        return std::unique_ptr<ListData>(last);
//    }

        };

    }
}