#pragma once

#include <pthread.h>
#include <vector>


class MCSP{
    public:
        MCSP();
        ~MCSP();

        bool enqueue(int item);
        bool dequeue(int& item);
        bool empty();

        void print_values();
        std::vector<int> get_items();
    private:
        class Node{
            public:
                Node(int item){
                    this->item = item;
                    next = nullptr;
                }
                ~Node() = default;

                int item;
                Node* next;
        };

        Node* head;
        Node* tail;

        pthread_mutex_t head_mutex;
        pthread_mutex_t tail_mutex;
        pthread_cond_t ready_to_read;
};
