#include <iostream>
#include "mcsp.hpp"


MCSP::MCSP(){
    pthread_mutex_init(&queue_mutex, nullptr);
    pthread_cond_init(&ready_to_read, nullptr);
}


MCSP::~MCSP(){
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&ready_to_read);
}


bool MCSP::empty(){
    return queue.empty();
}


bool MCSP::enqueue(int item){
    pthread_mutex_lock(&queue_mutex);
    queue.push(item);
    pthread_cond_signal(&ready_to_read);
    pthread_mutex_unlock(&queue_mutex);

    return true;
}


bool MCSP::dequeue(int& item){
    pthread_mutex_lock(&queue_mutex);

    while(empty()){ 
        pthread_cond_wait(&ready_to_read, &queue_mutex);
    }

    item = queue.front();
    queue.pop();
    pthread_mutex_unlock(&queue_mutex);

    return true;
}


std::vector<int> MCSP::get_items(){
    std::vector<int> items;

    while(!queue.empty()){
        items.push_back(queue.front());
        queue.pop();
    }

    return items;
}
