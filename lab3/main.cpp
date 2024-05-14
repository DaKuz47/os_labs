#include "tests/test_mcsp.hpp"
#include "tests/test_set.hpp"


int main(){
    const int item_count = 10;
    const int repeat_count = 10;

    std::cout << "CONSUMERS THREAD COUNT " << THREAD_COUNT << std::endl;

    std::cout << "==TEST 1. MCSP. PRODUCER. ITEMS COUNT " << item_count << std::endl;
    std::string res = mcsp_producer_test(item_count) ? "SUCCESS" : "FAILED";
    std::cout << res << std::endl;

    std::cout << "==TEST 2. SET WITH LAZY SYNC. PRODUCERS. ITEMS COUNT " << item_count << std::endl;
    res = set_producers_test(item_count) ? "SUCCESS" : "FAILED";
    std::cout << res << std::endl;

    std::cout << "==TEST 3. MCSP. CONSUMERS. ITEMS COUNT " << item_count << std::endl;
    res = mcsp_consumers_test(item_count) ? "SUCCESS" : "FAILED";
    std::cout << res << std::endl;

    std::cout << "==TEST 4. SET WITH LAZY SYNC. CONSUMERS. ITEMS COUNT " << item_count << std::endl;
    res = set_consumers_test(item_count) ? "SUCCESS" : "FAILED";
    std::cout << res << std::endl;

    std::cout << "==TEST 5. MCSP. GENERAL. ITEMS COUNT " << item_count << std::endl;
    res = mcsp_general_test(item_count) ? "SUCCESS" : "FAILED";
    std::cout << res << std::endl;

    std::cout << "==TEST 6. SET WITH LAZY SYNC. GENERAL. ITEMS COUNT " << item_count << std::endl;
    res = set_general_test(item_count) ? "SUCCESS" : "FAILED";
    std::cout << res << std::endl;

    std::cout << "==TEST 7 MCSP. SPEED TEST. ITEMS COUNT " << item_count << std::endl;
    mcsp_speed_test(item_count, repeat_count);

    std::cout << "==TEST 8. SET WITH LAZY SYNC. SPEED TEST. ITEMS COUNT " << item_count << std::endl;
    set_speed_test(item_count, repeat_count); 

    return 0;
}
