#include <limits>
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>

#include "set.hpp"


Set::Set(){
    head = new Node(std::numeric_limits<int>::min());
    head->next = new Node(std::numeric_limits<int>::max());
}


bool Set::validate(Node* prev, Node* current){
    return !prev->marked && !current->marked && prev->next == current;
}


bool Set::remove(int item){
    while(true){
        Node* prev = head;
        Node* current = head->next;

        while(current->item < item){
            prev = current;
            current = current->next;
        }
        pthread_mutex_lock(&prev->mutex);
        pthread_mutex_lock(&current->mutex);

        if(validate(prev, current)){
            if(current->item == item){
                current->marked = true;
                prev->next = current->next;

                pthread_mutex_unlock(&current->mutex);
                pthread_mutex_unlock(&prev->mutex);

                // delete current;
                return true;
            } else {
                pthread_mutex_unlock(&current->mutex);
                pthread_mutex_unlock(&prev->mutex);
                return false;
            }
        }

        pthread_mutex_unlock(&current->mutex);
        pthread_mutex_unlock(&prev->mutex);
    }
}


bool Set::add(int item){
    while(true){
        Node* prev = head;
        Node* current = head->next;

        while(current->item < item){
            prev = current;
            current = current->next;
        }
        pthread_mutex_lock(&prev->mutex);
        pthread_mutex_lock(&current->mutex);

        if(validate(prev, current)){
            if(current->item == item){
                pthread_mutex_unlock(&current->mutex);
                pthread_mutex_unlock(&prev->mutex);

                return false;
            } else {
                Node* new_node = new Node(item);
                prev->next = new_node;
                new_node->next = current;

                pthread_mutex_unlock(&current->mutex);
                pthread_mutex_unlock(&prev->mutex);
                return true;
            }
        }

        pthread_mutex_unlock(&current->mutex);
        pthread_mutex_unlock(&prev->mutex);
    }
}


bool Set::contains(int item){
    Node* current = head;
    while(current->item < item){
        current = current->next;
    }

    return current->item == item && !current->marked;
}


void Set::print_items(){
    Node* current = head->next;

    while(current->next != nullptr){
        std::cout << current->item << std::endl;
        current = current->next;
    }
}


std::vector<int> Set::get_items(){
    std::vector<int> items;
    
    Node* current = head->next;
    while(current->next != nullptr){
        items.push_back(current->item);
        current = current->next;
    }

    return items;
}
