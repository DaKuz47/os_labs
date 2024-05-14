#pragma once

#include <queue>
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
        std::queue<int> queue;

        pthread_mutex_t queue_mutex;
        pthread_cond_t ready_to_read;
};
