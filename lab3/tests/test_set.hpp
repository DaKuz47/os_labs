#include <pthread.h>
#include <vector>
#include <sstream>

#include "../set.hpp"


const int THREAD_COUNT = 10;


struct SetThreadData{
    Set* set;
    int* data;
    int data_size;
};


void* writer(void* raw_writer_data){
    SetThreadData* writer_data = (SetThreadData*)raw_writer_data;

    Set* set = writer_data->set;
    for(int i = 0; i < writer_data->data_size; i++){
        set->add(writer_data->data[i]);
    }

    return nullptr;
}


bool set_producers_test(int items_count, bool randomize_data = false){
    Set* set = new Set();
    Set* etalone_set = new Set();
    pthread_t threads[THREAD_COUNT];
    for(int i = 0; i < THREAD_COUNT; i++){
        int* data = new int[items_count];
        for(int j = 0; j < items_count; j++){
            if(randomize_data){
                // random number from 0 to 100
                data[j] = rand() % 101;
                etalone_set->add(data[j]);
            } else {
                data[j] = THREAD_COUNT * j + i;
                etalone_set->add(data[j]);
            }
        }

        auto writer_data = new SetThreadData();
        writer_data->set = set;
        writer_data->data = data;
        writer_data->data_size = items_count;

        pthread_create(&threads[i], nullptr, writer, writer_data);
    }

    for(int i = 0; i < THREAD_COUNT; i++){
        pthread_join(threads[i], nullptr);
    }

    auto result = set->get_items();
    std::vector<int> etalone_result;
    for(auto item : etalone_set->get_items()){
        etalone_result.push_back(item);
    }

    bool same_length = result.size() == etalone_result.size();
    for(int i = 0; i < THREAD_COUNT * items_count; i++){
        if(result[i] != etalone_result[i]){
            return false;
        }
    }

    return same_length;
}


void* reader(void* raw_reader_data){
    SetThreadData* reader_data = (SetThreadData*)raw_reader_data;
    Set* set = reader_data->set;

    int* data = reader_data->data;
    for(int i = 0; i < reader_data->data_size; i++){
        set->remove(data[i]);
    }

    return nullptr;
}


bool set_consumers_test(int items_count, bool randomize_data = false){
    Set* set = new Set();
    pthread_t threads[THREAD_COUNT];

    for(int i = 0; i < THREAD_COUNT; i++){
        int* data = new int[items_count];

        for(int j = 0; j < items_count; j++){
            if(randomize_data){
                // random number from 0 to 100
                data[j] = rand() % 101;
                set->add(data[j]);
            } else {
                data[j] = THREAD_COUNT * j + i;
                set->add(data[j]);
            }
        }

        auto reader_data = new SetThreadData();
        reader_data->set = set;
        reader_data->data = data;
        reader_data->data_size = items_count;

        pthread_create(&threads[i], nullptr, reader, reader_data);
    }

    for(int i = 0; i < THREAD_COUNT; i++){
        pthread_join(threads[i], nullptr);
    }

    auto result = set->get_items();
    std::vector<int> etalone_result;

    bool same_length = result.size() == etalone_result.size();

    return same_length;
}


struct ExtendedSetThreadData{
    int* read_states;
    int* data;
    int data_size;
    Set* set;
};


void* extended_reader(void* raw_extended_thread_data){
    ExtendedSetThreadData* thread_data = (ExtendedSetThreadData*)raw_extended_thread_data;

    for(int i = 0; i < thread_data->data_size; i++){
        int to_remove = thread_data->data[i];
        auto start_time = std::chrono::steady_clock::now();
        do{
            auto end_time = std::chrono::steady_clock::now();
            auto diff = end_time - start_time;
            bool res = false;
            if(thread_data->set->contains(to_remove)){
                res = thread_data->set->remove(to_remove);
            }

            if(res){
                break;
            }
            if(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() >= 300){
                sched_yield();
                start_time = std::chrono::steady_clock::now();
            }
        } while(true);

        thread_data->read_states[to_remove]++;
    }

    return nullptr;
}


bool set_general_test(int item_count){
    Set* set = new Set();
    pthread_t writers[THREAD_COUNT];
    pthread_t readers[THREAD_COUNT];
    int read_states[THREAD_COUNT * item_count];
    for(int i = 0; i < THREAD_COUNT * item_count; i++){
        read_states[i] = 0;
    }

    for(int i = 0; i < THREAD_COUNT; i++){
        int data[item_count];

        for(int j = 0; j < item_count; j++){
            data[j] = i * item_count + j;
        }


        SetThreadData* writer_data = new SetThreadData();
        writer_data->data = data;
        writer_data->set = set;
        writer_data->data_size = item_count;

        ExtendedSetThreadData* reader_data = new ExtendedSetThreadData();
        reader_data->data = data;
        reader_data->data_size = item_count;
        reader_data->set = set;
        reader_data->read_states = read_states;

        pthread_create(&writers[i], nullptr, writer, writer_data);
        pthread_create(&readers[i], nullptr, extended_reader, reader_data);
    }

    for(int i = 0; i < THREAD_COUNT; i++){
        pthread_join(writers[i], nullptr);
        pthread_join(readers[i], nullptr);
    }

    for(auto item : read_states){
        if(item != 1){
            return false;
        }
    }

    return true;
}


void run_test_check_time(
    const char* title,
    int repeat_count,
    int items_count,
    bool randomize,
    bool (*test_func)(int, bool)
){
    int64_t duration_sum{0};

    for(int i = 0; i < repeat_count; i++){
        auto start = std::chrono::steady_clock::now();
        test_func(items_count, randomize);
        auto end = std::chrono::steady_clock::now();
        auto diff = end - start;
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();

        duration_sum += duration;
    }

    std::cout << "\t" << title << std::endl;
    std::cout << "\tRESULT: " << duration_sum / repeat_count << " microseconds." << std::endl;
}


void set_speed_test(int items_count, int repeat_count){
    run_test_check_time("RANDOMIZE DATA PRODUCER TEST", repeat_count, items_count, true, set_producers_test);
    std::cout << std::endl;
    run_test_check_time("RANDOMIZE DATA CONSUMER TEST", repeat_count, items_count, true, set_consumers_test);
    std::cout << std::endl;
    run_test_check_time("FIXED DATA PRODUCER TEST", repeat_count, items_count, false, set_producers_test);
    std::cout << std::endl;
    run_test_check_time("FIXED DATA CONSUMER TEST", repeat_count, items_count, false, set_consumers_test);
}
