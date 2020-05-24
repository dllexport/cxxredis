//
// Created by Mario on 2020/5/18.
//

#pragma once
#include <string>

namespace cxxredis {
    namespace list {
        class ListNode {
        public:
            ListNode(std::string&& data) : payload(std::move(data)), prev(nullptr), next(nullptr) {}
            std::string payload;
        private:
            ~ListNode(){}
            friend class std::default_delete<ListNode>;
            friend class ListOps;
            ListNode* prev;
            ListNode* next;
        };

        struct ListHead {
            ListHead(ListNode* first = nullptr) : first(first), last(first) {
                count = first ? 1 : 0;
            }

            void Clear() {
                this->first = nullptr;
                this->last = nullptr;
            }

            uint64_t count;
            ListNode* first;
            ListNode* last;
        };
    }
}
