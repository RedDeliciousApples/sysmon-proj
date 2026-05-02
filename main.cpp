#include <chrono>
#include <thread>
#include <iostream>
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds
using namespace std;
int main() {
    FILE *procstat = fopen("/proc/stat", "r");
    char cpu[3];
    fscanf(procstat, "%s", cpu);
    printf("cpu is: %s", cpu);
    // for (int i =0; i < 20; i ++)
    // {
    //     cout << " test " << i << endl;
    //     //sleep_for(nanoseconds(10));
    //     sleep_for(seconds(1));
    // }
    return 0;
}