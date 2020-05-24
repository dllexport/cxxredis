//
// Created by Mario Lau on 2020/5/24.
//

#pragma once
#include "ListStruct.h"
namespace cxxredis {
    namespace list {
        class ListOps {
        public:
            static void ListInsertHead(ListHead* head, ListNode* node) {
                if (head->first == nullptr) {
                    head->first = node;
                    head->last = node;
                    node->next = nullptr;
                    node->prev = nullptr;
                }else {
                    auto old_first_node = head->first;
                    head->first = node;
                    node->next = old_first_node;
                    node->prev = nullptr;
                    old_first_node->prev = node;
                }
                ++head->count;
            }

            void ListInsertTail(ListHead* head, ListNode* node) {
                if (head->first == nullptr) {
                    head->first = node;
                    head->last = node;
                    node->next = nullptr;
                    node->prev = nullptr;
                }else {
                    head->last->next = node;
                    node->prev = head->last;
                    node->next = nullptr;
                    std::swap(head->last->payload, node->payload);
                    head->last = head->last->next;
                }
            }

        };
    }
}

