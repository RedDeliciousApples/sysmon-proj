#include <chrono>
#include <thread>
#include <iostream>
int main() {
    using namespace std::this_thread; // sleep_for, sleep_until
    using namespace std::chrono; // nanoseconds, system_clock, seconds
    for (int i =0; i < 999; i ++){
        printf("test %d", i);
        //sleep_for(nanoseconds(10));
        sleep_until(system_clock::now() + seconds(1));
    }

}