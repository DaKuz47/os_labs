#include <algorithm>
#include <chrono>
#include <iostream>
#include <pthread.h>
#include <vector>

#include "../mcsp.hpp"


const int MSCP_THREAD_COUNT = 10;


struct MCSPThreadData{
    MCSP* queue;
    int* data;
    int data_size;
};


void* mcsp_writer(void* raw_writer_data){
    MCSPThreadData* writer_data = (MCSPThreadData*)raw_writer_data;

    MCSP* queue = writer_data->queue;
    for(int i = 0; i < writer_data->data_size; i++){
        queue->enqueue(writer_data->data[i]);
    }

    return nullptr;
}


bool mcsp_producer_test(int items_count){
    MCSP* queue = new MCSP();
    pthread_t thread;
    int* data = new int[items_count];
    for(int j = 0; j < items_count; j++){
        data[j] = j;
    }

    auto writer_data = new MCSPThreadData();
    writer_data->queue = queue;
    writer_data->data = data;
    writer_data->data_size = items_count;

    pthread_create(&thread, nullptr, mcsp_writer, writer_data);
    pthread_join(thread, nullptr);

    auto result = queue->get_items();
    std::vector<int> etalone_result;
    for(int i = 0; i < items_count; i++){
        etalone_result.push_back(i);
    }
    std::sort(std::begin(etalone_result), std::end(etalone_result));
    std::sort(std::begin(result), std::end(result));

    bool same_length = result.size() == etalone_result.size();
    for(int i = 0; i < items_count; i++){
        if(result[i] != etalone_result[i]){
            return false;
        }
    }

    return same_length;
}


void* mcsp_reader(void* raw_reader_data){
    MCSPThreadData* reader_data = (MCSPThreadData*)raw_reader_data;
    MCSP* queue = reader_data->queue;

    int dummy_value;

    for(int i = 0; i < reader_data->data_size; i++){
        queue->dequeue(dummy_value);
    }

    return nullptr;
}


bool mcsp_consumers_test(int items_count){
    MCSP* queue = new MCSP();
    pthread_t threads[MSCP_THREAD_COUNT];

    for(int i = 0; i < MSCP_THREAD_COUNT * items_count; i++){
        queue->enqueue(i);
    }

    for(int i = 0; i < MSCP_THREAD_COUNT; i++){
        auto reader_data = new MCSPThreadData();
        reader_data->queue = queue;
        reader_data->data = nullptr;
        reader_data->data_size = items_count;

        pthread_create(&threads[i], nullptr, mcsp_reader, reader_data);
    }

    for(int i = 0; i < MSCP_THREAD_COUNT; i++){
        pthread_join(threads[i], nullptr);
    }

    return queue->empty();
}


struct ExtendedMCSPThreadData{
    int* read_states;
    int* data;
    int data_size;
    MCSP* queue;
};


void* mcsp_extended_reader(void* raw_extended_thread_data){
    ExtendedMCSPThreadData* thread_data = (ExtendedMCSPThreadData*)raw_extended_thread_data;

    for(int i = 0; i < thread_data->data_size; i++){
        int readed_item;
        auto start_time = std::chrono::steady_clock::now();
        do{
            auto end_time = std::chrono::steady_clock::now();
            auto diff = end_time - start_time;
            bool res = thread_data->queue->dequeue(readed_item);

            if(res){
                break;
            }
            if(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() >= 300){
                sched_yield();
                start_time = std::chrono::steady_clock::now();
            }
        } while(true);

        thread_data->read_states[readed_item]++;
    }

    return nullptr;
}


bool mcsp_general_test(int item_count){
    MCSP* queue = new MCSP();
    pthread_t writer_thread;
    pthread_t readers[MSCP_THREAD_COUNT];
    int read_states[MSCP_THREAD_COUNT * item_count];
    int data[MSCP_THREAD_COUNT * item_count];

    for(int i = 0; i < MSCP_THREAD_COUNT * item_count; i++){
        read_states[i] = 0;
        data[i] = i;
    }

    MCSPThreadData* writer_data = new MCSPThreadData();
    writer_data->data = data;
    writer_data->data_size = MSCP_THREAD_COUNT * item_count;
    writer_data->queue = queue;

    for(int i = 0; i < MSCP_THREAD_COUNT; i++){
        ExtendedMCSPThreadData* reader_data = new ExtendedMCSPThreadData();
        reader_data->data = nullptr;
        reader_data->data_size = item_count;
        reader_data->queue = queue;
        reader_data->read_states = read_states;

        pthread_create(&readers[i], nullptr, mcsp_extended_reader, reader_data);
    }
    pthread_create(&writer_thread, nullptr, mcsp_writer, writer_data);

    for(int i = 0; i < MSCP_THREAD_COUNT; i++){
        pthread_join(readers[i], nullptr);
    }
    pthread_join(writer_thread, nullptr);

    for(auto item : read_states){
        if(item != 1){
            return false;
        }
    }

    return true;
}


void run_mcsp_test_check_time(
    const char* title,
    int repeat_count,
    int items_count,
    bool (*test_func)(int)
){
    int64_t duration_sum{0};

    for(int i = 0; i < repeat_count; i++){
        auto start = std::chrono::steady_clock::now();
        test_func(items_count);
        auto end = std::chrono::steady_clock::now();
        auto diff = end - start;
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();

        duration_sum += duration;
    }

    std::cout << "\t" << title << std::endl;
    std::cout << "\tRESULT: " << duration_sum / repeat_count << " microseconds." << std::endl;
}


void mcsp_speed_test(int items_count, int repeat_count){
    run_mcsp_test_check_time("PRODUCER TEST", repeat_count, items_count, mcsp_producer_test);
    std::cout << std::endl;
    run_mcsp_test_check_time("CONSUMER TEST", repeat_count, items_count, mcsp_consumers_test);
}
