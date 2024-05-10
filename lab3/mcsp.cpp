#include <iostream>
#include "mcsp.hpp"


MCSP::MCSP(){
    head = tail = new Node(0);

    pthread_mutex_init(&head_mutex, nullptr);
    pthread_mutex_init(&tail_mutex, nullptr);
    pthread_cond_init(&ready_to_read, nullptr);
}


MCSP::~MCSP(){
    pthread_mutex_destroy(&head_mutex);
    pthread_mutex_destroy(&tail_mutex);
    pthread_cond_destroy(&ready_to_read);
}


bool MCSP::empty(){
    pthread_mutex_lock(&tail_mutex);
    bool is_empty = head->next == nullptr;
    pthread_mutex_unlock(&tail_mutex);
    return is_empty;
}


bool MCSP::enqueue(int item){
    Node* new_node = new Node(item);

    pthread_mutex_lock(&tail_mutex);
    tail->next = new_node;
    tail = tail->next;
    pthread_cond_signal(&ready_to_read);
    pthread_mutex_unlock(&tail_mutex);

    return true;
}


bool MCSP::dequeue(int& item){
    pthread_mutex_lock(&head_mutex);

    while(empty()){ 
        pthread_cond_wait(&ready_to_read, &head_mutex);
    }

    item = head->next->item;
    Node* to_remove = head->next;
    head->next = head->next->next;
    delete to_remove;
    pthread_mutex_unlock(&head_mutex);

    return true;
}


void MCSP::print_values(){
    Node* current_node = head->next;

    while (current_node){
        std::cout << current_node->item << std::endl;
        current_node = current_node->next;
    }
}


std::vector<int> MCSP::get_items(){
    std::vector<int> items;

    Node* current = head->next;
    while(current != nullptr){
        items.push_back(current->item);
        current = current->next;
    }

    return items;
}
