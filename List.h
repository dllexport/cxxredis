////
//// Created by Mario on 2020/5/18.
////
//
//#pragma once
//#include <string>
//#include "Database.h"
//#include "ListStruct.h"
//
//#define FWS(x) std::forward<const std::string>(x)
//
//class List {
//public:
//    /*
//     * return 0 if key not exist
//     * return > 0 if key exist
//     * there's no 0 length list cause it will be removed from hash_map
//     */
//    static uint64_t Length(uint8_t db_index, const std::string&& key) {
//        auto& list_find = List::find(db_index, std::forward<const std::string>(key));
//        if (list_find != Object::NONE_OBJECT) {
//            auto list_head = boost::any_cast<ListHead>(list_find.any);
//            return list_head.count;
//        }
//        return 0;
//    }
//
//
//    static const ListData* LIndex(uint8_t db_index, const std::string&& key, uint64_t lindex) {
//        auto& list_find = List::find(db_index, std::forward<const std::string>(key));
//        if (list_find == Object::NONE_OBJECT) return nullptr;
//
//        auto list_head = boost::any_cast<ListHead>(list_find.any);
//        if (lindex >= list_head.count)return nullptr;
//
//        auto p = list_head.first;
//        while(lindex--)
//            p = p->next;
//        return p;
//    }
//
//        // push element at front
//    static uint8_t LPush(uint8_t db_index, const std::string&& key, std::string&& value) {
//        auto& list_find = List::find(db_index, std::forward<const std::string>(key));
//        auto list_data = new ListData(std::move(value));
//
//        if (list_find != Object::NONE_OBJECT) {
//            auto list_head = boost::any_cast<ListHead>(list_find.any);
//            list_data->next = list_head.first;
//            list_head.first->prev = list_data;
//            list_head.first = list_data;
//            ++list_head.count;
//            list_find.any = list_head;
//            return 1;
//        }
//        // if list not exist we just create a new one
//        return Database::GetInstance()->Set(db_index, std::forward<const std::string>(key),
//                Object(ENCODING_TYPE::LIST, ListHead(list_data)));
//    }
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
//
//private:
//
//    static Object& find(uint8_t db_index, const std::string&& key) {
//        auto db = Database::GetInstance();
//        return db->Get(db_index, std::forward<const std::string>(key));
//    }
//
//};
//
