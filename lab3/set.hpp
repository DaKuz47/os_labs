#pragma once

#include <pthread.h>
#include <iostream>
#include <vector>

class Set{
    public:
        Set();
        ~Set() = default;

        bool add(int item);
        bool remove(int item);
        bool contains(int item);

        void print_items();
        std::vector<int> get_items();
    private:
        class Node{
            public:
                Node(int item){
                    this->item = item;

                    pthread_mutex_init(&mutex, nullptr);
                    next = nullptr;
                    marked = false;
                }
                ~Node(){ pthread_mutex_destroy(&mutex); }

                int item;
                bool marked;
                Node* next;

                pthread_mutex_t mutex;
        };

        class Lock{
            public:
                Lock(pthread_mutex_t* mutex){
                    this->mutex = mutex;
                    pthread_mutex_lock(mutex);
                    locked = true;
                }
                ~Lock(){
                    if(locked){
                        unlock();
                    }                     
                }

                void unlock(){
                    pthread_mutex_unlock(mutex);
                    locked = false;
                }
            private:
                pthread_mutex_t* mutex;
                bool locked;
        };

        Node* head;

        bool validate(Node* prev, Node* current);
};
