//
// Created by Mario on 2020/5/18.
//

#pragma once
#include <string>

class List;

class ListData {
public:
    ListData(std::string&& data) : payload(std::move(data)), prev(nullptr), next(nullptr) {}
    std::string payload;
private:
    ~ListData(){}
    friend class std::default_delete<ListData>;
    friend class List;
    ListData* prev;
    ListData* next;
};

struct ListHead {
    ListHead(ListData* first = nullptr) : first(first), last(first) {
        count = first ? 1 : 0;
    }

    void Clear() {
        this->first = nullptr;
        this->last = nullptr;
    }

    uint64_t count;
    ListData* first;
    ListData* last;
};